declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'CREATURE_OBJECTS' and column_name = 'WS_X';
  if (cnt = 0) then
    execute immediate 'alter table creature_objects add ws_x float';
    execute immediate 'alter table creature_objects add ws_y float';
    execute immediate 'alter table creature_objects add ws_z float';
  end if;
end;
/

update version_number set version_number=224, min_version_number=224;
