create table preload_list -- NO_IMPORT
(
	scene_id varchar2(50),
	object_id number,
	node_x int,
	node_z int,
	constraint pk_preload_list primary key (object_id) using index tablespace indexes
);

create index scene_idx on preload_list (scene_id) tablespace indexes;

update version_number set version_number = 37, min_version_number=37;
