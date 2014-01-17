create table res_pool_temp
as select * from resource_pool_objects;

drop table resource_pool_objects;
create table resource_pool_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	map_seed int,
	amount_remaining float,
	last_simulated_timestamp int,
	depleted_timestamp int,
	drain_rate float,
	tick_count float,
	regeneration_rate float,
	max_pool_size int,
	resource_type int, -- BIND_AS(DB::BindableNetworkId)
	planet int, -- BIND_AS(DB::BindableNetworkId)	
	primary key (object_id)
);
grant select on resource_pool_objects to public;

insert into resource_pool_objects
select * from res_pool_temp;

drop table res_pool_temp;


create table harv_inst_temp
as select * from harvester_installation_objects;

drop table harvester_installation_objects;
create table harvester_installation_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	installed_efficiency float,
	resource_pool number(20), -- BIND_AS(DB::BindableNetworkId)
	max_extraction_rate int,
	current_extraction_rate float,
	max_hopper_amount int,
	constraint pk_harvester_inst_objects primary key (object_id) using index tablespace indexes
);
grant select on harvester_installation_objects to public;

insert into harvester_installation_objects
select * from harv_inst_temp;

drop table harv_inst_temp;

grant select on delete_reasons to public;

update version_number set version_number=82, min_version_number=82;
