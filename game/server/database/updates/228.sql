declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'CLUSTER_LIST' and column_name = 'ONLINE_TUTORIAL_LIMIT';
  if (cnt = 0) then
    execute immediate 'alter table cluster_list add online_tutorial_limit number default 350';
  end if;
end;
/
update version_number set version_number=228, min_version_number=228;
