declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'CLUSTER_LIST' and column_name = 'ONLINE_PLAYER_LIMIT';
  if (cnt = 0) then
    execute immediate 'alter table cluster_list add online_player_limit number';
	execute immediate 'update cluster_list set online_player_limit = 2500';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'CLUSTER_LIST' and column_name = 'ONLINE_FREE_TRIAL_LIMIT';
  if (cnt = 0) then
    execute immediate 'alter table cluster_list add online_free_trial_limit number';
	execute immediate 'update cluster_list set online_free_trial_limit = 250';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'CLUSTER_LIST' and column_name = 'FREE_TRIAL_CAN_CREATE_CHAR';
  if (cnt = 0) then
    execute immediate 'alter table cluster_list add free_trial_can_create_char char(1)';
	execute immediate 'update cluster_list set free_trial_can_create_char = ''Y''';
  end if;
end;
/



update version_number set version_number=223, min_version_number=223;
