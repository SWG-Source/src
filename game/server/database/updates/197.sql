declare
  cnt number;
begin
  select count(*) into cnt from user_tab_columns 
  where table_name = 'PLAYER_OBJECTS' and column_name = 'ROLE_ICON_CHOICE';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add role_icon_choice int';
  end if;
end;
/
update version_number set version_number=197, min_version_number=197;
