declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'ACCOUNT_INFO';
  if (cnt = 0) then
    execute immediate 'create table account_info(station_id number not null, completed_tutorial char(1), constraint account_info_pk primary key (station_id))';
    execute immediate 'grant select on account_info to public';
  end if;
end;
/

declare
  cnt number;
begin
        for i in
                (select distinct station_id from swg_characters)
        loop
                select count(*) into cnt
                from account_info
                where station_id = i.station_id;
                if (cnt = 0)
                then
                        insert into account_info (station_id, completed_tutorial)
                        values (i.station_id, 'Y');
                end if;
        end loop;
end;
/

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
