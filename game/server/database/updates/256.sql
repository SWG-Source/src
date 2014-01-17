declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'FEATURE_ID_TRANSACTIONS';
  if (cnt = 0) then
    execute immediate 'create table feature_id_transactions (station_id number, cluster_id number, character_id number, item_id varchar2(255), date_updated date, count number, constraint pk_feature_id_transactions PRIMARY KEY (station_id, cluster_id, character_id, item_id))'; 
    execute immediate 'grant select on feature_id_transactions to public';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'VERSION_NUMBER';
  if (cnt > 0) then
    execute immediate 'update version_number set version_number=256, min_version_number=256 where version_number=255';
  end if;
end;
/
