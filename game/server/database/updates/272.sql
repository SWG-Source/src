-- 272: store unsigned 32-bit values in the form 32-bit servers always used.
--
-- 32-bit servers bind unsigned 32-bit values (station ids, CRCs) through a
-- 32-bit long, so values of 2^31 or more are stored as their negative two's
-- complement, and the packages compare against that form. 64-bit builds
-- before this fix stored some of them positive instead (2^31 .. 2^32-1),
-- which made the same id appear in two forms (for example ACCOUNT_INFO
-- positive, PLAYERS negative) and broke logins. From this version on every
-- build writes the negative form; this update converts rows written by the
-- earlier 64-bit builds.
--
-- On a database only ever used by 32-bit servers there is nothing in that
-- range and this update changes nothing. It is idempotent.
--
-- The update is one transaction together with the version change: on any
-- error nothing is changed, including version_number and
-- min_version_number. If converting a column would give two rows the same
-- unique key (the same id recorded in both forms), the update stops and
-- names the table; resolve those rows, then run the update again.
-- Columns are only touched in tables that exist in the schema being
-- upgraded (the login and cluster tables may live in different schemas).
--
-- The columns are those declared DB::BindableUint32 in the server code.
-- objects.type_id is one of them but holds only four-character Tags, which
-- are below 2^31, so it needs no conversion.

whenever sqlerror exit failure rollback

-- The login schema records which rule its station ids were derived with
-- (login_schema/station_id_scheme.tab). An existing database is left
-- unlabelled: the LoginServer will not start until the operator labels it
-- with the stationIdScheme tool (see the 272 notes). Created before the
-- data transaction below because DDL commits implicitly; skipped if present.
declare
	n number;
begin
	select count(*) into n from user_tables where table_name = 'ACCOUNT_INFO';
	if n > 0 then
		select count(*) into n from user_tables where table_name = 'STATION_ID_SCHEME';
		if n = 0 then
			execute immediate
				'create table station_id_scheme (' ||
				'id number(1) default 1 not null, ' ||
				'scheme varchar2(16) not null, ' ||
				'basis varchar2(16) not null, ' ||
				'recorded_at date default sysdate not null, ' ||
				'constraint station_id_scheme_pk primary key (id), ' ||
				'constraint station_id_scheme_one_row check (id = 1), ' ||
				'constraint station_id_scheme_scheme check (scheme in (''gcc32'', ''gcc64'')), ' ||
				'constraint station_id_scheme_basis check (basis in (''new'', ''evidence'', ''asserted'')))';
			execute immediate 'grant select on station_id_scheme to public';
		end if;
	end if;
end;
/

declare
	has_version_table number;

	procedure normalise(p_table in varchar2, p_column in varchar2)
	is
		n number;
	begin
		select count(*) into n
		from user_tab_columns
		where table_name = upper(p_table) and column_name = upper(p_column);

		if n = 0 then
			return;
		end if;

		execute immediate
			'update ' || p_table || ' set ' || p_column || ' = ' || p_column || ' - 4294967296' ||
			' where ' || p_column || ' between 2147483648 and 4294967295';
	exception
		when dup_val_on_index then
			raise_application_error(-20272,
				'update 272: ' || p_table || '.' || p_column ||
				' holds the same id in both forms under a unique key; resolve those rows, then run the update again');
	end;
begin
	-- station ids: login tables
	normalise('account_info', 'station_id');
	normalise('account_reward_events', 'station_id');
	normalise('account_reward_items', 'station_id');
	normalise('extra_character_slots', 'station_id');
	normalise('feature_id_transactions', 'station_id');
	normalise('purge_accounts', 'station_id');
	normalise('swg_characters', 'station_id');

	-- station ids: cluster tables
	normalise('players', 'station_id');
	normalise('player_objects', 'station_id');
	normalise('accounts', 'station_id');
	normalise('temp_characters', 'station_id');
	normalise('account_map', 'parent_id');
	normalise('account_map', 'child_id');

	-- station ids: station players collector
	normalise('character_profile', 'station_id');

	-- unsigned 32-bit CRC columns written by the object persister
	normalise('tangible_objects', 'pvp_faction');
	normalise('tangible_objects', 'source_draft_schematic');
	normalise('mission_objects', 'target_appearance');
	normalise('mission_objects', 'mission_type');
	normalise('mission_objects', 'start_scene');
	normalise('mission_objects', 'end_scene');
	normalise('ship_objects', 'chassis_type');
	normalise('player_objects', 'current_quest');
	normalise('waypoints', 'appearance_name_crc');
	normalise('waypoints', 'location_scene');

	-- A schema holding only login tables may have no version_number table.
	select count(*) into has_version_table from user_tables where table_name = 'VERSION_NUMBER';
	if has_version_table > 0 then
		execute immediate 'update version_number set version_number=272, min_version_number=272';
	end if;
	commit;
exception
	when others then
		rollback;
		raise;
end;
/
