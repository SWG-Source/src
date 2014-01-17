create table guild_objects
(
	object_id number(20),
	constraint pk_guild_objects primary key (object_id) using index tablespace indexes	
);
grant select on guild_objects to public;

update version_number set version_number=61, min_version_number=60;
