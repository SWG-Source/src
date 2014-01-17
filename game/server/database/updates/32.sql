alter table installation_objects drop column installed_efficiency;
alter table installation_objects drop column resource_pool;
alter table installation_objects drop column resource_tick_count;
alter table installation_objects add (tick_count float);
alter table installation_objects drop column max_extraction_rate;
alter table installation_objects drop column current_extraction_rate;
alter table installation_objects drop column max_hopper_amount;

create table harvester_installation_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	installed_efficiency int,
	resource_pool number(20), -- BIND_AS(DB::BindableNetworkId)
	max_extraction_rate int,
	current_extraction_rate int,
	max_hopper_amount int
);

create table manufacture_inst_objects
(
	object_id number(20)  -- BIND_AS(DB::BindableNetworkId)
);

update version_number set version_number = 32, min_version_number=32;
