create or replace package body lazy_deleter as

	procedure purge_one_object (object_in in number) as
	BEGIN
		DELETE FROM armor WHERE object_id = object_in;
		DELETE FROM battlefield_marker_objects WHERE object_id = object_in;
		DELETE FROM battlefield_participants WHERE region_object_id = object_in;
		DELETE FROM battlefield_participants WHERE character_object_id = object_in;
		DELETE FROM biographies WHERE object_id = object_in;
		DELETE FROM building_objects WHERE object_id = object_in;
		DELETE FROM cell_objects WHERE object_id = object_in;
		DELETE FROM city_objects WHERE object_id = object_in;
		DELETE FROM creature_objects WHERE object_id = object_in;
		DELETE FROM experience_points WHERE object_id = object_in;
		DELETE FROM factory_objects WHERE object_id = object_in;
		DELETE FROM guild_objects WHERE object_id = object_in;
		DELETE FROM harvester_installation_objects WHERE object_id = object_in;
		DELETE FROM installation_objects WHERE object_id = object_in;
		DELETE FROM intangible_objects WHERE object_id = object_in;
		DELETE FROM location_lists WHERE object_id = object_in;
		DELETE FROM manf_schematic_attributes WHERE object_id = object_in;
		DELETE FROM manf_schematic_objects WHERE object_id = object_in;
		DELETE FROM manufacture_inst_objects WHERE object_id = object_in;
		DELETE FROM mission_objects WHERE object_id = object_in;
		DELETE FROM object_variables WHERE object_id = object_in;
		DELETE FROM planet_objects WHERE object_id = object_in;
		DELETE FROM player_objects WHERE object_id = object_in;
		DELETE FROM player_quest_objects WHERE object_id = object_in;
		DELETE FROM property_lists WHERE object_id = object_in;
		DELETE FROM resource_container_objects WHERE object_id = object_in;
		DELETE FROM scripts WHERE object_id = object_in;
		DELETE FROM ship_objects WHERE object_id = object_in;
		DELETE FROM static_objects WHERE object_id = object_in;
		DELETE FROM swg_characters WHERE object_id = object_in;
		DELETE FROM tangible_objects WHERE object_id = object_in; 
		DELETE FROM temp_characters WHERE object_id = object_in; 
		DELETE FROM token_objects WHERE object_id = object_in; 
		DELETE FROM universe_objects WHERE object_id = object_in;
		DELETE FROM vehicle_objects WHERE object_id = object_in; 
		DELETE FROM waypoints WHERE object_id = object_in; 
		DELETE FROM weapon_objects WHERE object_id = object_in;
		DELETE FROM messages WHERE target = object_in;
		DELETE FROM objects WHERE object_id = object_in;
	END purge_one_object;

	procedure purge_objects_bulk(p_object_id VAOFSTRING, p_chunk_size number, p_enable_db_logging number )
	as

	errors	NUMBER;

	begin
		
	  	FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM armor WHERE object_id = p_object_id(i);
	  	FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM battlefield_marker_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM battlefield_participants WHERE region_object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM battlefield_participants WHERE character_object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM biographies WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM building_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM cell_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM city_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM creature_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM experience_points WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM factory_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM guild_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM harvester_installation_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM installation_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM intangible_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM location_lists WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM manf_schematic_attributes WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM manf_schematic_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM manufacture_inst_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM mission_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM object_variables WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM planet_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM player_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM player_quest_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM property_lists WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM resource_container_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM scripts WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM static_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM swg_characters WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM tangible_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM temp_characters WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM token_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM universe_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM vehicle_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM waypoints WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM weapon_objects WHERE object_id = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM messages WHERE target = p_object_id(i);
		FORALL i in 1..p_chunk_size SAVE EXCEPTIONS DELETE FROM objects WHERE object_id = p_object_id(i);

		exception when others then
			if ( p_enable_db_logging > 0 ) then
				errors:=SQL%BULK_EXCEPTIONS.COUNT;
				FOR x IN 1..errors LOOP
					db_error_logger.dblogerror( -SQL%BULK_EXCEPTIONS(x).ERROR_CODE, 'procedure purge_objects_bulk: error occurred in FORALL DELETE during iteration: ' || SQL%BULK_EXCEPTIONS(x).ERROR_INDEX );
						if ( p_enable_db_logging > 1 ) then
								db_error_logger.dblogerror_values( 'lazy deleter', 'object_id' , 'number', p_object_id(to_number(SQL%BULK_EXCEPTIONS(x).ERROR_INDEX)));
						end if;
				END LOOP;
				
			else
				NULL;
			end if;

	end;

end lazy_deleter;
/
