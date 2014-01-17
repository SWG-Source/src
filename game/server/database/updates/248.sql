declare
  cnt number;
begin
  select count(*) into cnt
  from user_indexes
  where table_name = 'PLAYERS' and index_name = 'PLAYERS_STATION_ID_IDX';
  if (cnt = 0) then
    execute immediate 'create index players_station_id_idx on players (station_id)';
  end if;	
end;
/

UPDATE version_number SET version_number = 248, min_version_number = 248;
