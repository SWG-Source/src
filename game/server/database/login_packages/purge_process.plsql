create or replace package body purge_process
as
	-- Station ids are stored as the signed 32-bit integer with the same bits
	-- (see DB::BindableUint32). The account extract may carry them unsigned,
	-- so its user_id is converted to that form wherever it is used.
	function station_id_of (source_table varchar2) return varchar2 as
	begin
		return '(case when ' || source_table || '.user_id >= 2147483648 then ' ||
			source_table || '.user_id - 4294967296 else ' || source_table || '.user_id end)';
	end;

	procedure update_account_list (source_table varchar2) as
		invalid_ids number;
	begin
		--Reject ids that are not 32-bit station ids in either form
		execute immediate
			'select count(*) from ' || source_table || ' ' ||
			'where user_id is null or user_id <> trunc(user_id) ' ||
			'or user_id < -2147483648 or user_id > 4294967295'
			into invalid_ids;
		if invalid_ids > 0 then
			raise_application_error(-20001, 'purge_process: ' || invalid_ids || ' rows of ' || source_table || ' have a user_id that is not a 32-bit station id');
		end if;

		--Add new status codes
		execute immediate
			'insert into statuses (id, description, do_purge, reviewed) ' ||
			'select distinct status, status_desc, ''N'', ''N'' ' ||
			'from ' || source_table || ' ' ||
			'where not exists (select * from statuses where statuses.id = ' || source_table || '.status)';

		--Add any new accounts.
		execute immediate
			'insert into purge_accounts (station_id,purge_phase) ' ||
			'select ' || station_id_of(source_table) || ', 0 ' ||
			'from ' || source_table || ' ' ||
			'where not exists (select * from purge_accounts where purge_accounts.station_id = ' || station_id_of(source_table) || ')';

		--Reset purge phase on existing accounts who may have reactivated:
		execute immediate
			'update purge_accounts ' ||
			'set purge_phase = 0, ' ||
			'purge_start_date = null ' ||
			'where purge_phase<>0 ' ||
			'and exists (select * from ' || source_table || ' , statuses ' ||
				'where ' || station_id_of(source_table) || ' = purge_accounts.station_id ' ||
				'and ' || source_table || '.status = statuses.id ' ||
				'and statuses.do_purge=''N'')';

		--Start purge process on accounts who have deactivated:
		execute immediate
			'update purge_accounts ' ||
			'set purge_phase = 1, ' ||
			'purge_start_date=(select nvl(max(close_date),sysdate) from ' || source_table ||
			' where ' || station_id_of(source_table) || ' = purge_accounts.station_id) ' ||
			'where purge_phase=0 ' ||
			'and exists (select * from ' || source_table || ', statuses ' ||
				'where ' || station_id_of(source_table) || ' = purge_accounts.station_id ' ||
				'and ' || source_table || '.status = statuses.id ' ||
				'and statuses.do_purge=''Y'')';
		--TODO:  immediate purge for banned accounts
	end;
	
	function get_account_for_purge(p_purge_phase number, p_min_age number) return number
	-- Grabs one account from the list to be purged, and sets a lock on it so that no other process
	-- will attempt to purge it.  (Ignores locks from more than 1 day ago, because those probably mean
	-- something in the process failed.)
	as
		result number;
	begin
		select station_id
		into result
		from purge_accounts
		where purge_accounts.purge_phase = p_purge_phase
		and sysdate - nvl(purge_accounts.purge_phase_date, purge_accounts.purge_start_date) > p_min_age
		and (purge_lock is null or sysdate - purge_lock > 1)
		and rownum < 2
		order by purge_accounts.purge_start_date
		for update;

		update purge_accounts
		set purge_lock=sysdate
		where station_id = result;

		return result;
	end;

	procedure set_purge_status(p_station_id number, p_new_phase number)
	as
	begin
		if (p_new_phase = 0) then
		update purge_accounts
		set purge_phase = p_new_phase,
			purge_start_date = null,
			purge_phase_date = null,
		purge_lock = null
		where station_id = p_station_id;
		else
			update purge_accounts
			set purge_phase = p_new_phase,
			purge_phase_date = sysdate,
			purge_lock = null
			where station_id = p_station_id;
		end if;

	end;
end;
/
