declare
	cnt number;
	stmt varchar2(4000);
begin

-- All of the following go into the LOGIN schema:
	select count(*) into cnt from user_tables 
	where table_name = 'ACCOUNT_EXTRACT';
	if (cnt = 0) then
		stmt := 'create table account_extract '
			|| '( user_id number,'
			|| 'subscription_id number,'
			|| 'plan_id number,'
			|| 'plan_desc varchar(255),'
			|| 'status number,'
			|| 'status_desc varchar2(60),'
			|| 'close_date date,'
			|| 'total_entitled_time number,'
			|| 'constraint account_extract_pk primary key (user_id)'
			|| ')';

		execute immediate stmt;
		execute immediate 'grant select on account_extract to public';
	end if;

	select count(*) into cnt from user_tables 
	where table_name = 'PURGE_ACCOUNTS';
	if (cnt = 0) then
		stmt := 'create table purge_accounts '
			|| '(station_id number,'
			|| 'purge_phase number,'
			|| 'purge_start_date date,'
			|| 'purge_lock date,'
			|| 'constraint purge_accounts_pk primary key (station_id)'
			|| ')';

		execute immediate stmt;
		execute immediate 'grant select on purge_accounts to public';
	end if;

	select count(*) into cnt from user_tables 
	where table_name = 'PURGE_PHASES';
	if (cnt = 0) then
		stmt := 'create table purge_phases '
			|| '(id number,'
			|| 'description varchar2(200),'
			|| 'constraint purge_phases_pk primary key (id)'
			|| ')';

		execute immediate stmt;
		execute immediate 'grant select on purge_phases to public';
	end if;

	select count(*) into cnt from user_tables 
	where table_name = 'STATUSES';
	if (cnt = 0) then
		stmt := 'create table statuses '
			|| '(id number,'
			|| 'description varchar2(200),'
			|| 'do_purge char(1),'
			|| 'reviewed char(1),'
			|| 'constraint statuses_pk primary key (id)'
			|| ')';

		execute immediate stmt;
		execute immediate 'grant select on statuses to public';
	end if;
end;
/
delete from purge_phases;

insert into purge_phases values (0, 'No Purge');
insert into purge_phases values (1, 'Not Warned');
insert into purge_phases values (2, 'Structure Purge Warning Sent');
insert into purge_phases values (3, 'Structure Purge Accomplished');
insert into purge_phases values (4, 'Character Purge Warning Sent');
insert into purge_phases values (5, 'Purged');

-- version # update:
update version_number set version_number=198, min_version_number=198;
