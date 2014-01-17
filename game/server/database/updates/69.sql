update objects set deleted = 1 where deleted = 0 and exists (select * from region_objects where region_objects.object_id = objects.object_id);
update objects set deleted = 1 where deleted = 0 and exists (select * from mission_data_objects where mission_data_objects.object_id = objects.object_id);
update objects set deleted = 1 where deleted = 0 and exists (select * from waypoint_objects where waypoint_objects.object_id = objects.object_id);

drop table region_circle_objects;
drop table region_rectangle_objects;
drop table region_objects;
drop table mission_data_objects;
drop table waypoint_objects;

create table mission_data_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	bond_amount int,
	difficulty int,
	efficiency_requirement float,
	end_x float,
	end_y float,
	end_z float,
	end_planet varchar2(500),
	end_region_name varchar2(500),
	end_cell number(20), -- BIND_AS(DB::BindableNetworkId)
	end_object number(20), -- BIND_AS(DB::BindableNetworkId)
	expire_time int,
	faction_declared char(1),
	faction_name varchar2(500),
	faction_standing int,
	friends_list char(1),
	max_holders int,
	mission_creator number(20), -- BIND_AS(DB::BindableNetworkId)
	mission_type varchar2(500),
	num_holders int,
	player_association number(20), -- BIND_AS(DB::BindableNetworkId)
	profession_name varchar2(500),
	profession_rank int,
	reward int,
	root_script_name varchar2(500),
	start_x float,
	start_y float,
	start_z float,
	start_planet varchar2(500),
	start_region_name varchar2(500),
	start_cell number(20), -- BIND_AS(DB::BindableNetworkId)
	start_object number(20), -- BIND_AS(DB::BindableNetworkId)
	target number(20), -- BIND_AS(DB::BindableNetworkId)
	target_count int,
	time_limit int,
	constraint pk_mission_data_objects primary key (object_id) using index tablespace indexes
);
grant select on mission_data_objects to public;

create table waypoint_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	active char(1),
	cell number(20),  -- BIND_AS(DB::BindableNetworkId)
	x float,
	y float,
	z float,
	planet_name varchar2(500),
	visible char(1),
	region_name varchar2(500),
	constraint pk_waypoint_objects primary key (object_id)
);
grant select on waypoint_objects to public;

update version_number set version_number=69, min_version_number=69;
