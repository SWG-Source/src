alter table armor add object_template varchar2(255);

update version_number set version_number = 6, min_version_number=6;
