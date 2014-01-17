create table experience_points
(
	object_id number, -- BIND_AS(DB::BindableNetworkId)
	experience_type varchar2(500),
	points number,
	constraint pk_experience_points primary key (object_id, experience_type) using index tablespace indexes
);

create table manf_schematic_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	draft_schematic varchar2(500),
	creator_id number, -- BIND_AS(DB::BindableNetworkId)
	creator_name varchar2(127),
	item_count int,
	items_per_container int,
	manufacture_time float,
	constraint pk_manf_schematic_objects primary key (object_id) using index tablespace indexes
);

create table manf_schematic_attributes -- NO_IMPORT
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	attribute_type varchar2(500),
	value int,
	constraint pk_manf_schematic_attributes primary key (object_id, attribute_type) using index tablespace indexes
);

update version_number set version_number=42, min_version_number=42;
