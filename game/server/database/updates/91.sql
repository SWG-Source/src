alter table creature_objects add standing_on number;

update version_number set version_number=91, min_version_number=91;
