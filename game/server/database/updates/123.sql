alter table resource_container_objects add source number(20);
update resource_container_objects set source = 0;

update version_number set version_number=123, min_version_number=123;
