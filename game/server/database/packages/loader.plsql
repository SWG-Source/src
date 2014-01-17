create or replace package body loader as

	procedure get_version_number(current_version in out number, min_version in out number)
	as
	begin
		select version_number, min_version_number
		into current_version, min_version
		from version_number;
	end;

	function get_character_name_list return cursortype
	as
		result_cursor cursortype;
	begin
		-- free up names that aren't in use anymore
		delete from players
		where not exists (select 1 from objects where objects.object_id = character_object);

		open result_cursor for
		select character_object, station_id, uc_character_name, character_full_name,
                       (create_time - to_date('01/01/1970', 'MM/DD/YYYY')) * 24 * 3600,
                       (last_login_time - to_date('01/01/1970', 'MM/DD/YYYY')) * 24 * 3600
		from players;

		return result_cursor;
	end;

	function locate_player (p_object_id number) return number
	as
	begin
		insert into object_list (object_id, container_level)
		select object_id, 100
		from objects
		where load_with = p_object_id and deleted = 0;

		return sql%rowcount;
	end;

	procedure locate_universe
	as
	begin
		insert into object_list (object_id)
		select o.object_id
		from objects o, universe_objects u
		where o.object_id = u.object_id
		and o.deleted=0;
	end;

	procedure locate_contained_object (p_container_id number, p_object_id number)
	as
	begin
		insert into object_list (object_id, container_level)
		select object_id, 100
		from objects o
		start with
			o.object_id = p_object_id
			and o.contained_by = p_container_id
			and o.deleted = 0
		connect by
			prior o.object_id = o.contained_by
			and o.deleted = 0;
	end;

	function locate_by_loadwith_batch (p_loadwiths vaofstring, p_chunk_size number)
	return number
	as
	begin
		forall i in 1..p_chunk_size
			insert into object_list (object_id)
			select o.object_id
			from objects o
			where o.load_with = p_loadwiths(i) and o.deleted = 0;

		return sql%rowcount;
	end;

	procedure locate_contents (p_container_id number)
	as
	begin
		insert into object_list (object_id, container_level)
		select object_id, 100
		from objects o
		where o.load_with = p_container_id and o.deleted = 0;
	end;

	procedure locate_structure (p_object_id number, p_x out number, p_z out number, p_scene_id out varchar2, p_found out number)
	as
	begin
		select x, z, scene_id, 1 into p_x, p_z, p_scene_id, p_found
		from objects
		where object_id = p_object_id and deleted = 0 and
		      type_id in (1112885583,  -- BuildingObject
				  1212763727,  -- HarvesterInstallationObject
				  1229869903,  -- InstallationObject
				  1296649807); -- ManufactureInstallationObject
	exception
	    when no_data_found then
		p_found := 0;
	end;

	function get_characters (p_station_id number) return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
		select
			character_object,
			object_template_id,
			scene_id,
			object_name,
			container,
			x,
			y,
			z
		from character_view
		where station_id = p_station_id;

		return result_cursor;
	end;

	function load_chunk_object_list (p_scene_id varchar, p_node_x float, p_node_z float) return number
	as
		num number;
	begin
		select count(*)
		into num
		from objects
		where node_x = p_node_x
			and node_z = p_node_z
			and scene_id = p_scene_id
			and deleted = 0;

		if (num > 0) then
			insert into object_list (object_id, container_level)
			select object_id, level
			from objects
			start with
				node_x = p_node_x
				and node_z = p_node_z
				and scene_id = p_scene_id
				and deleted = 0
				and contained_by = 0
				and player_controlled = 'N'
			connect by 
				prior object_id = contained_by
				and deleted = 0
				and player_controlled='N'
				and prior load_contents='Y';

			return sql%rowcount;
		else
			return 0;
		end if;		
	end;

	function load_manf_schematic_attributes return cursortype
	as
		result_cursor cursortype;
	begin
	open result_cursor for
		select  /*+ ORDERED USE_NL(T) */ t.object_id, t.attribute_type, t.value
		from	
			object_list l,
			manf_schematic_attributes t
		where t.object_id = l.object_id;

		return result_cursor;
	end;

	function load_armor return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.layer,
				t.object_template,
				t.effectiveness, 
				t.integrity,
				t.special_protections,
				t.encumberance_0,
				t.encumberance_1,
				t.encumberance_2,
				t.encumberance_3,
				t.encumberance_4,
				t.encumberance_5,
				t.encumberance_6,
				t.encumberance_7,
				t.encumberance_8
			from
				object_list l,
				armor t
			where t.object_id = l.object_id;

		return result_cursor;
	end;

	function load_scripts return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T)*/
				t.object_id, t.script, t.sequence_no
			from
				object_list l,
				scripts t
			where t.object_id = l.object_id;

		return result_cursor;
	end;

	function load_object_variables return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T)*/
				t.object_id, t.name_id, t.type, t.value
			from
				object_list l,
				object_variables t
			where l.object_id = t.object_id
			and nvl(t.detached,0) = 0;

		return result_cursor;
	end;

	function load_property_lists return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id, t.list_id, t.value
			from object_list l, property_lists t
			where t.object_id = l.object_id;

		return result_cursor;
	end;

	function load_experience return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id, t.experience_type, t.points
			from object_list l, experience_points t
			where t.object_id = l.object_id;

		return result_cursor;
	end;

	function load_battlefield_participants return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select t.region_object_id, t.character_object_id, t.faction_id  /*+ ORDERED USE_NL(T) */
			from object_list l, battlefield_participants t
			where t.region_object_id = l.object_id; -- order does not matter

		return result_cursor;
	end;

	function load_messages return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(MESSAGES) */
				target, message_id, method, data, call_time, guaranteed, delivery_type
			from  object_list, messages
			where target =object_list.object_id;

		return result_cursor;
	end;

	function load_location return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select  /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.list_id,
				t.sequence_number,
				t.name,
				t.scene,
				t.x,
				t.y,
				t.z,
				t.radius
			from
				object_list l,
				location_lists t
			where t.object_id = l.object_id;

		return result_cursor;
	end;

	function load_object
	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				x,
				y,
				z,
				quaternion_w,
				quaternion_x,
				quaternion_y,
				quaternion_z,
				node_x,
				node_y,
				node_z,
				object_template_id,
				type_id,
				scene_id,
				controller_type,
				deleted,
				object_name,
				volume,
				contained_by,
				slot_arrangement,
				player_controlled,
				cache_version,
				load_contents,
				cash_balance,
				bank_balance,
				complexity,
				name_string_table,
				name_string_text,
				static_item_name,
				nvl(static_item_version,0),
				nvl(conversion_id,0),
				load_with,
				objvar_0_name,
				objvar_0_type,
				objvar_0_value,
				objvar_1_name,
				objvar_1_type,
				objvar_1_value,
				objvar_2_name,
				objvar_2_type,
				objvar_2_value,
				objvar_3_name,
				objvar_3_type,
				objvar_3_value,
				objvar_4_name,
				objvar_4_type,
				objvar_4_value,
				objvar_5_name,
				objvar_5_type,
				objvar_5_value,
				objvar_6_name,
				objvar_6_type,
				objvar_6_value,
				objvar_7_name,
				objvar_7_type,
				objvar_7_value,
				objvar_8_name,
				objvar_8_type,
				objvar_8_value,
				objvar_9_name,
				objvar_9_type,
				objvar_9_value,
				objvar_10_name,
				objvar_10_type,
				objvar_10_value,
				objvar_11_name,
				objvar_11_type,
				objvar_11_value,
				objvar_12_name,
				objvar_12_type,
				objvar_12_value,
				objvar_13_name,
				objvar_13_type,
				objvar_13_value,
				objvar_14_name,
				objvar_14_type,
				objvar_14_value,
				objvar_15_name,
				objvar_15_type,
				objvar_15_value,
				objvar_16_name,
				objvar_16_type,
				objvar_16_value,
				objvar_17_name,
				objvar_17_type,
				objvar_17_value,
				objvar_18_name,
				objvar_18_type,
				objvar_18_value,
				objvar_19_name,
				objvar_19_type,
				objvar_19_value,
				script_list,
				l.container_level
			from
				object_list l,
				objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function verify_containment_chain(p_start_with_object_id in number) return number
	as
	-- result codes:
		--    1 = containment chain is recursive and is in starting(character) object chain 
		--        (can be fixed by placing the object in the world if its a character)	
		--    2 = containment chain is recursive but not in starting object chain (this must be fixed manually)
		--    3 = no recursion in containment chain
		
		m_retval pls_integer;
		m_temp   pls_integer;
	begin
/*	        m_retval :=  2;
		
		select count(*) into m_temp
		from objects 
		start with object_id = p_start_with_object_id
		connect by prior object_id = contained_by
		and object_id != p_start_with_object_id;
*/		
		m_retval :=  1;
		
		select count(*) into m_temp
		from objects 
		start with object_id = p_start_with_object_id
		connect by object_id = prior contained_by;
		
		m_retval :=  3;
				
		return m_retval;
	exception
		when others then
			return m_retval;
	end;
	
	function fix_player_containment_chain(p_start_with_object_id in number) return number
	as
	-- result codes:
		--    0 = database error		
		--    4 = success fix player containment issues
		
		PRAGMA AUTONOMOUS_TRANSACTION;
		
	begin
		update objects set contained_by = 0 where object_id = p_start_with_object_id;
		admin.fix_load_with(p_start_with_object_id, p_start_with_object_id);
		commit;
		
		return 4;
		
	exception
		when others then
			return 0;
	end;



procedure verify_character (p_station_id in number, p_character_id in number, p_gold_schema in varchar2, p_approved out varchar2, p_character_name out varchar2, p_scene_id out varchar2, p_container_id out number, p_x out number, p_y out number, p_z out number, p_containment_check out number)
	as

	-- p_containment_check is a flag indicating the containment stautus to be used for logging on the C sid
		-- 0 = there was a database error trying to fix containment
		-- 1 = there is a recursive containment error in the character object chain that wasn't fixed
		-- 2 = there is a recursive containment error in the containment chain not in the character chain
		-- 3 = the containment chain is ok
		-- 4 = there was a recursive containment error in the character object chain that was fixed

	m_containment_ok pls_integer;
	
	wsx number := null;
	wsy number := null;
	wsz number := null;

	begin
	

	-- check containment chain
	m_containment_ok := verify_containment_chain(p_character_id);

	-- if containment is bad for the character object try to fix it
	if (m_containment_ok = 1) then
		m_containment_ok := fix_player_containment_chain(p_character_id);
	end if;

	p_containment_check := m_containment_ok;

	if (m_containment_ok >= 3) then

		select object_name, scene_id, contained_by, x, y, z, ws_x, ws_y, ws_z
		into p_character_name, p_scene_id, p_container_id, p_x, p_y, p_z, wsx, wsy, wsz
		from players p, objects o, creature_objects c
		where p.station_id = p_station_id
		and p.character_object = p_character_id
		and o.object_id = p.character_object
		and c.object_id = o.object_id
		and o.deleted = 0;

		if (p_container_id = 0) then
			begin
				p_container_id := p_character_id;
				p_approved := 'Y';
			end;
		else
			begin
				select object_id, x,y,z,'Y'
				into p_container_id, p_x, p_y, p_z, p_approved
				from objects
				where contained_by=0
				start with object_id = p_character_id
				connect by prior contained_by=object_id and prior scene_id=scene_id and player_controlled = 'N';

			exception
				when no_data_found then
					-- might be in a gold building
					if (p_gold_schema is not null) then
						begin
							execute immediate
								'select object_id, x,y,z,''Y'' ' ||
								'from ' || p_gold_schema || 'objects ' ||
								'where contained_by=0 ' ||
								'start with object_id = :container_id ' ||
								'connect by prior contained_by=object_id'
							into p_container_id, p_x, p_y, p_z, p_approved
							using p_container_id;

						exception
							when no_data_found then
							select object_id, x,y,z,'Y'
							into p_container_id, p_x, p_y, p_z, p_approved
							from objects
							where object_id = p_character_id;
						end;
					else
						select object_id, x,y,z,'Y'
						into p_container_id, p_x, p_y, p_z, p_approved
						from objects
						where object_id = p_character_id;
					end if;
			end;
		end if;

	else

		select 'N' into p_approved from dual;

	end if;


                if ( wsx is not null ) then
                        begin
                                 p_x := wsx;
                                 p_y := wsy;
                                 p_z := wsz;
                        end;
		end if;


		update players
		set last_login_time = sysdate
		where station_id = p_station_id
		and character_object = p_character_id;

	exception
		when no_data_found then
			select 'N'
			into p_approved
			from dual;
		when too_many_rows then
			select 'N'
			into p_approved
			from dual;
		when others then
			select 'N'
			into p_approved
			from dual;
	end;




	function load_waypoint
	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.waypoint_id,
				t.appearance_name_crc,
				t.location_x,
				t.location_y,
				t.location_z,
				t.location_cell,
				t.location_scene,
				t.name,
				t.color,
				t.active
			from
				object_list l,
				waypoints t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_player_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) USE_NL(A)*/
				t.object_id,
				t.station_id,
				a.house_id,
 				t.num_lots,
 				a.is_outcast,
 				a.cheater_level,
 				a.max_lots_adjustment,
				t.personal_profile_id,
				t.character_profile_id,
				t.skill_title,
				t.born_date,
				t.played_time,
				t.force_regen_rate,
				t.force_power,
				t.max_force_power,
				t.active_quests,
				t.completed_quests,
				t.current_quest,
				t.quests,
				t.role_icon_choice,
				t.quests2,
				t.quests3,
				t.quests4,
				t.skill_template,
				t.working_skill,
				nvl(t.current_gcw_points,0),
				nvl(t.current_gcw_rating,-1),
				nvl(t.current_pvp_kills,0),
				nvl(t.lifetime_gcw_points,0),
				nvl(t.max_gcw_imperial_rating,-1),
				nvl(t.max_gcw_rebel_rating,-1),
				nvl(t.lifetime_pvp_kills,0),
				nvl(t.next_gcw_rating_calc_time,0),
				t.collections,
				nvl(t.show_backpack,'Y'),
				nvl(t.show_helmet,'Y'),
				t.collections2
			from
				object_list l,
				player_objects t,
				accounts a
			where t.station_id = a.station_id and
				t.object_id=l.object_id;

		return result_cursor;
	end;
	
	function load_resource_types return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select
				resource_id,
				resource_name,
				resource_class,
				attributes,
				fractal_seeds,
				depleted_timestamp
			from
				resource_types;
		
		return result_cursor;
	end;

 	function load_bounty_hunter_targets return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select object_id, target_id
			from bounty_hunter_targets
			where target_id <> 0;
		return result_cursor;
	end;

-- GENERATED PLSQL FOLLOWS
-- generated by makeloader.pl

	function load_battlefield_marker_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.region_name
			from
				object_list l,
				battlefield_marker_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_building_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.maintenance_cost,
				t.time_last_checked,
				t.is_public,
				t.city_id
			from
				object_list l,
				building_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_cell_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.cell_number,
				t.is_public
			from
				object_list l,
				cell_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_city_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id
			from
				object_list l,
				city_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_creature_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.scale_factor,
				t.states,
				t.posture,
				t.shock_wounds,
				t.master_id,
				t.rank,
				t.base_walk_speed,
				t.base_run_speed,
				t.attribute_0,
				t.attribute_1,
				t.attribute_2,
				t.attribute_3,
				t.attribute_4,
				t.attribute_5,
				t.attribute_6,
				t.attribute_7,
				t.attribute_8,
				t.attribute_9,
				t.attribute_10,
				t.attribute_11,
				t.attribute_12,
				t.attribute_13,
				t.attribute_14,
				t.attribute_15,
				t.attribute_16,
				t.attribute_17,
				t.attribute_18,
				t.attribute_19,
				t.attribute_20,
				t.attribute_21,
				t.attribute_22,
				t.attribute_23,
				t.attribute_24,
				t.attribute_25,
				t.attribute_26,
				t.persisted_buffs,
				t.ws_x,
				t.ws_y,
				t.ws_z
			from
				object_list l,
				creature_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_factory_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id
			from
				object_list l,
				factory_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_guild_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id
			from
				object_list l,
				guild_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_harvester_inst_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.installed_efficiency,
				t.max_extraction_rate,
				t.current_extraction_rate,
				t.max_hopper_amount,
				t.hopper_resource,
				t.hopper_amount,
				t.resource_type
			from
				object_list l,
				harvester_installation_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_installation_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.installation_type,
				t.activated,
				t.tick_count,
				t.activate_start_time,
				t.power,
				t.power_rate
			from
				object_list l,
				installation_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_intangible_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.count
			from
				object_list l,
				intangible_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_manf_schematic_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.creator_id,
				t.creator_name,
				t.items_per_container,
				t.manufacture_time,
				t.draft_schematic
			from
				object_list l,
				manf_schematic_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_manufacture_inst_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id
			from
				object_list l,
				manufacture_inst_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_mission_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.difficulty,
				t.end_x,
				t.end_y,
				t.end_z,
				t.end_cell,
				t.end_scene,
				t.mission_creator,
				t.reward,
				t.root_script_name,
				t.start_x,
				t.start_y,
				t.start_z,
				t.start_cell,
				t.start_scene,
				t.description_table,
				t.description_text,
				t.title_table,
				t.title_text,
				t.mission_holder_id,
				t.status,
				t.mission_type,
				t.target_appearance,
				t.target_name
			from
				object_list l,
				mission_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_planet_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.planet_name
			from
				object_list l,
				planet_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_resource_container_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.resource_type,
				t.quantity,
				t.source
			from
				object_list l,
				resource_container_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_ship_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.slide_dampener,
				t.current_chassis_hit_points,
				t.maximum_chassis_hit_points,
				t.chassis_type,
				t.cmp_armor_hp_maximum,
				t.cmp_armor_hp_current,
				t.cmp_efficiency_general,
				t.cmp_efficiency_eng,
				t.cmp_eng_maintenance,
				t.cmp_mass,
				t.cmp_crc,
				t.cmp_hp_current,
				t.cmp_hp_maximum,
				t.cmp_flags,
				t.cmp_names,
				t.weapon_damage_maximum,
				t.weapon_damage_minimum,
				t.weapon_effectiveness_shields,
				t.weapon_effectiveness_armor,
				t.weapon_eng_per_shot,
				t.weapon_refire_rate,
				t.weapon_ammo_current,
				t.weapon_ammo_maximum,
				t.weapon_ammo_type,
				t.shield_hp_front_maximum,
				t.shield_hp_back_maximum,
				t.shield_recharge_rate,
				t.capacitor_eng_maximum,
				t.capacitor_eng_recharge_rate,
				t.engine_acc_rate,
				t.engine_deceleration_rate,
				t.engine_pitch_acc_rate,
				t.engine_yaw_acc_rate,
				t.engine_roll_acc_rate,
				t.engine_pitch_rate_maximum,
				t.engine_yaw_rate_maximum,
				t.engine_roll_rate_maximum,
				t.engine_speed_maximum,
				t.reactor_eng_generation_rate,
				t.booster_eng_maximum,
				t.booster_eng_recharge_rate,
				t.booster_eng_consumption_rate,
				t.booster_acc,
				t.booster_speed_maximum,
				t.droid_if_cmd_speed,
				t.installed_dcd,
				t.chassis_cmp_mass_maximum,
				t.cmp_creators,
				t.cargo_hold_contents_maximum,
				t.cargo_hold_contents_current,
				t.cargo_hold_contents
			from
				object_list l,
				ship_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_static_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id
			from
				object_list l,
				static_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_tangible_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.max_hit_points,
				t.owner_id,
				t.visible,
				t.appearance_data,
				t.interest_radius,
				t.pvp_type,
				t.pvp_faction,
				t.damage_taken,
				t.custom_appearance,
				t.count,
				t.condition,
				t.creator_id,
				t.source_draft_schematic
			from
				object_list l,
				tangible_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_token_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.reference,
				t.target_server_template_name,
				t.target_shared_template_name,
				t.waypoint
			from
				object_list l,
				token_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_universe_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id
			from
				object_list l,
				universe_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_vehicle_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.bogus
			from
				object_list l,
				vehicle_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;

	function load_weapon_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.min_damage,
				t.max_damage,
				t.damage_type,
				t.elemental_type,
				t.elemental_value,
				t.attack_speed,
				t.wound_chance,
				t.accuracy,
				t.attack_cost,
				t.damage_radius,
				t.min_range,
				t.max_range
			from
				object_list l,
				weapon_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;
	

	function load_player_quest_object	return cursortype
	as
		result_cursor cursortype;
	begin
		open result_cursor for
			select /*+ ORDERED USE_NL(T) */
				t.object_id,
				t.title,
				t.description,
				t.creator,
				t.total_tasks,
				t.difficulty,
				t.task_title1,
				t.task_description1,
				t.task_title2,
				t.task_description2,
				t.task_title3,
				t.task_description3,
				t.task_title4,
				t.task_description4,
				t.task_title5,
				t.task_description5,
				t.task_title6,
				t.task_description6,
				t.task_title7,
				t.task_description7,
				t.task_title8,
				t.task_description8,
				t.task_title9,
				t.task_description9,
				t.task_title10,
				t.task_description10,
				t.task_title11,
				t.task_description11,
				t.task_title12,
				t.task_description12
			from
				object_list l,
				player_quest_objects t
			where
				t.object_id=l.object_id;

		return result_cursor;
	end;
end;
/
