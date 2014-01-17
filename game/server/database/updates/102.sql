alter table installation_objects add activate_start_time float;
alter table installation_objects add power float;
alter table installation_objects add power_rate float;

update version_number set version_number=102, min_version_number=102;
