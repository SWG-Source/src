declare
  cnt number;
begin
  select count(*) into cnt from user_tab_columns 
  where table_name = 'CREATURE_OBJECTS' and column_name = 'PERSISTED_BUFFS';
  if (cnt = 0) then
    execute immediate 'alter table creature_objects add persisted_buffs varchar2(1000)';
  end if;
end;
/
update version_number set version_number=196, min_version_number=196;
