alter table creature_objects add master_id number(20);

update version_number set version_number=66, min_version_number=66;
