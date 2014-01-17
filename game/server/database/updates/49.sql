update objects set deleted = 1 where object_id in (select object_id from weapon_objects);

drop table weapon_objects;

create table weapon_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	attack_mode int,      
	min_damage int,
	max_damage int,
	attack_speed float,
	wound_chance float,
	zero_range_mod int,
	max_range_mod int,
	mid_range_mod int,
	mid_range float,
	attack_health_cost int,
	attack_action_cost int,
	attack_mind_cost int,
	constraint pk_weapon_objects primary key (object_id) using index tablespace indexes
);

grant select on weapon_objects to public;

update version_number set version_number=49, min_version_number=49;
