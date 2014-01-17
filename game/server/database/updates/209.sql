declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'VERSION_NUMBER' and column_name = 'BRANCH_NAME';
  if (cnt = 0) then
    execute immediate 'alter table version_number add branch_name varchar2(30)';
  end if;
end;
/

update version_number set version_number=209, min_version_number=209;
