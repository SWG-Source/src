declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'PLAYER_OBJECTS' and column_name = 'COLLECTIONS2';
  if (cnt = 0) then
    execute immediate 'alter table player_objects add collections2 varchar2(4000)';
  end if;
end;
/

update version_number set version_number=269, min_version_number=269;
