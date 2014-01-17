declare
  cnt number;
begin
  select count(*) into cnt from user_tab_columns 
  where table_name = 'ERRORLOG_VALUES' and column_name = 'ELEMENT_VALUE' and nullable = 'N';
  if (cnt > 0) then
    execute immediate 'alter table errorlog_values modify("ELEMENT_VALUE" NULL)';
  end if;
end;
/
update version_number set version_number=190, min_version_number=190;
