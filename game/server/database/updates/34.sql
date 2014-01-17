alter table resource_class_objects add crate_template varchar2(255);
alter table resource_class_objects add friendly_name varchar2(255);
update resource_class_objects set friendly_name = resource_class_name;

update version_number set version_number = 34, min_version_number=34;
