drop index chunk_idx;
drop index container_idx;

create index chunk_idx on objects (node_x, node_z, scene_id, deleted) tablespace indexes;
create index container_idx on objects (contained_by,deleted) tablespace indexes;

update version_number set version_number=81, min_version_number=80;
