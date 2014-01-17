alter table creature_objects drop column standing_on;
alter table creature_objects add rank int;

alter table accounts add house_id number(20);
update accounts
  set house_id = (select min(house_id) from player_objects where player_objects.station_id = accounts.station_id);
alter table player_objects drop column house_id;

alter table objects drop column loaded_server;

alter table player_objects add born_date int;
alter table player_objects add played_time int;

update version_number set version_number=111, min_version_number=111;
