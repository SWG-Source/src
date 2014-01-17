declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'SKILL_TEMPLATE';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add skill_template varchar2(200)';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'WORKING_SKILL';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add working_skill varchar2(200)';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'ACCOUNT_INFO';
  if (cnt = 0) then
    execute immediate 'create table account_info(station_id number not null, completed_tutorial char(1), constraint account_info_pk primary key (station_id))';
    execute immediate 'grant select on account_info to public';
  end if;
end;
/

declare
  cnt number;
begin
	for i in 
		(select distinct station_id from swg_characters)
	loop
		select count(*) into cnt
		from account_info
		where station_id = i.station_id;
		if (cnt = 0)
		then 
			insert into account_info (station_id, completed_tutorial)
			values (i.station_id, 'Y');
		end if;
	end loop;
end;
/

update version_number set version_number=220, min_version_number=220;