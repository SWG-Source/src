declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'VERSION_NUMBER' and column_name = 'GOLDDATA';
  if (cnt = 0) then
    execute immediate 'alter table version_number add golddata varchar2(100)';
  end if;	
end;
/

update version_number set version_number=205, min_version_number=205;
