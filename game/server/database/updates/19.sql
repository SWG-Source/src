alter table creature_objects add states int;

update creature_objects set states=0;

update version_number set version_number = 19, min_version_number=19;
