create or replace package body custserv_procs
as

	function get_characters_for_account
	( 
	station_id_in IN players.station_id%TYPE 
	)
	return cursortype
	is
		result cursortype;
	begin
		open result for
		select 
			character_object, uc_character_name
		from
			players
		where
			station_id = station_id_in;
		return result;
	end get_characters_for_account;

	function get_deleted_items
	(
	character_id_in IN objects.load_with%TYPE,
	start_page_in IN number
	)
	return cursortype
	is
		result cursortype;
	begin
		open result for
		select * from
			(select a.*, rownum RN from
			( select
			o.object_id, name_string_table, name_string_text, object_name
			from objects o,
				(select object_id from objects where contained_by = character_id_in and object_template_id = -1783727815) datapad,
				(select object_id from tangible_objects where owner_id = character_id_in) tangibles
		where
			( deleted > 0 )
			and ( o.object_id = tangibles.object_id
			    or contained_by = datapad.object_id)
			    order by deleted_date desc ) a
				where rownum <= start_page_in * 20 + 20)
			where rn >= start_page_in * 20;
		return result;
	end get_deleted_items;
	
	function get_structures
	(
		character_id_in IN objects.object_id%TYPE
	)
	return cursortype
	is
		result cursortype;
	begin
		open result for
		select o.object_id as object_id, 
			o.x as x, 
			o.y as y, 
			o.z as z, 
			o.scene_id as scene_id, 
			o.name_string_text as object_template, 
			o.deleted as deleted
		from objects o, tangible_objects tan
		where tan.owner_id = character_id_in
			and tan.object_id = o.object_id
				and ( (o.object_id in (select object_id from building_objects ) )
					or ( o.object_id in (select object_id from installation_objects) ) );
		return result;
	end get_structures;		
	
	function get_player_id
	(
		character_name_in IN players.uc_character_name%TYPE
	)
	return cursortype
	is
		result cursortype;
	begin
		open result for
		select character_object, station_id
		from players
		where upper(uc_character_name) = upper(character_name_in);
	return result;
	end get_player_id;

	procedure move_player
	(
		character_id_in IN objects.object_id%TYPE,
		x_in IN objects.x%TYPE,
		y_in IN objects.y%TYPE,
		z_in IN objects.z%TYPE,
		scene_in IN objects.scene_id%TYPE
	)
	as
	begin
		update objects
			set x = x_in,
			    y = y_in,
			    z = z_in,
			    scene_id = scene_in
			where object_id = character_id_in;
	end move_player;

	procedure undelete_item
	(
		character_id_in IN objects.object_id%TYPE,
		object_in IN objects.object_id%TYPE,
		move_in IN number
	)
	as
		restore_result number;
	begin
		restore_result := admin.undelete_item(object_in);
		if ((restore_result != 1) and (restore_result != 4)) then
			return;
		end if;
		
		if (move_in > 0) then
			restore_result := admin.move_item_to_player(object_in, character_id_in);
		end if;
		return;
	exception	
		when others then
			return;
	end undelete_item;

end custserv_procs;
/
