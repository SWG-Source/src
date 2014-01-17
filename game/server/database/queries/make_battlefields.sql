update objects set deleted=1 where exists (select 1 from battlefield_marker_objects b where objects.object_id = b.object_id);
delete from battlefield_participants;

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(2488, 0, 4388, 1, 0, 0, 0,
		2400, 0, 4300, -1985150318, 1112363595, 'tatooine',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:two_fortresses');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(4950, 0, 4650, 1, 0, 0, 0,
		4900, 0, 4600, 1128961081, 1112363595, 'tatooine',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:dune_sea');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-4113, 50, -533, 1, 0, 0, 0,
		-4200, 0, -600, 1128961081, 1112363595, 'naboo',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:naboo_pvp_01');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-3762, 10, -5377, 1, 0, 0, 0,
		-3800, 0, -5400, -1985150318, 1112363595, 'naboo',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:naboo_gungan_ruins');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-5044, -207, 6662, 1, 0, 0, 0,
		-5100, -300, 6600, -1985150318, 1112363595, 'naboo',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:imperial_beach_fortress');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-2337, 77, 6427, 1, 0, 0, 0,
		-2400, 0, 6400, 1128961081, 1112363595, 'rori',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:rori_pvp_01');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(2879, 107, -1277, 1, 0, 0, 0,
		2800, 100, -1300, 1128961081, 1112363595, 'rori',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:rori_pvp_02');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-3372, 76, -6972, 1, 0, 0, 0,
		-3400, 0, -7000, -1985150318, 1112363595, 'rori',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:rori_mudflats_pve');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-3420, 22, -3196, 1, 0, 0, 0,
		-3500, 0, -3200, -1985150318, 1112363595, 'talus',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:talus_pve_01');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-5355, 0, 2783.5, 1, 0, 0, 0,
		-5400, 0, 2700, 1128961081, 1112363595, 'talus',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:talus_pvp_01');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-411, 0, 4639, 1, 0, 0, 0,
		-500, 0, 4600, 1128961081, 1112363595, 'talus',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:talus_pvp_02');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(3791, 0, -2416, 1, 0, 0, 0,
		3700, 0, -2500, 1128961081, 1112363595, 'yavin4',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:yavin4_mountain_pvp');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-4230, 0, 3755, 1, 0, 0, 0,
		-4300, 0, 3700, 1128961081, 1112363595, 'yavin4',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:yavin4_forest_pvp');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(4162, 0, -4279, 1, 0, 0, 0,
		4100, 0, -4300, 1128961081, 1112363595, 'dathomir',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:dathomir_canyon_pvp');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-469, 0, 3651, 1, 0, 0, 0,
		-500, 0, 3600, 1128961081, 1112363595, 'endor',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:endor_salma_desert');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-2790.5, 0, -4457.5, 1, 0, 0, 0,
		-2800, 0, -4500, 1128961081, 1112363595, 'endor',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:endor_forest');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(263, 0, 4620, 1, 0, 0, 0,
		200, 0, 4600, 1128961081, 1112363595, 'corellia',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:corellia_pvp');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(-1870, 7, -1223, 1, 0, 0, 0,
		-1900, 0, -1300, -1985150318, 1112363595, 'corellia',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:corellia_rebel_riverside_fort');
end;
/

declare
	new_obj_id number;
begin
	new_obj_id := objectidmanager.get_single_id();
	insert into objects
		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,
		node_x, node_y, node_z, object_template_id, type_id, scene_id,
		controller_type, deleted, object_name, volume, contained_by,
		slot_arrangement, player_controlled, cache_version, load_contents,
		cash_balance, bank_balance, complexity, name_string_table,
		name_string_text, load_with, script_list, object_id)
		values
		(3785, 380, -4037, 1, 0, 0, 0,
		3700, 300, -4100, -1985150318, 1112363595, 'corellia',
		0, 0, '', 1, 0,
		-1, 'N', 0, 'Y',
		0, 0, 1, 'battlefield',
		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);
	insert into tangible_objects
		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,
		count, condition, creator_id, source_draft_schematic, object_id)
		values
		(0, 'Y', 0, 0, 0,
		0,768,0,0,new_obj_id);
	insert into battlefield_marker_objects
		(object_id, region_name)
		values
		(new_obj_id, '@battlefield:corellia_mountain_fortress');
end;
/
