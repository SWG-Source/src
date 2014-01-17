drop table battlefield_marker_objects;

create table battlefield_marker_objects
(
	object_id number(20),
	region_name varchar2(500),
	constraint pk_battlefield_marker_objects primary key (object_id) using index tablespace indexes
);
grant select on battlefield_marker_objects to public;

update version_number set version_number=93, min_version_number=93;
