declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'SHOW_BACKPACK';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add show_backpack char(1)';
    execute immediate 'alter table player_objects add show_helmet char(1)';
  end if;
end;
/

update version_number set version_number=254, min_version_number=254;