create table city_objects
(
	object_id number(20),
	constraint pk_city_objects primary key (object_id) using index tablespace indexes	
);
grant select on city_objects to public;

update version_number set version_number=128, min_version_number=128;
