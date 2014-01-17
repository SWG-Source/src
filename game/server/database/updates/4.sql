alter table version_number add min_version_number int;
alter table creature_objects drop column gender;

update version_number set version_number = 4, min_version_number=4;
