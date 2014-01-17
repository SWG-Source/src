alter table player_objects add num_lots int;
update player_objects
set num_lots = (select num_lots from accounts where player_objects.station_id = accounts.station_id);

update version_number set version_number=140, min_version_number=140;
