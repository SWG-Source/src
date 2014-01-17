create index owner_id_idx on tangible_objects(owner_id) tablespace indexes;
create index region_name_idx on battlefield_marker_objects (region_name) tablespace indexes;

update version_number set version_number=118, min_version_number=118;
