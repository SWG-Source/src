drop index chunk_idx;
create index chunk_idx on objects (node_x, node_z, scene_id, deleted, contained_by, player_controlled, load_contents) tablespace indexes;

update version_number set version_number=99, min_version_number=98;
