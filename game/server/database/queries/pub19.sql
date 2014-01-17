declare
	i number;

begin
	select count(*) into i
	from object_variable_names
	where name = 'armor.wookieeDeconstruct';

	if i = 0 then
		select max(id) into i
		from object_variable_names;

		insert into object_variable_names
		values (i + 1, 'armor.wookieeDeconstruct');
		
		commit;
	end if;

	select min(id) into i
	from object_variable_names
	where name = 'armor.wookieeDeconstruct';

	update objects
	set script_list = trim(script_list) || 'item.conversion.armor_wookiee:'
	where object_template_id in (
		66083222,   --object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_chest_plate.iff
		1193819287, --object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_chest_plate.iff
		856491201   --object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_chest_plate.iff
	)
	and script_list not like '%item.conversion.armor_wookiee%';

	commit;
	
	insert into object_variables
	select 	object_id, i as name_id, 0 as type, '1' as value, 0 as detached
	from objects
	where object_template_id in (
		66083222,   --object/tangible/wearables/armor/kashyyykian_black_mtn/armor_kashyyykian_black_mtn_chest_plate.iff
		1193819287, --object/tangible/wearables/armor/kashyyykian_hunting/armor_kashyyykian_hunting_chest_plate.iff
		856491201   --object/tangible/wearables/armor/kashyyykian_ceremonial/armor_kashyyykian_ceremonial_chest_plate.iff
	);
	
	commit;
	
	select count(*) into i
	from object_variable_names
	where name = 'intWeaponType';

	if i = 0 then
		select max(id) into i
		from object_variable_names;

		insert into object_variable_names
		values (i + 1, 'intWeaponType');
		
		commit;
	end if;

	select min(id) into i
	from object_variable_names
	where name = 'intWeaponType';

	insert into object_variables
	select 	object_id, i as name_id, 0 as type, '1' as value, 0 as detached
	from objects
	where object_template_id = -635207508; -- object/weapon/ranged/rifle/rifle_proton.iff
	
	commit;
	
	update objects
	set script_list = trim(script_list) || 'item.slicing.salvage_kit:'
	where object_template_id in (
		  -14212001, -- object/tangible/slicing/slicing_armor_upgrade_kit.iff
		  96542655	 -- object/tangible/slicing/slicing_weapon_upgrade_kit.iff
	)
	and script_list not like '%item.slicing.salvage_kit%';

	commit;
	
end;
/
