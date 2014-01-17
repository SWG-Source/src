/*
	SQL for gold database update 20090604
*/

--------------------------------------------------------------------------------

/*
	Attach script and set objvar on various buildings for Empire Day 2009
*/

declare
	type va_number is varray(100) of number;
	type va_vchar2 is varray(100) of varchar2(256);
	c number;
	i number;
	obj_ids va_number;
	obj_values va_vchar2;

begin
	select count(*) into c
	from object_variable_names
	where name = 'empire_day_spawn_table';

	-- make sure name id exists
	if c = 0 then
		select max(id) into c
		from object_variable_names;

		insert into object_variable_names
		values (c + 1, 'empire_day_spawn_table');
	end if;

	-- get the name id
	select max(id) into i
	from object_variable_names
	where name = 'empire_day_spawn_table';

	-- these arrays must match in size	
	obj_ids := va_number (
		88,
		108,
		1028644,
		1213343,
		3375352,
		8105493
	);

	obj_values := va_vchar2 (
		'datatables/spawning/building_spawns/empire_day_imperial_theed.iff',
		'datatables/spawning/building_spawns/empire_day_rebel_moenia.iff',
		'datatables/spawning/building_spawns/empire_day_imperial_bestine.iff',
		'datatables/spawning/building_spawns/empire_day_rebel_anchorhead.iff',
		'datatables/spawning/building_spawns/empire_day_imperial_bela_vistal.iff',
		'datatables/spawning/building_spawns/empire_day_rebel_coronet.iff'
	);

	for o in obj_ids.first .. obj_ids.last
	loop

		-- attach script if not there already
		update objects
		set script_list = trim(script_list) || 'theme_park.dungeon.empire_day_interior_npc_spawner:'
		where object_id = obj_ids(o)
		and script_list not like '%theme_park.dungeon.empire_day_interior_npc_spawner%';

		-- determine if objvar exists
		select count(*) into c
		from object_variables
		where object_id = obj_ids(o)
		and name_id = i;

		if c = 0 then
			-- add new objvar
			insert into object_variables
			values (obj_ids(o), i, 4, obj_values(o), 0);
		else
			-- update existing
			update object_variables
			set value = obj_values(o),
				type = 4,
				detached = 0
			where object_id = obj_ids(o)
			and name_id = i;
		end if;
	end loop;
end;
/

commit;
