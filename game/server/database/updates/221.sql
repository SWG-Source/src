declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'CURRENT_QUEST';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add current_quest int';
  end if;
end;
/

update version_number set version_number=221, min_version_number=221;
