-- Lists every station id persisted in this schema, one per line:
--   <station id> TAB <account name, if known> TAB <table>
-- Run it in each schema that holds login or cluster tables and give the
-- combined output to stationIdScheme (see README.md). Nothing is changed.

set serveroutput on size unlimited format wrapped
set feedback off
set heading off
set verify off
set echo off
whenever sqlerror exit failure

declare
	tab constant varchar2(1) := chr(9);

	function has_column(p_table varchar2, p_column varchar2) return boolean is
		n number;
	begin
		select count(*) into n from user_tab_columns
		where table_name = upper(p_table) and column_name = upper(p_column);
		return n > 0;
	end;

	procedure ids(p_table varchar2, p_column varchar2) is
		c sys_refcursor;
		v number;
	begin
		if not has_column(p_table, p_column) then
			return;
		end if;
		open c for 'select distinct ' || p_column || ' from ' || p_table || ' where ' || p_column || ' is not null';
		loop
			fetch c into v;
			exit when c%notfound;
			dbms_output.put_line(v || tab || tab || p_table || '.' || p_column);
		end loop;
		close c;
	end;

	-- Witnesses: the account name the game recorded for each character that
	-- has entered the world (objvar system.accountUsername).
	procedure witnesses is
		c sys_refcursor;
		v number;
		name varchar2(1000);
		n number;
	begin
		if not has_column('players', 'station_id') then
			return;
		end if;
		select count(*) into n from user_views where view_name = 'OBJECT_VARIABLES_VIEW';
		if n = 0 then
			return;
		end if;
		open c for
			'select distinct p.station_id, v.value from players p, object_variables_view v ' ||
			'where v.object_id = p.character_object and v.name = ''system.accountUsername'' ' ||
			'and p.station_id is not null and v.value is not null';
		loop
			fetch c into v, name;
			exit when c%notfound;
			dbms_output.put_line(v || tab || name || tab || 'players.station_id');
		end loop;
		close c;
	end;
begin
	-- login tables
	ids('account_info', 'station_id');
	ids('account_reward_events', 'station_id');
	ids('account_reward_items', 'station_id');
	ids('extra_character_slots', 'station_id');
	ids('feature_id_transactions', 'station_id');
	ids('purge_accounts', 'station_id');
	ids('swg_characters', 'station_id');
	-- cluster tables
	ids('players', 'station_id');
	ids('player_objects', 'station_id');
	ids('accounts', 'station_id');
	ids('temp_characters', 'station_id');
	ids('account_map', 'parent_id');
	ids('account_map', 'child_id');
	ids('character_profile', 'station_id');
	witnesses;
end;
/
exit
