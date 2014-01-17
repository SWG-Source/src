drop table resource_import;

create table resource_import  --NO_IMPORT
(
	id number , 
	parent_id number, 
	name varchar2(500), 
	friendly_name varchar2(500),
	max_types number, 
	min_types number, 
	min_pools number, 
	max_pools number, 
	resource_container_type varchar2(500),
	name_table varchar2(500),
	constraint pk_resource_import primary key (id) using index tablespace indexes
);

update version_number set version_number = 38, min_version_number=37;
