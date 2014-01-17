create table resource_import
(
	id number ,
	parent_id number,
	name varchar2(500),
	max_types number,
	min_types number,
	min_pools number,
	max_pools number,
	constraint pk_resource_import primary key (id)
);

create table res_imp_oid_map
(
	object_id number ,
	reference_id number,
	new number,
	constraint pk_res_imp_oid_map primary key (object_id)
);

create table res_imp_object_variables
(
	id number(20),
	name varchar(500),
	type int,
	value varchar(1000),
	constraint pk_res_imp_object_variables primary key (id,name)
);

update version_number set version_number = 12, min_version_number=9;
