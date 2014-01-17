alter table objects add deleted_date date;

create index deleted_object_idx on objects (deleted_date) tablespace indexes;

update version_number set version_number=112, min_version_number=112;
