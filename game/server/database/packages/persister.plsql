create or replace package body persister
as

	procedure set_clock (p_time number)
	as
	begin
		update clock set last_save_time = p_time, last_save_timestamp = sysdate;
	end;

	procedure add_player (p_station_id number, p_character_object objectid, p_character_name varchar2, p_normalized_name varchar2)
	as
		rc number;
	begin
		insert into players (station_id, character_object,uc_character_name, character_full_name, create_time)
		values (p_station_id, p_character_object, p_normalized_name, p_character_name, sysdate);
	end;

	function delete_character (p_station_id number, p_character_id number, p_delete_minutes number) return number
	as
		valid_check number;
		l_deleted_name number;
	begin
		l_deleted_name := 0;		

		select count(*)
		into valid_check
		from players
		where players.character_object = p_character_id
			and players.station_id = p_station_id;

		if (valid_check = 1) then

			-- recursive query needed in order to get bank box contents, etc.
			update objects
			set
				deleted = (select reason_code from delete_reasons where tag='CharacterDeleted'),
				deleted_date = sysdate
			where object_id in (select object_id
				from objects
				where deleted = 0
				start with load_with = p_character_id
				connect by prior object_id = load_with and prior load_with <> load_with);

			-- remove character record if the character is new enough (so that players can experiment creating new characters
			-- and deleting them right away)
			delete players
			where players.character_object = p_character_id
			and players.station_id = p_station_id
			and (sysdate - create_time) * (1440) < p_delete_minutes;

			if (sql%rowcount > 0) then
				l_deleted_name := 1;
			end if;

			-- Add a message for all deleted buildings owned by this character.  The game will delete the buildings
			-- and run appropriate cleanup the next time the buildings are loaded.

			for oid in (select o.object_id from objects o, tangible_objects t, building_objects b
				where o.object_id = t.object_id
				and o.object_id = b.object_id
				and t.object_id = b.object_id
				and t.owner_id = p_character_id
				and o.deleted = 0) 
			loop
				save_message (objectidmanager.get_single_id(), oid.object_id, 'msgDestroyStructure',NULL,0,'Y',1);
			end loop;

			for oid1 in (select o.object_id from objects o, tangible_objects t, installation_objects b
				where o.object_id = t.object_id
				and o.object_id = b.object_id
				and t.owner_id = p_character_id
				and o.deleted = 0) 
			loop
				save_message (objectidmanager.get_single_id(), oid1.object_id, 'msgDestroyStructure',NULL,0,'Y',1);
			end loop;

			if (l_deleted_name=0) then
				return 0;
			else
				return 2;
			end if;
		else
			return 1;
		end if;
	end;


	procedure update_object_variable_batch (p_object_id vaofstring, p_name_id VAOFNUMBER, p_type VAOFNUMBER, p_value VAOFSTRING, p_chunk_size number)
	as
	begin
		FORALL i in 1..p_chunk_size
			UPDATE OBJECT_VARIABLES Set
				type = p_type(i), 
				value = p_value(i), 
				detached = 0
			WHERE 
				object_id = p_object_id(i) and 
				name_id = p_name_id(i);
	end;

	procedure add_object_variable_batch (p_object_id vaofstring, p_name_id VAOFNUMBER, p_type VAOFNUMBER, p_value VAOFSTRING, p_chunk_size number)
	as
		errors number;
		error_index number;
	begin
		FORALL i in 1..p_chunk_size
			INSERT into OBJECT_VARIABLES (
				object_id, 
				name_id, 
				type, 
				value, 
				detached )
			VALUES ( 
				p_object_id(i), 
				p_name_id(i), 
				p_type(i), 
				p_value(i), 
				0 );          

		exception
			when DUP_VAL_ON_INDEX then
				for i in 1..p_chunk_size loop
					UPDATE OBJECT_VARIABLES Set
						type = p_type(i), 
						value = p_value(i), 
						detached = 0
					WHERE 
						object_id = p_object_id(i) and 
						name_id = p_name_id(i);

					if SQL%ROWCOUNT=0 then
						INSERT into OBJECT_VARIABLES (
							object_id, 
							name_id, 
							type, 
							value, 
							detached )
						VALUES ( 
							p_object_id(i), 
							p_name_id(i), 
							p_type(i), 
							p_value(i), 
							0 );          
					end if;
				end loop;
	end;

	procedure remove_object_variable_batch (p_object_id vaofstring, p_name_id VAOFNUMBER, p_chunk_size number)
	as
	begin
		FORALL i in 1..p_chunk_size
			delete from object_variables
			where object_id = p_object_id(i)
			and name_id = p_name_id(i);      
	end;

	procedure clear_scripts (p_object_id objectid)
	as
	begin
		delete from scripts
		where object_id = p_object_id;
	end;
	
	procedure save_message (p_message_id number, p_object_id objectid, p_method varchar, p_data varchar, p_call_time number, p_guaranteed varchar, p_delivery_type int)
	as
	begin
		insert into messages (message_id, target, method, data, call_time, guaranteed, delivery_type)
		values (p_message_id, p_object_id, p_method, p_data, p_call_time, p_guaranteed, p_delivery_type);
	end;

	procedure save_message_batch (p_message_id VAOFSTRING, p_object_id VAOFSTRING, p_method VAOFSTRING, p_data VAOFLONGSTRING, p_call_time VAOFNUMBER, p_guaranteed VAOFSTRING, p_delivery_type VAOFNUMBER, p_chunk_size number, p_enable_db_logging number )
	as
	begin

		forall i in 1..p_chunk_size
			insert into messages (message_id, target, method, data, call_time, guaranteed, delivery_type)
			values (p_message_id(i), p_object_id(i), p_method(i), p_data(i), p_call_time(i), p_guaranteed(i), p_delivery_type(i));

		exception when dup_val_on_index then
				for i in 1..p_chunk_size loop
					begin
						insert into messages (message_id, target, method, data, call_time, guaranteed, delivery_type)
						values (p_message_id(i), p_object_id(i), p_method(i), p_data(i), p_call_time(i), p_guaranteed(i), p_delivery_type(i));
					exception when dup_val_on_index then
						if ( p_enable_db_logging > 0 ) then
					
							db_error_logger.dblogerror( SQLCODE, 'procedure save_message_batch: duplicate rows were sent to the message buffer that resulted in a dup_val_on_index error.');
							if ( p_enable_db_logging > 1 ) then
						
								db_error_logger.dblogerror_values( 'messages', 'message_id' , 'number', p_message_id(i));
								db_error_logger.dblogerror_values( 'messages', 'target' , 'number', p_object_id(i));
								db_error_logger.dblogerror_values( 'messages', 'method' , 'varchar2', p_method(i));
								db_error_logger.dblogerror_values( 'messages', 'data' , 'varchar2', p_data(i));
								db_error_logger.dblogerror_values( 'messages', 'call_time' , 'number', to_char(p_call_time(i)));
								db_error_logger.dblogerror_values( 'messages', 'guaranteed' , 'varchar2', p_guaranteed(i));
								db_error_logger.dblogerror_values( 'messages', 'delivery_type' , 'number', to_char(p_delivery_type(i)));

							end if;
					
						else
							NULL;
						end if;
					end;
				end loop;
	end;


	procedure acknowledge_message(p_message_id number)
	as
	begin
		delete from messages
		where message_id = p_message_id;
	end;

	procedure acknowledge_message_batch (p_message_id VAOFSTRING, p_chunk_size number, p_enable_db_logging number)
	as
	begin

		forall i in 1..p_chunk_size
			delete from messages
			where message_id = p_message_id(i);

		exception when dup_val_on_index then
				for i in 1..p_chunk_size loop
					begin
						delete from messages
						where message_id = p_message_id(i);
					exception when dup_val_on_index then
						if ( p_enable_db_logging > 0 ) then
					
							db_error_logger.dblogerror( SQLCODE, 'procedure acknowledge_message_batch: duplicate rows were sent to the message buffer that resulted in a dup_val_on_index error.');
							if ( p_enable_db_logging > 1 ) then
						
								db_error_logger.dblogerror_values( 'messages', 'message_id' , 'number', p_message_id(i));
								
							end if;
					
						else
							NULL;
						end if;
					end;
				end loop;


	end;

	procedure save_object ( p_x VAOFNUMBER, p_y VAOFNUMBER, p_z VAOFNUMBER, p_quaternion_w VAOFNUMBER, p_quaternion_x VAOFNUMBER, p_quaternion_y VAOFNUMBER, p_quaternion_z VAOFNUMBER, p_node_x VAOFNUMBER, p_node_y VAOFNUMBER, p_node_z VAOFNUMBER, p_object_template_id VAOFNUMBER, p_type_id VAOFNUMBER, p_scene_id VAOFSTRING, p_controller_type VAOFNUMBER, p_deleted VAOFNUMBER, p_object_name VAOFSTRING, p_volume VAOFNUMBER, p_contained_by VAOFSTRING, p_slot_arrangement VAOFNUMBER, p_player_controlled VAOFSTRING, p_cache_version VAOFNUMBER, p_load_contents VAOFSTRING, p_cash_balance VAOFNUMBER, p_bank_balance VAOFNUMBER, p_complexity VAOFNUMBER, p_name_string_table VAOFSTRING, p_name_string_text VAOFSTRING, p_static_item_name VAOFSTRING, p_static_item_version VAOFNUMBER, p_conversion_id VAOFNUMBER, p_load_with VAOFSTRING, 
		p_objvar_0_name VAOFSTRING,p_objvar_0_type VAOFNUMBER,p_objvar_0_value VAOFSTRING,p_objvar_1_name VAOFSTRING,p_objvar_1_type VAOFNUMBER,p_objvar_1_value VAOFSTRING,p_objvar_2_name VAOFSTRING,p_objvar_2_type VAOFNUMBER,p_objvar_2_value VAOFSTRING,p_objvar_3_name VAOFSTRING,p_objvar_3_type VAOFNUMBER,p_objvar_3_value VAOFSTRING,p_objvar_4_name VAOFSTRING,p_objvar_4_type VAOFNUMBER,p_objvar_4_value VAOFSTRING,p_objvar_5_name VAOFSTRING,p_objvar_5_type VAOFNUMBER,p_objvar_5_value VAOFSTRING,p_objvar_6_name VAOFSTRING,p_objvar_6_type VAOFNUMBER,p_objvar_6_value VAOFSTRING,p_objvar_7_name VAOFSTRING,p_objvar_7_type VAOFNUMBER,p_objvar_7_value VAOFSTRING,p_objvar_8_name VAOFSTRING,p_objvar_8_type VAOFNUMBER,p_objvar_8_value VAOFSTRING,p_objvar_9_name VAOFSTRING,p_objvar_9_type VAOFNUMBER,p_objvar_9_value VAOFSTRING, p_objvar_10_name VAOFSTRING,p_objvar_10_type VAOFNUMBER,p_objvar_10_value VAOFSTRING,p_objvar_11_name VAOFSTRING,p_objvar_11_type VAOFNUMBER,p_objvar_11_value VAOFSTRING,p_objvar_12_name VAOFSTRING,p_objvar_12_type VAOFNUMBER,p_objvar_12_value VAOFSTRING,p_objvar_13_name VAOFSTRING,p_objvar_13_type VAOFNUMBER,p_objvar_13_value VAOFSTRING,p_objvar_14_name VAOFSTRING,p_objvar_14_type VAOFNUMBER,p_objvar_14_value VAOFSTRING,p_objvar_15_name VAOFSTRING,p_objvar_15_type VAOFNUMBER,p_objvar_15_value VAOFSTRING,p_objvar_16_name VAOFSTRING,p_objvar_16_type VAOFNUMBER,p_objvar_16_value VAOFSTRING,p_objvar_17_name VAOFSTRING,p_objvar_17_type VAOFNUMBER,p_objvar_17_value VAOFSTRING,p_objvar_18_name VAOFSTRING,p_objvar_18_type VAOFNUMBER,p_objvar_18_value VAOFSTRING,p_objvar_19_name VAOFSTRING,p_objvar_19_type VAOFNUMBER,p_objvar_19_value VAOFSTRING, p_script_list VAOFLONGSTRING, p_object_id VAOFSTRING, p_chunk_size number)
	as
		cache_update VAOFNUMBER;
	begin

	  cache_update := VAOFNUMBER();   -- initialize VARRAY object
	  cache_update.extend(p_chunk_size); -- allocate VARRAY object memory to chunk size

	  -- create cache_update array for use in bulk update
	  FOR i IN 1..p_chunk_size
	  LOOP
		if
			((p_x(i) is not null)
			or (p_y(i) is not null) 
			or (p_z(i) is not null)
			or (p_quaternion_w(i) is not null)
			or (p_quaternion_x(i) is not null)
			or (p_quaternion_y(i) is not null)
			or (p_quaternion_z(i) is not null)
			or (p_contained_by(i) is not null)
			or (p_cache_version(i) is not null))
		then
			cache_update(i) := 0;
		else
			cache_update(i) := NULL;
		end if;
	  END LOOP;

		
	  -- process deletes
	  FORALL i IN 1..p_chunk_size
	  update objects set
	  deleted = p_deleted(i),
	  deleted_date = sysdate,
	  load_with = NULL
	  where object_id = p_object_id(i) and
	  p_deleted(i) <> 0;

	  -- process updates
	  FORALL i IN 1..p_chunk_size
	  update objects set
	  x = nvl(p_x(i),x),
	  y = nvl(p_y(i),y),
	  z = nvl(p_z(i),z),
	  quaternion_w = nvl(p_quaternion_w(i),quaternion_w),
	  quaternion_x = nvl(p_quaternion_x(i),quaternion_x),
	  quaternion_y = nvl(p_quaternion_y(i),quaternion_y),
	  quaternion_z = nvl(p_quaternion_z(i),quaternion_z),
	  node_x = nvl(p_node_x(i),node_x),
	  node_y = nvl(p_node_y(i),node_y),
	  node_z = nvl(p_node_z(i),node_z),
	  object_template_id = nvl(p_object_template_id(i),object_template_id),
	  type_id = nvl(p_type_id(i),type_id),
	  scene_id = nvl(p_scene_id(i),scene_id),
	  controller_type = nvl(p_controller_type(i),controller_type),
	  deleted = nvl(p_deleted(i),deleted),
	  object_name = nvl(p_object_name(i),object_name),
	  volume = nvl(p_volume(i),volume),
	  contained_by = nvl(p_contained_by(i),contained_by),
	  slot_arrangement = nvl(p_slot_arrangement(i),slot_arrangement),
	  player_controlled = nvl(p_player_controlled(i),player_controlled),
	  cache_version = nvl(cache_update(i),cache_version),
	  load_contents = nvl(p_load_contents(i),load_contents),
	  cash_balance = nvl(p_cash_balance(i),cash_balance),
	  bank_balance = nvl(p_bank_balance(i),bank_balance),
	  complexity = nvl(p_complexity(i),complexity),
	  name_string_table = nvl(p_name_string_table(i),name_string_table),
	  name_string_text = nvl(p_name_string_text(i),name_string_text),
      static_item_name = nvl(p_static_item_name(i),static_item_name),
      static_item_version = nvl(p_static_item_version(i),static_item_version),
      conversion_id = nvl(p_conversion_id(i),conversion_id),
	  load_with = nvl(p_load_with(i),load_with),

	  objvar_0_name = nvl(p_objvar_0_name(i), objvar_0_name),
	  objvar_0_type = nvl(p_objvar_0_type(i), objvar_0_type),
	  objvar_0_value= nvl(p_objvar_0_value(i),objvar_0_value),
	  objvar_1_name = nvl(p_objvar_1_name(i), objvar_1_name),
	  objvar_1_type = nvl(p_objvar_1_type(i), objvar_1_type),
	  objvar_1_value= nvl(p_objvar_1_value(i),objvar_1_value),
	  objvar_2_name = nvl(p_objvar_2_name(i), objvar_2_name),
	  objvar_2_type = nvl(p_objvar_2_type(i), objvar_2_type),
	  objvar_2_value= nvl(p_objvar_2_value(i),objvar_2_value),
	  objvar_3_name = nvl(p_objvar_3_name(i), objvar_3_name),
	  objvar_3_type = nvl(p_objvar_3_type(i), objvar_3_type),
	  objvar_3_value= nvl(p_objvar_3_value(i),objvar_3_value),
	  objvar_4_name = nvl(p_objvar_4_name(i), objvar_4_name),
	  objvar_4_type = nvl(p_objvar_4_type(i), objvar_4_type),
	  objvar_4_value= nvl(p_objvar_4_value(i),objvar_4_value),
	  objvar_5_name = nvl(p_objvar_5_name(i), objvar_5_name),
	  objvar_5_type = nvl(p_objvar_5_type(i), objvar_5_type),
	  objvar_5_value= nvl(p_objvar_5_value(i),objvar_5_value),
	  objvar_6_name = nvl(p_objvar_6_name(i), objvar_6_name),
	  objvar_6_type = nvl(p_objvar_6_type(i), objvar_6_type),
	  objvar_6_value= nvl(p_objvar_6_value(i),objvar_6_value),
	  objvar_7_name = nvl(p_objvar_7_name(i), objvar_7_name),
	  objvar_7_type = nvl(p_objvar_7_type(i), objvar_7_type),
	  objvar_7_value= nvl(p_objvar_7_value(i),objvar_7_value),
	  objvar_8_name = nvl(p_objvar_8_name(i), objvar_8_name),
	  objvar_8_type = nvl(p_objvar_8_type(i), objvar_8_type),
	  objvar_8_value= nvl(p_objvar_8_value(i),objvar_8_value),
	  objvar_9_name = nvl(p_objvar_9_name(i), objvar_9_name),
	  objvar_9_type = nvl(p_objvar_9_type(i), objvar_9_type),
	  objvar_9_value= nvl(p_objvar_9_value(i),objvar_9_value),

	  objvar_10_name = nvl(p_objvar_10_name(i), objvar_10_name),
	  objvar_10_type = nvl(p_objvar_10_type(i), objvar_10_type),
	  objvar_10_value= nvl(p_objvar_10_value(i),objvar_10_value),
	  objvar_11_name = nvl(p_objvar_11_name(i), objvar_11_name),
	  objvar_11_type = nvl(p_objvar_11_type(i), objvar_11_type),
	  objvar_11_value= nvl(p_objvar_11_value(i),objvar_11_value),
	  objvar_12_name = nvl(p_objvar_12_name(i), objvar_12_name),
	  objvar_12_type = nvl(p_objvar_12_type(i), objvar_12_type),
	  objvar_12_value= nvl(p_objvar_12_value(i),objvar_12_value),
	  objvar_13_name = nvl(p_objvar_13_name(i), objvar_13_name),
	  objvar_13_type = nvl(p_objvar_13_type(i), objvar_13_type),
	  objvar_13_value= nvl(p_objvar_13_value(i),objvar_13_value),
	  objvar_14_name = nvl(p_objvar_14_name(i), objvar_14_name),
	  objvar_14_type = nvl(p_objvar_14_type(i), objvar_14_type),
	  objvar_14_value= nvl(p_objvar_14_value(i),objvar_14_value),
	  objvar_15_name = nvl(p_objvar_15_name(i), objvar_15_name),
	  objvar_15_type = nvl(p_objvar_15_type(i), objvar_15_type),
	  objvar_15_value= nvl(p_objvar_15_value(i),objvar_15_value),
	  objvar_16_name = nvl(p_objvar_16_name(i), objvar_16_name),
	  objvar_16_type = nvl(p_objvar_16_type(i), objvar_16_type),
	  objvar_16_value= nvl(p_objvar_16_value(i),objvar_16_value),
	  objvar_17_name = nvl(p_objvar_17_name(i), objvar_17_name),
	  objvar_17_type = nvl(p_objvar_17_type(i), objvar_17_type),
	  objvar_17_value= nvl(p_objvar_17_value(i),objvar_17_value),
	  objvar_18_name = nvl(p_objvar_18_name(i), objvar_18_name),
	  objvar_18_type = nvl(p_objvar_18_type(i), objvar_18_type),
	  objvar_18_value= nvl(p_objvar_18_value(i),objvar_18_value),
	  objvar_19_name = nvl(p_objvar_19_name(i), objvar_19_name),
	  objvar_19_type = nvl(p_objvar_19_type(i), objvar_19_type),
	  objvar_19_value= nvl(p_objvar_19_value(i),objvar_19_value),

	  script_list = nvl(p_script_list(i),script_list)
	  where
	  object_id=p_object_id(i) and
	  (p_deleted(i) = 0 or p_deleted(i) is null);

	  cache_update.delete; -- cleanup - delete VARRAY list 

	end;

	procedure add_object ( p_x VAOFNUMBER, p_y VAOFNUMBER, p_z VAOFNUMBER, p_quaternion_w VAOFNUMBER, p_quaternion_x VAOFNUMBER, p_quaternion_y VAOFNUMBER, p_quaternion_z VAOFNUMBER, p_node_x VAOFNUMBER, p_node_y VAOFNUMBER, p_node_z VAOFNUMBER, p_object_template_id VAOFNUMBER, p_type_id VAOFNUMBER, p_scene_id VAOFSTRING, p_controller_type VAOFNUMBER, p_deleted VAOFNUMBER, p_object_name VAOFSTRING, p_volume VAOFNUMBER, p_contained_by VAOFSTRING, p_slot_arrangement VAOFNUMBER, p_player_controlled VAOFSTRING, p_cache_version VAOFNUMBER, p_load_contents VAOFSTRING, p_cash_balance VAOFNUMBER, p_bank_balance VAOFNUMBER, p_complexity VAOFNUMBER, p_name_string_table VAOFSTRING, p_name_string_text VAOFSTRING, p_static_item_name VAOFSTRING, p_static_item_version VAOFNUMBER, p_conversion_id VAOFNUMBER, p_load_with VAOFSTRING, 
		p_objvar_0_name VAOFSTRING,p_objvar_0_type VAOFNUMBER,p_objvar_0_value VAOFSTRING,p_objvar_1_name VAOFSTRING,p_objvar_1_type VAOFNUMBER,p_objvar_1_value VAOFSTRING,p_objvar_2_name VAOFSTRING,p_objvar_2_type VAOFNUMBER,p_objvar_2_value VAOFSTRING,p_objvar_3_name VAOFSTRING,p_objvar_3_type VAOFNUMBER,p_objvar_3_value VAOFSTRING,p_objvar_4_name VAOFSTRING,p_objvar_4_type VAOFNUMBER,p_objvar_4_value VAOFSTRING,p_objvar_5_name VAOFSTRING,p_objvar_5_type VAOFNUMBER,p_objvar_5_value VAOFSTRING,p_objvar_6_name VAOFSTRING,p_objvar_6_type VAOFNUMBER,p_objvar_6_value VAOFSTRING,p_objvar_7_name VAOFSTRING,p_objvar_7_type VAOFNUMBER,p_objvar_7_value VAOFSTRING,p_objvar_8_name VAOFSTRING,p_objvar_8_type VAOFNUMBER,p_objvar_8_value VAOFSTRING,p_objvar_9_name VAOFSTRING,p_objvar_9_type VAOFNUMBER,p_objvar_9_value VAOFSTRING, p_objvar_10_name VAOFSTRING,p_objvar_10_type VAOFNUMBER,p_objvar_10_value VAOFSTRING,p_objvar_11_name VAOFSTRING,p_objvar_11_type VAOFNUMBER,p_objvar_11_value VAOFSTRING,p_objvar_12_name VAOFSTRING,p_objvar_12_type VAOFNUMBER,p_objvar_12_value VAOFSTRING,p_objvar_13_name VAOFSTRING,p_objvar_13_type VAOFNUMBER,p_objvar_13_value VAOFSTRING,p_objvar_14_name VAOFSTRING,p_objvar_14_type VAOFNUMBER,p_objvar_14_value VAOFSTRING,p_objvar_15_name VAOFSTRING,p_objvar_15_type VAOFNUMBER,p_objvar_15_value VAOFSTRING,p_objvar_16_name VAOFSTRING,p_objvar_16_type VAOFNUMBER,p_objvar_16_value VAOFSTRING,p_objvar_17_name VAOFSTRING,p_objvar_17_type VAOFNUMBER,p_objvar_17_value VAOFSTRING,p_objvar_18_name VAOFSTRING,p_objvar_18_type VAOFNUMBER,p_objvar_18_value VAOFSTRING,p_objvar_19_name VAOFSTRING,p_objvar_19_type VAOFNUMBER,p_objvar_19_value VAOFSTRING, p_script_list VAOFLONGSTRING, p_object_id VAOFSTRING, p_chunk_size number)
	as

 	p_cache_update NUMBER;

	begin
	    FORALL i IN 1..p_chunk_size
		insert into objects
		(
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
            static_item_version,
            conversion_id, 
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

			object_id
		)
		values
		(
			p_x(i),
			p_y(i),
			p_z(i),
			p_quaternion_w(i),
			p_quaternion_x(i),
			p_quaternion_y(i),
			p_quaternion_z(i),
			p_node_x(i),
			p_node_y(i),
			p_node_z(i),
			p_object_template_id(i),
			p_type_id(i),
			p_scene_id(i),
			p_controller_type(i),
			p_deleted(i),
			p_object_name(i),
			p_volume(i),
			p_contained_by(i),
			p_slot_arrangement(i),
			p_player_controlled(i),
			0,
			p_load_contents(i),
			p_cash_balance(i),
			p_bank_balance(i),
			p_complexity(i),
			p_name_string_table(i),
			p_name_string_text(i),
            p_static_item_name(i),
            p_static_item_version(i),
            p_conversion_id(i),
			p_load_with(i),

			p_objvar_0_name(i),
			p_objvar_0_type(i),
			p_objvar_0_value(i),
			p_objvar_1_name(i),
			p_objvar_1_type(i),
			p_objvar_1_value(i),
			p_objvar_2_name(i),
			p_objvar_2_type(i),
			p_objvar_2_value(i),
			p_objvar_3_name(i),
			p_objvar_3_type(i),
			p_objvar_3_value(i),
			p_objvar_4_name(i),
			p_objvar_4_type(i),
			p_objvar_4_value(i),
			p_objvar_5_name(i),
			p_objvar_5_type(i),
			p_objvar_5_value(i),
			p_objvar_6_name(i),
			p_objvar_6_type(i),
			p_objvar_6_value(i),
			p_objvar_7_name(i),
			p_objvar_7_type(i),
			p_objvar_7_value(i),
			p_objvar_8_name(i),
			p_objvar_8_type(i),
			p_objvar_8_value(i),
			p_objvar_9_name(i),
			p_objvar_9_type(i),
			p_objvar_9_value(i),

			p_objvar_10_name(i),
			p_objvar_10_type(i),
			p_objvar_10_value(i),
			p_objvar_11_name(i),
			p_objvar_11_type(i),
			p_objvar_11_value(i),
			p_objvar_12_name(i),
			p_objvar_12_type(i),
			p_objvar_12_value(i),
			p_objvar_13_name(i),
			p_objvar_13_type(i),
			p_objvar_13_value(i),
			p_objvar_14_name(i),
			p_objvar_14_type(i),
			p_objvar_14_value(i),
			p_objvar_15_name(i),
			p_objvar_15_type(i),
			p_objvar_15_value(i),
			p_objvar_16_name(i),
			p_objvar_16_type(i),
			p_objvar_16_value(i),
			p_objvar_17_name(i),
			p_objvar_17_type(i),
			p_objvar_17_value(i),
			p_objvar_18_name(i),
			p_objvar_18_type(i),
			p_objvar_18_value(i),
			p_objvar_19_name(i),
			p_objvar_19_type(i),
			p_objvar_19_value(i),

			p_script_list(i),

			p_object_id(i)
		);

		exception when dup_val_on_index then
			for i in 1..p_chunk_size loop

				if
    			 ((p_x(i) is not null)
			     or (p_y(i) is not null)
			     or (p_z(i) is not null)
			     or (p_quaternion_w(i) is not null)
			     or (p_quaternion_x(i) is not null)
			     or (p_quaternion_y(i) is not null)
			     or (p_quaternion_z(i) is not null)
			     or (p_contained_by(i) is not null)
			     or (p_cache_version(i) is not null))
	      then
			     p_cache_update := 0;
	      else
			     p_cache_update := NULL;
	      end if;

				UPDATE OBJECTS Set
					x = nvl(p_x(i),x),
	  			y = nvl(p_y(i),y),
	  			z = nvl(p_z(i),z),
	  			quaternion_w = nvl(p_quaternion_w(i),quaternion_w),
	  			quaternion_x = nvl(p_quaternion_x(i),quaternion_x),
	  			quaternion_y = nvl(p_quaternion_y(i),quaternion_y),
	  			quaternion_z = nvl(p_quaternion_z(i),quaternion_z),
	  			node_x = nvl(p_node_x(i),node_x),
	  			node_y = nvl(p_node_y(i),node_y),
	  			node_z = nvl(p_node_z(i),node_z),
	  			object_template_id = nvl(p_object_template_id(i),object_template_id),
	  			type_id = nvl(p_type_id(i),type_id),
	  			scene_id = nvl(p_scene_id(i),scene_id),
	  			controller_type = nvl(p_controller_type(i),controller_type),
	  			deleted = nvl(p_deleted(i),deleted),
	  			object_name = nvl(p_object_name(i),object_name),
	  			volume = nvl(p_volume(i),volume),
	  			contained_by = nvl(p_contained_by(i),contained_by),
	  			slot_arrangement = nvl(p_slot_arrangement(i),slot_arrangement),
	  			player_controlled = nvl(p_player_controlled(i),player_controlled),
	  			cache_version = nvl(p_cache_update,cache_version),
	  			load_contents = nvl(p_load_contents(i),load_contents),
	  			cash_balance = nvl(p_cash_balance(i),cash_balance),
	  			bank_balance = nvl(p_bank_balance(i),bank_balance),
	  			complexity = nvl(p_complexity(i),complexity),
	  			name_string_table = nvl(p_name_string_table(i),name_string_table),
	  			name_string_text = nvl(p_name_string_text(i),name_string_text),
                static_item_name = nvl(p_static_item_name(i),static_item_name),
                static_item_version = nvl(p_static_item_version(i),static_item_version),
                conversion_id = nvl(p_conversion_id(i),conversion_id),
	  			load_with = nvl(p_load_with(i),load_with),

	  			objvar_0_name = nvl(p_objvar_0_name(i), objvar_0_name),
	  			objvar_0_type = nvl(p_objvar_0_type(i), objvar_0_type),
	  			objvar_0_value= nvl(p_objvar_0_value(i),objvar_0_value),
	  			objvar_1_name = nvl(p_objvar_1_name(i), objvar_1_name),
	  			objvar_1_type = nvl(p_objvar_1_type(i), objvar_1_type),
	  			objvar_1_value= nvl(p_objvar_1_value(i),objvar_1_value),
	  			objvar_2_name = nvl(p_objvar_2_name(i), objvar_2_name),
	  			objvar_2_type = nvl(p_objvar_2_type(i), objvar_2_type),
	  			objvar_2_value= nvl(p_objvar_2_value(i),objvar_2_value),
	  			objvar_3_name = nvl(p_objvar_3_name(i), objvar_3_name),
	  			objvar_3_type = nvl(p_objvar_3_type(i), objvar_3_type),
	  			objvar_3_value= nvl(p_objvar_3_value(i),objvar_3_value),
	  			objvar_4_name = nvl(p_objvar_4_name(i), objvar_4_name),
	  			objvar_4_type = nvl(p_objvar_4_type(i), objvar_4_type),
	  			objvar_4_value= nvl(p_objvar_4_value(i),objvar_4_value),
	  			objvar_5_name = nvl(p_objvar_5_name(i), objvar_5_name),
	  			objvar_5_type = nvl(p_objvar_5_type(i), objvar_5_type),
	  			objvar_5_value= nvl(p_objvar_5_value(i),objvar_5_value),
	  			objvar_6_name = nvl(p_objvar_6_name(i), objvar_6_name),
	  			objvar_6_type = nvl(p_objvar_6_type(i), objvar_6_type),
	  			objvar_6_value= nvl(p_objvar_6_value(i),objvar_6_value),
	  			objvar_7_name = nvl(p_objvar_7_name(i), objvar_7_name),
	  			objvar_7_type = nvl(p_objvar_7_type(i), objvar_7_type),
	  			objvar_7_value= nvl(p_objvar_7_value(i),objvar_7_value),
	  			objvar_8_name = nvl(p_objvar_8_name(i), objvar_8_name),
	  			objvar_8_type = nvl(p_objvar_8_type(i), objvar_8_type),
	  			objvar_8_value= nvl(p_objvar_8_value(i),objvar_8_value),
	  			objvar_9_name = nvl(p_objvar_9_name(i), objvar_9_name),
	  			objvar_9_type = nvl(p_objvar_9_type(i), objvar_9_type),
	  			objvar_9_value= nvl(p_objvar_9_value(i),objvar_9_value),

				objvar_10_name = nvl(p_objvar_10_name(i), objvar_10_name),
	  			objvar_10_type = nvl(p_objvar_10_type(i), objvar_10_type),
	  			objvar_10_value= nvl(p_objvar_10_value(i),objvar_10_value),
	  			objvar_11_name = nvl(p_objvar_11_name(i), objvar_11_name),
	  			objvar_11_type = nvl(p_objvar_11_type(i), objvar_11_type),
	  			objvar_11_value= nvl(p_objvar_11_value(i),objvar_11_value),
	  			objvar_12_name = nvl(p_objvar_12_name(i), objvar_12_name),
	  			objvar_12_type = nvl(p_objvar_12_type(i), objvar_12_type),
	  			objvar_12_value= nvl(p_objvar_12_value(i),objvar_12_value),
	  			objvar_13_name = nvl(p_objvar_13_name(i), objvar_13_name),
	  			objvar_13_type = nvl(p_objvar_13_type(i), objvar_13_type),
	  			objvar_13_value= nvl(p_objvar_13_value(i),objvar_13_value),
	  			objvar_14_name = nvl(p_objvar_14_name(i), objvar_14_name),
	  			objvar_14_type = nvl(p_objvar_14_type(i), objvar_14_type),
	  			objvar_14_value= nvl(p_objvar_14_value(i),objvar_14_value),
	  			objvar_15_name = nvl(p_objvar_15_name(i), objvar_15_name),
	  			objvar_15_type = nvl(p_objvar_15_type(i), objvar_15_type),
	  			objvar_15_value= nvl(p_objvar_15_value(i),objvar_15_value),
	  			objvar_16_name = nvl(p_objvar_16_name(i), objvar_16_name),
	  			objvar_16_type = nvl(p_objvar_16_type(i), objvar_16_type),
	  			objvar_16_value= nvl(p_objvar_16_value(i),objvar_16_value),
	  			objvar_17_name = nvl(p_objvar_17_name(i), objvar_17_name),
	  			objvar_17_type = nvl(p_objvar_17_type(i), objvar_17_type),
	  			objvar_17_value= nvl(p_objvar_17_value(i),objvar_17_value),
	  			objvar_18_name = nvl(p_objvar_18_name(i), objvar_18_name),
	  			objvar_18_type = nvl(p_objvar_18_type(i), objvar_18_type),
	  			objvar_18_value= nvl(p_objvar_18_value(i),objvar_18_value),
	  			objvar_19_name = nvl(p_objvar_19_name(i), objvar_19_name),
	  			objvar_19_type = nvl(p_objvar_19_type(i), objvar_19_type),
	  			objvar_19_value= nvl(p_objvar_19_value(i),objvar_19_value),

				script_list = nvl(p_script_list(i), script_list)
	
				WHERE
					object_id=p_object_id(i) and
	  				p_deleted(i) = 0;

				if SQL%ROWCOUNT=0 then
				insert into objects
				(
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
                    static_item_version,
                    conversion_id, 
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

					object_id
				)
				values
				(
					p_x(i),
					p_y(i),
					p_z(i),
					p_quaternion_w(i),
					p_quaternion_x(i),
					p_quaternion_y(i),
					p_quaternion_z(i),
					p_node_x(i),
					p_node_y(i),
					p_node_z(i),
					p_object_template_id(i),
					p_type_id(i),
					p_scene_id(i),
					p_controller_type(i),
					p_deleted(i),
					p_object_name(i),
					p_volume(i),
					p_contained_by(i),
					p_slot_arrangement(i),
					p_player_controlled(i),
					0,
					p_load_contents(i),
					p_cash_balance(i),
					p_bank_balance(i),
					p_complexity(i),
					p_name_string_table(i),
					p_name_string_text(i),
                    p_static_item_name(i),
                    p_static_item_version(i),
                    p_conversion_id(i),
					p_load_with(i),

					p_objvar_0_name(i),
					p_objvar_0_type(i),
					p_objvar_0_value(i),
					p_objvar_1_name(i),
					p_objvar_1_type(i),
					p_objvar_1_value(i),
					p_objvar_2_name(i),
					p_objvar_2_type(i),
					p_objvar_2_value(i),
					p_objvar_3_name(i),
					p_objvar_3_type(i),
					p_objvar_3_value(i),
					p_objvar_4_name(i),
					p_objvar_4_type(i),
					p_objvar_4_value(i),
					p_objvar_5_name(i),
					p_objvar_5_type(i),
					p_objvar_5_value(i),
					p_objvar_6_name(i),
					p_objvar_6_type(i),
					p_objvar_6_value(i),
					p_objvar_7_name(i),
					p_objvar_7_type(i),
					p_objvar_7_value(i),
					p_objvar_8_name(i),
					p_objvar_8_type(i),
					p_objvar_8_value(i),
					p_objvar_9_name(i),
					p_objvar_9_type(i),
					p_objvar_9_value(i),

					p_objvar_10_name(i),
					p_objvar_10_type(i),
					p_objvar_10_value(i),
					p_objvar_11_name(i),
					p_objvar_11_type(i),
					p_objvar_11_value(i),
					p_objvar_12_name(i),
					p_objvar_12_type(i),
					p_objvar_12_value(i),
					p_objvar_13_name(i),
					p_objvar_13_type(i),
					p_objvar_13_value(i),
					p_objvar_14_name(i),
					p_objvar_14_type(i),
					p_objvar_14_value(i),
					p_objvar_15_name(i),
					p_objvar_15_type(i),
					p_objvar_15_value(i),
					p_objvar_16_name(i),
					p_objvar_16_type(i),
					p_objvar_16_value(i),
					p_objvar_17_name(i),
					p_objvar_17_type(i),
					p_objvar_17_value(i),
					p_objvar_18_name(i),
					p_objvar_18_type(i),
					p_objvar_18_value(i),
					p_objvar_19_name(i),
					p_objvar_19_type(i),
					p_objvar_19_value(i),

					p_script_list(i),

					p_object_id(i)
				);

    		end if;
			end loop;
	end;

	procedure remove_object ( p_object_id VAOFSTRING, p_chunk_size number)
	as
		begin

		FORALL i IN 1..p_chunk_size
		delete from objects
		where object_id = p_object_id(i);

	end;

	procedure save_manf_schem_attribute
	(
		p_action number,
		p_object_id objectid,
		p_attribute_type varchar2,
		p_value number
	)
	as
	begin
		if (p_action = 0) then
			delete manf_schematic_attributes
			where object_id = p_object_id
			and attribute_type = p_attribute_type;
		else
			update manf_schematic_attributes
			set value = p_value
			where
				object_id = p_object_id
				and attribute_type = p_attribute_type;
		
			if (sql%rowcount = 0) then
				insert into manf_schematic_attributes (object_id, attribute_type, value)
				values (p_object_id, p_attribute_type, p_value);
			end if;
		end if;
	end;

	procedure update_property_list
	(
		p_object_id objectid,
		p_list_id number,
		p_operation number,
		p_value varchar2
	)
	as
	begin
		-- Operations:
		--  0=ERASE
		--  1=INSERT
		--  2=CLEAR

		IF p_operation = 0 THEN
			delete from property_lists
			where object_id = p_object_id
			and list_id = p_list_id
			and value = p_value;

		ELSIF p_operation = 1 THEN
			insert into property_lists (object_id, list_id, value)
			values (p_object_id, p_list_id, p_value);

		ELSIF p_operation = 2 THEN
			delete from property_lists
			where object_id = p_object_id
			and list_id = p_list_id;

		ELSE
			raise_application_error(-20000,'Invalid operation code passed to update_command.');

		END IF;

		exception when dup_val_on_index then
			null;
	end;

	procedure update_property_list_batch (p_object_id VAOFSTRING, p_list_id VAOFNUMBER, p_operation number, p_value VAOFSTRING, p_chunk_size number, p_enable_db_logging number )
	as
	begin
		-- Operations:
		--  0=ERASE
		--  1=INSERT
		--  2=CLEAR

	if (p_operation = 0) THEN
		FORALL i IN 1.. p_chunk_size
		delete from property_lists
			where object_id = p_object_id(i)
			and list_id = p_list_id(i)
			and value = p_value(i);

	ELSIF (p_operation = 1) THEN
	FORALL i IN 1.. p_chunk_size
		insert into property_lists (object_id, list_id, value)
			values (p_object_id(i), p_list_id(i), p_value(i));

	ELSIF (p_operation = 2) THEN
	FORALL i IN 1.. p_chunk_size
			delete from property_lists
			where object_id = p_object_id(i)
			and list_id = p_list_id(i);

	ELSE
		raise_application_error(-20000,'Invalid operation code passed to update_command.');

	END IF;

		exception when dup_val_on_index then
				for i in 1..p_chunk_size loop
					begin
						insert into property_lists
						(
			  			property_lists.list_id,
						property_lists.value,
						property_lists.object_id
						)
						VALUES
						(
						p_list_id(i),
						p_value(i),
						p_object_id(i)
						);
					exception when dup_val_on_index then
						if ( p_enable_db_logging > 0 ) then
					
							db_error_logger.dblogerror( SQLCODE, 'procedure update_property_list_batch: duplicate rows were sent to the property list buffer that resulted in a dup_val_on_index error.');
							if ( p_enable_db_logging > 1 ) then
						
								db_error_logger.dblogerror_values( 'property_lists', 'object_id' , 'number', p_object_id(i));
								db_error_logger.dblogerror_values( 'property_lists', 'list_id' , 'number', to_char(p_list_id(i)));
								db_error_logger.dblogerror_values( 'property_lists', 'value' , 'varchar2', p_value(i));

							end if;
					
						else
							NULL;
						end if;
					end;
				end loop;
	end;


	procedure save_armor
	(
		p_object_template varchar2,
		p_effectiveness number, 
		p_integrity number,
		p_special_protections varchar2,
		p_encumberance_0 number,
		p_encumberance_1 number,
		p_encumberance_2 number,
		p_encumberance_3 number,
		p_encumberance_4 number,
		p_encumberance_5 number,
		p_encumberance_6 number,
		p_encumberance_7 number,
		p_encumberance_8 number,
		p_layer number,
		p_object_id objectid
	)
	as
	begin
		-- Note: armor always updates all properties
		update armor
		set
			object_template = p_object_template,
			effectiveness = p_effectiveness,
			integrity = p_integrity,
			special_protections = p_special_protections,
			encumberance_0 = p_encumberance_0,
			encumberance_1 = p_encumberance_1,
			encumberance_2 = p_encumberance_2,
			encumberance_3 = p_encumberance_3,
			encumberance_4 = p_encumberance_4,
			encumberance_5 = p_encumberance_5,
			encumberance_6 = p_encumberance_6,
			encumberance_7 = p_encumberance_7,
			encumberance_8 = p_encumberance_8
		where
			layer = p_layer
			and object_id = p_object_id;
	end;

	procedure add_armor
	(
		p_object_template varchar2,
		p_effectiveness number, 
		p_integrity number,
		p_special_protections varchar2,
		p_encumberance_0 number,
		p_encumberance_1 number,
		p_encumberance_2 number,
		p_encumberance_3 number,
		p_encumberance_4 number,
		p_encumberance_5 number,
		p_encumberance_6 number,
		p_encumberance_7 number,
		p_encumberance_8 number,
		p_layer number,
		p_object_id objectid
	)
	as
	begin
		insert into armor
		(
			object_template,
			effectiveness, 
			integrity,
			special_protections,
			encumberance_0,
			encumberance_1,
			encumberance_2,

			encumberance_3,
			encumberance_4,
			encumberance_5,
			encumberance_6,
			encumberance_7,
			encumberance_8,
			layer,
			object_id
		)
		values
		(
			p_object_template,
			p_effectiveness, 
			p_integrity,
			p_special_protections,
			p_encumberance_0,
			p_encumberance_1,
			p_encumberance_2,
			p_encumberance_3,
			p_encumberance_4,
			p_encumberance_5,
			p_encumberance_6,
			p_encumberance_7,
			p_encumberance_8,
			p_layer,
			p_object_id
		);

		exception when dup_val_on_index then
			null;
	end;

	procedure save_location
	(
		p_object_id number,
		p_list_id number,
		p_sequence_number number,
		p_operation number,
		p_name varchar2,
		p_scene varchar2,
		p_x number,
		p_y number,
		p_z number,
		p_radius number
	)
	as
	begin
		-- Operations:
		--  0=INSERT

		--  1=DELETE
		--  2=SET

		IF p_operation = 0 THEN
			insert into location_lists (object_id, list_id, sequence_number, name, scene, x, y, z, radius)
			select p_object_id, p_list_id, p_sequence_number, p_name, p_scene, p_x, p_y, p_z, p_radius
			from dual
			where not exists (select * from location_lists
			where object_id = p_object_id
			and list_id = p_list_id
			and sequence_number = p_sequence_number);

		ELSIF p_operation = 1 THEN
			delete from location_lists
			where object_id = p_object_id
			and list_id = p_list_id
			and sequence_number = p_sequence_number;

			update location_lists
			set sequence_number = sequence_number - 1
			where object_id = p_object_id

			and list_id = p_list_id
			and sequence_number > p_sequence_number;

		ELSIF p_operation = 2 THEN
			update location_lists
			set
				name = p_name,
				scene = p_scene,
				x = p_x,
				y = p_y,
				z = p_z,
				radius = p_radius
			where object_id = p_object_id
			and list_id = p_list_id
			and sequence_number = p_sequence_number;

		ELSE
			raise_application_error(-20000,'Invalid operation code passed to update_skill.');

		END IF;
	end;

	procedure save_experience ( p_points number, p_experience_type varchar2, p_object_id number )
	as
	begin
		update experience_points
		set points = p_points
		where object_id = p_object_id and experience_type = p_experience_type;

		if sql%rowcount = 0 then
			insert into experience_points (object_id, experience_type, points)
			values (p_object_id, p_experience_type, p_points);
		end if;
	end;

	procedure save_battlefield_participants ( p_faction_id number, p_character_object_id number, p_region_object_id number )
	as
	begin
		if (p_faction_id <> 0) then
			update battlefield_participants
			set faction_id = p_faction_id
			where region_object_id = p_region_object_id and character_object_id = p_character_object_id;

			if sql%rowcount = 0 then
				insert into battlefield_participants (region_object_id, character_object_id, faction_id)
				values (p_region_object_id, p_character_object_id, p_faction_id);
			end if;
		else
			delete battlefield_participants
			where region_object_id = p_region_object_id and character_object_id = p_character_object_id;
		end if;
	end;

	function rename_character( p_character_id objectid, p_new_name varchar2, p_normalized_name varchar2) return number
	as		
		l_existing_object_with_name objectid;
	begin
		select character_object
		into l_existing_object_with_name
		from players
		where uc_character_name = p_normalized_name;

		if (l_existing_object_with_name = p_character_id) then
		update objects
		set object_name = p_new_name
			where object_id = p_character_id;

		if (sql%rowcount=1) then
			update players
			set uc_character_name = p_normalized_name, character_full_name = p_new_name
			where character_object = p_character_id;

			return 1;
		else
			return 0;
		end if;
		else
			return 0;
		end if;

		exception
			when no_data_found then
				update objects
				set object_name = p_new_name
				where object_id = p_character_id;

				if (sql%rowcount=1) then
					update players
					set uc_character_name = p_normalized_name, character_full_name = p_new_name
					where character_object = p_character_id;

					return 1;
				else
					return 0;
				end if;

			when others then
				return 0;
	end;

	procedure save_waypoint
	(
		p_object_id objectid,
		p_waypoint_id objectid,
		p_appearance_name_crc number,
		p_location_x number,
		p_location_y number,
		p_location_z number,
		p_location_cell number,
		p_location_scene number,
		p_name varchar2,
		p_color number,
		p_active char,
		p_detached char
	)
	as
	begin
		if p_detached='N' then

			update waypoints set
				appearance_name_crc = p_appearance_name_crc,
				location_x = p_location_x,
				location_y = p_location_y,
				location_z = p_location_z,
				location_cell = p_location_cell,
				location_scene = p_location_scene,
				name = p_name,
				color = p_color,
				active = p_active

			where
				waypoint_id = p_waypoint_id;

			if (sql%rowcount=0) then
				insert into waypoints
				(
					object_id,
					waypoint_id,
					appearance_name_crc,
					location_x,
					location_y,
					location_z,

					location_cell,
					location_scene,
					name,
					color,
					active
				)
				values
				(
					p_object_id,
					p_waypoint_id,
					p_appearance_name_crc,
					p_location_x,
					p_location_y,
					p_location_z,
					p_location_cell,
					p_location_scene,
					p_name,
					p_color,
					p_active
				);
			end if;

		else -- p_detached <> 'N'

			delete waypoints
			where waypoint_id = p_waypoint_id;

		end if;
	EXCEPTION
	    WHEN OTHERS THEN
	     BEGIN

	      Db_Error_Logger.dblogerror( SQLCODE, 'procedure save_waypoint: attempting to find numeric overflow error.');

		Db_Error_Logger.dblogerror_values( 'waypoints', 'object_id' , 'number', p_object_id);
		Db_Error_Logger.dblogerror_values( 'waypoints', 'waypoint_id' , 'number', p_waypoint_id);
		Db_Error_Logger.dblogerror_values( 'waypoints', 'appearance_name_crc' , 'number', TO_CHAR(p_appearance_name_crc));
		Db_Error_Logger.dblogerror_values( 'waypoints', 'location_x' , 'number', TO_CHAR(p_location_x));
		Db_Error_Logger.dblogerror_values( 'waypoints', 'location_y' , 'number', TO_CHAR(p_location_y));
		Db_Error_Logger.dblogerror_values( 'waypoints', 'location_z' , 'number', TO_CHAR(p_location_z));
		Db_Error_Logger.dblogerror_values( 'waypoints', 'location_cell' , 'number', TO_CHAR(p_location_cell));
		Db_Error_Logger.dblogerror_values( 'waypoints', 'location_scene' , 'number', TO_CHAR(p_location_scene));
        	Db_Error_Logger.dblogerror_values( 'waypoints', 'name' , 'varchar2', p_name);
		Db_Error_Logger.dblogerror_values( 'waypoints', 'color' , 'char', p_color);
		Db_Error_Logger.dblogerror_values( 'waypoints', 'active' , 'char', p_active);					
             EXCEPTION
               WHEN OTHERS THEN
                  NULL;	
	     END;				
	end;

	procedure save_player_obj( p_object_id VAOFSTRING, p_station_id VAOFNUMBER, p_house_id VAOFSTRING, p_account_num_lots VAOFNUMBER, p_account_is_outcast VAOFSTRING, p_account_cheater_level VAOFNUMBER, p_account_max_lots_adjustment VAOFNUMBER, p_personal_profile_id VAOFSTRING, p_character_profile_id VAOFSTRING, p_skill_title VAOFSTRING, p_born_date VAOFNUMBER, p_played_time VAOFNUMBER, p_force_regen_rate VAOFNUMBER, p_force_power VAOFNUMBER, p_max_force_power VAOFNUMBER, p_active_quests VAOFSTRING, p_completed_quests VAOFSTRING, p_current_quest VAOFNUMBER, p_quests VAOFLONGSTRING, p_role_icon_choice VAOFNUMBER, p_quests2 VAOFLONGSTRING, p_quests3 VAOFLONGSTRING, p_quests4 VAOFLONGSTRING, p_skill_template VAOFSTRING, p_working_skill VAOFSTRING, p_current_gcw_points VAOFNUMBER, p_current_gcw_rating VAOFNUMBER, p_current_pvp_kills VAOFNUMBER, p_lifetime_gcw_points VAOFNUMBER, p_max_gcw_imperial_rating VAOFNUMBER, p_max_gcw_rebel_rating VAOFNUMBER, p_lifetime_pvp_kills VAOFNUMBER, p_next_gcw_rating_calc_time VAOFNUMBER, p_collections VAOFLONGSTRING, p_show_backpack VAOFSTRING, p_show_helmet VAOFSTRING, p_collections2 VAOFLONGSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		update player_objects set
			player_objects.station_id = nvl(p_station_id(i),player_objects.station_id),
			player_objects.personal_profile_id = nvl(p_personal_profile_id(i),player_objects.personal_profile_id),
			player_objects.character_profile_id = nvl(p_character_profile_id(i),player_objects.character_profile_id),
			player_objects.skill_title = nvl(p_skill_title(i),player_objects.skill_title),
			player_objects.born_date = nvl(p_born_date(i),player_objects.born_date),
			player_objects.played_time = nvl(p_played_time(i),player_objects.played_time),
			player_objects.force_regen_rate = nvl(p_force_regen_rate(i),player_objects.force_regen_rate),
			player_objects.force_power = nvl(p_force_power(i),player_objects.force_power),
			player_objects.max_force_power = nvl(p_max_force_power(i),player_objects.max_force_power),
			player_objects.active_quests = nvl(p_active_quests(i),player_objects.active_quests),
			player_objects.completed_quests = nvl(p_completed_quests(i),player_objects.completed_quests),
			player_objects.current_quest = nvl(p_current_quest(i),player_objects.current_quest),
			player_objects.quests = nvl(p_quests(i),player_objects.quests),
			player_objects.role_icon_choice = nvl(p_role_icon_choice(i),player_objects.role_icon_choice),
			player_objects.quests2 = nvl(p_quests2(i),player_objects.quests2),
			player_objects.quests3 = nvl(p_quests3(i),player_objects.quests3),
			player_objects.quests4 = nvl(p_quests4(i),player_objects.quests4),
			player_objects.skill_template = nvl(p_skill_template(i),player_objects.skill_template),
			player_objects.working_skill = nvl(p_working_skill(i),player_objects.working_skill),
			player_objects.current_gcw_points = nvl(p_current_gcw_points(i),player_objects.current_gcw_points),
			player_objects.current_gcw_rating = nvl(p_current_gcw_rating(i),player_objects.current_gcw_rating),
			player_objects.current_pvp_kills = nvl(p_current_pvp_kills(i),player_objects.current_pvp_kills),
			player_objects.lifetime_gcw_points = nvl(p_lifetime_gcw_points(i),player_objects.lifetime_gcw_points),
			player_objects.max_gcw_imperial_rating = nvl(p_max_gcw_imperial_rating(i),player_objects.max_gcw_imperial_rating),
			player_objects.max_gcw_rebel_rating = nvl(p_max_gcw_rebel_rating(i),player_objects.max_gcw_rebel_rating),
			player_objects.lifetime_pvp_kills = nvl(p_lifetime_pvp_kills(i),player_objects.lifetime_pvp_kills),
			player_objects.next_gcw_rating_calc_time = nvl(p_next_gcw_rating_calc_time(i),player_objects.next_gcw_rating_calc_time),
			player_objects.collections = nvl(p_collections(i),player_objects.collections),
			player_objects.show_backpack = nvl(p_show_backpack(i),player_objects.show_backpack),
			player_objects.show_helmet = nvl(p_show_helmet(i),player_objects.show_helmet),
			player_objects.collections2 = nvl(p_collections2(i),player_objects.collections2),
			num_lots = nvl(p_account_num_lots(i), num_lots)
		where
			player_objects.object_id=p_object_id(i);

		FOR i IN 1.. p_chunk_size
		LOOP
			if (p_station_id(i) is not null) then
				begin
					insert into accounts (station_id, is_outcast, cheater_level)
					values (p_station_id(i), 0,0);

					exception when dup_val_on_index then
					null;
				end;
			end if;
		end LOOP;

		FORALL i IN 1..p_chunk_size
		update accounts
		set
			is_outcast = nvl(p_account_is_outcast(i), is_outcast),
			cheater_level = nvl(p_account_cheater_level(i), cheater_level),
			max_lots_adjustment = nvl(p_account_max_lots_adjustment(i), max_lots_adjustment),
			house_id = nvl(p_house_id(i), house_id)
		where
			station_id = (select station_id from player_objects where object_id = p_object_id(i));

	end;

	procedure add_player_obj( p_object_id VAOFSTRING, p_station_id VAOFNUMBER, p_house_id VAOFSTRING, p_account_num_lots VAOFNUMBER, p_account_is_outcast VAOFSTRING, p_account_cheater_level VAOFNUMBER, p_account_max_lots_adjustment VAOFNUMBER, p_personal_profile_id VAOFSTRING, p_character_profile_id VAOFSTRING, p_skill_title VAOFSTRING, p_born_date VAOFNUMBER, p_played_time VAOFNUMBER, p_force_regen_rate VAOFNUMBER, p_force_power VAOFNUMBER, p_max_force_power VAOFNUMBER, p_active_quests VAOFSTRING, p_completed_quests VAOFSTRING, p_current_quest VAOFNUMBER, p_quests VAOFLONGSTRING, p_role_icon_choice VAOFNUMBER, p_quests2 VAOFLONGSTRING, p_quests3 VAOFLONGSTRING, p_quests4 VAOFLONGSTRING, p_skill_template VAOFSTRING, p_working_skill VAOFSTRING, p_current_gcw_points VAOFNUMBER, p_current_gcw_rating VAOFNUMBER, p_current_pvp_kills VAOFNUMBER, p_lifetime_gcw_points VAOFNUMBER, p_max_gcw_imperial_rating VAOFNUMBER, p_max_gcw_rebel_rating VAOFNUMBER, p_lifetime_pvp_kills VAOFNUMBER, p_next_gcw_rating_calc_time VAOFNUMBER, p_collections VAOFLONGSTRING, p_show_backpack VAOFSTRING, p_show_helmet VAOFSTRING, p_collections2 VAOFLONGSTRING, p_chunk_size number )
	as
	begin

		FORALL i IN 1..p_chunk_size
		insert into player_objects
		(
			player_objects.station_id,
			player_objects.personal_profile_id,
			player_objects.character_profile_id,
			player_objects.skill_title,
			player_objects.born_date,
			player_objects.played_time,
			player_objects.force_regen_rate,
			player_objects.force_power,
			player_objects.max_force_power,
			player_objects.num_lots,
			player_objects.object_id,
			player_objects.active_quests,
			player_objects.completed_quests,
			player_objects.current_quest,
			player_objects.quests,
			player_objects.role_icon_choice,
			player_objects.quests2,
			player_objects.quests3,
			player_objects.quests4,
			player_objects.skill_template,
			player_objects.working_skill,
			player_objects.current_gcw_points,
			player_objects.current_gcw_rating,
			player_objects.current_pvp_kills,
			player_objects.lifetime_gcw_points,
			player_objects.max_gcw_imperial_rating,
			player_objects.max_gcw_rebel_rating,
			player_objects.lifetime_pvp_kills,
			player_objects.next_gcw_rating_calc_time,
			player_objects.collections,
			player_objects.show_backpack,
			player_objects.show_helmet,
			player_objects.collections2
		)
		VALUES
		(
			p_station_id(i),
			p_personal_profile_id(i),
			p_character_profile_id(i),
			p_skill_title(i),
			p_born_date(i),
			p_played_time(i),
			p_force_regen_rate(i),
			p_force_power(i),
			p_max_force_power(i),
			p_account_num_lots(i),
			p_object_id(i),
			p_active_quests(i),
			p_completed_quests(i),
			p_current_quest(i),
			p_quests(i),
			p_role_icon_choice(i),
			p_quests2(i),
			p_quests3(i),
			p_quests4(i),
			p_skill_template(i),
			p_working_skill(i),
			p_current_gcw_points(i),
			p_current_gcw_rating(i),
			p_current_pvp_kills(i),
			p_lifetime_gcw_points(i),
			p_max_gcw_imperial_rating(i),
			p_max_gcw_rebel_rating(i),
			p_lifetime_pvp_kills(i),
			p_next_gcw_rating_calc_time(i),
			p_collections(i),
			p_show_backpack(i),
			p_show_helmet(i),
			p_collections2(i)
		);

		FOR i IN 1.. p_chunk_size
		LOOP
			if (p_station_id(i) is not null) then
				begin
					insert into accounts (station_id, is_outcast, cheater_level, max_lots_adjustment, house_id)
					values (p_station_id(i), p_account_is_outcast(i), p_account_cheater_level(i), p_account_max_lots_adjustment(i), p_house_id(i));

					exception when dup_val_on_index then
						null;
				end;
			end if;
		end LOOP;

		exception when dup_val_on_index then
			for i in 1..p_chunk_size loop
				UPDATE player_objects Set
					player_objects.station_id = nvl(p_station_id(i),player_objects.station_id),
					player_objects.personal_profile_id = nvl(p_personal_profile_id(i),player_objects.personal_profile_id),
					player_objects.character_profile_id = nvl(p_character_profile_id(i),player_objects.character_profile_id),
					player_objects.skill_title = nvl(p_skill_title(i),player_objects.skill_title),
					player_objects.born_date = nvl(p_born_date(i),player_objects.born_date),
					player_objects.played_time = nvl(p_played_time(i),player_objects.played_time),
					player_objects.force_regen_rate = nvl(p_force_regen_rate(i),player_objects.force_regen_rate),
					player_objects.force_power = nvl(p_force_power(i),player_objects.force_power),
					player_objects.max_force_power = nvl(p_max_force_power(i),player_objects.max_force_power),
					player_objects.num_lots = nvl(p_account_num_lots(i), num_lots),
					player_objects.active_quests = nvl(p_active_quests(i),player_objects.active_quests),
					player_objects.completed_quests = nvl(p_completed_quests(i),player_objects.completed_quests),
					player_objects.current_quest = nvl(p_current_quest(i),player_objects.current_quest),
					player_objects.quests = nvl(p_quests(i),player_objects.quests),
					player_objects.role_icon_choice = nvl(p_role_icon_choice(i),player_objects.role_icon_choice),
					player_objects.quests2 = nvl(p_quests(i),player_objects.quests2),
					player_objects.quests3 = nvl(p_quests(i),player_objects.quests3),
					player_objects.quests4 = nvl(p_quests(i),player_objects.quests4),
					player_objects.skill_template = nvl(p_skill_template(i),player_objects.skill_template),
					player_objects.working_skill = nvl(p_working_skill(i),player_objects.working_skill),
					player_objects.current_gcw_points = nvl(p_current_gcw_points(i),player_objects.current_gcw_points),
					player_objects.current_gcw_rating = nvl(p_current_gcw_rating(i),player_objects.current_gcw_rating),
					player_objects.current_pvp_kills = nvl(p_current_pvp_kills(i),player_objects.current_pvp_kills),
					player_objects.lifetime_gcw_points = nvl(p_lifetime_gcw_points(i),player_objects.lifetime_gcw_points),
					player_objects.max_gcw_imperial_rating = nvl(p_max_gcw_imperial_rating(i),player_objects.max_gcw_imperial_rating),
					player_objects.max_gcw_rebel_rating = nvl(p_max_gcw_rebel_rating(i),player_objects.max_gcw_rebel_rating),
					player_objects.lifetime_pvp_kills = nvl(p_lifetime_pvp_kills(i),player_objects.lifetime_pvp_kills),
					player_objects.next_gcw_rating_calc_time = nvl(p_next_gcw_rating_calc_time(i),player_objects.next_gcw_rating_calc_time),
					player_objects.collections = nvl(p_collections(i),player_objects.collections),
					player_objects.show_backpack = nvl(p_show_backpack(i),player_objects.show_backpack),
					player_objects.show_helmet = nvl(p_show_helmet(i),player_objects.show_helmet),
					player_objects.collections2 = nvl(p_collections2(i),player_objects.collections2)
				where
					player_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into player_objects
					(
					player_objects.station_id,
					player_objects.personal_profile_id,
					player_objects.character_profile_id,
					player_objects.skill_title,
					player_objects.born_date,
					player_objects.played_time,
					player_objects.force_regen_rate,
					player_objects.force_power,
					player_objects.max_force_power,
					player_objects.num_lots,
					player_objects.object_id,
					player_objects.active_quests,
					player_objects.completed_quests,
					player_objects.current_quest,
					player_objects.quests,
					player_objects.role_icon_choice,
					player_objects.quests2,
					player_objects.quests3,
					player_objects.quests4,
					player_objects.skill_template,
					player_objects.working_skill,
					player_objects.current_gcw_points,
					player_objects.current_gcw_rating,
					player_objects.current_pvp_kills,
					player_objects.lifetime_gcw_points,
					player_objects.max_gcw_imperial_rating,
					player_objects.max_gcw_rebel_rating,
					player_objects.lifetime_pvp_kills,
					player_objects.next_gcw_rating_calc_time,
					player_objects.collections,
					player_objects.show_backpack,
					player_objects.show_helmet,
					player_objects.collections2
					)
					VALUES
					(
					p_station_id(i),
					p_personal_profile_id(i),
					p_character_profile_id(i),
					p_skill_title(i),
					p_born_date(i),
					p_played_time(i),
					p_force_regen_rate(i),
					p_force_power(i),
					p_max_force_power(i),
					p_account_num_lots(i),
					p_object_id(i),
					p_active_quests(i),
					p_completed_quests(i),
					p_current_quest(i),
					p_quests(i),
					p_role_icon_choice(i),
					p_quests2(i),
					p_quests3(i),
					p_quests4(i),
					p_skill_template(i),
					p_working_skill(i),
					p_current_gcw_points(i),
					p_current_gcw_rating(i),
					p_current_pvp_kills(i),
					p_lifetime_gcw_points(i),
					p_max_gcw_imperial_rating(i),
					p_max_gcw_rebel_rating(i),
					p_lifetime_pvp_kills(i),
					p_next_gcw_rating_calc_time(i),
					p_collections(i),
					p_show_backpack(i),
					p_show_helmet(i),
					p_collections2(i)
					);
				end if;
			end loop;

			FOR i IN 1.. p_chunk_size
			LOOP
				if (p_station_id(i) is not null) then
					begin
						insert into accounts (station_id, is_outcast, cheater_level, max_lots_adjustment, house_id)
						values (p_station_id(i), p_account_is_outcast(i), p_account_cheater_level(i), p_account_max_lots_adjustment(i), p_house_id(i));

						exception when dup_val_on_index then
							null;
					end;
				end if;
			end LOOP;
	end;

	procedure remove_player_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete player_objects
		where
			object_id=p_object_id(i);
	end;

	procedure delete_demand_container( p_object_id VAOFSTRING, p_reason VAOFNUMBER, p_chunk_size number )
	as
		l_delete_reason number;
	begin
		select reason_code
		into l_delete_reason
		from delete_reasons
		where tag='ContainerDeleted';

		FOR i IN 1.. p_chunk_size
		LOOP
			update objects
			set deleted = decode(p_reason(i),0,l_delete_reason,p_reason(i)), -- use reason if specified, otherwise use 'ContainerDeleted' if it is 0
			load_with = null,
			deleted_date = sysdate
			where load_with = p_object_id(i);
		end LOOP;
	end;

	procedure save_resource_type(p_resource_id VAOFSTRING, p_resource_name VAOFSTRING, p_resource_class VAOFSTRING, p_attributes VAOFSTRING, p_fractal_seeds VAOFSTRING, p_depleted_timestamp VAOFNUMBER, p_chunk_size number) 
	as
	begin
		forall i in 1..p_chunk_size
		insert into resource_types
		(
			resource_id,
			resource_name,
			resource_class,
			attributes,
			fractal_seeds,
			depleted_timestamp
		)
		values
		(
			p_resource_id(i),
			p_resource_name(i),
			p_resource_class(i),
			p_attributes(i),
			p_fractal_seeds(i),
			p_depleted_timestamp(i)
		);

		exception when dup_val_on_index then
			for i in 1..p_chunk_size loop
				update resource_types set
					resource_name = p_resource_name(i),
					resource_class = p_resource_class(i),
					attributes = p_attributes(i),
					fractal_seeds = p_fractal_seeds(i),
					depleted_timestamp = p_depleted_timestamp(i)
				where
					resource_id=p_resource_id(i);

				if SQL%ROWCOUNT=0 then
					insert into resource_types
					(
						resource_id,
						resource_name,
						resource_class,
						attributes,
						fractal_seeds,
						depleted_timestamp
					)
					values
					(
						p_resource_id(i),
						p_resource_name(i),
						p_resource_class(i),
						p_attributes(i),
						p_fractal_seeds(i),
						p_depleted_timestamp(i)
					);
				end if;
			end loop;
	end;

	procedure save_bounty_hunter_target(p_object_id VAOFSTRING, p_target_id VAOFSTRING, p_chunk_size number)
	as
	begin
		forall i in 1..p_chunk_size
		insert into bounty_hunter_targets
		(
			object_id,
			target_id
		)
		values
		(
			p_object_id(i),
			p_target_id(i)
		);

		delete bounty_hunter_targets
		where target_id = 0;
		
	exception when dup_val_on_index then
		begin
			for i in 1..p_chunk_size loop
				update bounty_hunter_targets set
					target_id = p_target_id(i)
				where
					object_id = p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into bounty_hunter_targets
					(
						object_id,
						target_id
					)
					values
					(
						p_object_id(i),
						p_target_id(i)
					);
				end if;
			end loop;
			
			delete bounty_hunter_targets
			where target_id = 0;
		end;
	end;

	function subtract_money_from_object(p_object_id number, p_amount number) return char
	as
		new_bank_balance number;
		new_cash_balance number;
	begin
		select bank_balance, cash_balance 
		into new_bank_balance, new_cash_balance
		from objects
		where object_id = p_object_id
		for update;
	
		if (new_bank_balance + new_cash_balance < p_amount) then
			return 'N';
		end if;

		new_bank_balance := new_bank_balance - p_amount;
		if (new_bank_balance < 0) then
			new_cash_balance := new_cash_balance + new_bank_balance;
			new_bank_balance := 0;
		end if;

		update objects
		set bank_balance = new_bank_balance, cash_balance = new_cash_balance
		where object_id = p_object_id;

		return 'Y';
	exception
		when others then
			return 'N';
	end;

-- GENERATED PLSQL FOLLOWS
-- generated by makepersister.pl

	procedure save_battlefield_marker_obj( p_object_id VAOFSTRING, p_region_name VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update battlefield_marker_objects set
			battlefield_marker_objects.region_name = nvl(p_region_name(i),battlefield_marker_objects.region_name)
		where
			battlefield_marker_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update battlefield_marker_objects set
					battlefield_marker_objects.region_name = nvl(p_region_name(i),battlefield_marker_objects.region_name)
					where
					battlefield_marker_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_battlefield_marker_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_battlefield_marker_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_battlefield_marker_obj','region_name','varchar2',p_region_name(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_battlefield_marker_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_battlefield_marker_obj( p_object_id VAOFSTRING, p_region_name VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into battlefield_marker_objects
		(
			battlefield_marker_objects.region_name,
			battlefield_marker_objects.object_id
		)
		VALUES
		(
			p_region_name(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_battlefield_marker_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE battlefield_marker_objects Set
					battlefield_marker_objects.region_name = nvl(p_region_name(i),battlefield_marker_objects.region_name)
				where
					battlefield_marker_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into battlefield_marker_objects
					(
					battlefield_marker_objects.region_name,
					battlefield_marker_objects.object_id
					)
					VALUES
					(
					p_region_name(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into battlefield_marker_objects
			(
				battlefield_marker_objects.region_name,
				battlefield_marker_objects.object_id
			)
			VALUES
			(
				p_region_name(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_battlefield_marker_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_battlefield_marker_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_battlefield_marker_obj','region_name','varchar2',p_region_name(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_battlefield_marker_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_battlefield_marker_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete battlefield_marker_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_building_obj( p_object_id VAOFSTRING, p_maintenance_cost VAOFNUMBER, p_time_last_checked VAOFNUMBER, p_is_public VAOFSTRING, p_city_id VAOFNUMBER, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update building_objects set
			building_objects.maintenance_cost = nvl(p_maintenance_cost(i),building_objects.maintenance_cost),
			building_objects.time_last_checked = nvl(p_time_last_checked(i),building_objects.time_last_checked),
			building_objects.is_public = nvl(p_is_public(i),building_objects.is_public),
			building_objects.city_id = nvl(p_city_id(i),building_objects.city_id)
		where
			building_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update building_objects set
					building_objects.maintenance_cost = nvl(p_maintenance_cost(i),building_objects.maintenance_cost),
					building_objects.time_last_checked = nvl(p_time_last_checked(i),building_objects.time_last_checked),
					building_objects.is_public = nvl(p_is_public(i),building_objects.is_public),
					building_objects.city_id = nvl(p_city_id(i),building_objects.city_id)
					where
					building_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_building_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_building_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_building_obj','maintenance_cost','int',to_char(p_maintenance_cost(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_building_obj','time_last_checked','float',to_char(p_time_last_checked(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_building_obj','is_public','char',p_is_public(m_error_index));
								db_error_logger.dblogerror_values('persister.save_building_obj','city_id','int',to_char(p_city_id(m_error_index)));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_building_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_building_obj( p_object_id VAOFSTRING, p_maintenance_cost VAOFNUMBER, p_time_last_checked VAOFNUMBER, p_is_public VAOFSTRING, p_city_id VAOFNUMBER, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into building_objects
		(
			building_objects.maintenance_cost,
			building_objects.time_last_checked,
			building_objects.is_public,
			building_objects.city_id,
			building_objects.object_id
		)
		VALUES
		(
			p_maintenance_cost(i),
			p_time_last_checked(i),
			p_is_public(i),
			p_city_id(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_building_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE building_objects Set
					building_objects.maintenance_cost = nvl(p_maintenance_cost(i),building_objects.maintenance_cost),
					building_objects.time_last_checked = nvl(p_time_last_checked(i),building_objects.time_last_checked),
					building_objects.is_public = nvl(p_is_public(i),building_objects.is_public),
					building_objects.city_id = nvl(p_city_id(i),building_objects.city_id)
				where
					building_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into building_objects
					(
					building_objects.maintenance_cost,
					building_objects.time_last_checked,
					building_objects.is_public,
					building_objects.city_id,
					building_objects.object_id
					)
					VALUES
					(
					p_maintenance_cost(i),
					p_time_last_checked(i),
					p_is_public(i),
					p_city_id(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into building_objects
			(
				building_objects.maintenance_cost,
				building_objects.time_last_checked,
				building_objects.is_public,
				building_objects.city_id,
				building_objects.object_id
			)
			VALUES
			(
				p_maintenance_cost(i),
				p_time_last_checked(i),
				p_is_public(i),
				p_city_id(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_building_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_building_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_building_obj','maintenance_cost','int',to_char(p_maintenance_cost(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_building_obj','time_last_checked','float',to_char(p_time_last_checked(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_building_obj','is_public','char',p_is_public(m_error_index));
						db_error_logger.dblogerror_values('persister.add_building_obj','city_id','int',to_char(p_city_id(m_error_index)));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_building_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_building_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete building_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_cell_obj( p_object_id VAOFSTRING, p_cell_number VAOFNUMBER, p_is_public VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update cell_objects set
			cell_objects.cell_number = nvl(p_cell_number(i),cell_objects.cell_number),
			cell_objects.is_public = nvl(p_is_public(i),cell_objects.is_public)
		where
			cell_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update cell_objects set
					cell_objects.cell_number = nvl(p_cell_number(i),cell_objects.cell_number),
					cell_objects.is_public = nvl(p_is_public(i),cell_objects.is_public)
					where
					cell_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_cell_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_cell_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_cell_obj','cell_number','int',to_char(p_cell_number(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_cell_obj','is_public','char',p_is_public(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_cell_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_cell_obj( p_object_id VAOFSTRING, p_cell_number VAOFNUMBER, p_is_public VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into cell_objects
		(
			cell_objects.cell_number,
			cell_objects.is_public,
			cell_objects.object_id
		)
		VALUES
		(
			p_cell_number(i),
			p_is_public(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_cell_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE cell_objects Set
					cell_objects.cell_number = nvl(p_cell_number(i),cell_objects.cell_number),
					cell_objects.is_public = nvl(p_is_public(i),cell_objects.is_public)
				where
					cell_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into cell_objects
					(
					cell_objects.cell_number,
					cell_objects.is_public,
					cell_objects.object_id
					)
					VALUES
					(
					p_cell_number(i),
					p_is_public(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into cell_objects
			(
				cell_objects.cell_number,
				cell_objects.is_public,
				cell_objects.object_id
			)
			VALUES
			(
				p_cell_number(i),
				p_is_public(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_cell_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_cell_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_cell_obj','cell_number','int',to_char(p_cell_number(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_cell_obj','is_public','char',p_is_public(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_cell_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_cell_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete cell_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_city_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		null;
	end;

	procedure add_city_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into city_objects
		(
			city_objects.object_id
		)
		VALUES
		(
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_city_obj : dup_val_on_index error.');
			END IF;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into city_objects
			(
				city_objects.object_id
			)
			VALUES
			(
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_city_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_city_obj','object_id','number',p_object_id(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_city_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_city_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		null;
	end;

	procedure save_creature_obj( p_object_id VAOFSTRING, p_scale_factor VAOFNUMBER, p_states VAOFNUMBER, p_posture VAOFNUMBER, p_shock_wounds VAOFNUMBER, p_master_id VAOFSTRING, p_rank VAOFNUMBER, p_base_walk_speed VAOFNUMBER, p_base_run_speed VAOFNUMBER, p_attribute_0 VAOFNUMBER, p_attribute_1 VAOFNUMBER, p_attribute_2 VAOFNUMBER, p_attribute_3 VAOFNUMBER, p_attribute_4 VAOFNUMBER, p_attribute_5 VAOFNUMBER, p_attribute_6 VAOFNUMBER, p_attribute_7 VAOFNUMBER, p_attribute_8 VAOFNUMBER, p_attribute_9 VAOFNUMBER, p_attribute_10 VAOFNUMBER, p_attribute_11 VAOFNUMBER, p_attribute_12 VAOFNUMBER, p_attribute_13 VAOFNUMBER, p_attribute_14 VAOFNUMBER, p_attribute_15 VAOFNUMBER, p_attribute_16 VAOFNUMBER, p_attribute_17 VAOFNUMBER, p_attribute_18 VAOFNUMBER, p_attribute_19 VAOFNUMBER, p_attribute_20 VAOFNUMBER, p_attribute_21 VAOFNUMBER, p_attribute_22 VAOFNUMBER, p_attribute_23 VAOFNUMBER, p_attribute_24 VAOFNUMBER, p_attribute_25 VAOFNUMBER, p_attribute_26 VAOFNUMBER, p_persisted_buffs VAOFSTRING, p_ws_x VAOFNUMBER, p_ws_y VAOFNUMBER, p_ws_z VAOFNUMBER, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update creature_objects set
			creature_objects.scale_factor = nvl(p_scale_factor(i),creature_objects.scale_factor),
			creature_objects.states = nvl(p_states(i),creature_objects.states),
			creature_objects.posture = nvl(p_posture(i),creature_objects.posture),
			creature_objects.shock_wounds = nvl(p_shock_wounds(i),creature_objects.shock_wounds),
			creature_objects.master_id = nvl(p_master_id(i),creature_objects.master_id),
			creature_objects.rank = nvl(p_rank(i),creature_objects.rank),
			creature_objects.base_walk_speed = nvl(p_base_walk_speed(i),creature_objects.base_walk_speed),
			creature_objects.base_run_speed = nvl(p_base_run_speed(i),creature_objects.base_run_speed),
			creature_objects.attribute_0 = nvl(p_attribute_0(i),creature_objects.attribute_0),
			creature_objects.attribute_1 = nvl(p_attribute_1(i),creature_objects.attribute_1),
			creature_objects.attribute_2 = nvl(p_attribute_2(i),creature_objects.attribute_2),
			creature_objects.attribute_3 = nvl(p_attribute_3(i),creature_objects.attribute_3),
			creature_objects.attribute_4 = nvl(p_attribute_4(i),creature_objects.attribute_4),
			creature_objects.attribute_5 = nvl(p_attribute_5(i),creature_objects.attribute_5),
			creature_objects.attribute_6 = nvl(p_attribute_6(i),creature_objects.attribute_6),
			creature_objects.attribute_7 = nvl(p_attribute_7(i),creature_objects.attribute_7),
			creature_objects.attribute_8 = nvl(p_attribute_8(i),creature_objects.attribute_8),
			creature_objects.attribute_9 = nvl(p_attribute_9(i),creature_objects.attribute_9),
			creature_objects.attribute_10 = nvl(p_attribute_10(i),creature_objects.attribute_10),
			creature_objects.attribute_11 = nvl(p_attribute_11(i),creature_objects.attribute_11),
			creature_objects.attribute_12 = nvl(p_attribute_12(i),creature_objects.attribute_12),
			creature_objects.attribute_13 = nvl(p_attribute_13(i),creature_objects.attribute_13),
			creature_objects.attribute_14 = nvl(p_attribute_14(i),creature_objects.attribute_14),
			creature_objects.attribute_15 = nvl(p_attribute_15(i),creature_objects.attribute_15),
			creature_objects.attribute_16 = nvl(p_attribute_16(i),creature_objects.attribute_16),
			creature_objects.attribute_17 = nvl(p_attribute_17(i),creature_objects.attribute_17),
			creature_objects.attribute_18 = nvl(p_attribute_18(i),creature_objects.attribute_18),
			creature_objects.attribute_19 = nvl(p_attribute_19(i),creature_objects.attribute_19),
			creature_objects.attribute_20 = nvl(p_attribute_20(i),creature_objects.attribute_20),
			creature_objects.attribute_21 = nvl(p_attribute_21(i),creature_objects.attribute_21),
			creature_objects.attribute_22 = nvl(p_attribute_22(i),creature_objects.attribute_22),
			creature_objects.attribute_23 = nvl(p_attribute_23(i),creature_objects.attribute_23),
			creature_objects.attribute_24 = nvl(p_attribute_24(i),creature_objects.attribute_24),
			creature_objects.attribute_25 = nvl(p_attribute_25(i),creature_objects.attribute_25),
			creature_objects.attribute_26 = nvl(p_attribute_26(i),creature_objects.attribute_26),
			creature_objects.persisted_buffs = nvl(p_persisted_buffs(i),creature_objects.persisted_buffs),
			creature_objects.ws_x = nvl(p_ws_x(i),creature_objects.ws_x),
			creature_objects.ws_y = nvl(p_ws_y(i),creature_objects.ws_y),
			creature_objects.ws_z = nvl(p_ws_z(i),creature_objects.ws_z)
		where
			creature_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update creature_objects set
					creature_objects.scale_factor = nvl(p_scale_factor(i),creature_objects.scale_factor),
					creature_objects.states = nvl(p_states(i),creature_objects.states),
					creature_objects.posture = nvl(p_posture(i),creature_objects.posture),
					creature_objects.shock_wounds = nvl(p_shock_wounds(i),creature_objects.shock_wounds),
					creature_objects.master_id = nvl(p_master_id(i),creature_objects.master_id),
					creature_objects.rank = nvl(p_rank(i),creature_objects.rank),
					creature_objects.base_walk_speed = nvl(p_base_walk_speed(i),creature_objects.base_walk_speed),
					creature_objects.base_run_speed = nvl(p_base_run_speed(i),creature_objects.base_run_speed),
					creature_objects.attribute_0 = nvl(p_attribute_0(i),creature_objects.attribute_0),
					creature_objects.attribute_1 = nvl(p_attribute_1(i),creature_objects.attribute_1),
					creature_objects.attribute_2 = nvl(p_attribute_2(i),creature_objects.attribute_2),
					creature_objects.attribute_3 = nvl(p_attribute_3(i),creature_objects.attribute_3),
					creature_objects.attribute_4 = nvl(p_attribute_4(i),creature_objects.attribute_4),
					creature_objects.attribute_5 = nvl(p_attribute_5(i),creature_objects.attribute_5),
					creature_objects.attribute_6 = nvl(p_attribute_6(i),creature_objects.attribute_6),
					creature_objects.attribute_7 = nvl(p_attribute_7(i),creature_objects.attribute_7),
					creature_objects.attribute_8 = nvl(p_attribute_8(i),creature_objects.attribute_8),
					creature_objects.attribute_9 = nvl(p_attribute_9(i),creature_objects.attribute_9),
					creature_objects.attribute_10 = nvl(p_attribute_10(i),creature_objects.attribute_10),
					creature_objects.attribute_11 = nvl(p_attribute_11(i),creature_objects.attribute_11),
					creature_objects.attribute_12 = nvl(p_attribute_12(i),creature_objects.attribute_12),
					creature_objects.attribute_13 = nvl(p_attribute_13(i),creature_objects.attribute_13),
					creature_objects.attribute_14 = nvl(p_attribute_14(i),creature_objects.attribute_14),
					creature_objects.attribute_15 = nvl(p_attribute_15(i),creature_objects.attribute_15),
					creature_objects.attribute_16 = nvl(p_attribute_16(i),creature_objects.attribute_16),
					creature_objects.attribute_17 = nvl(p_attribute_17(i),creature_objects.attribute_17),
					creature_objects.attribute_18 = nvl(p_attribute_18(i),creature_objects.attribute_18),
					creature_objects.attribute_19 = nvl(p_attribute_19(i),creature_objects.attribute_19),
					creature_objects.attribute_20 = nvl(p_attribute_20(i),creature_objects.attribute_20),
					creature_objects.attribute_21 = nvl(p_attribute_21(i),creature_objects.attribute_21),
					creature_objects.attribute_22 = nvl(p_attribute_22(i),creature_objects.attribute_22),
					creature_objects.attribute_23 = nvl(p_attribute_23(i),creature_objects.attribute_23),
					creature_objects.attribute_24 = nvl(p_attribute_24(i),creature_objects.attribute_24),
					creature_objects.attribute_25 = nvl(p_attribute_25(i),creature_objects.attribute_25),
					creature_objects.attribute_26 = nvl(p_attribute_26(i),creature_objects.attribute_26),
					creature_objects.persisted_buffs = nvl(p_persisted_buffs(i),creature_objects.persisted_buffs),
					creature_objects.ws_x = nvl(p_ws_x(i),creature_objects.ws_x),
					creature_objects.ws_y = nvl(p_ws_y(i),creature_objects.ws_y),
					creature_objects.ws_z = nvl(p_ws_z(i),creature_objects.ws_z)
					where
					creature_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_creature_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_creature_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_creature_obj','scale_factor','float',to_char(p_scale_factor(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','states','int',to_char(p_states(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','posture','int',to_char(p_posture(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','shock_wounds','int',to_char(p_shock_wounds(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','master_id','number',p_master_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_creature_obj','rank','int',to_char(p_rank(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','base_walk_speed','float',to_char(p_base_walk_speed(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','base_run_speed','float',to_char(p_base_run_speed(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_0','int',to_char(p_attribute_0(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_1','int',to_char(p_attribute_1(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_2','int',to_char(p_attribute_2(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_3','int',to_char(p_attribute_3(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_4','int',to_char(p_attribute_4(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_5','int',to_char(p_attribute_5(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_6','int',to_char(p_attribute_6(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_7','int',to_char(p_attribute_7(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_8','int',to_char(p_attribute_8(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_9','int',to_char(p_attribute_9(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_10','int',to_char(p_attribute_10(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_11','int',to_char(p_attribute_11(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_12','int',to_char(p_attribute_12(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_13','int',to_char(p_attribute_13(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_14','int',to_char(p_attribute_14(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_15','int',to_char(p_attribute_15(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_16','int',to_char(p_attribute_16(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_17','int',to_char(p_attribute_17(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_18','int',to_char(p_attribute_18(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_19','int',to_char(p_attribute_19(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_20','int',to_char(p_attribute_20(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_21','int',to_char(p_attribute_21(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_22','int',to_char(p_attribute_22(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_23','int',to_char(p_attribute_23(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_24','int',to_char(p_attribute_24(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_25','int',to_char(p_attribute_25(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','attribute_26','int',to_char(p_attribute_26(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','persisted_buffs','varchar2',p_persisted_buffs(m_error_index));

								db_error_logger.dblogerror_values('persister.save_creature_obj','ws_x','float',to_char(p_ws_x(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','ws_y','float',to_char(p_ws_y(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_creature_obj','ws_z','float',to_char(p_ws_z(m_error_index)));

							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_creature_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_creature_obj( p_object_id VAOFSTRING, p_scale_factor VAOFNUMBER, p_states VAOFNUMBER, p_posture VAOFNUMBER, p_shock_wounds VAOFNUMBER, p_master_id VAOFSTRING, p_rank VAOFNUMBER, p_base_walk_speed VAOFNUMBER, p_base_run_speed VAOFNUMBER, p_attribute_0 VAOFNUMBER, p_attribute_1 VAOFNUMBER, p_attribute_2 VAOFNUMBER, p_attribute_3 VAOFNUMBER, p_attribute_4 VAOFNUMBER, p_attribute_5 VAOFNUMBER, p_attribute_6 VAOFNUMBER, p_attribute_7 VAOFNUMBER, p_attribute_8 VAOFNUMBER, p_attribute_9 VAOFNUMBER, p_attribute_10 VAOFNUMBER, p_attribute_11 VAOFNUMBER, p_attribute_12 VAOFNUMBER, p_attribute_13 VAOFNUMBER, p_attribute_14 VAOFNUMBER, p_attribute_15 VAOFNUMBER, p_attribute_16 VAOFNUMBER, p_attribute_17 VAOFNUMBER, p_attribute_18 VAOFNUMBER, p_attribute_19 VAOFNUMBER, p_attribute_20 VAOFNUMBER, p_attribute_21 VAOFNUMBER, p_attribute_22 VAOFNUMBER, p_attribute_23 VAOFNUMBER, p_attribute_24 VAOFNUMBER, p_attribute_25 VAOFNUMBER, p_attribute_26 VAOFNUMBER, p_persisted_buffs VAOFSTRING, p_ws_x VAOFNUMBER, p_ws_y VAOFNUMBER, p_ws_z VAOFNUMBER, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into creature_objects
		(
			creature_objects.scale_factor,
			creature_objects.states,
			creature_objects.posture,
			creature_objects.shock_wounds,
			creature_objects.master_id,
			creature_objects.rank,
			creature_objects.base_walk_speed,
			creature_objects.base_run_speed,
			creature_objects.attribute_0,
			creature_objects.attribute_1,
			creature_objects.attribute_2,
			creature_objects.attribute_3,
			creature_objects.attribute_4,
			creature_objects.attribute_5,
			creature_objects.attribute_6,
			creature_objects.attribute_7,
			creature_objects.attribute_8,
			creature_objects.attribute_9,
			creature_objects.attribute_10,
			creature_objects.attribute_11,
			creature_objects.attribute_12,
			creature_objects.attribute_13,
			creature_objects.attribute_14,
			creature_objects.attribute_15,
			creature_objects.attribute_16,
			creature_objects.attribute_17,
			creature_objects.attribute_18,
			creature_objects.attribute_19,
			creature_objects.attribute_20,
			creature_objects.attribute_21,
			creature_objects.attribute_22,
			creature_objects.attribute_23,
			creature_objects.attribute_24,
			creature_objects.attribute_25,
			creature_objects.attribute_26,
			creature_objects.persisted_buffs,
			creature_objects.object_id,
			creature_objects.ws_x,
			creature_objects.ws_y,
			creature_objects.ws_z
		)
		VALUES
		(
			p_scale_factor(i),
			p_states(i),
			p_posture(i),
			p_shock_wounds(i),
			p_master_id(i),
			p_rank(i),
			p_base_walk_speed(i),
			p_base_run_speed(i),
			p_attribute_0(i),
			p_attribute_1(i),
			p_attribute_2(i),
			p_attribute_3(i),
			p_attribute_4(i),
			p_attribute_5(i),
			p_attribute_6(i),
			p_attribute_7(i),
			p_attribute_8(i),
			p_attribute_9(i),
			p_attribute_10(i),
			p_attribute_11(i),
			p_attribute_12(i),
			p_attribute_13(i),
			p_attribute_14(i),
			p_attribute_15(i),
			p_attribute_16(i),
			p_attribute_17(i),
			p_attribute_18(i),
			p_attribute_19(i),
			p_attribute_20(i),
			p_attribute_21(i),
			p_attribute_22(i),
			p_attribute_23(i),
			p_attribute_24(i),
			p_attribute_25(i),
			p_attribute_26(i),
			p_persisted_buffs(i),
			p_object_id(i),
			NULL,
			NULL,
			NULL
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_creature_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE creature_objects Set
					creature_objects.scale_factor = nvl(p_scale_factor(i),creature_objects.scale_factor),
					creature_objects.states = nvl(p_states(i),creature_objects.states),
					creature_objects.posture = nvl(p_posture(i),creature_objects.posture),
					creature_objects.shock_wounds = nvl(p_shock_wounds(i),creature_objects.shock_wounds),
					creature_objects.master_id = nvl(p_master_id(i),creature_objects.master_id),
					creature_objects.rank = nvl(p_rank(i),creature_objects.rank),
					creature_objects.base_walk_speed = nvl(p_base_walk_speed(i),creature_objects.base_walk_speed),
					creature_objects.base_run_speed = nvl(p_base_run_speed(i),creature_objects.base_run_speed),
					creature_objects.attribute_0 = nvl(p_attribute_0(i),creature_objects.attribute_0),
					creature_objects.attribute_1 = nvl(p_attribute_1(i),creature_objects.attribute_1),
					creature_objects.attribute_2 = nvl(p_attribute_2(i),creature_objects.attribute_2),
					creature_objects.attribute_3 = nvl(p_attribute_3(i),creature_objects.attribute_3),
					creature_objects.attribute_4 = nvl(p_attribute_4(i),creature_objects.attribute_4),
					creature_objects.attribute_5 = nvl(p_attribute_5(i),creature_objects.attribute_5),
					creature_objects.attribute_6 = nvl(p_attribute_6(i),creature_objects.attribute_6),
					creature_objects.attribute_7 = nvl(p_attribute_7(i),creature_objects.attribute_7),
					creature_objects.attribute_8 = nvl(p_attribute_8(i),creature_objects.attribute_8),
					creature_objects.attribute_9 = nvl(p_attribute_9(i),creature_objects.attribute_9),
					creature_objects.attribute_10 = nvl(p_attribute_10(i),creature_objects.attribute_10),
					creature_objects.attribute_11 = nvl(p_attribute_11(i),creature_objects.attribute_11),
					creature_objects.attribute_12 = nvl(p_attribute_12(i),creature_objects.attribute_12),
					creature_objects.attribute_13 = nvl(p_attribute_13(i),creature_objects.attribute_13),
					creature_objects.attribute_14 = nvl(p_attribute_14(i),creature_objects.attribute_14),
					creature_objects.attribute_15 = nvl(p_attribute_15(i),creature_objects.attribute_15),
					creature_objects.attribute_16 = nvl(p_attribute_16(i),creature_objects.attribute_16),
					creature_objects.attribute_17 = nvl(p_attribute_17(i),creature_objects.attribute_17),
					creature_objects.attribute_18 = nvl(p_attribute_18(i),creature_objects.attribute_18),
					creature_objects.attribute_19 = nvl(p_attribute_19(i),creature_objects.attribute_19),
					creature_objects.attribute_20 = nvl(p_attribute_20(i),creature_objects.attribute_20),
					creature_objects.attribute_21 = nvl(p_attribute_21(i),creature_objects.attribute_21),
					creature_objects.attribute_22 = nvl(p_attribute_22(i),creature_objects.attribute_22),
					creature_objects.attribute_23 = nvl(p_attribute_23(i),creature_objects.attribute_23),
					creature_objects.attribute_24 = nvl(p_attribute_24(i),creature_objects.attribute_24),
					creature_objects.attribute_25 = nvl(p_attribute_25(i),creature_objects.attribute_25),
					creature_objects.attribute_26 = nvl(p_attribute_26(i),creature_objects.attribute_26),
					creature_objects.persisted_buffs = nvl(p_persisted_buffs(i),creature_objects.persisted_buffs),
					creature_objects.ws_x = nvl(p_ws_x(i),creature_objects.ws_x),
					creature_objects.ws_y = nvl(p_ws_y(i),creature_objects.ws_y),
					creature_objects.ws_z = nvl(p_ws_z(i),creature_objects.ws_z)
				where
					creature_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into creature_objects
					(
					creature_objects.scale_factor,
					creature_objects.states,
					creature_objects.posture,
					creature_objects.shock_wounds,
					creature_objects.master_id,
					creature_objects.rank,
					creature_objects.base_walk_speed,
					creature_objects.base_run_speed,
					creature_objects.attribute_0,
					creature_objects.attribute_1,
					creature_objects.attribute_2,
					creature_objects.attribute_3,
					creature_objects.attribute_4,
					creature_objects.attribute_5,
					creature_objects.attribute_6,
					creature_objects.attribute_7,
					creature_objects.attribute_8,
					creature_objects.attribute_9,
					creature_objects.attribute_10,
					creature_objects.attribute_11,
					creature_objects.attribute_12,
					creature_objects.attribute_13,
					creature_objects.attribute_14,
					creature_objects.attribute_15,
					creature_objects.attribute_16,
					creature_objects.attribute_17,
					creature_objects.attribute_18,
					creature_objects.attribute_19,
					creature_objects.attribute_20,
					creature_objects.attribute_21,
					creature_objects.attribute_22,
					creature_objects.attribute_23,
					creature_objects.attribute_24,
					creature_objects.attribute_25,
					creature_objects.attribute_26,
					creature_objects.persisted_buffs,
					creature_objects.object_id,
					creature_objects.ws_x,
					creature_objects.ws_y,
					creature_objects.ws_z
					)
					VALUES
					(
					p_scale_factor(i),
					p_states(i),
					p_posture(i),
					p_shock_wounds(i),
					p_master_id(i),
					p_rank(i),
					p_base_walk_speed(i),
					p_base_run_speed(i),
					p_attribute_0(i),
					p_attribute_1(i),
					p_attribute_2(i),
					p_attribute_3(i),
					p_attribute_4(i),
					p_attribute_5(i),
					p_attribute_6(i),
					p_attribute_7(i),
					p_attribute_8(i),
					p_attribute_9(i),
					p_attribute_10(i),
					p_attribute_11(i),
					p_attribute_12(i),
					p_attribute_13(i),
					p_attribute_14(i),
					p_attribute_15(i),
					p_attribute_16(i),
					p_attribute_17(i),
					p_attribute_18(i),
					p_attribute_19(i),
					p_attribute_20(i),
					p_attribute_21(i),
					p_attribute_22(i),
					p_attribute_23(i),
					p_attribute_24(i),
					p_attribute_25(i),
					p_attribute_26(i),
					p_persisted_buffs(i),
					p_object_id(i),
					NULL,
					NULL,
					NULL
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into creature_objects
			(
				creature_objects.scale_factor,
				creature_objects.states,
				creature_objects.posture,
				creature_objects.shock_wounds,
				creature_objects.master_id,
				creature_objects.rank,
				creature_objects.base_walk_speed,
				creature_objects.base_run_speed,
				creature_objects.attribute_0,
				creature_objects.attribute_1,
				creature_objects.attribute_2,
				creature_objects.attribute_3,
				creature_objects.attribute_4,
				creature_objects.attribute_5,
				creature_objects.attribute_6,
				creature_objects.attribute_7,
				creature_objects.attribute_8,
				creature_objects.attribute_9,
				creature_objects.attribute_10,
				creature_objects.attribute_11,
				creature_objects.attribute_12,
				creature_objects.attribute_13,
				creature_objects.attribute_14,
				creature_objects.attribute_15,
				creature_objects.attribute_16,
				creature_objects.attribute_17,
				creature_objects.attribute_18,
				creature_objects.attribute_19,
				creature_objects.attribute_20,
				creature_objects.attribute_21,
				creature_objects.attribute_22,
				creature_objects.attribute_23,
				creature_objects.attribute_24,
				creature_objects.attribute_25,
				creature_objects.attribute_26,
				creature_objects.persisted_buffs,
				creature_objects.object_id,
				creature_objects.ws_x,
				creature_objects.ws_y,
				creature_objects.ws_z
			)
			VALUES
			(
				p_scale_factor(i),
				p_states(i),
				p_posture(i),
				p_shock_wounds(i),
				p_master_id(i),
				p_rank(i),
				p_base_walk_speed(i),
				p_base_run_speed(i),
				p_attribute_0(i),
				p_attribute_1(i),
				p_attribute_2(i),
				p_attribute_3(i),
				p_attribute_4(i),
				p_attribute_5(i),
				p_attribute_6(i),
				p_attribute_7(i),
				p_attribute_8(i),
				p_attribute_9(i),
				p_attribute_10(i),
				p_attribute_11(i),
				p_attribute_12(i),
				p_attribute_13(i),
				p_attribute_14(i),
				p_attribute_15(i),
				p_attribute_16(i),
				p_attribute_17(i),
				p_attribute_18(i),
				p_attribute_19(i),
				p_attribute_20(i),
				p_attribute_21(i),
				p_attribute_22(i),
				p_attribute_23(i),
				p_attribute_24(i),
				p_attribute_25(i),
				p_attribute_26(i),
				p_persisted_buffs(i),
				p_object_id(i),
				NULL,
				NULL,
				NULL
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_creature_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_creature_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_creature_obj','scale_factor','float',to_char(p_scale_factor(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','states','int',to_char(p_states(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','posture','int',to_char(p_posture(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','shock_wounds','int',to_char(p_shock_wounds(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','master_id','number',p_master_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_creature_obj','rank','int',to_char(p_rank(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','base_walk_speed','float',to_char(p_base_walk_speed(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','base_run_speed','float',to_char(p_base_run_speed(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_0','int',to_char(p_attribute_0(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_1','int',to_char(p_attribute_1(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_2','int',to_char(p_attribute_2(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_3','int',to_char(p_attribute_3(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_4','int',to_char(p_attribute_4(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_5','int',to_char(p_attribute_5(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_6','int',to_char(p_attribute_6(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_7','int',to_char(p_attribute_7(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_8','int',to_char(p_attribute_8(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_9','int',to_char(p_attribute_9(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_10','int',to_char(p_attribute_10(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_11','int',to_char(p_attribute_11(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_12','int',to_char(p_attribute_12(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_13','int',to_char(p_attribute_13(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_14','int',to_char(p_attribute_14(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_15','int',to_char(p_attribute_15(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_16','int',to_char(p_attribute_16(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_17','int',to_char(p_attribute_17(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_18','int',to_char(p_attribute_18(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_19','int',to_char(p_attribute_19(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_20','int',to_char(p_attribute_20(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_21','int',to_char(p_attribute_21(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_22','int',to_char(p_attribute_22(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_23','int',to_char(p_attribute_23(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_24','int',to_char(p_attribute_24(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_25','int',to_char(p_attribute_25(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','attribute_26','int',to_char(p_attribute_26(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_creature_obj','persisted_buffs','varchar2',p_persisted_buffs(m_error_index));
						db_error_logger.dblogerror_values('persister.add_creature_obj','ws_x','float',p_ws_x(m_error_index));
						db_error_logger.dblogerror_values('persister.add_creature_obj','ws_y','float',p_ws_y(m_error_index));
						db_error_logger.dblogerror_values('persister.add_creature_obj','ws_z','float',p_ws_z(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_creature_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_creature_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete creature_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_factory_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		null;
	end;

	procedure add_factory_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into factory_objects
		(
			factory_objects.object_id
		)
		VALUES
		(
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_factory_obj : dup_val_on_index error.');
			END IF;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into factory_objects
			(
				factory_objects.object_id
			)
			VALUES
			(
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_factory_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_factory_obj','object_id','number',p_object_id(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_factory_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_factory_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		null;
	end;

	procedure save_guild_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		null;
	end;

	procedure add_guild_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into guild_objects
		(
			guild_objects.object_id
		)
		VALUES
		(
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_guild_obj : dup_val_on_index error.');
			END IF;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into guild_objects
			(
				guild_objects.object_id
			)
			VALUES
			(
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_guild_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_guild_obj','object_id','number',p_object_id(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_guild_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_guild_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		null;
	end;

	procedure save_harvester_inst_obj( p_object_id VAOFSTRING, p_installed_efficiency VAOFNUMBER, p_max_extraction_rate VAOFNUMBER, p_current_extraction_rate VAOFNUMBER, p_max_hopper_amount VAOFNUMBER, p_hopper_resource VAOFSTRING, p_hopper_amount VAOFNUMBER, p_resource_type VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update harvester_installation_objects set
			harvester_installation_objects.installed_efficiency = nvl(p_installed_efficiency(i),harvester_installation_objects.installed_efficiency),
			harvester_installation_objects.max_extraction_rate = nvl(p_max_extraction_rate(i),harvester_installation_objects.max_extraction_rate),
			harvester_installation_objects.current_extraction_rate = nvl(p_current_extraction_rate(i),harvester_installation_objects.current_extraction_rate),
			harvester_installation_objects.max_hopper_amount = nvl(p_max_hopper_amount(i),harvester_installation_objects.max_hopper_amount),
			harvester_installation_objects.hopper_resource = nvl(p_hopper_resource(i),harvester_installation_objects.hopper_resource),
			harvester_installation_objects.hopper_amount = nvl(p_hopper_amount(i),harvester_installation_objects.hopper_amount),
			harvester_installation_objects.resource_type = nvl(p_resource_type(i),harvester_installation_objects.resource_type)
		where
			harvester_installation_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update harvester_installation_objects set
					harvester_installation_objects.installed_efficiency = nvl(p_installed_efficiency(i),harvester_installation_objects.installed_efficiency),
					harvester_installation_objects.max_extraction_rate = nvl(p_max_extraction_rate(i),harvester_installation_objects.max_extraction_rate),
					harvester_installation_objects.current_extraction_rate = nvl(p_current_extraction_rate(i),harvester_installation_objects.current_extraction_rate),
					harvester_installation_objects.max_hopper_amount = nvl(p_max_hopper_amount(i),harvester_installation_objects.max_hopper_amount),
					harvester_installation_objects.hopper_resource = nvl(p_hopper_resource(i),harvester_installation_objects.hopper_resource),
					harvester_installation_objects.hopper_amount = nvl(p_hopper_amount(i),harvester_installation_objects.hopper_amount),
					harvester_installation_objects.resource_type = nvl(p_resource_type(i),harvester_installation_objects.resource_type)
					where
					harvester_installation_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_harvester_inst_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_harvester_inst_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_harvester_inst_obj','installed_efficiency','float',to_char(p_installed_efficiency(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_harvester_inst_obj','max_extraction_rate','int',to_char(p_max_extraction_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_harvester_inst_obj','current_extraction_rate','float',to_char(p_current_extraction_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_harvester_inst_obj','max_hopper_amount','int',to_char(p_max_hopper_amount(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_harvester_inst_obj','hopper_resource','number',p_hopper_resource(m_error_index));
								db_error_logger.dblogerror_values('persister.save_harvester_inst_obj','hopper_amount','float',to_char(p_hopper_amount(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_harvester_inst_obj','resource_type','number',p_resource_type(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_harvester_inst_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_harvester_inst_obj( p_object_id VAOFSTRING, p_installed_efficiency VAOFNUMBER, p_max_extraction_rate VAOFNUMBER, p_current_extraction_rate VAOFNUMBER, p_max_hopper_amount VAOFNUMBER, p_hopper_resource VAOFSTRING, p_hopper_amount VAOFNUMBER, p_resource_type VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into harvester_installation_objects
		(
			harvester_installation_objects.installed_efficiency,
			harvester_installation_objects.max_extraction_rate,
			harvester_installation_objects.current_extraction_rate,
			harvester_installation_objects.max_hopper_amount,
			harvester_installation_objects.hopper_resource,
			harvester_installation_objects.hopper_amount,
			harvester_installation_objects.resource_type,
			harvester_installation_objects.object_id
		)
		VALUES
		(
			p_installed_efficiency(i),
			p_max_extraction_rate(i),
			p_current_extraction_rate(i),
			p_max_hopper_amount(i),
			p_hopper_resource(i),
			p_hopper_amount(i),
			p_resource_type(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_harvester_inst_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE harvester_installation_objects Set
					harvester_installation_objects.installed_efficiency = nvl(p_installed_efficiency(i),harvester_installation_objects.installed_efficiency),
					harvester_installation_objects.max_extraction_rate = nvl(p_max_extraction_rate(i),harvester_installation_objects.max_extraction_rate),
					harvester_installation_objects.current_extraction_rate = nvl(p_current_extraction_rate(i),harvester_installation_objects.current_extraction_rate),
					harvester_installation_objects.max_hopper_amount = nvl(p_max_hopper_amount(i),harvester_installation_objects.max_hopper_amount),
					harvester_installation_objects.hopper_resource = nvl(p_hopper_resource(i),harvester_installation_objects.hopper_resource),
					harvester_installation_objects.hopper_amount = nvl(p_hopper_amount(i),harvester_installation_objects.hopper_amount),
					harvester_installation_objects.resource_type = nvl(p_resource_type(i),harvester_installation_objects.resource_type)
				where
					harvester_installation_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into harvester_installation_objects
					(
					harvester_installation_objects.installed_efficiency,
					harvester_installation_objects.max_extraction_rate,
					harvester_installation_objects.current_extraction_rate,
					harvester_installation_objects.max_hopper_amount,
					harvester_installation_objects.hopper_resource,
					harvester_installation_objects.hopper_amount,
					harvester_installation_objects.resource_type,
					harvester_installation_objects.object_id
					)
					VALUES
					(
					p_installed_efficiency(i),
					p_max_extraction_rate(i),
					p_current_extraction_rate(i),
					p_max_hopper_amount(i),
					p_hopper_resource(i),
					p_hopper_amount(i),
					p_resource_type(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into harvester_installation_objects
			(
				harvester_installation_objects.installed_efficiency,
				harvester_installation_objects.max_extraction_rate,
				harvester_installation_objects.current_extraction_rate,
				harvester_installation_objects.max_hopper_amount,
				harvester_installation_objects.hopper_resource,
				harvester_installation_objects.hopper_amount,
				harvester_installation_objects.resource_type,
				harvester_installation_objects.object_id
			)
			VALUES
			(
				p_installed_efficiency(i),
				p_max_extraction_rate(i),
				p_current_extraction_rate(i),
				p_max_hopper_amount(i),
				p_hopper_resource(i),
				p_hopper_amount(i),
				p_resource_type(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_harvester_inst_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_harvester_inst_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_harvester_inst_obj','installed_efficiency','float',to_char(p_installed_efficiency(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_harvester_inst_obj','max_extraction_rate','int',to_char(p_max_extraction_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_harvester_inst_obj','current_extraction_rate','float',to_char(p_current_extraction_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_harvester_inst_obj','max_hopper_amount','int',to_char(p_max_hopper_amount(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_harvester_inst_obj','hopper_resource','number',p_hopper_resource(m_error_index));
						db_error_logger.dblogerror_values('persister.add_harvester_inst_obj','hopper_amount','float',to_char(p_hopper_amount(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_harvester_inst_obj','resource_type','number',p_resource_type(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_harvester_inst_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_harvester_inst_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete harvester_installation_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_installation_obj( p_object_id VAOFSTRING, p_installation_type VAOFNUMBER, p_activated VAOFSTRING, p_tick_count VAOFNUMBER, p_activate_start_time VAOFNUMBER, p_power VAOFNUMBER, p_power_rate VAOFNUMBER, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update installation_objects set
			installation_objects.installation_type = nvl(p_installation_type(i),installation_objects.installation_type),
			installation_objects.activated = nvl(p_activated(i),installation_objects.activated),
			installation_objects.tick_count = nvl(p_tick_count(i),installation_objects.tick_count),
			installation_objects.activate_start_time = nvl(p_activate_start_time(i),installation_objects.activate_start_time),
			installation_objects.power = nvl(p_power(i),installation_objects.power),
			installation_objects.power_rate = nvl(p_power_rate(i),installation_objects.power_rate)
		where
			installation_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update installation_objects set
					installation_objects.installation_type = nvl(p_installation_type(i),installation_objects.installation_type),
					installation_objects.activated = nvl(p_activated(i),installation_objects.activated),
					installation_objects.tick_count = nvl(p_tick_count(i),installation_objects.tick_count),
					installation_objects.activate_start_time = nvl(p_activate_start_time(i),installation_objects.activate_start_time),
					installation_objects.power = nvl(p_power(i),installation_objects.power),
					installation_objects.power_rate = nvl(p_power_rate(i),installation_objects.power_rate)
					where
					installation_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_installation_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_installation_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_installation_obj','installation_type','int',to_char(p_installation_type(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_installation_obj','activated','char',p_activated(m_error_index));
								db_error_logger.dblogerror_values('persister.save_installation_obj','tick_count','float',to_char(p_tick_count(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_installation_obj','activate_start_time','float',to_char(p_activate_start_time(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_installation_obj','power','float',to_char(p_power(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_installation_obj','power_rate','float',to_char(p_power_rate(m_error_index)));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_installation_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_installation_obj( p_object_id VAOFSTRING, p_installation_type VAOFNUMBER, p_activated VAOFSTRING, p_tick_count VAOFNUMBER, p_activate_start_time VAOFNUMBER, p_power VAOFNUMBER, p_power_rate VAOFNUMBER, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into installation_objects
		(
			installation_objects.installation_type,
			installation_objects.activated,
			installation_objects.tick_count,
			installation_objects.activate_start_time,
			installation_objects.power,
			installation_objects.power_rate,
			installation_objects.object_id
		)
		VALUES
		(
			p_installation_type(i),
			p_activated(i),
			p_tick_count(i),
			p_activate_start_time(i),
			p_power(i),
			p_power_rate(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_installation_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE installation_objects Set
					installation_objects.installation_type = nvl(p_installation_type(i),installation_objects.installation_type),
					installation_objects.activated = nvl(p_activated(i),installation_objects.activated),
					installation_objects.tick_count = nvl(p_tick_count(i),installation_objects.tick_count),
					installation_objects.activate_start_time = nvl(p_activate_start_time(i),installation_objects.activate_start_time),
					installation_objects.power = nvl(p_power(i),installation_objects.power),
					installation_objects.power_rate = nvl(p_power_rate(i),installation_objects.power_rate)
				where
					installation_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into installation_objects
					(
					installation_objects.installation_type,
					installation_objects.activated,
					installation_objects.tick_count,
					installation_objects.activate_start_time,
					installation_objects.power,
					installation_objects.power_rate,
					installation_objects.object_id
					)
					VALUES
					(
					p_installation_type(i),
					p_activated(i),
					p_tick_count(i),
					p_activate_start_time(i),
					p_power(i),
					p_power_rate(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into installation_objects
			(
				installation_objects.installation_type,
				installation_objects.activated,
				installation_objects.tick_count,
				installation_objects.activate_start_time,
				installation_objects.power,
				installation_objects.power_rate,
				installation_objects.object_id
			)
			VALUES
			(
				p_installation_type(i),
				p_activated(i),
				p_tick_count(i),
				p_activate_start_time(i),
				p_power(i),
				p_power_rate(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_installation_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_installation_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_installation_obj','installation_type','int',to_char(p_installation_type(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_installation_obj','activated','char',p_activated(m_error_index));
						db_error_logger.dblogerror_values('persister.add_installation_obj','tick_count','float',to_char(p_tick_count(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_installation_obj','activate_start_time','float',to_char(p_activate_start_time(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_installation_obj','power','float',to_char(p_power(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_installation_obj','power_rate','float',to_char(p_power_rate(m_error_index)));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_installation_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_installation_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete installation_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_intangible_obj( p_object_id VAOFSTRING, p_count VAOFNUMBER, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update intangible_objects set
			intangible_objects.count = nvl(p_count(i),intangible_objects.count)
		where
			intangible_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update intangible_objects set
					intangible_objects.count = nvl(p_count(i),intangible_objects.count)
					where
					intangible_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_intangible_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_intangible_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_intangible_obj','count','int',to_char(p_count(m_error_index)));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_intangible_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_intangible_obj( p_object_id VAOFSTRING, p_count VAOFNUMBER, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into intangible_objects
		(
			intangible_objects.count,
			intangible_objects.object_id
		)
		VALUES
		(
			p_count(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_intangible_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE intangible_objects Set
					intangible_objects.count = nvl(p_count(i),intangible_objects.count)
				where
					intangible_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into intangible_objects
					(
					intangible_objects.count,
					intangible_objects.object_id
					)
					VALUES
					(
					p_count(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into intangible_objects
			(
				intangible_objects.count,
				intangible_objects.object_id
			)
			VALUES
			(
				p_count(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_intangible_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_intangible_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_intangible_obj','count','int',to_char(p_count(m_error_index)));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_intangible_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_intangible_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete intangible_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_manf_schematic_obj( p_object_id VAOFSTRING, p_creator_id VAOFSTRING, p_creator_name VAOFSTRING, p_items_per_container VAOFNUMBER, p_manufacture_time VAOFNUMBER, p_draft_schematic VAOFNUMBER, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update manf_schematic_objects set
			manf_schematic_objects.creator_id = nvl(p_creator_id(i),manf_schematic_objects.creator_id),
			manf_schematic_objects.creator_name = nvl(p_creator_name(i),manf_schematic_objects.creator_name),
			manf_schematic_objects.items_per_container = nvl(p_items_per_container(i),manf_schematic_objects.items_per_container),
			manf_schematic_objects.manufacture_time = nvl(p_manufacture_time(i),manf_schematic_objects.manufacture_time),
			manf_schematic_objects.draft_schematic = nvl(p_draft_schematic(i),manf_schematic_objects.draft_schematic)
		where
			manf_schematic_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update manf_schematic_objects set
					manf_schematic_objects.creator_id = nvl(p_creator_id(i),manf_schematic_objects.creator_id),
					manf_schematic_objects.creator_name = nvl(p_creator_name(i),manf_schematic_objects.creator_name),
					manf_schematic_objects.items_per_container = nvl(p_items_per_container(i),manf_schematic_objects.items_per_container),
					manf_schematic_objects.manufacture_time = nvl(p_manufacture_time(i),manf_schematic_objects.manufacture_time),
					manf_schematic_objects.draft_schematic = nvl(p_draft_schematic(i),manf_schematic_objects.draft_schematic)
					where
					manf_schematic_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_manf_schematic_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_manf_schematic_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_manf_schematic_obj','creator_id','number',p_creator_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_manf_schematic_obj','creator_name','varchar2',p_creator_name(m_error_index));
								db_error_logger.dblogerror_values('persister.save_manf_schematic_obj','items_per_container','int',to_char(p_items_per_container(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_manf_schematic_obj','manufacture_time','float',to_char(p_manufacture_time(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_manf_schematic_obj','draft_schematic','int',to_char(p_draft_schematic(m_error_index)));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_manf_schematic_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_manf_schematic_obj( p_object_id VAOFSTRING, p_creator_id VAOFSTRING, p_creator_name VAOFSTRING, p_items_per_container VAOFNUMBER, p_manufacture_time VAOFNUMBER, p_draft_schematic VAOFNUMBER, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into manf_schematic_objects
		(
			manf_schematic_objects.creator_id,
			manf_schematic_objects.creator_name,
			manf_schematic_objects.items_per_container,
			manf_schematic_objects.manufacture_time,
			manf_schematic_objects.draft_schematic,
			manf_schematic_objects.object_id
		)
		VALUES
		(
			p_creator_id(i),
			p_creator_name(i),
			p_items_per_container(i),
			p_manufacture_time(i),
			p_draft_schematic(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_manf_schematic_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE manf_schematic_objects Set
					manf_schematic_objects.creator_id = nvl(p_creator_id(i),manf_schematic_objects.creator_id),
					manf_schematic_objects.creator_name = nvl(p_creator_name(i),manf_schematic_objects.creator_name),
					manf_schematic_objects.items_per_container = nvl(p_items_per_container(i),manf_schematic_objects.items_per_container),
					manf_schematic_objects.manufacture_time = nvl(p_manufacture_time(i),manf_schematic_objects.manufacture_time),
					manf_schematic_objects.draft_schematic = nvl(p_draft_schematic(i),manf_schematic_objects.draft_schematic)
				where
					manf_schematic_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into manf_schematic_objects
					(
					manf_schematic_objects.creator_id,
					manf_schematic_objects.creator_name,
					manf_schematic_objects.items_per_container,
					manf_schematic_objects.manufacture_time,
					manf_schematic_objects.draft_schematic,
					manf_schematic_objects.object_id
					)
					VALUES
					(
					p_creator_id(i),
					p_creator_name(i),
					p_items_per_container(i),
					p_manufacture_time(i),
					p_draft_schematic(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into manf_schematic_objects
			(
				manf_schematic_objects.creator_id,
				manf_schematic_objects.creator_name,
				manf_schematic_objects.items_per_container,
				manf_schematic_objects.manufacture_time,
				manf_schematic_objects.draft_schematic,
				manf_schematic_objects.object_id
			)
			VALUES
			(
				p_creator_id(i),
				p_creator_name(i),
				p_items_per_container(i),
				p_manufacture_time(i),
				p_draft_schematic(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_manf_schematic_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_manf_schematic_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_manf_schematic_obj','creator_id','number',p_creator_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_manf_schematic_obj','creator_name','varchar2',p_creator_name(m_error_index));
						db_error_logger.dblogerror_values('persister.add_manf_schematic_obj','items_per_container','int',to_char(p_items_per_container(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_manf_schematic_obj','manufacture_time','float',to_char(p_manufacture_time(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_manf_schematic_obj','draft_schematic','int',to_char(p_draft_schematic(m_error_index)));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_manf_schematic_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_manf_schematic_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete manf_schematic_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_manufacture_inst_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		null;
	end;

	procedure add_manufacture_inst_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into manufacture_inst_objects
		(
			manufacture_inst_objects.object_id
		)
		VALUES
		(
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_manufacture_inst_obj : dup_val_on_index error.');
			END IF;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into manufacture_inst_objects
			(
				manufacture_inst_objects.object_id
			)
			VALUES
			(
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_manufacture_inst_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_manufacture_inst_obj','object_id','number',p_object_id(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_manufacture_inst_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_manufacture_inst_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		null;
	end;

	procedure save_mission_obj( p_object_id VAOFSTRING, p_difficulty VAOFNUMBER, p_end_x VAOFNUMBER, p_end_y VAOFNUMBER, p_end_z VAOFNUMBER, p_end_cell VAOFSTRING, p_end_scene VAOFNUMBER, p_mission_creator VAOFSTRING, p_reward VAOFNUMBER, p_root_script_name VAOFSTRING, p_start_x VAOFNUMBER, p_start_y VAOFNUMBER, p_start_z VAOFNUMBER, p_start_cell VAOFSTRING, p_start_scene VAOFNUMBER, p_description_table VAOFSTRING, p_description_text VAOFSTRING, p_title_table VAOFSTRING, p_title_text VAOFSTRING, p_mission_holder_id VAOFSTRING, p_status VAOFNUMBER, p_mission_type VAOFNUMBER, p_target_appearance VAOFNUMBER, p_target_name VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update mission_objects set
			mission_objects.difficulty = nvl(p_difficulty(i),mission_objects.difficulty),
			mission_objects.end_x = nvl(p_end_x(i),mission_objects.end_x),
			mission_objects.end_y = nvl(p_end_y(i),mission_objects.end_y),
			mission_objects.end_z = nvl(p_end_z(i),mission_objects.end_z),
			mission_objects.end_cell = nvl(p_end_cell(i),mission_objects.end_cell),
			mission_objects.end_scene = nvl(p_end_scene(i),mission_objects.end_scene),
			mission_objects.mission_creator = nvl(p_mission_creator(i),mission_objects.mission_creator),
			mission_objects.reward = nvl(p_reward(i),mission_objects.reward),
			mission_objects.root_script_name = nvl(p_root_script_name(i),mission_objects.root_script_name),
			mission_objects.start_x = nvl(p_start_x(i),mission_objects.start_x),
			mission_objects.start_y = nvl(p_start_y(i),mission_objects.start_y),
			mission_objects.start_z = nvl(p_start_z(i),mission_objects.start_z),
			mission_objects.start_cell = nvl(p_start_cell(i),mission_objects.start_cell),
			mission_objects.start_scene = nvl(p_start_scene(i),mission_objects.start_scene),
			mission_objects.description_table = nvl(p_description_table(i),mission_objects.description_table),
			mission_objects.description_text = nvl(p_description_text(i),mission_objects.description_text),
			mission_objects.title_table = nvl(p_title_table(i),mission_objects.title_table),
			mission_objects.title_text = nvl(p_title_text(i),mission_objects.title_text),
			mission_objects.mission_holder_id = nvl(p_mission_holder_id(i),mission_objects.mission_holder_id),
			mission_objects.status = nvl(p_status(i),mission_objects.status),
			mission_objects.mission_type = nvl(p_mission_type(i),mission_objects.mission_type),
			mission_objects.target_appearance = nvl(p_target_appearance(i),mission_objects.target_appearance),
			mission_objects.target_name = nvl(p_target_name(i),mission_objects.target_name)
		where
			mission_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update mission_objects set
					mission_objects.difficulty = nvl(p_difficulty(i),mission_objects.difficulty),
					mission_objects.end_x = nvl(p_end_x(i),mission_objects.end_x),
					mission_objects.end_y = nvl(p_end_y(i),mission_objects.end_y),
					mission_objects.end_z = nvl(p_end_z(i),mission_objects.end_z),
					mission_objects.end_cell = nvl(p_end_cell(i),mission_objects.end_cell),
					mission_objects.end_scene = nvl(p_end_scene(i),mission_objects.end_scene),
					mission_objects.mission_creator = nvl(p_mission_creator(i),mission_objects.mission_creator),
					mission_objects.reward = nvl(p_reward(i),mission_objects.reward),
					mission_objects.root_script_name = nvl(p_root_script_name(i),mission_objects.root_script_name),
					mission_objects.start_x = nvl(p_start_x(i),mission_objects.start_x),
					mission_objects.start_y = nvl(p_start_y(i),mission_objects.start_y),
					mission_objects.start_z = nvl(p_start_z(i),mission_objects.start_z),
					mission_objects.start_cell = nvl(p_start_cell(i),mission_objects.start_cell),
					mission_objects.start_scene = nvl(p_start_scene(i),mission_objects.start_scene),
					mission_objects.description_table = nvl(p_description_table(i),mission_objects.description_table),
					mission_objects.description_text = nvl(p_description_text(i),mission_objects.description_text),
					mission_objects.title_table = nvl(p_title_table(i),mission_objects.title_table),
					mission_objects.title_text = nvl(p_title_text(i),mission_objects.title_text),
					mission_objects.mission_holder_id = nvl(p_mission_holder_id(i),mission_objects.mission_holder_id),
					mission_objects.status = nvl(p_status(i),mission_objects.status),
					mission_objects.mission_type = nvl(p_mission_type(i),mission_objects.mission_type),
					mission_objects.target_appearance = nvl(p_target_appearance(i),mission_objects.target_appearance),
					mission_objects.target_name = nvl(p_target_name(i),mission_objects.target_name)
					where
					mission_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_mission_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_mission_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','difficulty','int',to_char(p_difficulty(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','end_x','float',to_char(p_end_x(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','end_y','float',to_char(p_end_y(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','end_z','float',to_char(p_end_z(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','end_cell','number',p_end_cell(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','end_scene','int',to_char(p_end_scene(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','mission_creator','varchar2',p_mission_creator(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','reward','int',to_char(p_reward(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','root_script_name','varchar2',p_root_script_name(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','start_x','float',to_char(p_start_x(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','start_y','float',to_char(p_start_y(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','start_z','float',to_char(p_start_z(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','start_cell','number',p_start_cell(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','start_scene','int',to_char(p_start_scene(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','description_table','varchar2',p_description_table(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','description_text','varchar2',p_description_text(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','title_table','varchar2',p_title_table(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','title_text','varchar2',p_title_text(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','mission_holder_id','number',p_mission_holder_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_mission_obj','status','int',to_char(p_status(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','mission_type','int',to_char(p_mission_type(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','target_appearance','int',to_char(p_target_appearance(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_mission_obj','target_name','varchar2',p_target_name(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_mission_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_mission_obj( p_object_id VAOFSTRING, p_difficulty VAOFNUMBER, p_end_x VAOFNUMBER, p_end_y VAOFNUMBER, p_end_z VAOFNUMBER, p_end_cell VAOFSTRING, p_end_scene VAOFNUMBER, p_mission_creator VAOFSTRING, p_reward VAOFNUMBER, p_root_script_name VAOFSTRING, p_start_x VAOFNUMBER, p_start_y VAOFNUMBER, p_start_z VAOFNUMBER, p_start_cell VAOFSTRING, p_start_scene VAOFNUMBER, p_description_table VAOFSTRING, p_description_text VAOFSTRING, p_title_table VAOFSTRING, p_title_text VAOFSTRING, p_mission_holder_id VAOFSTRING, p_status VAOFNUMBER, p_mission_type VAOFNUMBER, p_target_appearance VAOFNUMBER, p_target_name VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into mission_objects
		(
			mission_objects.difficulty,
			mission_objects.end_x,
			mission_objects.end_y,
			mission_objects.end_z,
			mission_objects.end_cell,
			mission_objects.end_scene,
			mission_objects.mission_creator,
			mission_objects.reward,
			mission_objects.root_script_name,
			mission_objects.start_x,
			mission_objects.start_y,
			mission_objects.start_z,
			mission_objects.start_cell,
			mission_objects.start_scene,
			mission_objects.description_table,
			mission_objects.description_text,
			mission_objects.title_table,
			mission_objects.title_text,
			mission_objects.mission_holder_id,
			mission_objects.status,
			mission_objects.mission_type,
			mission_objects.target_appearance,
			mission_objects.target_name,
			mission_objects.object_id
		)
		VALUES
		(
			p_difficulty(i),
			p_end_x(i),
			p_end_y(i),
			p_end_z(i),
			p_end_cell(i),
			p_end_scene(i),
			p_mission_creator(i),
			p_reward(i),
			p_root_script_name(i),
			p_start_x(i),
			p_start_y(i),
			p_start_z(i),
			p_start_cell(i),
			p_start_scene(i),
			p_description_table(i),
			p_description_text(i),
			p_title_table(i),
			p_title_text(i),
			p_mission_holder_id(i),
			p_status(i),
			p_mission_type(i),
			p_target_appearance(i),
			p_target_name(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_mission_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE mission_objects Set
					mission_objects.difficulty = nvl(p_difficulty(i),mission_objects.difficulty),
					mission_objects.end_x = nvl(p_end_x(i),mission_objects.end_x),
					mission_objects.end_y = nvl(p_end_y(i),mission_objects.end_y),
					mission_objects.end_z = nvl(p_end_z(i),mission_objects.end_z),
					mission_objects.end_cell = nvl(p_end_cell(i),mission_objects.end_cell),
					mission_objects.end_scene = nvl(p_end_scene(i),mission_objects.end_scene),
					mission_objects.mission_creator = nvl(p_mission_creator(i),mission_objects.mission_creator),
					mission_objects.reward = nvl(p_reward(i),mission_objects.reward),
					mission_objects.root_script_name = nvl(p_root_script_name(i),mission_objects.root_script_name),
					mission_objects.start_x = nvl(p_start_x(i),mission_objects.start_x),
					mission_objects.start_y = nvl(p_start_y(i),mission_objects.start_y),
					mission_objects.start_z = nvl(p_start_z(i),mission_objects.start_z),
					mission_objects.start_cell = nvl(p_start_cell(i),mission_objects.start_cell),
					mission_objects.start_scene = nvl(p_start_scene(i),mission_objects.start_scene),
					mission_objects.description_table = nvl(p_description_table(i),mission_objects.description_table),
					mission_objects.description_text = nvl(p_description_text(i),mission_objects.description_text),
					mission_objects.title_table = nvl(p_title_table(i),mission_objects.title_table),
					mission_objects.title_text = nvl(p_title_text(i),mission_objects.title_text),
					mission_objects.mission_holder_id = nvl(p_mission_holder_id(i),mission_objects.mission_holder_id),
					mission_objects.status = nvl(p_status(i),mission_objects.status),
					mission_objects.mission_type = nvl(p_mission_type(i),mission_objects.mission_type),
					mission_objects.target_appearance = nvl(p_target_appearance(i),mission_objects.target_appearance),
					mission_objects.target_name = nvl(p_target_name(i),mission_objects.target_name)
				where
					mission_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into mission_objects
					(
					mission_objects.difficulty,
					mission_objects.end_x,
					mission_objects.end_y,
					mission_objects.end_z,
					mission_objects.end_cell,
					mission_objects.end_scene,
					mission_objects.mission_creator,
					mission_objects.reward,
					mission_objects.root_script_name,
					mission_objects.start_x,
					mission_objects.start_y,
					mission_objects.start_z,
					mission_objects.start_cell,
					mission_objects.start_scene,
					mission_objects.description_table,
					mission_objects.description_text,
					mission_objects.title_table,
					mission_objects.title_text,
					mission_objects.mission_holder_id,
					mission_objects.status,
					mission_objects.mission_type,
					mission_objects.target_appearance,
					mission_objects.target_name,
					mission_objects.object_id
					)
					VALUES
					(
					p_difficulty(i),
					p_end_x(i),
					p_end_y(i),
					p_end_z(i),
					p_end_cell(i),
					p_end_scene(i),
					p_mission_creator(i),
					p_reward(i),
					p_root_script_name(i),
					p_start_x(i),
					p_start_y(i),
					p_start_z(i),
					p_start_cell(i),
					p_start_scene(i),
					p_description_table(i),
					p_description_text(i),
					p_title_table(i),
					p_title_text(i),
					p_mission_holder_id(i),
					p_status(i),
					p_mission_type(i),
					p_target_appearance(i),
					p_target_name(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into mission_objects
			(
				mission_objects.difficulty,
				mission_objects.end_x,
				mission_objects.end_y,
				mission_objects.end_z,
				mission_objects.end_cell,
				mission_objects.end_scene,
				mission_objects.mission_creator,
				mission_objects.reward,
				mission_objects.root_script_name,
				mission_objects.start_x,
				mission_objects.start_y,
				mission_objects.start_z,
				mission_objects.start_cell,
				mission_objects.start_scene,
				mission_objects.description_table,
				mission_objects.description_text,
				mission_objects.title_table,
				mission_objects.title_text,
				mission_objects.mission_holder_id,
				mission_objects.status,
				mission_objects.mission_type,
				mission_objects.target_appearance,
				mission_objects.target_name,
				mission_objects.object_id
			)
			VALUES
			(
				p_difficulty(i),
				p_end_x(i),
				p_end_y(i),
				p_end_z(i),
				p_end_cell(i),
				p_end_scene(i),
				p_mission_creator(i),
				p_reward(i),
				p_root_script_name(i),
				p_start_x(i),
				p_start_y(i),
				p_start_z(i),
				p_start_cell(i),
				p_start_scene(i),
				p_description_table(i),
				p_description_text(i),
				p_title_table(i),
				p_title_text(i),
				p_mission_holder_id(i),
				p_status(i),
				p_mission_type(i),
				p_target_appearance(i),
				p_target_name(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_mission_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_mission_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','difficulty','int',to_char(p_difficulty(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','end_x','float',to_char(p_end_x(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','end_y','float',to_char(p_end_y(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','end_z','float',to_char(p_end_z(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','end_cell','number',p_end_cell(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','end_scene','int',to_char(p_end_scene(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','mission_creator','varchar2',p_mission_creator(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','reward','int',to_char(p_reward(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','root_script_name','varchar2',p_root_script_name(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','start_x','float',to_char(p_start_x(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','start_y','float',to_char(p_start_y(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','start_z','float',to_char(p_start_z(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','start_cell','number',p_start_cell(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','start_scene','int',to_char(p_start_scene(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','description_table','varchar2',p_description_table(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','description_text','varchar2',p_description_text(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','title_table','varchar2',p_title_table(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','title_text','varchar2',p_title_text(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','mission_holder_id','number',p_mission_holder_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_mission_obj','status','int',to_char(p_status(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','mission_type','int',to_char(p_mission_type(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','target_appearance','int',to_char(p_target_appearance(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_mission_obj','target_name','varchar2',p_target_name(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_mission_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_mission_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete mission_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_planet_obj( p_object_id VAOFSTRING, p_planet_name VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update planet_objects set
			planet_objects.planet_name = nvl(p_planet_name(i),planet_objects.planet_name)
		where
			planet_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update planet_objects set
					planet_objects.planet_name = nvl(p_planet_name(i),planet_objects.planet_name)
					where
					planet_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_planet_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_planet_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_planet_obj','planet_name','varchar',p_planet_name(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_planet_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_planet_obj( p_object_id VAOFSTRING, p_planet_name VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into planet_objects
		(
			planet_objects.planet_name,
			planet_objects.object_id
		)
		VALUES
		(
			p_planet_name(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_planet_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE planet_objects Set
					planet_objects.planet_name = nvl(p_planet_name(i),planet_objects.planet_name)
				where
					planet_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into planet_objects
					(
					planet_objects.planet_name,
					planet_objects.object_id
					)
					VALUES
					(
					p_planet_name(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into planet_objects
			(
				planet_objects.planet_name,
				planet_objects.object_id
			)
			VALUES
			(
				p_planet_name(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_planet_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_planet_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_planet_obj','planet_name','varchar',p_planet_name(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_planet_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_planet_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete planet_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_resource_container_obj( p_object_id VAOFSTRING, p_resource_type VAOFSTRING, p_quantity VAOFNUMBER, p_source VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update resource_container_objects set
			resource_container_objects.resource_type = nvl(p_resource_type(i),resource_container_objects.resource_type),
			resource_container_objects.quantity = nvl(p_quantity(i),resource_container_objects.quantity),
			resource_container_objects.source = nvl(p_source(i),resource_container_objects.source)
		where
			resource_container_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update resource_container_objects set
					resource_container_objects.resource_type = nvl(p_resource_type(i),resource_container_objects.resource_type),
					resource_container_objects.quantity = nvl(p_quantity(i),resource_container_objects.quantity),
					resource_container_objects.source = nvl(p_source(i),resource_container_objects.source)
					where
					resource_container_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_resource_container_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_resource_container_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_resource_container_obj','resource_type','number',p_resource_type(m_error_index));
								db_error_logger.dblogerror_values('persister.save_resource_container_obj','quantity','int',to_char(p_quantity(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_resource_container_obj','source','number',p_source(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_resource_container_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_resource_container_obj( p_object_id VAOFSTRING, p_resource_type VAOFSTRING, p_quantity VAOFNUMBER, p_source VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into resource_container_objects
		(
			resource_container_objects.resource_type,
			resource_container_objects.quantity,
			resource_container_objects.source,
			resource_container_objects.object_id
		)
		VALUES
		(
			p_resource_type(i),
			p_quantity(i),
			p_source(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_resource_container_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE resource_container_objects Set
					resource_container_objects.resource_type = nvl(p_resource_type(i),resource_container_objects.resource_type),
					resource_container_objects.quantity = nvl(p_quantity(i),resource_container_objects.quantity),
					resource_container_objects.source = nvl(p_source(i),resource_container_objects.source)
				where
					resource_container_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into resource_container_objects
					(
					resource_container_objects.resource_type,
					resource_container_objects.quantity,
					resource_container_objects.source,
					resource_container_objects.object_id
					)
					VALUES
					(
					p_resource_type(i),
					p_quantity(i),
					p_source(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into resource_container_objects
			(
				resource_container_objects.resource_type,
				resource_container_objects.quantity,
				resource_container_objects.source,
				resource_container_objects.object_id
			)
			VALUES
			(
				p_resource_type(i),
				p_quantity(i),
				p_source(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_resource_container_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_resource_container_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_resource_container_obj','resource_type','number',p_resource_type(m_error_index));
						db_error_logger.dblogerror_values('persister.add_resource_container_obj','quantity','int',to_char(p_quantity(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_resource_container_obj','source','number',p_source(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_resource_container_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_resource_container_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete resource_container_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_ship_obj(p_object_id VAOFSTRING, p_slide_dampener VAOFNUMBER, p_current_chassis_hit_points VAOFNUMBER, p_maximum_chassis_hit_points VAOFNUMBER, p_chassis_type VAOFNUMBER, p_cmp_armor_hp_maximum VAOFSTRING, p_cmp_armor_hp_current VAOFSTRING, p_cmp_efficiency_general VAOFSTRING, p_cmp_efficiency_eng VAOFSTRING, p_cmp_eng_maintenance VAOFSTRING, p_cmp_mass VAOFSTRING, p_cmp_crc VAOFSTRING, p_cmp_hp_current VAOFSTRING, p_cmp_hp_maximum VAOFSTRING, p_cmp_flags VAOFSTRING, p_cmp_names VAOFLONGSTRING, p_weapon_damage_maximum VAOFSTRING, p_weapon_damage_minimum VAOFSTRING, p_weapon_effectiveness_shields VAOFSTRING, p_weapon_effectiveness_armor VAOFSTRING, p_weapon_eng_per_shot VAOFSTRING, p_weapon_refire_rate VAOFSTRING, p_weapon_ammo_current VAOFSTRING, p_weapon_ammo_maximum VAOFSTRING, p_weapon_ammo_type VAOFSTRING, p_shield_hp_front_maximum VAOFNUMBER, p_shield_hp_back_maximum VAOFNUMBER, p_shield_recharge_rate VAOFNUMBER, p_capacitor_eng_maximum VAOFNUMBER, p_capacitor_eng_recharge_rate VAOFNUMBER, p_engine_acc_rate VAOFNUMBER, p_engine_deceleration_rate VAOFNUMBER, p_engine_pitch_acc_rate VAOFNUMBER, p_engine_yaw_acc_rate VAOFNUMBER, p_engine_roll_acc_rate VAOFNUMBER, p_engine_pitch_rate_maximum VAOFNUMBER, p_engine_yaw_rate_maximum VAOFNUMBER, p_engine_roll_rate_maximum VAOFNUMBER, p_engine_speed_maximum VAOFNUMBER, p_reactor_eng_generation_rate VAOFNUMBER, p_booster_eng_maximum VAOFNUMBER, p_booster_eng_recharge_rate VAOFNUMBER, p_booster_eng_consumption_rate VAOFNUMBER, p_booster_acc VAOFNUMBER, p_booster_speed_maximum VAOFNUMBER, p_droid_if_cmd_speed VAOFNUMBER, p_installed_dcd VAOFSTRING, p_chassis_cmp_mass_maximum VAOFNUMBER, p_cmp_creators VAOFSTRING, p_cargo_hold_contents_maximum VAOFNUMBER, p_cargo_hold_contents_current VAOFNUMBER, p_cargo_hold_contents VAOFSTRING, p_chunk_size number)
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update ship_objects set
			ship_objects.slide_dampener = nvl(p_slide_dampener(i),ship_objects.slide_dampener),
			ship_objects.current_chassis_hit_points = nvl(p_current_chassis_hit_points(i),ship_objects.current_chassis_hit_points),
			ship_objects.maximum_chassis_hit_points = nvl(p_maximum_chassis_hit_points(i),ship_objects.maximum_chassis_hit_points),
			ship_objects.chassis_type = nvl(p_chassis_type(i),ship_objects.chassis_type),
			ship_objects.cmp_armor_hp_maximum = nvl(p_cmp_armor_hp_maximum(i),ship_objects.cmp_armor_hp_maximum),
			ship_objects.cmp_armor_hp_current = nvl(p_cmp_armor_hp_current(i),ship_objects.cmp_armor_hp_current),
			ship_objects.cmp_efficiency_general = nvl(p_cmp_efficiency_general(i),ship_objects.cmp_efficiency_general),
			ship_objects.cmp_efficiency_eng = nvl(p_cmp_efficiency_eng(i),ship_objects.cmp_efficiency_eng),
			ship_objects.cmp_eng_maintenance = nvl(p_cmp_eng_maintenance(i),ship_objects.cmp_eng_maintenance),
			ship_objects.cmp_mass = nvl(p_cmp_mass(i),ship_objects.cmp_mass),
			ship_objects.cmp_crc = nvl(p_cmp_crc(i),ship_objects.cmp_crc),
			ship_objects.cmp_hp_current = nvl(p_cmp_hp_current(i),ship_objects.cmp_hp_current),
			ship_objects.cmp_hp_maximum = nvl(p_cmp_hp_maximum(i),ship_objects.cmp_hp_maximum),
			ship_objects.cmp_flags = nvl(p_cmp_flags(i),ship_objects.cmp_flags),
			ship_objects.cmp_names = nvl(p_cmp_names(i),ship_objects.cmp_names),
			ship_objects.weapon_damage_maximum = nvl(p_weapon_damage_maximum(i),ship_objects.weapon_damage_maximum),
			ship_objects.weapon_damage_minimum = nvl(p_weapon_damage_minimum(i),ship_objects.weapon_damage_minimum),
			ship_objects.weapon_effectiveness_shields = nvl(p_weapon_effectiveness_shields(i),ship_objects.weapon_effectiveness_shields),
			ship_objects.weapon_effectiveness_armor = nvl(p_weapon_effectiveness_armor(i),ship_objects.weapon_effectiveness_armor),
			ship_objects.weapon_eng_per_shot = nvl(p_weapon_eng_per_shot(i),ship_objects.weapon_eng_per_shot),
			ship_objects.weapon_refire_rate = nvl(p_weapon_refire_rate(i),ship_objects.weapon_refire_rate),
			ship_objects.weapon_ammo_current = nvl(p_weapon_ammo_current(i),ship_objects.weapon_ammo_current),
			ship_objects.weapon_ammo_maximum = nvl(p_weapon_ammo_maximum(i),ship_objects.weapon_ammo_maximum),
			ship_objects.weapon_ammo_type = nvl(p_weapon_ammo_type(i),ship_objects.weapon_ammo_type),
			ship_objects.shield_hp_front_maximum = nvl(p_shield_hp_front_maximum(i),ship_objects.shield_hp_front_maximum),
			ship_objects.shield_hp_back_maximum = nvl(p_shield_hp_back_maximum(i),ship_objects.shield_hp_back_maximum),
			ship_objects.shield_recharge_rate = nvl(p_shield_recharge_rate(i),ship_objects.shield_recharge_rate),
			ship_objects.capacitor_eng_maximum = nvl(p_capacitor_eng_maximum(i),ship_objects.capacitor_eng_maximum),
			ship_objects.capacitor_eng_recharge_rate = nvl(p_capacitor_eng_recharge_rate(i),ship_objects.capacitor_eng_recharge_rate),
			ship_objects.engine_acc_rate = nvl(p_engine_acc_rate(i),ship_objects.engine_acc_rate),
			ship_objects.engine_deceleration_rate = nvl(p_engine_deceleration_rate(i),ship_objects.engine_deceleration_rate),
			ship_objects.engine_pitch_acc_rate = nvl(p_engine_pitch_acc_rate(i),ship_objects.engine_pitch_acc_rate),
			ship_objects.engine_yaw_acc_rate = nvl(p_engine_yaw_acc_rate(i),ship_objects.engine_yaw_acc_rate),
			ship_objects.engine_roll_acc_rate = nvl(p_engine_roll_acc_rate(i),ship_objects.engine_roll_acc_rate),
			ship_objects.engine_pitch_rate_maximum = nvl(p_engine_pitch_rate_maximum(i),ship_objects.engine_pitch_rate_maximum),
			ship_objects.engine_yaw_rate_maximum = nvl(p_engine_yaw_rate_maximum(i),ship_objects.engine_yaw_rate_maximum),
			ship_objects.engine_roll_rate_maximum = nvl(p_engine_roll_rate_maximum(i),ship_objects.engine_roll_rate_maximum),
			ship_objects.engine_speed_maximum = nvl(p_engine_speed_maximum(i),ship_objects.engine_speed_maximum),
			ship_objects.reactor_eng_generation_rate = nvl(p_reactor_eng_generation_rate(i),ship_objects.reactor_eng_generation_rate),
			ship_objects.booster_eng_maximum = nvl(p_booster_eng_maximum(i),ship_objects.booster_eng_maximum),
			ship_objects.booster_eng_recharge_rate = nvl(p_booster_eng_recharge_rate(i),ship_objects.booster_eng_recharge_rate),
			ship_objects.booster_eng_consumption_rate = nvl(p_booster_eng_consumption_rate(i),ship_objects.booster_eng_consumption_rate),
			ship_objects.booster_acc = nvl(p_booster_acc(i),ship_objects.booster_acc),
			ship_objects.booster_speed_maximum = nvl(p_booster_speed_maximum(i),ship_objects.booster_speed_maximum),
			ship_objects.droid_if_cmd_speed = nvl(p_droid_if_cmd_speed(i),ship_objects.droid_if_cmd_speed),
			ship_objects.installed_dcd = nvl(p_installed_dcd(i),ship_objects.installed_dcd),
			ship_objects.chassis_cmp_mass_maximum = nvl(p_chassis_cmp_mass_maximum(i),ship_objects.chassis_cmp_mass_maximum),
			ship_objects.cmp_creators = nvl(p_cmp_creators(i),ship_objects.cmp_creators),
			ship_objects.cargo_hold_contents_maximum = nvl(p_cargo_hold_contents_maximum(i),ship_objects.cargo_hold_contents_maximum),
			ship_objects.cargo_hold_contents_current = nvl(p_cargo_hold_contents_current(i),ship_objects.cargo_hold_contents_current),
			ship_objects.cargo_hold_contents = nvl(p_cargo_hold_contents(i),ship_objects.cargo_hold_contents)
		where
			ship_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update ship_objects set
					ship_objects.slide_dampener = nvl(p_slide_dampener(i),ship_objects.slide_dampener),
					ship_objects.current_chassis_hit_points = nvl(p_current_chassis_hit_points(i),ship_objects.current_chassis_hit_points),
					ship_objects.maximum_chassis_hit_points = nvl(p_maximum_chassis_hit_points(i),ship_objects.maximum_chassis_hit_points),
					ship_objects.chassis_type = nvl(p_chassis_type(i),ship_objects.chassis_type),
					ship_objects.cmp_armor_hp_maximum = nvl(p_cmp_armor_hp_maximum(i),ship_objects.cmp_armor_hp_maximum),
					ship_objects.cmp_armor_hp_current = nvl(p_cmp_armor_hp_current(i),ship_objects.cmp_armor_hp_current),
					ship_objects.cmp_efficiency_general = nvl(p_cmp_efficiency_general(i),ship_objects.cmp_efficiency_general),
					ship_objects.cmp_efficiency_eng = nvl(p_cmp_efficiency_eng(i),ship_objects.cmp_efficiency_eng),
					ship_objects.cmp_eng_maintenance = nvl(p_cmp_eng_maintenance(i),ship_objects.cmp_eng_maintenance),
					ship_objects.cmp_mass = nvl(p_cmp_mass(i),ship_objects.cmp_mass),
					ship_objects.cmp_crc = nvl(p_cmp_crc(i),ship_objects.cmp_crc),
					ship_objects.cmp_hp_current = nvl(p_cmp_hp_current(i),ship_objects.cmp_hp_current),
					ship_objects.cmp_hp_maximum = nvl(p_cmp_hp_maximum(i),ship_objects.cmp_hp_maximum),
					ship_objects.cmp_flags = nvl(p_cmp_flags(i),ship_objects.cmp_flags),
					ship_objects.cmp_names = nvl(p_cmp_names(i),ship_objects.cmp_names),
					ship_objects.weapon_damage_maximum = nvl(p_weapon_damage_maximum(i),ship_objects.weapon_damage_maximum),
					ship_objects.weapon_damage_minimum = nvl(p_weapon_damage_minimum(i),ship_objects.weapon_damage_minimum),
					ship_objects.weapon_effectiveness_shields = nvl(p_weapon_effectiveness_shields(i),ship_objects.weapon_effectiveness_shields),
					ship_objects.weapon_effectiveness_armor = nvl(p_weapon_effectiveness_armor(i),ship_objects.weapon_effectiveness_armor),
					ship_objects.weapon_eng_per_shot = nvl(p_weapon_eng_per_shot(i),ship_objects.weapon_eng_per_shot),
					ship_objects.weapon_refire_rate = nvl(p_weapon_refire_rate(i),ship_objects.weapon_refire_rate),
					ship_objects.weapon_ammo_current = nvl(p_weapon_ammo_current(i),ship_objects.weapon_ammo_current),
					ship_objects.weapon_ammo_maximum = nvl(p_weapon_ammo_maximum(i),ship_objects.weapon_ammo_maximum),
					ship_objects.weapon_ammo_type = nvl(p_weapon_ammo_type(i),ship_objects.weapon_ammo_type),
					ship_objects.shield_hp_front_maximum = nvl(p_shield_hp_front_maximum(i),ship_objects.shield_hp_front_maximum),
					ship_objects.shield_hp_back_maximum = nvl(p_shield_hp_back_maximum(i),ship_objects.shield_hp_back_maximum),
					ship_objects.shield_recharge_rate = nvl(p_shield_recharge_rate(i),ship_objects.shield_recharge_rate),
					ship_objects.capacitor_eng_maximum = nvl(p_capacitor_eng_maximum(i),ship_objects.capacitor_eng_maximum),
					ship_objects.capacitor_eng_recharge_rate = nvl(p_capacitor_eng_recharge_rate(i),ship_objects.capacitor_eng_recharge_rate),
					ship_objects.engine_acc_rate = nvl(p_engine_acc_rate(i),ship_objects.engine_acc_rate),
					ship_objects.engine_deceleration_rate = nvl(p_engine_deceleration_rate(i),ship_objects.engine_deceleration_rate),
					ship_objects.engine_pitch_acc_rate = nvl(p_engine_pitch_acc_rate(i),ship_objects.engine_pitch_acc_rate),
					ship_objects.engine_yaw_acc_rate = nvl(p_engine_yaw_acc_rate(i),ship_objects.engine_yaw_acc_rate),
					ship_objects.engine_roll_acc_rate = nvl(p_engine_roll_acc_rate(i),ship_objects.engine_roll_acc_rate),
					ship_objects.engine_pitch_rate_maximum = nvl(p_engine_pitch_rate_maximum(i),ship_objects.engine_pitch_rate_maximum),
					ship_objects.engine_yaw_rate_maximum = nvl(p_engine_yaw_rate_maximum(i),ship_objects.engine_yaw_rate_maximum),
					ship_objects.engine_roll_rate_maximum = nvl(p_engine_roll_rate_maximum(i),ship_objects.engine_roll_rate_maximum),
					ship_objects.engine_speed_maximum = nvl(p_engine_speed_maximum(i),ship_objects.engine_speed_maximum),
					ship_objects.reactor_eng_generation_rate = nvl(p_reactor_eng_generation_rate(i),ship_objects.reactor_eng_generation_rate),
					ship_objects.booster_eng_maximum = nvl(p_booster_eng_maximum(i),ship_objects.booster_eng_maximum),
					ship_objects.booster_eng_recharge_rate = nvl(p_booster_eng_recharge_rate(i),ship_objects.booster_eng_recharge_rate),
					ship_objects.booster_eng_consumption_rate = nvl(p_booster_eng_consumption_rate(i),ship_objects.booster_eng_consumption_rate),
					ship_objects.booster_acc = nvl(p_booster_acc(i),ship_objects.booster_acc),
					ship_objects.booster_speed_maximum = nvl(p_booster_speed_maximum(i),ship_objects.booster_speed_maximum),
					ship_objects.droid_if_cmd_speed = nvl(p_droid_if_cmd_speed(i),ship_objects.droid_if_cmd_speed),
					ship_objects.installed_dcd = nvl(p_installed_dcd(i),ship_objects.installed_dcd),
					ship_objects.chassis_cmp_mass_maximum = nvl(p_chassis_cmp_mass_maximum(i),ship_objects.chassis_cmp_mass_maximum),
					ship_objects.cmp_creators = nvl(p_cmp_creators(i),ship_objects.cmp_creators),
					ship_objects.cargo_hold_contents_maximum = nvl(p_cargo_hold_contents_maximum(i),ship_objects.cargo_hold_contents_maximum),
					ship_objects.cargo_hold_contents_current = nvl(p_cargo_hold_contents_current(i),ship_objects.cargo_hold_contents_current),
					ship_objects.cargo_hold_contents = nvl(p_cargo_hold_contents(i),ship_objects.cargo_hold_contents)
					where
					ship_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_ship_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_ship_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','slide_dampener','float',to_char(p_slide_dampener(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','current_chassis_hit_points','float',to_char(p_current_chassis_hit_points(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','maximum_chassis_hit_points','float',to_char(p_maximum_chassis_hit_points(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','chassis_type','int',to_char(p_chassis_type(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_armor_hp_maximum','varchar2',p_cmp_armor_hp_maximum(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_armor_hp_current','varchar2',p_cmp_armor_hp_current(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_efficiency_general','varchar2',p_cmp_efficiency_general(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_efficiency_eng','varchar2',p_cmp_efficiency_eng(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_eng_maintenance','varchar2',p_cmp_eng_maintenance(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_mass','varchar2',p_cmp_mass(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_crc','varchar2',p_cmp_crc(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_hp_current','varchar2',p_cmp_hp_current(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_hp_maximum','varchar2',p_cmp_hp_maximum(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_flags','varchar2',p_cmp_flags(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_names','varchar2',p_cmp_names(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_damage_maximum','varchar2',p_weapon_damage_maximum(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_damage_minimum','varchar2',p_weapon_damage_minimum(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_effectiveness_shields','varchar2',p_weapon_effectiveness_shields(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_effectiveness_armor','varchar2',p_weapon_effectiveness_armor(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_eng_per_shot','varchar2',p_weapon_eng_per_shot(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_refire_rate','varchar2',p_weapon_refire_rate(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_ammo_current','varchar2',p_weapon_ammo_current(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_ammo_maximum','varchar2',p_weapon_ammo_maximum(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','weapon_ammo_type','varchar2',p_weapon_ammo_type(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','shield_hp_front_maximum','float',to_char(p_shield_hp_front_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','shield_hp_back_maximum','float',to_char(p_shield_hp_back_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','shield_recharge_rate','float',to_char(p_shield_recharge_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','capacitor_eng_maximum','float',to_char(p_capacitor_eng_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','capacitor_eng_recharge_rate','float',to_char(p_capacitor_eng_recharge_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_acc_rate','float',to_char(p_engine_acc_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_deceleration_rate','float',to_char(p_engine_deceleration_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_pitch_acc_rate','float',to_char(p_engine_pitch_acc_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_yaw_acc_rate','float',to_char(p_engine_yaw_acc_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_roll_acc_rate','float',to_char(p_engine_roll_acc_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_pitch_rate_maximum','float',to_char(p_engine_pitch_rate_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_yaw_rate_maximum','float',to_char(p_engine_yaw_rate_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_roll_rate_maximum','float',to_char(p_engine_roll_rate_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','engine_speed_maximum','float',to_char(p_engine_speed_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','reactor_eng_generation_rate','float',to_char(p_reactor_eng_generation_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','booster_eng_maximum','float',to_char(p_booster_eng_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','booster_eng_recharge_rate','float',to_char(p_booster_eng_recharge_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','booster_eng_consumption_rate','float',to_char(p_booster_eng_consumption_rate(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','booster_acc','float',to_char(p_booster_acc(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','booster_speed_maximum','float',to_char(p_booster_speed_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','droid_if_cmd_speed','float',to_char(p_droid_if_cmd_speed(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','installed_dcd','number',p_installed_dcd(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','chassis_cmp_mass_maximum','float',to_char(p_chassis_cmp_mass_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cmp_creators','varchar2',p_cmp_creators(m_error_index));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cargo_hold_contents_maximum','number',to_char(p_cargo_hold_contents_maximum(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cargo_hold_contents_current','number',to_char(p_cargo_hold_contents_current(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_ship_obj','cargo_hold_contents','varchar2',p_cargo_hold_contents(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_ship_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_ship_obj(p_object_id VAOFSTRING, p_slide_dampener VAOFNUMBER, p_current_chassis_hit_points VAOFNUMBER, p_maximum_chassis_hit_points VAOFNUMBER, p_chassis_type VAOFNUMBER, p_cmp_armor_hp_maximum VAOFSTRING, p_cmp_armor_hp_current VAOFSTRING, p_cmp_efficiency_general VAOFSTRING, p_cmp_efficiency_eng VAOFSTRING, p_cmp_eng_maintenance VAOFSTRING, p_cmp_mass VAOFSTRING, p_cmp_crc VAOFSTRING, p_cmp_hp_current VAOFSTRING, p_cmp_hp_maximum VAOFSTRING, p_cmp_flags VAOFSTRING, p_cmp_names VAOFLONGSTRING, p_weapon_damage_maximum VAOFSTRING, p_weapon_damage_minimum VAOFSTRING, p_weapon_effectiveness_shields VAOFSTRING, p_weapon_effectiveness_armor VAOFSTRING, p_weapon_eng_per_shot VAOFSTRING, p_weapon_refire_rate VAOFSTRING, p_weapon_ammo_current VAOFSTRING, p_weapon_ammo_maximum VAOFSTRING, p_weapon_ammo_type VAOFSTRING, p_shield_hp_front_maximum VAOFNUMBER, p_shield_hp_back_maximum VAOFNUMBER, p_shield_recharge_rate VAOFNUMBER, p_capacitor_eng_maximum VAOFNUMBER, p_capacitor_eng_recharge_rate VAOFNUMBER, p_engine_acc_rate VAOFNUMBER, p_engine_deceleration_rate VAOFNUMBER, p_engine_pitch_acc_rate VAOFNUMBER, p_engine_yaw_acc_rate VAOFNUMBER, p_engine_roll_acc_rate VAOFNUMBER, p_engine_pitch_rate_maximum VAOFNUMBER, p_engine_yaw_rate_maximum VAOFNUMBER, p_engine_roll_rate_maximum VAOFNUMBER, p_engine_speed_maximum VAOFNUMBER, p_reactor_eng_generation_rate VAOFNUMBER, p_booster_eng_maximum VAOFNUMBER, p_booster_eng_recharge_rate VAOFNUMBER, p_booster_eng_consumption_rate VAOFNUMBER, p_booster_acc VAOFNUMBER, p_booster_speed_maximum VAOFNUMBER, p_droid_if_cmd_speed VAOFNUMBER, p_installed_dcd VAOFSTRING, p_chassis_cmp_mass_maximum VAOFNUMBER, p_cmp_creators VAOFSTRING, p_cargo_hold_contents_maximum VAOFNUMBER, p_cargo_hold_contents_current VAOFNUMBER, p_cargo_hold_contents VAOFSTRING, p_chunk_size number)
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into ship_objects
		(
			ship_objects.slide_dampener,
			ship_objects.current_chassis_hit_points,
			ship_objects.maximum_chassis_hit_points,
			ship_objects.chassis_type,
			ship_objects.cmp_armor_hp_maximum,
			ship_objects.cmp_armor_hp_current,
			ship_objects.cmp_efficiency_general,
			ship_objects.cmp_efficiency_eng,
			ship_objects.cmp_eng_maintenance,
			ship_objects.cmp_mass,
			ship_objects.cmp_crc,
			ship_objects.cmp_hp_current,
			ship_objects.cmp_hp_maximum,
			ship_objects.cmp_flags,
			ship_objects.cmp_names,
			ship_objects.weapon_damage_maximum,
			ship_objects.weapon_damage_minimum,
			ship_objects.weapon_effectiveness_shields,
			ship_objects.weapon_effectiveness_armor,
			ship_objects.weapon_eng_per_shot,
			ship_objects.weapon_refire_rate,
			ship_objects.weapon_ammo_current,
			ship_objects.weapon_ammo_maximum,
			ship_objects.weapon_ammo_type,
			ship_objects.shield_hp_front_maximum,
			ship_objects.shield_hp_back_maximum,
			ship_objects.shield_recharge_rate,
			ship_objects.capacitor_eng_maximum,
			ship_objects.capacitor_eng_recharge_rate,
			ship_objects.engine_acc_rate,
			ship_objects.engine_deceleration_rate,
			ship_objects.engine_pitch_acc_rate,
			ship_objects.engine_yaw_acc_rate,
			ship_objects.engine_roll_acc_rate,
			ship_objects.engine_pitch_rate_maximum,
			ship_objects.engine_yaw_rate_maximum,
			ship_objects.engine_roll_rate_maximum,
			ship_objects.engine_speed_maximum,
			ship_objects.reactor_eng_generation_rate,
			ship_objects.booster_eng_maximum,
			ship_objects.booster_eng_recharge_rate,
			ship_objects.booster_eng_consumption_rate,
			ship_objects.booster_acc,
			ship_objects.booster_speed_maximum,
			ship_objects.droid_if_cmd_speed,
			ship_objects.installed_dcd,
			ship_objects.chassis_cmp_mass_maximum,
			ship_objects.cmp_creators,
			ship_objects.cargo_hold_contents_maximum,
			ship_objects.cargo_hold_contents_current,
			ship_objects.cargo_hold_contents,
			ship_objects.object_id
		)
		VALUES
		(
			p_slide_dampener(i),
			p_current_chassis_hit_points(i),
			p_maximum_chassis_hit_points(i),
			p_chassis_type(i),
			p_cmp_armor_hp_maximum(i),
			p_cmp_armor_hp_current(i),
			p_cmp_efficiency_general(i),
			p_cmp_efficiency_eng(i),
			p_cmp_eng_maintenance(i),
			p_cmp_mass(i),
			p_cmp_crc(i),
			p_cmp_hp_current(i),
			p_cmp_hp_maximum(i),
			p_cmp_flags(i),
			p_cmp_names(i),
			p_weapon_damage_maximum(i),
			p_weapon_damage_minimum(i),
			p_weapon_effectiveness_shields(i),
			p_weapon_effectiveness_armor(i),
			p_weapon_eng_per_shot(i),
			p_weapon_refire_rate(i),
			p_weapon_ammo_current(i),
			p_weapon_ammo_maximum(i),
			p_weapon_ammo_type(i),
			p_shield_hp_front_maximum(i),
			p_shield_hp_back_maximum(i),
			p_shield_recharge_rate(i),
			p_capacitor_eng_maximum(i),
			p_capacitor_eng_recharge_rate(i),
			p_engine_acc_rate(i),
			p_engine_deceleration_rate(i),
			p_engine_pitch_acc_rate(i),
			p_engine_yaw_acc_rate(i),
			p_engine_roll_acc_rate(i),
			p_engine_pitch_rate_maximum(i),
			p_engine_yaw_rate_maximum(i),
			p_engine_roll_rate_maximum(i),
			p_engine_speed_maximum(i),
			p_reactor_eng_generation_rate(i),
			p_booster_eng_maximum(i),
			p_booster_eng_recharge_rate(i),
			p_booster_eng_consumption_rate(i),
			p_booster_acc(i),
			p_booster_speed_maximum(i),
			p_droid_if_cmd_speed(i),
			p_installed_dcd(i),
			p_chassis_cmp_mass_maximum(i),
			p_cmp_creators(i),
			p_cargo_hold_contents_maximum(i),
			p_cargo_hold_contents_current(i),
			p_cargo_hold_contents(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_ship_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE ship_objects Set
					ship_objects.slide_dampener = nvl(p_slide_dampener(i),ship_objects.slide_dampener),
					ship_objects.current_chassis_hit_points = nvl(p_current_chassis_hit_points(i),ship_objects.current_chassis_hit_points),
					ship_objects.maximum_chassis_hit_points = nvl(p_maximum_chassis_hit_points(i),ship_objects.maximum_chassis_hit_points),
					ship_objects.chassis_type = nvl(p_chassis_type(i),ship_objects.chassis_type),
					ship_objects.cmp_armor_hp_maximum = nvl(p_cmp_armor_hp_maximum(i),ship_objects.cmp_armor_hp_maximum),
					ship_objects.cmp_armor_hp_current = nvl(p_cmp_armor_hp_current(i),ship_objects.cmp_armor_hp_current),
					ship_objects.cmp_efficiency_general = nvl(p_cmp_efficiency_general(i),ship_objects.cmp_efficiency_general),
					ship_objects.cmp_efficiency_eng = nvl(p_cmp_efficiency_eng(i),ship_objects.cmp_efficiency_eng),
					ship_objects.cmp_eng_maintenance = nvl(p_cmp_eng_maintenance(i),ship_objects.cmp_eng_maintenance),
					ship_objects.cmp_mass = nvl(p_cmp_mass(i),ship_objects.cmp_mass),
					ship_objects.cmp_crc = nvl(p_cmp_crc(i),ship_objects.cmp_crc),
					ship_objects.cmp_hp_current = nvl(p_cmp_hp_current(i),ship_objects.cmp_hp_current),
					ship_objects.cmp_hp_maximum = nvl(p_cmp_hp_maximum(i),ship_objects.cmp_hp_maximum),
					ship_objects.cmp_flags = nvl(p_cmp_flags(i),ship_objects.cmp_flags),
					ship_objects.cmp_names = nvl(p_cmp_names(i),ship_objects.cmp_names),
					ship_objects.weapon_damage_maximum = nvl(p_weapon_damage_maximum(i),ship_objects.weapon_damage_maximum),
					ship_objects.weapon_damage_minimum = nvl(p_weapon_damage_minimum(i),ship_objects.weapon_damage_minimum),
					ship_objects.weapon_effectiveness_shields = nvl(p_weapon_effectiveness_shields(i),ship_objects.weapon_effectiveness_shields),
					ship_objects.weapon_effectiveness_armor = nvl(p_weapon_effectiveness_armor(i),ship_objects.weapon_effectiveness_armor),
					ship_objects.weapon_eng_per_shot = nvl(p_weapon_eng_per_shot(i),ship_objects.weapon_eng_per_shot),
					ship_objects.weapon_refire_rate = nvl(p_weapon_refire_rate(i),ship_objects.weapon_refire_rate),
					ship_objects.weapon_ammo_current = nvl(p_weapon_ammo_current(i),ship_objects.weapon_ammo_current),
					ship_objects.weapon_ammo_maximum = nvl(p_weapon_ammo_maximum(i),ship_objects.weapon_ammo_maximum),
					ship_objects.weapon_ammo_type = nvl(p_weapon_ammo_type(i),ship_objects.weapon_ammo_type),
					ship_objects.shield_hp_front_maximum = nvl(p_shield_hp_front_maximum(i),ship_objects.shield_hp_front_maximum),
					ship_objects.shield_hp_back_maximum = nvl(p_shield_hp_back_maximum(i),ship_objects.shield_hp_back_maximum),
					ship_objects.shield_recharge_rate = nvl(p_shield_recharge_rate(i),ship_objects.shield_recharge_rate),
					ship_objects.capacitor_eng_maximum = nvl(p_capacitor_eng_maximum(i),ship_objects.capacitor_eng_maximum),
					ship_objects.capacitor_eng_recharge_rate = nvl(p_capacitor_eng_recharge_rate(i),ship_objects.capacitor_eng_recharge_rate),
					ship_objects.engine_acc_rate = nvl(p_engine_acc_rate(i),ship_objects.engine_acc_rate),
					ship_objects.engine_deceleration_rate = nvl(p_engine_deceleration_rate(i),ship_objects.engine_deceleration_rate),
					ship_objects.engine_pitch_acc_rate = nvl(p_engine_pitch_acc_rate(i),ship_objects.engine_pitch_acc_rate),
					ship_objects.engine_yaw_acc_rate = nvl(p_engine_yaw_acc_rate(i),ship_objects.engine_yaw_acc_rate),
					ship_objects.engine_roll_acc_rate = nvl(p_engine_roll_acc_rate(i),ship_objects.engine_roll_acc_rate),
					ship_objects.engine_pitch_rate_maximum = nvl(p_engine_pitch_rate_maximum(i),ship_objects.engine_pitch_rate_maximum),
					ship_objects.engine_yaw_rate_maximum = nvl(p_engine_yaw_rate_maximum(i),ship_objects.engine_yaw_rate_maximum),
					ship_objects.engine_roll_rate_maximum = nvl(p_engine_roll_rate_maximum(i),ship_objects.engine_roll_rate_maximum),
					ship_objects.engine_speed_maximum = nvl(p_engine_speed_maximum(i),ship_objects.engine_speed_maximum),
					ship_objects.reactor_eng_generation_rate = nvl(p_reactor_eng_generation_rate(i),ship_objects.reactor_eng_generation_rate),
					ship_objects.booster_eng_maximum = nvl(p_booster_eng_maximum(i),ship_objects.booster_eng_maximum),
					ship_objects.booster_eng_recharge_rate = nvl(p_booster_eng_recharge_rate(i),ship_objects.booster_eng_recharge_rate),
					ship_objects.booster_eng_consumption_rate = nvl(p_booster_eng_consumption_rate(i),ship_objects.booster_eng_consumption_rate),
					ship_objects.booster_acc = nvl(p_booster_acc(i),ship_objects.booster_acc),
					ship_objects.booster_speed_maximum = nvl(p_booster_speed_maximum(i),ship_objects.booster_speed_maximum),
					ship_objects.droid_if_cmd_speed = nvl(p_droid_if_cmd_speed(i),ship_objects.droid_if_cmd_speed),
					ship_objects.installed_dcd = nvl(p_installed_dcd(i),ship_objects.installed_dcd),
					ship_objects.chassis_cmp_mass_maximum = nvl(p_chassis_cmp_mass_maximum(i),ship_objects.chassis_cmp_mass_maximum),
					ship_objects.cmp_creators = nvl(p_cmp_creators(i),ship_objects.cmp_creators),
					ship_objects.cargo_hold_contents_maximum = nvl(p_cargo_hold_contents_maximum(i),ship_objects.cargo_hold_contents_maximum),
					ship_objects.cargo_hold_contents_current = nvl(p_cargo_hold_contents_current(i),ship_objects.cargo_hold_contents_current),
					ship_objects.cargo_hold_contents = nvl(p_cargo_hold_contents(i),ship_objects.cargo_hold_contents)
				where
					ship_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into ship_objects
					(
					ship_objects.slide_dampener,
					ship_objects.current_chassis_hit_points,
					ship_objects.maximum_chassis_hit_points,
					ship_objects.chassis_type,
					ship_objects.cmp_armor_hp_maximum,
					ship_objects.cmp_armor_hp_current,
					ship_objects.cmp_efficiency_general,
					ship_objects.cmp_efficiency_eng,
					ship_objects.cmp_eng_maintenance,
					ship_objects.cmp_mass,
					ship_objects.cmp_crc,
					ship_objects.cmp_hp_current,
					ship_objects.cmp_hp_maximum,
					ship_objects.cmp_flags,
					ship_objects.cmp_names,
					ship_objects.weapon_damage_maximum,
					ship_objects.weapon_damage_minimum,
					ship_objects.weapon_effectiveness_shields,
					ship_objects.weapon_effectiveness_armor,
					ship_objects.weapon_eng_per_shot,
					ship_objects.weapon_refire_rate,
					ship_objects.weapon_ammo_current,
					ship_objects.weapon_ammo_maximum,
					ship_objects.weapon_ammo_type,
					ship_objects.shield_hp_front_maximum,
					ship_objects.shield_hp_back_maximum,
					ship_objects.shield_recharge_rate,
					ship_objects.capacitor_eng_maximum,
					ship_objects.capacitor_eng_recharge_rate,
					ship_objects.engine_acc_rate,
					ship_objects.engine_deceleration_rate,
					ship_objects.engine_pitch_acc_rate,
					ship_objects.engine_yaw_acc_rate,
					ship_objects.engine_roll_acc_rate,
					ship_objects.engine_pitch_rate_maximum,
					ship_objects.engine_yaw_rate_maximum,
					ship_objects.engine_roll_rate_maximum,
					ship_objects.engine_speed_maximum,
					ship_objects.reactor_eng_generation_rate,
					ship_objects.booster_eng_maximum,
					ship_objects.booster_eng_recharge_rate,
					ship_objects.booster_eng_consumption_rate,
					ship_objects.booster_acc,
					ship_objects.booster_speed_maximum,
					ship_objects.droid_if_cmd_speed,
					ship_objects.installed_dcd,
					ship_objects.chassis_cmp_mass_maximum,
					ship_objects.cmp_creators,
					ship_objects.cargo_hold_contents_maximum,
					ship_objects.cargo_hold_contents_current,
					ship_objects.cargo_hold_contents,
					ship_objects.object_id
					)
					VALUES
					(
					p_slide_dampener(i),
					p_current_chassis_hit_points(i),
					p_maximum_chassis_hit_points(i),
					p_chassis_type(i),
					p_cmp_armor_hp_maximum(i),
					p_cmp_armor_hp_current(i),
					p_cmp_efficiency_general(i),
					p_cmp_efficiency_eng(i),
					p_cmp_eng_maintenance(i),
					p_cmp_mass(i),
					p_cmp_crc(i),
					p_cmp_hp_current(i),
					p_cmp_hp_maximum(i),
					p_cmp_flags(i),
					p_cmp_names(i),
					p_weapon_damage_maximum(i),
					p_weapon_damage_minimum(i),
					p_weapon_effectiveness_shields(i),
					p_weapon_effectiveness_armor(i),
					p_weapon_eng_per_shot(i),
					p_weapon_refire_rate(i),
					p_weapon_ammo_current(i),
					p_weapon_ammo_maximum(i),
					p_weapon_ammo_type(i),
					p_shield_hp_front_maximum(i),
					p_shield_hp_back_maximum(i),
					p_shield_recharge_rate(i),
					p_capacitor_eng_maximum(i),
					p_capacitor_eng_recharge_rate(i),
					p_engine_acc_rate(i),
					p_engine_deceleration_rate(i),
					p_engine_pitch_acc_rate(i),
					p_engine_yaw_acc_rate(i),
					p_engine_roll_acc_rate(i),
					p_engine_pitch_rate_maximum(i),
					p_engine_yaw_rate_maximum(i),
					p_engine_roll_rate_maximum(i),
					p_engine_speed_maximum(i),
					p_reactor_eng_generation_rate(i),
					p_booster_eng_maximum(i),
					p_booster_eng_recharge_rate(i),
					p_booster_eng_consumption_rate(i),
					p_booster_acc(i),
					p_booster_speed_maximum(i),
					p_droid_if_cmd_speed(i),
					p_installed_dcd(i),
					p_chassis_cmp_mass_maximum(i),
					p_cmp_creators(i),
					p_cargo_hold_contents_maximum(i),
					p_cargo_hold_contents_current(i),
					p_cargo_hold_contents(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into ship_objects
			(
				ship_objects.slide_dampener,
				ship_objects.current_chassis_hit_points,
				ship_objects.maximum_chassis_hit_points,
				ship_objects.chassis_type,
				ship_objects.cmp_armor_hp_maximum,
				ship_objects.cmp_armor_hp_current,
				ship_objects.cmp_efficiency_general,
				ship_objects.cmp_efficiency_eng,
				ship_objects.cmp_eng_maintenance,
				ship_objects.cmp_mass,
				ship_objects.cmp_crc,
				ship_objects.cmp_hp_current,
				ship_objects.cmp_hp_maximum,
				ship_objects.cmp_flags,
				ship_objects.cmp_names,
				ship_objects.weapon_damage_maximum,
				ship_objects.weapon_damage_minimum,
				ship_objects.weapon_effectiveness_shields,
				ship_objects.weapon_effectiveness_armor,
				ship_objects.weapon_eng_per_shot,
				ship_objects.weapon_refire_rate,
				ship_objects.weapon_ammo_current,
				ship_objects.weapon_ammo_maximum,
				ship_objects.weapon_ammo_type,
				ship_objects.shield_hp_front_maximum,
				ship_objects.shield_hp_back_maximum,
				ship_objects.shield_recharge_rate,
				ship_objects.capacitor_eng_maximum,
				ship_objects.capacitor_eng_recharge_rate,
				ship_objects.engine_acc_rate,
				ship_objects.engine_deceleration_rate,
				ship_objects.engine_pitch_acc_rate,
				ship_objects.engine_yaw_acc_rate,
				ship_objects.engine_roll_acc_rate,
				ship_objects.engine_pitch_rate_maximum,
				ship_objects.engine_yaw_rate_maximum,
				ship_objects.engine_roll_rate_maximum,
				ship_objects.engine_speed_maximum,
				ship_objects.reactor_eng_generation_rate,
				ship_objects.booster_eng_maximum,
				ship_objects.booster_eng_recharge_rate,
				ship_objects.booster_eng_consumption_rate,
				ship_objects.booster_acc,
				ship_objects.booster_speed_maximum,
				ship_objects.droid_if_cmd_speed,
				ship_objects.installed_dcd,
				ship_objects.chassis_cmp_mass_maximum,
				ship_objects.cmp_creators,
				ship_objects.cargo_hold_contents_maximum,
				ship_objects.cargo_hold_contents_current,
				ship_objects.cargo_hold_contents,
				ship_objects.object_id
			)
			VALUES
			(
				p_slide_dampener(i),
				p_current_chassis_hit_points(i),
				p_maximum_chassis_hit_points(i),
				p_chassis_type(i),
				p_cmp_armor_hp_maximum(i),
				p_cmp_armor_hp_current(i),
				p_cmp_efficiency_general(i),
				p_cmp_efficiency_eng(i),
				p_cmp_eng_maintenance(i),
				p_cmp_mass(i),
				p_cmp_crc(i),
				p_cmp_hp_current(i),
				p_cmp_hp_maximum(i),
				p_cmp_flags(i),
				p_cmp_names(i),
				p_weapon_damage_maximum(i),
				p_weapon_damage_minimum(i),
				p_weapon_effectiveness_shields(i),
				p_weapon_effectiveness_armor(i),
				p_weapon_eng_per_shot(i),
				p_weapon_refire_rate(i),
				p_weapon_ammo_current(i),
				p_weapon_ammo_maximum(i),
				p_weapon_ammo_type(i),
				p_shield_hp_front_maximum(i),
				p_shield_hp_back_maximum(i),
				p_shield_recharge_rate(i),
				p_capacitor_eng_maximum(i),
				p_capacitor_eng_recharge_rate(i),
				p_engine_acc_rate(i),
				p_engine_deceleration_rate(i),
				p_engine_pitch_acc_rate(i),
				p_engine_yaw_acc_rate(i),
				p_engine_roll_acc_rate(i),
				p_engine_pitch_rate_maximum(i),
				p_engine_yaw_rate_maximum(i),
				p_engine_roll_rate_maximum(i),
				p_engine_speed_maximum(i),
				p_reactor_eng_generation_rate(i),
				p_booster_eng_maximum(i),
				p_booster_eng_recharge_rate(i),
				p_booster_eng_consumption_rate(i),
				p_booster_acc(i),
				p_booster_speed_maximum(i),
				p_droid_if_cmd_speed(i),
				p_installed_dcd(i),
				p_chassis_cmp_mass_maximum(i),
				p_cmp_creators(i),
				p_cargo_hold_contents_maximum(i),
				p_cargo_hold_contents_current(i),
				p_cargo_hold_contents(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_ship_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_ship_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','slide_dampener','float',to_char(p_slide_dampener(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','current_chassis_hit_points','float',to_char(p_current_chassis_hit_points(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','maximum_chassis_hit_points','float',to_char(p_maximum_chassis_hit_points(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','chassis_type','int',to_char(p_chassis_type(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_armor_hp_maximum','varchar2',p_cmp_armor_hp_maximum(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_armor_hp_current','varchar2',p_cmp_armor_hp_current(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_efficiency_general','varchar2',p_cmp_efficiency_general(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_efficiency_eng','varchar2',p_cmp_efficiency_eng(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_eng_maintenance','varchar2',p_cmp_eng_maintenance(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_mass','varchar2',p_cmp_mass(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_crc','varchar2',p_cmp_crc(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_hp_current','varchar2',p_cmp_hp_current(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_hp_maximum','varchar2',p_cmp_hp_maximum(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_flags','varchar2',p_cmp_flags(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_names','varchar2',p_cmp_names(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_damage_maximum','varchar2',p_weapon_damage_maximum(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_damage_minimum','varchar2',p_weapon_damage_minimum(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_effectiveness_shields','varchar2',p_weapon_effectiveness_shields(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_effectiveness_armor','varchar2',p_weapon_effectiveness_armor(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_eng_per_shot','varchar2',p_weapon_eng_per_shot(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_refire_rate','varchar2',p_weapon_refire_rate(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_ammo_current','varchar2',p_weapon_ammo_current(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_ammo_maximum','varchar2',p_weapon_ammo_maximum(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','weapon_ammo_type','varchar2',p_weapon_ammo_type(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','shield_hp_front_maximum','float',to_char(p_shield_hp_front_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','shield_hp_back_maximum','float',to_char(p_shield_hp_back_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','shield_recharge_rate','float',to_char(p_shield_recharge_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','capacitor_eng_maximum','float',to_char(p_capacitor_eng_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','capacitor_eng_recharge_rate','float',to_char(p_capacitor_eng_recharge_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_acc_rate','float',to_char(p_engine_acc_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_deceleration_rate','float',to_char(p_engine_deceleration_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_pitch_acc_rate','float',to_char(p_engine_pitch_acc_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_yaw_acc_rate','float',to_char(p_engine_yaw_acc_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_roll_acc_rate','float',to_char(p_engine_roll_acc_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_pitch_rate_maximum','float',to_char(p_engine_pitch_rate_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_yaw_rate_maximum','float',to_char(p_engine_yaw_rate_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_roll_rate_maximum','float',to_char(p_engine_roll_rate_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','engine_speed_maximum','float',to_char(p_engine_speed_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','reactor_eng_generation_rate','float',to_char(p_reactor_eng_generation_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','booster_eng_maximum','float',to_char(p_booster_eng_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','booster_eng_recharge_rate','float',to_char(p_booster_eng_recharge_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','booster_eng_consumption_rate','float',to_char(p_booster_eng_consumption_rate(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','booster_acc','float',to_char(p_booster_acc(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','booster_speed_maximum','float',to_char(p_booster_speed_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','droid_if_cmd_speed','float',to_char(p_droid_if_cmd_speed(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','installed_dcd','number',p_installed_dcd(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','chassis_cmp_mass_maximum','float',to_char(p_chassis_cmp_mass_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cmp_creators','varchar2',p_cmp_creators(m_error_index));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cargo_hold_contents_maximum','number',to_char(p_cargo_hold_contents_maximum(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cargo_hold_contents_current','number',to_char(p_cargo_hold_contents_current(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_ship_obj','cargo_hold_contents','varchar2',p_cargo_hold_contents(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_ship_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_ship_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete ship_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_static_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		null;
	end;

	procedure add_static_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into static_objects
		(
			static_objects.object_id
		)
		VALUES
		(
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_static_obj : dup_val_on_index error.');
			END IF;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into static_objects
			(
				static_objects.object_id
			)
			VALUES
			(
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_static_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_static_obj','object_id','number',p_object_id(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_static_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_static_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		null;
	end;

	procedure save_tangible_obj( p_object_id VAOFSTRING, p_max_hit_points VAOFNUMBER, p_owner_id VAOFSTRING, p_visible VAOFSTRING, p_appearance_data VAOFSTRING, p_interest_radius VAOFNUMBER, p_pvp_type VAOFNUMBER, p_pvp_faction VAOFNUMBER, p_damage_taken VAOFNUMBER, p_custom_appearance VAOFSTRING, p_count VAOFNUMBER, p_condition VAOFNUMBER, p_creator_id VAOFSTRING, p_source_draft_schematic VAOFNUMBER, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update tangible_objects set
			tangible_objects.max_hit_points = nvl(p_max_hit_points(i),tangible_objects.max_hit_points),
			tangible_objects.owner_id = nvl(p_owner_id(i),tangible_objects.owner_id),
			tangible_objects.visible = nvl(p_visible(i),tangible_objects.visible),
			tangible_objects.appearance_data = nvl(p_appearance_data(i),tangible_objects.appearance_data),
			tangible_objects.interest_radius = nvl(p_interest_radius(i),tangible_objects.interest_radius),
			tangible_objects.pvp_type = nvl(p_pvp_type(i),tangible_objects.pvp_type),
			tangible_objects.pvp_faction = nvl(p_pvp_faction(i),tangible_objects.pvp_faction),
			tangible_objects.damage_taken = nvl(p_damage_taken(i),tangible_objects.damage_taken),
			tangible_objects.custom_appearance = nvl(p_custom_appearance(i),tangible_objects.custom_appearance),
			tangible_objects.count = nvl(p_count(i),tangible_objects.count),
			tangible_objects.condition = nvl(p_condition(i),tangible_objects.condition),
			tangible_objects.creator_id = nvl(p_creator_id(i),tangible_objects.creator_id),
			tangible_objects.source_draft_schematic = nvl(p_source_draft_schematic(i),tangible_objects.source_draft_schematic)
		where
			tangible_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update tangible_objects set
					tangible_objects.max_hit_points = nvl(p_max_hit_points(i),tangible_objects.max_hit_points),
					tangible_objects.owner_id = nvl(p_owner_id(i),tangible_objects.owner_id),
					tangible_objects.visible = nvl(p_visible(i),tangible_objects.visible),
					tangible_objects.appearance_data = nvl(p_appearance_data(i),tangible_objects.appearance_data),
					tangible_objects.interest_radius = nvl(p_interest_radius(i),tangible_objects.interest_radius),
					tangible_objects.pvp_type = nvl(p_pvp_type(i),tangible_objects.pvp_type),
					tangible_objects.pvp_faction = nvl(p_pvp_faction(i),tangible_objects.pvp_faction),
					tangible_objects.damage_taken = nvl(p_damage_taken(i),tangible_objects.damage_taken),
					tangible_objects.custom_appearance = nvl(p_custom_appearance(i),tangible_objects.custom_appearance),
					tangible_objects.count = nvl(p_count(i),tangible_objects.count),
					tangible_objects.condition = nvl(p_condition(i),tangible_objects.condition),
					tangible_objects.creator_id = nvl(p_creator_id(i),tangible_objects.creator_id),
					tangible_objects.source_draft_schematic = nvl(p_source_draft_schematic(i),tangible_objects.source_draft_schematic)
					where
					tangible_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_tangible_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_tangible_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','max_hit_points','int',to_char(p_max_hit_points(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','owner_id','int',p_owner_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','visible','char',p_visible(m_error_index));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','appearance_data','varchar',p_appearance_data(m_error_index));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','interest_radius','int',to_char(p_interest_radius(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','pvp_type','int',to_char(p_pvp_type(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','pvp_faction','int',to_char(p_pvp_faction(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','damage_taken','int',to_char(p_damage_taken(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','custom_appearance','varchar2',p_custom_appearance(m_error_index));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','count','int',to_char(p_count(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','condition','int',to_char(p_condition(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','creator_id','number',p_creator_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_tangible_obj','source_draft_schematic','int',to_char(p_source_draft_schematic(m_error_index)));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_tangible_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_tangible_obj( p_object_id VAOFSTRING, p_max_hit_points VAOFNUMBER, p_owner_id VAOFSTRING, p_visible VAOFSTRING, p_appearance_data VAOFSTRING, p_interest_radius VAOFNUMBER, p_pvp_type VAOFNUMBER, p_pvp_faction VAOFNUMBER, p_damage_taken VAOFNUMBER, p_custom_appearance VAOFSTRING, p_count VAOFNUMBER, p_condition VAOFNUMBER, p_creator_id VAOFSTRING, p_source_draft_schematic VAOFNUMBER, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into tangible_objects
		(
			tangible_objects.max_hit_points,
			tangible_objects.owner_id,
			tangible_objects.visible,
			tangible_objects.appearance_data,
			tangible_objects.interest_radius,
			tangible_objects.pvp_type,
			tangible_objects.pvp_faction,
			tangible_objects.damage_taken,
			tangible_objects.custom_appearance,
			tangible_objects.count,
			tangible_objects.condition,
			tangible_objects.creator_id,
			tangible_objects.source_draft_schematic,
			tangible_objects.object_id
		)
		VALUES
		(
			p_max_hit_points(i),
			p_owner_id(i),
			p_visible(i),
			p_appearance_data(i),
			p_interest_radius(i),
			p_pvp_type(i),
			p_pvp_faction(i),
			p_damage_taken(i),
			p_custom_appearance(i),
			p_count(i),
			p_condition(i),
			p_creator_id(i),
			p_source_draft_schematic(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_tangible_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE tangible_objects Set
					tangible_objects.max_hit_points = nvl(p_max_hit_points(i),tangible_objects.max_hit_points),
					tangible_objects.owner_id = nvl(p_owner_id(i),tangible_objects.owner_id),
					tangible_objects.visible = nvl(p_visible(i),tangible_objects.visible),
					tangible_objects.appearance_data = nvl(p_appearance_data(i),tangible_objects.appearance_data),
					tangible_objects.interest_radius = nvl(p_interest_radius(i),tangible_objects.interest_radius),
					tangible_objects.pvp_type = nvl(p_pvp_type(i),tangible_objects.pvp_type),
					tangible_objects.pvp_faction = nvl(p_pvp_faction(i),tangible_objects.pvp_faction),
					tangible_objects.damage_taken = nvl(p_damage_taken(i),tangible_objects.damage_taken),
					tangible_objects.custom_appearance = nvl(p_custom_appearance(i),tangible_objects.custom_appearance),
					tangible_objects.count = nvl(p_count(i),tangible_objects.count),
					tangible_objects.condition = nvl(p_condition(i),tangible_objects.condition),
					tangible_objects.creator_id = nvl(p_creator_id(i),tangible_objects.creator_id),
					tangible_objects.source_draft_schematic = nvl(p_source_draft_schematic(i),tangible_objects.source_draft_schematic)
				where
					tangible_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into tangible_objects
					(
					tangible_objects.max_hit_points,
					tangible_objects.owner_id,
					tangible_objects.visible,
					tangible_objects.appearance_data,
					tangible_objects.interest_radius,
					tangible_objects.pvp_type,
					tangible_objects.pvp_faction,
					tangible_objects.damage_taken,
					tangible_objects.custom_appearance,
					tangible_objects.count,
					tangible_objects.condition,
					tangible_objects.creator_id,
					tangible_objects.source_draft_schematic,
					tangible_objects.object_id
					)
					VALUES
					(
					p_max_hit_points(i),
					p_owner_id(i),
					p_visible(i),
					p_appearance_data(i),
					p_interest_radius(i),
					p_pvp_type(i),
					p_pvp_faction(i),
					p_damage_taken(i),
					p_custom_appearance(i),
					p_count(i),
					p_condition(i),
					p_creator_id(i),
					p_source_draft_schematic(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into tangible_objects
			(
				tangible_objects.max_hit_points,
				tangible_objects.owner_id,
				tangible_objects.visible,
				tangible_objects.appearance_data,
				tangible_objects.interest_radius,
				tangible_objects.pvp_type,
				tangible_objects.pvp_faction,
				tangible_objects.damage_taken,
				tangible_objects.custom_appearance,
				tangible_objects.count,
				tangible_objects.condition,
				tangible_objects.creator_id,
				tangible_objects.source_draft_schematic,
				tangible_objects.object_id
			)
			VALUES
			(
				p_max_hit_points(i),
				p_owner_id(i),
				p_visible(i),
				p_appearance_data(i),
				p_interest_radius(i),
				p_pvp_type(i),
				p_pvp_faction(i),
				p_damage_taken(i),
				p_custom_appearance(i),
				p_count(i),
				p_condition(i),
				p_creator_id(i),
				p_source_draft_schematic(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_tangible_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_tangible_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','max_hit_points','int',to_char(p_max_hit_points(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','owner_id','int',p_owner_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','visible','char',p_visible(m_error_index));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','appearance_data','varchar',p_appearance_data(m_error_index));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','interest_radius','int',to_char(p_interest_radius(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','pvp_type','int',to_char(p_pvp_type(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','pvp_faction','int',to_char(p_pvp_faction(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','damage_taken','int',to_char(p_damage_taken(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','custom_appearance','varchar2',p_custom_appearance(m_error_index));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','count','int',to_char(p_count(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','condition','int',to_char(p_condition(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','creator_id','number',p_creator_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_tangible_obj','source_draft_schematic','int',to_char(p_source_draft_schematic(m_error_index)));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_tangible_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_tangible_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete tangible_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_token_obj( p_object_id VAOFSTRING, p_reference VAOFSTRING, p_target_server_template_name VAOFSTRING, p_target_shared_template_name VAOFSTRING, p_waypoint VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update token_objects set
			token_objects.reference = nvl(p_reference(i),token_objects.reference),
			token_objects.target_server_template_name = nvl(p_target_server_template_name(i),token_objects.target_server_template_name),
			token_objects.target_shared_template_name = nvl(p_target_shared_template_name(i),token_objects.target_shared_template_name),
			token_objects.waypoint = nvl(p_waypoint(i),token_objects.waypoint)
		where
			token_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update token_objects set
					token_objects.reference = nvl(p_reference(i),token_objects.reference),
					token_objects.target_server_template_name = nvl(p_target_server_template_name(i),token_objects.target_server_template_name),
					token_objects.target_shared_template_name = nvl(p_target_shared_template_name(i),token_objects.target_shared_template_name),
					token_objects.waypoint = nvl(p_waypoint(i),token_objects.waypoint)
					where
					token_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_token_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_token_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_token_obj','reference','number',p_reference(m_error_index));
								db_error_logger.dblogerror_values('persister.save_token_obj','target_server_template_name','varchar2',p_target_server_template_name(m_error_index));
								db_error_logger.dblogerror_values('persister.save_token_obj','target_shared_template_name','varchar2',p_target_shared_template_name(m_error_index));
								db_error_logger.dblogerror_values('persister.save_token_obj','waypoint','number',p_waypoint(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_token_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_token_obj( p_object_id VAOFSTRING, p_reference VAOFSTRING, p_target_server_template_name VAOFSTRING, p_target_shared_template_name VAOFSTRING, p_waypoint VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into token_objects
		(
			token_objects.reference,
			token_objects.target_server_template_name,
			token_objects.target_shared_template_name,
			token_objects.waypoint,
			token_objects.object_id
		)
		VALUES
		(
			p_reference(i),
			p_target_server_template_name(i),
			p_target_shared_template_name(i),
			p_waypoint(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_token_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE token_objects Set
					token_objects.reference = nvl(p_reference(i),token_objects.reference),
					token_objects.target_server_template_name = nvl(p_target_server_template_name(i),token_objects.target_server_template_name),
					token_objects.target_shared_template_name = nvl(p_target_shared_template_name(i),token_objects.target_shared_template_name),
					token_objects.waypoint = nvl(p_waypoint(i),token_objects.waypoint)
				where
					token_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into token_objects
					(
					token_objects.reference,
					token_objects.target_server_template_name,
					token_objects.target_shared_template_name,
					token_objects.waypoint,
					token_objects.object_id
					)
					VALUES
					(
					p_reference(i),
					p_target_server_template_name(i),
					p_target_shared_template_name(i),
					p_waypoint(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into token_objects
			(
				token_objects.reference,
				token_objects.target_server_template_name,
				token_objects.target_shared_template_name,
				token_objects.waypoint,
				token_objects.object_id
			)
			VALUES
			(
				p_reference(i),
				p_target_server_template_name(i),
				p_target_shared_template_name(i),
				p_waypoint(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_token_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_token_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_token_obj','reference','number',p_reference(m_error_index));
						db_error_logger.dblogerror_values('persister.add_token_obj','target_server_template_name','varchar2',p_target_server_template_name(m_error_index));
						db_error_logger.dblogerror_values('persister.add_token_obj','target_shared_template_name','varchar2',p_target_shared_template_name(m_error_index));
						db_error_logger.dblogerror_values('persister.add_token_obj','waypoint','number',p_waypoint(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_token_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_token_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete token_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_universe_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		null;
	end;

	procedure add_universe_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into universe_objects
		(
			universe_objects.object_id
		)
		VALUES
		(
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_universe_obj : dup_val_on_index error.');
			END IF;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into universe_objects
			(
				universe_objects.object_id
			)
			VALUES
			(
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_universe_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_universe_obj','object_id','number',p_object_id(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_universe_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_universe_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		null;
	end;

	procedure save_vehicle_obj( p_object_id VAOFSTRING, p_bogus VAOFNUMBER, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update vehicle_objects set
			vehicle_objects.bogus = nvl(p_bogus(i),vehicle_objects.bogus)
		where
			vehicle_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update vehicle_objects set
					vehicle_objects.bogus = nvl(p_bogus(i),vehicle_objects.bogus)
					where
					vehicle_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_vehicle_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_vehicle_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_vehicle_obj','bogus','int',to_char(p_bogus(m_error_index)));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_vehicle_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_vehicle_obj( p_object_id VAOFSTRING, p_bogus VAOFNUMBER, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into vehicle_objects
		(
			vehicle_objects.bogus,
			vehicle_objects.object_id
		)
		VALUES
		(
			p_bogus(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_vehicle_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE vehicle_objects Set
					vehicle_objects.bogus = nvl(p_bogus(i),vehicle_objects.bogus)
				where
					vehicle_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into vehicle_objects
					(
					vehicle_objects.bogus,
					vehicle_objects.object_id
					)
					VALUES
					(
					p_bogus(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into vehicle_objects
			(
				vehicle_objects.bogus,
				vehicle_objects.object_id
			)
			VALUES
			(
				p_bogus(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_vehicle_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_vehicle_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_vehicle_obj','bogus','int',to_char(p_bogus(m_error_index)));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_vehicle_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_vehicle_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete vehicle_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_weapon_obj( p_object_id VAOFSTRING, p_min_damage VAOFNUMBER, p_max_damage VAOFNUMBER, p_damage_type VAOFNUMBER, p_elemental_type VAOFNUMBER, p_elemental_value VAOFNUMBER, p_attack_speed VAOFNUMBER, p_wound_chance VAOFNUMBER, p_accuracy VAOFNUMBER, p_attack_cost VAOFNUMBER, p_damage_radius VAOFNUMBER, p_min_range VAOFNUMBER, p_max_range VAOFNUMBER, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		update weapon_objects set
			weapon_objects.min_damage = nvl(p_min_damage(i),weapon_objects.min_damage),
			weapon_objects.max_damage = nvl(p_max_damage(i),weapon_objects.max_damage),
			weapon_objects.damage_type = nvl(p_damage_type(i),weapon_objects.damage_type),
			weapon_objects.elemental_type = nvl(p_elemental_type(i),weapon_objects.elemental_type),
			weapon_objects.elemental_value = nvl(p_elemental_value(i),weapon_objects.elemental_value),
			weapon_objects.attack_speed = nvl(p_attack_speed(i),weapon_objects.attack_speed),
			weapon_objects.wound_chance = nvl(p_wound_chance(i),weapon_objects.wound_chance),
			weapon_objects.accuracy = nvl(p_accuracy(i),weapon_objects.accuracy),
			weapon_objects.attack_cost = nvl(p_attack_cost(i),weapon_objects.attack_cost),
			weapon_objects.damage_radius = nvl(p_damage_radius(i),weapon_objects.damage_radius),
			weapon_objects.min_range = nvl(p_min_range(i),weapon_objects.min_range),
			weapon_objects.max_range = nvl(p_max_range(i),weapon_objects.max_range)
		where
			weapon_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update weapon_objects set
					weapon_objects.min_damage = nvl(p_min_damage(i),weapon_objects.min_damage),
					weapon_objects.max_damage = nvl(p_max_damage(i),weapon_objects.max_damage),
					weapon_objects.damage_type = nvl(p_damage_type(i),weapon_objects.damage_type),
					weapon_objects.elemental_type = nvl(p_elemental_type(i),weapon_objects.elemental_type),
					weapon_objects.elemental_value = nvl(p_elemental_value(i),weapon_objects.elemental_value),
					weapon_objects.attack_speed = nvl(p_attack_speed(i),weapon_objects.attack_speed),
					weapon_objects.wound_chance = nvl(p_wound_chance(i),weapon_objects.wound_chance),
					weapon_objects.accuracy = nvl(p_accuracy(i),weapon_objects.accuracy),
					weapon_objects.attack_cost = nvl(p_attack_cost(i),weapon_objects.attack_cost),
					weapon_objects.damage_radius = nvl(p_damage_radius(i),weapon_objects.damage_radius),
					weapon_objects.min_range = nvl(p_min_range(i),weapon_objects.min_range),
					weapon_objects.max_range = nvl(p_max_range(i),weapon_objects.max_range)
					where
					weapon_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_weapon_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_weapon_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','min_damage','int',to_char(p_min_damage(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','max_damage','int',to_char(p_max_damage(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','damage_type','int',to_char(p_damage_type(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','elemental_type','int',to_char(p_elemental_type(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','elemental_value','int',to_char(p_elemental_value(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','attack_speed','float',to_char(p_attack_speed(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','wound_chance','float',to_char(p_wound_chance(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','accuracy','int',to_char(p_accuracy(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','attack_cost','int',to_char(p_attack_cost(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','damage_radius','float',to_char(p_damage_radius(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','min_range','float',to_char(p_min_range(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_weapon_obj','max_range','float',to_char(p_max_range(m_error_index)));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_weapon_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_weapon_obj( p_object_id VAOFSTRING, p_min_damage VAOFNUMBER, p_max_damage VAOFNUMBER, p_damage_type VAOFNUMBER, p_elemental_type VAOFNUMBER, p_elemental_value VAOFNUMBER, p_attack_speed VAOFNUMBER, p_wound_chance VAOFNUMBER, p_accuracy VAOFNUMBER, p_attack_cost VAOFNUMBER, p_damage_radius VAOFNUMBER, p_min_range VAOFNUMBER, p_max_range VAOFNUMBER, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=1;
	begin
		FORALL i IN 1..p_chunk_size
		insert into weapon_objects
		(
			weapon_objects.min_damage,
			weapon_objects.max_damage,
			weapon_objects.damage_type,
			weapon_objects.elemental_type,
			weapon_objects.elemental_value,
			weapon_objects.attack_speed,
			weapon_objects.wound_chance,
			weapon_objects.accuracy,
			weapon_objects.attack_cost,
			weapon_objects.damage_radius,
			weapon_objects.min_range,
			weapon_objects.max_range,
			weapon_objects.object_id
		)
		VALUES
		(
			p_min_damage(i),
			p_max_damage(i),
			p_damage_type(i),
			p_elemental_type(i),
			p_elemental_value(i),
			p_attack_speed(i),
			p_wound_chance(i),
			p_accuracy(i),
			p_attack_cost(i),
			p_damage_radius(i),
			p_min_range(i),
			p_max_range(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_weapon_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE weapon_objects Set
					weapon_objects.min_damage = nvl(p_min_damage(i),weapon_objects.min_damage),
					weapon_objects.max_damage = nvl(p_max_damage(i),weapon_objects.max_damage),
					weapon_objects.damage_type = nvl(p_damage_type(i),weapon_objects.damage_type),
					weapon_objects.elemental_type = nvl(p_elemental_type(i),weapon_objects.elemental_type),
					weapon_objects.elemental_value = nvl(p_elemental_value(i),weapon_objects.elemental_value),
					weapon_objects.attack_speed = nvl(p_attack_speed(i),weapon_objects.attack_speed),
					weapon_objects.wound_chance = nvl(p_wound_chance(i),weapon_objects.wound_chance),
					weapon_objects.accuracy = nvl(p_accuracy(i),weapon_objects.accuracy),
					weapon_objects.attack_cost = nvl(p_attack_cost(i),weapon_objects.attack_cost),
					weapon_objects.damage_radius = nvl(p_damage_radius(i),weapon_objects.damage_radius),
					weapon_objects.min_range = nvl(p_min_range(i),weapon_objects.min_range),
					weapon_objects.max_range = nvl(p_max_range(i),weapon_objects.max_range)
				where
					weapon_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into weapon_objects
					(
					weapon_objects.min_damage,
					weapon_objects.max_damage,
					weapon_objects.damage_type,
					weapon_objects.elemental_type,
					weapon_objects.elemental_value,
					weapon_objects.attack_speed,
					weapon_objects.wound_chance,
					weapon_objects.accuracy,
					weapon_objects.attack_cost,
					weapon_objects.damage_radius,
					weapon_objects.min_range,
					weapon_objects.max_range,
					weapon_objects.object_id
					)
					VALUES
					(
					p_min_damage(i),
					p_max_damage(i),
					p_damage_type(i),
					p_elemental_type(i),
					p_elemental_value(i),
					p_attack_speed(i),
					p_wound_chance(i),
					p_accuracy(i),
					p_attack_cost(i),
					p_damage_radius(i),
					p_min_range(i),
					p_max_range(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into weapon_objects
			(
				weapon_objects.min_damage,
				weapon_objects.max_damage,
				weapon_objects.damage_type,
				weapon_objects.elemental_type,
				weapon_objects.elemental_value,
				weapon_objects.attack_speed,
				weapon_objects.wound_chance,
				weapon_objects.accuracy,
				weapon_objects.attack_cost,
				weapon_objects.damage_radius,
				weapon_objects.min_range,
				weapon_objects.max_range,
				weapon_objects.object_id
			)
			VALUES
			(
				p_min_damage(i),
				p_max_damage(i),
				p_damage_type(i),
				p_elemental_type(i),
				p_elemental_value(i),
				p_attack_speed(i),
				p_wound_chance(i),
				p_accuracy(i),
				p_attack_cost(i),
				p_damage_radius(i),
				p_min_range(i),
				p_max_range(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_weapon_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_weapon_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','min_damage','int',to_char(p_min_damage(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','max_damage','int',to_char(p_max_damage(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','damage_type','int',to_char(p_damage_type(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','elemental_type','int',to_char(p_elemental_type(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','elemental_value','int',to_char(p_elemental_value(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','attack_speed','float',to_char(p_attack_speed(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','wound_chance','float',to_char(p_wound_chance(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','accuracy','int',to_char(p_accuracy(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','attack_cost','int',to_char(p_attack_cost(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','damage_radius','float',to_char(p_damage_radius(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','min_range','float',to_char(p_min_range(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_weapon_obj','max_range','float',to_char(p_max_range(m_error_index)));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_weapon_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_weapon_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete weapon_objects
		where
			object_id=p_object_id(i);
	end;

	procedure save_player_quest_obj( p_object_id VAOFSTRING, p_title VAOFSTRING, p_description VAOFSTRING, p_creator VAOFSTRING, p_total_tasks VAOFNUMBER, p_difficulty VAOFNUMBER, p_task_title1 VAOFSTRING, p_task_description1 VAOFSTRING, p_task_title2 VAOFSTRING, p_task_description2 VAOFSTRING, p_task_title3 VAOFSTRING, p_task_description3 VAOFSTRING, p_task_title4 VAOFSTRING, p_task_description4 VAOFSTRING, p_task_title5 VAOFSTRING, p_task_description5 VAOFSTRING, p_task_title6 VAOFSTRING, p_task_description6 VAOFSTRING, p_task_title7 VAOFSTRING, p_task_description7 VAOFSTRING, p_task_title8 VAOFSTRING, p_task_description8 VAOFSTRING, p_task_title9 VAOFSTRING, p_task_description9 VAOFSTRING, p_task_title10 VAOFSTRING, p_task_description10 VAOFSTRING, p_task_title11 VAOFSTRING, p_task_description11 VAOFSTRING, p_task_title12 VAOFSTRING, p_task_description12 VAOFSTRING, p_chunk_size number )
	as

	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=0;
	begin
		FORALL i IN 1..p_chunk_size
		update player_quest_objects set
			player_quest_objects.title = nvl(p_title(i),player_quest_objects.title),
			player_quest_objects.description = nvl(p_description(i),player_quest_objects.description),
			player_quest_objects.creator = nvl(p_creator(i),player_quest_objects.creator),
			player_quest_objects.total_tasks = nvl(p_total_tasks(i),player_quest_objects.total_tasks),
			player_quest_objects.difficulty = nvl(p_difficulty(i),player_quest_objects.difficulty),
			player_quest_objects.task_title1 = nvl(p_task_title1(i),player_quest_objects.task_title1),
			player_quest_objects.task_description1 = nvl(p_task_description1(i),player_quest_objects.task_description1),
			player_quest_objects.task_title2 = nvl(p_task_title2(i),player_quest_objects.task_title2),
			player_quest_objects.task_description2 = nvl(p_task_description2(i),player_quest_objects.task_description2),
			player_quest_objects.task_title3 = nvl(p_task_title3(i),player_quest_objects.task_title3),
			player_quest_objects.task_description3 = nvl(p_task_description3(i),player_quest_objects.task_description3),
			player_quest_objects.task_title4 = nvl(p_task_title4(i),player_quest_objects.task_title4),
			player_quest_objects.task_description4 = nvl(p_task_description4(i),player_quest_objects.task_description4),
			player_quest_objects.task_title5 = nvl(p_task_title5(i),player_quest_objects.task_title5),
			player_quest_objects.task_description5 = nvl(p_task_description5(i),player_quest_objects.task_description5),
			player_quest_objects.task_title6 = nvl(p_task_title6(i),player_quest_objects.task_title6),
			player_quest_objects.task_description6 = nvl(p_task_description6(i),player_quest_objects.task_description6),
			player_quest_objects.task_title7 = nvl(p_task_title7(i),player_quest_objects.task_title7),
			player_quest_objects.task_description7 = nvl(p_task_description7(i),player_quest_objects.task_description7),
			player_quest_objects.task_title8 = nvl(p_task_title8(i),player_quest_objects.task_title8),
			player_quest_objects.task_description8 = nvl(p_task_description8(i),player_quest_objects.task_description8),
			player_quest_objects.task_title9 = nvl(p_task_title9(i),player_quest_objects.task_title9),
			player_quest_objects.task_description9 = nvl(p_task_description9(i),player_quest_objects.task_description9),
			player_quest_objects.task_title10 = nvl(p_task_title10(i),player_quest_objects.task_title10),
			player_quest_objects.task_description10 = nvl(p_task_description10(i),player_quest_objects.task_description10),
			player_quest_objects.task_title11 = nvl(p_task_title11(i),player_quest_objects.task_title11),
			player_quest_objects.task_description11 = nvl(p_task_description11(i),player_quest_objects.task_description11),
			player_quest_objects.task_title12 = nvl(p_task_title12(i),player_quest_objects.task_title12),
			player_quest_objects.task_description12 = nvl(p_task_description12(i),player_quest_objects.task_description12)
		where
			player_quest_objects.object_id=p_object_id(i);
	exception
		when others then
			begin
				FOR i IN 1..p_chunk_size LOOP
					m_error_index := i;
					update player_quest_objects set
					player_quest_objects.title = nvl(p_title(i),player_quest_objects.title),
					player_quest_objects.description = nvl(p_description(i),player_quest_objects.description),
					player_quest_objects.creator = nvl(p_creator(i),player_quest_objects.creator),
					player_quest_objects.total_tasks = nvl(p_total_tasks(i),player_quest_objects.total_tasks),
					player_quest_objects.difficulty = nvl(p_difficulty(i),player_quest_objects.difficulty),
					player_quest_objects.task_title1 = nvl(p_task_title1(i),player_quest_objects.task_title1),
					player_quest_objects.task_description1 = nvl(p_task_description1(i),player_quest_objects.task_description1),
					player_quest_objects.task_title2 = nvl(p_task_title2(i),player_quest_objects.task_title2),
					player_quest_objects.task_description2 = nvl(p_task_description2(i),player_quest_objects.task_description2),
					player_quest_objects.task_title3 = nvl(p_task_title3(i),player_quest_objects.task_title3),
					player_quest_objects.task_description3 = nvl(p_task_description3(i),player_quest_objects.task_description3),
					player_quest_objects.task_title4 = nvl(p_task_title4(i),player_quest_objects.task_title4),
					player_quest_objects.task_description4 = nvl(p_task_description4(i),player_quest_objects.task_description4),
					player_quest_objects.task_title5 = nvl(p_task_title5(i),player_quest_objects.task_title5),
					player_quest_objects.task_description5 = nvl(p_task_description5(i),player_quest_objects.task_description5),
					player_quest_objects.task_title6 = nvl(p_task_title6(i),player_quest_objects.task_title6),
					player_quest_objects.task_description6 = nvl(p_task_description6(i),player_quest_objects.task_description6),
					player_quest_objects.task_title7 = nvl(p_task_title7(i),player_quest_objects.task_title7),
					player_quest_objects.task_description7 = nvl(p_task_description7(i),player_quest_objects.task_description7),
					player_quest_objects.task_title8 = nvl(p_task_title8(i),player_quest_objects.task_title8),
					player_quest_objects.task_description8 = nvl(p_task_description8(i),player_quest_objects.task_description8),
					player_quest_objects.task_title9 = nvl(p_task_title9(i),player_quest_objects.task_title9),
					player_quest_objects.task_description9 = nvl(p_task_description9(i),player_quest_objects.task_description9),
					player_quest_objects.task_title10 = nvl(p_task_title10(i),player_quest_objects.task_title10),
					player_quest_objects.task_description10 = nvl(p_task_description10(i),player_quest_objects.task_description10),
					player_quest_objects.task_title11 = nvl(p_task_title11(i),player_quest_objects.task_title11),
					player_quest_objects.task_description11 = nvl(p_task_description11(i),player_quest_objects.task_description11),
					player_quest_objects.task_title12 = nvl(p_task_title12(i),player_quest_objects.task_title12),
					player_quest_objects.task_description12 = nvl(p_task_description12(i),player_quest_objects.task_description12)
					where
					player_quest_objects.object_id=p_object_id(i);
				end LOOP;
			exception
				when others then
					begin
						m_enable_db_logging := db_error_logger.getLogLevel();
						IF (m_enable_db_logging > 0) THEN
							db_error_logger.dblogerror(SQLCODE,'persister.save_player_quest_obj : update error.');
							IF (m_enable_db_logging > 1) THEN
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','object_id','number',p_object_id(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','title','varchar2',p_title(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','description','varchar2',p_description(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','creator','number',p_creator(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','total_tasks','int',to_char(p_total_tasks(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','difficulty','int',to_char(p_difficulty(m_error_index)));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title1','varchar2',p_task_title1(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description1','varchar2',p_task_description1(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title2','varchar2',p_task_title2(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description2','varchar2',p_task_description2(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title3','varchar2',p_task_title3(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description3','varchar2',p_task_description3(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title4','varchar2',p_task_title4(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description4','varchar2',p_task_description4(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title5','varchar2',p_task_title5(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description5','varchar2',p_task_description5(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title6','varchar2',p_task_title6(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description6','varchar2',p_task_description6(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title7','varchar2',p_task_title7(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description7','varchar2',p_task_description7(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title8','varchar2',p_task_title8(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description8','varchar2',p_task_description8(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title9','varchar2',p_task_title9(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description9','varchar2',p_task_description9(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title10','varchar2',p_task_title10(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description10','varchar2',p_task_description10(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title11','varchar2',p_task_title11(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description11','varchar2',p_task_description11(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_title12','varchar2',p_task_title12(m_error_index));
								db_error_logger.dblogerror_values('persister.save_player_quest_obj','task_description12','varchar2',p_task_description12(m_error_index));
							END IF;
						END IF;
						IF (db_error_logger.reraisecheck('persister','save_player_quest_obj') = 1) THEN
							RAISE;
						END IF;
					end;
			end;
	end;

	procedure add_player_quest_obj( p_object_id VAOFSTRING, p_title VAOFSTRING, p_description VAOFSTRING, p_creator VAOFSTRING, p_total_tasks VAOFNUMBER, p_difficulty VAOFNUMBER, p_task_title1 VAOFSTRING, p_task_description1 VAOFSTRING, p_task_title2 VAOFSTRING, p_task_description2 VAOFSTRING, p_task_title3 VAOFSTRING, p_task_description3 VAOFSTRING, p_task_title4 VAOFSTRING, p_task_description4 VAOFSTRING, p_task_title5 VAOFSTRING, p_task_description5 VAOFSTRING, p_task_title6 VAOFSTRING, p_task_description6 VAOFSTRING, p_task_title7 VAOFSTRING, p_task_description7 VAOFSTRING, p_task_title8 VAOFSTRING, p_task_description8 VAOFSTRING, p_task_title9 VAOFSTRING, p_task_description9 VAOFSTRING, p_task_title10 VAOFSTRING, p_task_description10 VAOFSTRING, p_task_title11 VAOFSTRING, p_task_description11 VAOFSTRING, p_task_title12 VAOFSTRING, p_task_description12 VAOFSTRING, p_chunk_size number )
	as
	m_enable_db_logging INTEGER := 0;
	m_error_index INTEGER :=0;
	begin
		FORALL i IN 1..p_chunk_size
		insert into player_quest_objects
		(
			player_quest_objects.title,
			player_quest_objects.description,
			player_quest_objects.creator,
			player_quest_objects.total_tasks,
			player_quest_objects.difficulty,
			player_quest_objects.task_title1,
			player_quest_objects.task_description1,
			player_quest_objects.task_title2,
			player_quest_objects.task_description2,
			player_quest_objects.task_title3,
			player_quest_objects.task_description3,
			player_quest_objects.task_title4,
			player_quest_objects.task_description4,
			player_quest_objects.task_title5,
			player_quest_objects.task_description5,
			player_quest_objects.task_title6,
			player_quest_objects.task_description6,
			player_quest_objects.task_title7,
			player_quest_objects.task_description7,
			player_quest_objects.task_title8,
			player_quest_objects.task_description8,
			player_quest_objects.task_title9,
			player_quest_objects.task_description9,
			player_quest_objects.task_title10,
			player_quest_objects.task_description10,
			player_quest_objects.task_title11,
			player_quest_objects.task_description11,
			player_quest_objects.task_title12,
			player_quest_objects.task_description12,
			player_quest_objects.object_id
		)
		VALUES
		(
			p_title(i),
			p_description(i),
			p_creator(i),
			p_total_tasks(i),
			p_difficulty(i),
			p_task_title1(i),
			p_task_description1(i),
			p_task_title2(i),
			p_task_description2(i),
			p_task_title3(i),
			p_task_description3(i),
			p_task_title4(i),
			p_task_description4(i),
			p_task_title5(i),
			p_task_description5(i),
			p_task_title6(i),
			p_task_description6(i),
			p_task_title7(i),
			p_task_description7(i),
			p_task_title8(i),
			p_task_description8(i),
			p_task_title9(i),
			p_task_description9(i),
			p_task_title10(i),
			p_task_description10(i),
			p_task_title11(i),
			p_task_description11(i),
			p_task_title12(i),
			p_task_description12(i),
			p_object_id(i)
		);

	exception
		when dup_val_on_index then
		begin
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'persister.add_player_quest_obj : dup_val_on_index error.');
			END IF;
			for i in 1..p_chunk_size loop
				m_error_index := i;
				UPDATE player_quest_objects Set
					player_quest_objects.title = nvl(p_title(i),player_quest_objects.title),
					player_quest_objects.description = nvl(p_description(i),player_quest_objects.description),
					player_quest_objects.creator = nvl(p_creator(i),player_quest_objects.creator),
					player_quest_objects.total_tasks = nvl(p_total_tasks(i),player_quest_objects.total_tasks),
					player_quest_objects.difficulty = nvl(p_difficulty(i),player_quest_objects.difficulty),
					player_quest_objects.task_title1 = nvl(p_task_title1(i),player_quest_objects.task_title1),
					player_quest_objects.task_description1 = nvl(p_task_description1(i),player_quest_objects.task_description1),
					player_quest_objects.task_title2 = nvl(p_task_title2(i),player_quest_objects.task_title2),
					player_quest_objects.task_description2 = nvl(p_task_description2(i),player_quest_objects.task_description2),
					player_quest_objects.task_title3 = nvl(p_task_title3(i),player_quest_objects.task_title3),
					player_quest_objects.task_description3 = nvl(p_task_description3(i),player_quest_objects.task_description3),
					player_quest_objects.task_title4 = nvl(p_task_title4(i),player_quest_objects.task_title4),
					player_quest_objects.task_description4 = nvl(p_task_description4(i),player_quest_objects.task_description4),
					player_quest_objects.task_title5 = nvl(p_task_title5(i),player_quest_objects.task_title5),
					player_quest_objects.task_description5 = nvl(p_task_description5(i),player_quest_objects.task_description5),
					player_quest_objects.task_title6 = nvl(p_task_title6(i),player_quest_objects.task_title6),
					player_quest_objects.task_description6 = nvl(p_task_description6(i),player_quest_objects.task_description6),
					player_quest_objects.task_title7 = nvl(p_task_title7(i),player_quest_objects.task_title7),
					player_quest_objects.task_description7 = nvl(p_task_description7(i),player_quest_objects.task_description7),
					player_quest_objects.task_title8 = nvl(p_task_title8(i),player_quest_objects.task_title8),
					player_quest_objects.task_description8 = nvl(p_task_description8(i),player_quest_objects.task_description8),
					player_quest_objects.task_title9 = nvl(p_task_title9(i),player_quest_objects.task_title9),
					player_quest_objects.task_description9 = nvl(p_task_description9(i),player_quest_objects.task_description9),
					player_quest_objects.task_title10 = nvl(p_task_title10(i),player_quest_objects.task_title10),
					player_quest_objects.task_description10 = nvl(p_task_description10(i),player_quest_objects.task_description10),
					player_quest_objects.task_title11 = nvl(p_task_title11(i),player_quest_objects.task_title11),
					player_quest_objects.task_description11 = nvl(p_task_description11(i),player_quest_objects.task_description11),
					player_quest_objects.task_title12 = nvl(p_task_title12(i),player_quest_objects.task_title12),
					player_quest_objects.task_description12 = nvl(p_task_description12(i),player_quest_objects.task_description12)
				where
					player_quest_objects.object_id=p_object_id(i);

				if SQL%ROWCOUNT=0 then
					insert into player_quest_objects
					(
					player_quest_objects.title,
					player_quest_objects.description,
					player_quest_objects.creator,
					player_quest_objects.total_tasks,
					player_quest_objects.difficulty,
					player_quest_objects.task_title1,
					player_quest_objects.task_description1,
					player_quest_objects.task_title2,
					player_quest_objects.task_description2,
					player_quest_objects.task_title3,
					player_quest_objects.task_description3,
					player_quest_objects.task_title4,
					player_quest_objects.task_description4,
					player_quest_objects.task_title5,
					player_quest_objects.task_description5,
					player_quest_objects.task_title6,
					player_quest_objects.task_description6,
					player_quest_objects.task_title7,
					player_quest_objects.task_description7,
					player_quest_objects.task_title8,
					player_quest_objects.task_description8,
					player_quest_objects.task_title9,
					player_quest_objects.task_description9,
					player_quest_objects.task_title10,
					player_quest_objects.task_description10,
					player_quest_objects.task_title11,
					player_quest_objects.task_description11,
					player_quest_objects.task_title12,
					player_quest_objects.task_description12,
					player_quest_objects.object_id
					)
					VALUES
					(
					p_title(i),
					p_description(i),
					p_creator(i),
					p_total_tasks(i),
					p_difficulty(i),
					p_task_title1(i),
					p_task_description1(i),
					p_task_title2(i),
					p_task_description2(i),
					p_task_title3(i),
					p_task_description3(i),
					p_task_title4(i),
					p_task_description4(i),
					p_task_title5(i),
					p_task_description5(i),
					p_task_title6(i),
					p_task_description6(i),
					p_task_title7(i),
					p_task_description7(i),
					p_task_title8(i),
					p_task_description8(i),
					p_task_title9(i),
					p_task_description9(i),
					p_task_title10(i),
					p_task_description10(i),
					p_task_title11(i),
					p_task_description11(i),
					p_task_title12(i),
					p_task_description12(i),
					p_object_id(i)
					);
				end if;
			end loop;
		end;
		when others then
		begin
			for i in 1..p_chunk_size loop
			m_error_index := i;
			insert into player_quest_objects
			(
				player_quest_objects.title,
				player_quest_objects.description,
				player_quest_objects.creator,
				player_quest_objects.total_tasks,
				player_quest_objects.difficulty,
				player_quest_objects.task_title1,
				player_quest_objects.task_description1,
				player_quest_objects.task_title2,
				player_quest_objects.task_description2,
				player_quest_objects.task_title3,
				player_quest_objects.task_description3,
				player_quest_objects.task_title4,
				player_quest_objects.task_description4,
				player_quest_objects.task_title5,
				player_quest_objects.task_description5,
				player_quest_objects.task_title6,
				player_quest_objects.task_description6,
				player_quest_objects.task_title7,
				player_quest_objects.task_description7,
				player_quest_objects.task_title8,
				player_quest_objects.task_description8,
				player_quest_objects.task_title9,
				player_quest_objects.task_description9,
				player_quest_objects.task_title10,
				player_quest_objects.task_description10,
				player_quest_objects.task_title11,
				player_quest_objects.task_description11,
				player_quest_objects.task_title12,
				player_quest_objects.task_description12,
				player_quest_objects.object_id
			)
			VALUES
			(
				p_title(i),
				p_description(i),
				p_creator(i),
				p_total_tasks(i),
				p_difficulty(i),
				p_task_title1(i),
				p_task_description1(i),
				p_task_title2(i),
				p_task_description2(i),
				p_task_title3(i),
				p_task_description3(i),
				p_task_title4(i),
				p_task_description4(i),
				p_task_title5(i),
				p_task_description5(i),
				p_task_title6(i),
				p_task_description6(i),
				p_task_title7(i),
				p_task_description7(i),
				p_task_title8(i),
				p_task_description8(i),
				p_task_title9(i),
				p_task_description9(i),
				p_task_title10(i),
				p_task_description10(i),
				p_task_title11(i),
				p_task_description11(i),
				p_task_title12(i),
				p_task_description12(i),
				p_object_id(i)
			);

			end loop;
		exception
			when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'persister.add_player_quest_obj : error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','object_id','number',p_object_id(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','title','varchar2',p_title(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','description','varchar2',p_description(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','creator','number',p_creator(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','total_tasks','int',to_char(p_total_tasks(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','difficulty','int',to_char(p_difficulty(m_error_index)));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title1','varchar2',p_task_title1(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description1','varchar2',p_task_description1(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title2','varchar2',p_task_title2(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description2','varchar2',p_task_description2(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title3','varchar2',p_task_title3(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description3','varchar2',p_task_description3(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title4','varchar2',p_task_title4(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description4','varchar2',p_task_description4(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title5','varchar2',p_task_title5(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description5','varchar2',p_task_description5(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title6','varchar2',p_task_title6(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description6','varchar2',p_task_description6(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title7','varchar2',p_task_title7(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description7','varchar2',p_task_description7(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title8','varchar2',p_task_title8(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description8','varchar2',p_task_description8(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title9','varchar2',p_task_title9(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description9','varchar2',p_task_description9(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title10','varchar2',p_task_title10(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description10','varchar2',p_task_description10(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title11','varchar2',p_task_title11(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description11','varchar2',p_task_description11(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_title12','varchar2',p_task_title12(m_error_index));
						db_error_logger.dblogerror_values('persister.add_player_quest_obj','task_description12','varchar2',p_task_description12(m_error_index));
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('persister','add_player_quest_obj') = 1) THEN
					RAISE;
				END IF;
			end;
		end;
	end;

	procedure remove_player_quest_obj( p_object_id VAOFSTRING, p_chunk_size number )
	as
	begin
		FORALL i IN 1..p_chunk_size
		delete player_quest_objects
		where
			object_id=p_object_id(i);
	end;

end;

/
