create or replace package body admin
as
	
	procedure move_to_player (p_object_id objectid, p_target_player objectid)
	as
		l_target_inventory objectid;
	begin
		l_target_inventory := admin.get_inventory_for_player(p_target_player);

		if (l_target_inventory <> 0) then
			move_to_container(p_object_id, l_target_inventory, p_target_player);
		end if;
	end;

	procedure move_to_player_bank (p_object_id objectid, p_target_player objectid)
	as
		l_target_bank objectid;
	begin
		l_target_bank := admin.get_bank_for_player(p_target_player);

		if (l_target_bank <> 0) then
			move_to_container(p_object_id, l_target_bank, l_target_bank);
		end if;
	end;

	procedure move_to_player_datapad (p_object_id objectid, p_target_player objectid, p_max_depth number)
	as
		l_target_datapad objectid;
	begin
		l_target_datapad := admin.get_datapad_for_player(p_target_player);

		if (l_target_datapad <> 0) then
			move_to_container_depth(p_object_id, l_target_datapad, p_target_player, p_max_depth);
		end if;

	exception
		when others then
			NULL;
	end;

	procedure move_to_container_depth (p_object_id objectid, p_target_container objectid, p_target_load_with objectid, p_max_depth number)
	as
	begin
		update objects
		set contained_by = p_target_container,
		load_with = p_target_load_with,
		x = 0,
		y = 0,
		z = 0,
		node_x = 0,
		node_z = 0
		where object_id = p_object_id;

		delete market_auctions
		where item_id = p_object_id;

		fix_load_with_depth(p_object_id, p_target_load_with, p_max_depth);
	end;

	procedure move_to_container (p_object_id objectid, p_target_container objectid, p_target_load_with objectid)
	as
	begin
		move_to_container_depth(p_object_id, p_target_container, p_target_load_with, -1);
	end;

	function get_inventory_for_player (p_player_id objectid) return objectid
	as
		result objectid;
	begin
		select object_id
		into result
		from objects
		where contained_by = p_player_id
		and object_template_id = 2007924155
		and deleted = 0;

		return result;

		exception when others then
			return 0;	
	end;

	function get_bank_for_player (p_player_id objectid) return objectid
	as
		result objectid;
	begin
		select object_id
		into result
		from objects
		where contained_by = p_player_id
		and object_template_id = -172438875
		and deleted = 0;

		return result;

		exception when others then
			return 0;	
	end;

	function get_datapad_for_player (p_player_id objectid) return objectid
	as
		result objectid;
	begin
		select object_id
		into result
		from objects
		where contained_by = p_player_id
		and object_template_id = -1783727815
		and deleted = 0;

		return result;

		exception when others then
			return 0;
	end;

	function get_player_for_player (p_player_id objectid) return objectid
	as
		result objectid;
	begin
		select object_id
		into result
		from objects
		where contained_by = p_player_id
		and object_template_id = -640104330
		and deleted = 0;

		return result;

		exception when others then
			return 0;	
	end;

	function get_container_for_object (p_object_id objectid) return objectid
	as
		result objectid;
	begin
		select contained_by 
			into result 
			from objects 
			where object_id = p_object_id;
		return result;
		exception when others then
			return 0;
	end;	

	function get_object_template_id (p_object_id objectid) return number
	as
		result number;
	begin
		select object_template_id
		into result
		from objects
		where object_id = p_object_id;

		return result;

		exception when others then
			return 0;	
	end;

	function restore_house (p_house_id objectid) return number -- 1=success, 2=no such object or not deleted
	-- result codes:
	--    1 = success
	--    2 = not a character or not deleted
	--    3 = database error
	as
		cells number;
	begin

		select count(*) into cells
		from objects
		where contained_by = p_house_id;

		if (cells = 0) then
			return 3;
		end if;

		update objects
		set deleted = 0, deleted_date = NULL, bank_balance = bank_balance + 1000 -- roughly 1 day maintenance
		where object_id = p_house_id
		and deleted <> 0;

		if (sql%rowcount <> 1) then
			return 2;
		end if;

		update tangible_objects
		set damage_taken = 0
		where object_id = p_house_id;

		update objects
		set deleted = 0, deleted_date = NULL
		where object_id in (
			select object_id
			from objects
			start with object_id = p_house_id
			connect by prior object_id = contained_by) and
                      deleted in (select reason_code
                                  from delete_reasons
                                  where tag in ('House', 'ContainerDeleted'));

		fix_load_with (p_house_id, p_house_id);

		return 1;

	exception
		when others then
			return 3;
	end;


	function restore_character (p_player_id objectid, p_name out varchar2, p_account out number, p_template_id out number) return number
	-- result codes:
	--    1 = success
	--    2 = not a character or not deleted
	--    3 = database error
	as
		character_reason number;
	begin
		select reason_code
		into character_reason
		from delete_reasons
		where tag='CharacterDeleted';

		begin
			select station_id, object_name, object_template_id
			into p_account, p_name, p_template_id
			from players p, objects o
			where p.character_object = p_player_id
			and o.object_id = p_player_id
			and o.object_id = p.character_object;
		exception
			when no_data_found then
				return 2;
		end;

		update objects
		set deleted = 0, deleted_date = NULL
		where object_id in (
			select object_id
			from objects
			start with object_id = p_player_id
			connect by prior object_id = contained_by
			and deleted = character_reason);

		return 1;

	exception
		when others then
			return 3;
	end;

	function undelete_item (p_item_id objectid) return number -- 1=success, 2=no such object or not deleted
	-- result codes:
	--    1 = success
	--    2 = not exist
	--    3 = database error
	--    4 = success, items needs to be loaded
        as
            cnt number;
            l_type_id number;
            l_name varchar2(4000);
            l_account number;
            l_template_id number;
            l_load_with number;
	    l_result number;
        begin
	    select count(*) into cnt
	    from objects
	    where object_id = p_item_id;

            if (cnt = 0) then
                return 2;
            end if;

            select count(*) into cnt
            from objects
            where object_id = p_item_id and (deleted <> 0 or deleted_date is not null);

            if (cnt = 0) then
                return 1;
            end if;

            select type_id into l_type_id
            from objects
            where object_id = p_item_id;

	    select count(*) into cnt
	    from players
	    where character_object = p_item_id;

            if (l_type_id = 1112885583) then
                -- 0x4255494f = 'BUIO', building objects
                l_result := restore_house(p_item_id);
		if (l_result = 1) then
		    return 4;
		else
		    return l_result;
		end if;
            else if (l_type_id = 1129465167 and cnt > 0) then
                -- 0x4352454f = 'CREO', creture objects and has a row in players table
		-- This is a player
                return restore_character(p_item_id, l_name, l_account, l_template_id);
                else
                    update objects
                    set deleted = 0, deleted_date = null
                    where object_id in (
                        select object_id
                                from objects
                                start with object_id = p_item_id
                                connect by prior object_id = contained_by);

                    -- find out the top most load_with
                    begin
                        select count(*) into cnt
                        from objects
                        where (contained_by = 0 or load_contents = 'N') and rownum = 1
                        start with object_id = p_item_id
                        connect by object_id = prior contained_by;
                        
                        if (cnt > 0) then
                            select object_id into l_load_with
                            from objects
                            where (contained_by = 0 or load_contents = 'N') and rownum = 1
                            start with object_id = p_item_id
                            connect by object_id = prior contained_by;
                            fix_load_with(p_item_id, l_load_with);
                        else
                            fix_load_with(p_item_id, p_item_id);
                        end if;
                        
                    exception
                        when others then
                                return 2;
                        end;
                end if;
            end if;
	    select count(*) into cnt
	    from objects
	    where object_id = p_item_id and
		  type_id in (1112885583,  -- BuildingObject
			      1212763727,  -- HarvesterInstallationObject
			      1229869903,  -- InstallationObject
			      1296649807); -- ManufactureInstallationObject
	    if (cnt = 0) then
		return 1;
	    else
		return 4;
	    end if;
        exception
            when others then
                return 2;
        end;

	/**
	 *  move an object to a player
	    parameters:
		p_object_id	item object id to move
		p_target_player	target player id
	    result:
		1:  success
		2:  cannot find player's inventory
		3:  object not found
		4:  unknown database error
		5:  object is a player

		if you add an item type here, also add it to ConsoleCommandParserObject::performParsing()
		6:  failed because object_template_id is  2007924155 (object/tangible/inventory/character_inventory.iff)
		7:  failed because object_template_id is -1436615854 (object/tangible/mission_bag/mission_bag.iff)
		8:  failed because object_template_id is -1783727815 (object/tangible/datapad/character_datapad.iff)
		9:  failed because object_template_id is -172438875  (object/tangible/bank/character_bank.iff)
		10: failed because object_template_id is  2131722719 (object/weapon/melee/unarmed/unarmed_default_player.iff)
		11: failed because object_template_id is -640104330  (object/player/player.iff)
		12: failed because object_template_id is -1388112109 (object/cell/cell.iff)
		13: failed because object_template_id is -1324492681 (object/tangible/inventory/vendor_inventory.iff)
		14: failed because direct parent is a datapad object.
		15: failed because object is a building object (has an entry in the building_objects table)
		16: failed because object is an installation object (has an entry in the installation_objects table)
		17: failed because object is a ship object (has an entry in the ship_objects table)
	 */
	function move_item_to_player (p_object_id objectid, p_target_player objectid) return number
	as
		l_target_inventory objectid;
		l_object_player objectid;
		l_object_template_id number;
		l_container_id number;
		l_container_template_id number;
		l_row_count number;
	begin
		l_object_template_id := admin.get_object_template_id(p_object_id);
		l_object_player := admin.get_player_for_player(p_object_id);
		l_target_inventory := admin.get_inventory_for_player(p_target_player);

		if (l_object_player <> 0) then
			return 5;
		end if;

		if (l_object_template_id = 2007924155) then
			return 6;
		end if;

		if (l_object_template_id = -1436615854) then
			return 7;
		end if;

		if (l_object_template_id = -1783727815) then
			return 8;
		end if;

		if (l_object_template_id = -172438875) then
			return 9;
		end if;

		if (l_object_template_id = 2131722719) then
			return 10;
		end if;

		if (l_object_template_id = -640104330) then
			return 11;
		end if;

		if (l_object_template_id = -1388112109) then
			return 12;
		end if;
		
		if (l_object_template_id = -1324492681) then
			return 13;
		end if;

		-- check to see if our parent is a datapad.

		l_container_id := admin.get_container_for_object(p_object_id);
		l_container_template_id := get_object_template_id(l_container_id);
		if (l_container_template_id = -1783727815) then
			return 14;
		end if;

		-- check to see if we are a building
		select count(*)
			into l_row_count
			from building_objects
			where object_id = p_object_id;
		if (l_row_count > 0) then
			return 15;
		end if;

		select count(*)
			into l_row_count
			from installation_objects
			where object_id = p_object_id;
		if (l_row_count > 0) then
			return 16;
		end if;

		-- check to see if we're trying to move a ship object.
		select count(*)
			into l_row_count
			from ship_objects
			where object_id = p_object_id;
		if (l_row_count >0) then
			return 17;
		end if;

		if (l_target_inventory <> 0) then
			update objects
			set contained_by = l_target_inventory,
			load_with = p_target_player,
			x = 0,
			y = 0,
			z = 0,
			node_x = 0,
			node_z = 0
			where object_id = p_object_id;
			if (sql%rowcount = 0) then
			    return 3;
			end if;
			delete market_auctions
			where item_id = p_object_id;
			fix_load_with(p_object_id, p_target_player);
		else
			return 2;
		end if;
		return 1;
	exception
		when others then
			return 4;
	end;

	/**
	 *  Fix the load_with values for anything contained by the specified object with a max depth or unlimited if max depth = -1
	 */
	procedure fix_load_with_depth (p_topmost_object objectid, p_starting_loadwith objectid, p_max_depth number) as
		oid_stack vaofnumber := vaofnumber(p_topmost_object);  -- objects to look at
		load_with_stack vaofnumber := vaofnumber(p_starting_loadwith); -- the load_with to inherit from these objects
		stack_top number := 1;
		parent_object number := 0;
		parent_load_with number := 0;
		current_load_with number := 0;
		inherit_load_with number := 0;
		stack_top_depth vaofnumber := vaofnumber(0); -- the depth of the object at the top of the stack
		current_depth number := 0;
		topmost_load_contents char(1) := '';
	begin
		-- Depth-first search

		update objects set load_with = p_starting_loadwith where object_id = p_topmost_object;

		-- if the first object being fixed is demand loaded, set child object load_with values to this object
		if (sql%rowcount = 1) then
			select load_contents into topmost_load_contents from objects where object_id = p_topmost_object;
			if (topmost_load_contents = 'N') then
				load_with_stack(stack_top) := p_topmost_object;
			end if;
		end if;

		while (stack_top <> 0) loop
			parent_object := oid_stack(stack_top);
			parent_load_with := load_with_stack(stack_top);
			current_depth := stack_top_depth(stack_top) + 1;
			stack_top := stack_top - 1;
			oid_stack.trim;
			load_with_stack.trim;

			-- keep adding children unless at max depth
			if (p_max_depth = -1 or current_depth <= p_max_depth) then
				for x in (select object_id, player_controlled, deleted, load_contents from objects where contained_by = parent_object) loop
					-- look at child objects and set their load_withs
	
					current_load_with := parent_load_with;
					inherit_load_with := parent_load_with;
	
					-- load_with defaults to the parent's load with, except in special cases:
					
					if (x.deleted <> 0 ) then -- deleted, load with is 0 for this and all its contents
						current_load_with := NULL;
						inherit_load_with := NULL;
					else
						if (x.player_controlled = 'Y') then -- player. load with = self for this and contents, even if container is deleted
							current_load_with := x.object_id;
							inherit_load_with := x.object_id;
						else
							if (x.load_contents = 'N' and current_load_with IS NOT NULL) then -- demand-load container.  contents get this as their load_with except if a parent is deleted
								inherit_load_with := x.object_id;
							end if;
						end if;
					end if;
				
					stack_top := stack_top + 1;
					oid_stack.extend;
					load_with_stack.extend;
					stack_top_depth.extend;
					oid_stack(stack_top) := x.object_id;
					load_with_stack(stack_top) := inherit_load_with;
					stack_top_depth(stack_top) := current_depth;

					update objects set load_with = current_load_with where object_id = x.object_id;
				end loop;
			end if;
		end loop;
	end;

	/**
	 *  Fix the load_with values for anything contained by the specified object
	 */
	procedure fix_load_with (p_topmost_object objectid, p_starting_loadwith objectid) as
	begin
		fix_load_with_depth(p_topmost_object, p_starting_loadwith, -1);
	end;

end;
/
