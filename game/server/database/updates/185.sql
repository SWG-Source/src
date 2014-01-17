alter table ship_objects modify(
	cmp_armor_hp_maximum varchar2(400),
	cmp_armor_hp_current varchar2(400),
	cmp_efficiency_general varchar2(400),
	cmp_efficiency_eng varchar2(400),
	cmp_eng_maintenance varchar2(400),
	cmp_mass varchar2(400),
	cmp_hp_current varchar2(400),
	cmp_hp_maximum varchar2(400),
	cmp_flags varchar2(400),
	weapon_damage_maximum varchar2(400),
	weapon_damage_minimum varchar2(400),
	weapon_effectiveness_shields varchar2(400),
	weapon_effectiveness_armor varchar2(400),
	weapon_eng_per_shot varchar2(400),
	weapon_refire_rate varchar2(400),
	weapon_ammo_current varchar2(400),
	weapon_ammo_maximum varchar2(400),
	weapon_ammo_type varchar2(400),
	cmp_creators varchar2(400));
update version_number set version_number=185, min_version_number=185;

