alter table resource_class_objects add name_table varchar2(255);

update version_number set version_number = 36, min_version_number=36;
