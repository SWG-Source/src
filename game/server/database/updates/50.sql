alter table resource_pool_objects drop column last_harvested_timestamp;

update version_number set version_number=50, min_version_number=50;
