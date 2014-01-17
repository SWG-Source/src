alter table creature_objects add base_walk_speed float;
alter table creature_objects add base_run_speed float;

update version_number set version_number=141, min_version_number=141;
