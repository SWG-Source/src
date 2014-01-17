alter table objects add cache_version int;
update objects set cache_version = 0;

update version_number set version_number = 22, min_version_number=22;
