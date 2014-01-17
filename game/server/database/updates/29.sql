alter table creature_objects add posture int;
update creature_objects set posture = 0;

update version_number set version_number = 29, min_version_number=29;
