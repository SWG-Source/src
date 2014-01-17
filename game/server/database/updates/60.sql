create table counting_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	count int,
	constraint pk_counting_objects primary key (object_id) using index tablespace indexes
);

grant select on counting_objects to public;

create table factory_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	constraint pk_factory_objects primary key (object_id) using index tablespace indexes
);

grant select on factory_objects to public;

update version_number set version_number=60, min_version_number=60;
