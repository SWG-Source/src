declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'QUESTS3';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add quests3 varchar2(4000)';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'QUESTS4';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add quests4 varchar2(4000)';
  end if;
end;
/

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

update version_number set version_number=219, min_version_number=219;
