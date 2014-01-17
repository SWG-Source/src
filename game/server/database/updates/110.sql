update objects set deleted = 1 where deleted = 0 and exists (select * from waypoint_objects w where w.object_id = objects.object_id);

drop table waypoint_objects;

create table waypoints -- NO_IMPORT
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	waypoint_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	appearance_name_crc int,
	location_x float,
	location_y float,
	location_z float,
	location_cell number(20),  -- BIND_AS(DB::BindableNetworkId)
	location_scene int,
	name varchar2(100),
        color int,
        active char(1),
	constraint pk_waypoints primary key (waypoint_id)
);
grant select on waypoints to public;

create index waypoint_load_idx on waypoints (object_id) tablespace indexes;

update objects set deleted = 1 where deleted = 0 and exists (select * from mission_objects m where m.object_id = objects.object_id);

update version_number set version_number=110, min_version_number=110;
