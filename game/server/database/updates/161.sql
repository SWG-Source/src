update objects
set deleted = 1, deleted_date = sysdate, load_with = null
where exists (select 1 from ship_objects where objects.object_id = ship_objects.object_id);

drop table ship_objects;

create table ship_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	slide_dampener float,
	current_chassis_hit_points float,
	maximum_chassis_hit_points float,
	chassis_type int,
	cmp_armor_hp_maximum varchar2(200),
	cmp_armor_hp_current varchar2(200),
	cmp_efficiency_general varchar2(200),
	cmp_efficiency_eng varchar2(200),
	cmp_eng_maintenance varchar2(200),
	cmp_mass varchar2(200),
	cmp_crc varchar2(200),
	cmp_hp_current varchar2(200),
	cmp_hp_maximum varchar2(200),
	cmp_flags varchar2(200),
	cmp_names varchar2(2000),
	weapon_damage_maximum varchar2(200),
	weapon_damage_minimum varchar2(200),
	weapon_effectiveness_shields varchar2(200),
	weapon_effectiveness_armor varchar2(200),
	weapon_eng_per_shot varchar2(200),
	weapon_refire_rate varchar2(200),
	weapon_ammo_current varchar2(200),
	weapon_ammo_maximum varchar2(200),
	weapon_ammo_type varchar2(200),
	shield_hp_front_maximum float,
	shield_hp_back_maximum float,
	shield_recharge_rate float,
	capacitor_eng_maximum float,
	capacitor_eng_recharge_rate float,
	engine_acc_rate float,
	engine_deceleration_rate float,
	engine_pitch_acc_rate float,
	engine_yaw_acc_rate float,
	engine_roll_acc_rate float,
	engine_pitch_rate_maximum float,
	engine_yaw_rate_maximum float,
	engine_roll_rate_maximum float,
	engine_speed_maximum float,
	reactor_eng_generation_rate float,
	booster_eng_maximum float,
	booster_eng_recharge_rate float,
	booster_eng_consumption_rate float,
	booster_acc float,
	booster_speed_maximum float,
	droid_if_cmd_speed float,
	installed_dcd number(20),  -- BIND_AS(DB::BindableNetworkId)
	constraint pk_ship_objects primary key (object_id)
);

grant select on ship_objects to public;

update version_number set version_number=161, min_version_number=161;
