declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'ACCOUNT_REWARD_ITEMS';
  if (cnt = 0) then
    execute immediate 'create table account_reward_items ( station_id number, item_id varchar2(255), date_claimed date, cluster_id number, character_id number, constraint pk_account_reward_items PRIMARY KEY (station_id, item_id))';
    execute immediate 'grant select on account_reward_items to public';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'ACCOUNT_REWARD_EVENTS';
  if (cnt = 0) then
    execute immediate 'create table account_reward_events (station_id number, event_id varchar2(255), date_consumed date, cluster_id number, character_id number, constraint pk_account_reward_events PRIMARY KEY (station_id, event_id))';
    execute immediate 'grant select on account_reward_events to public';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tab_columns
  where table_name = 'CLUSTER_LIST' and column_name = 'GROUP_ID';
  if (cnt = 0) then
    execute immediate 'alter table cluster_list add group_id int default 1 not null';
  end if;
end;
/

update version_number set version_number=208, min_version_number=208;
