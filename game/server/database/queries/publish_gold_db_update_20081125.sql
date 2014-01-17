/*
	SQL for gold database update 20081125
*/

--------------------------------------------------------------------------------

/*
	Attach script and set objvar on the cantina in Mos Espa for the Life Day event
*/

declare
	c number;
	i number;

begin
	select count(*) into c
	from object_variable_names
	where name = 'spawn_table';

	-- make sure spawn_table name id exists
	if c = 0 then
		select max(id) into c
		from object_variable_names;

		insert into object_variable_names
		values (c + 1, 'spawn_table');
	end if;

	-- get the spawn_table name id
	select max(id) into i
	from object_variable_names
	where name = 'spawn_table';

	-- attach script if not there already
	update objects
	set script_list = trim(script_list) || 'theme_park.dungeon.ep3_clone_relics_generic_spawner:'
	where object_id = 1256055
	and script_list not like '%theme_park.dungeon.ep3_clone_relics_generic_spawner%';

	-- determine if spawn_table objvar exists
	select count(*) into c
	from object_variables
	where object_id = 1256055
	and name_id = i;

	if c = 0 then
		-- add new spawn_table objvar
		insert into object_variables
		values (1256055, i, 4, 'datatables/spawning/building_spawns/mos_espa_cantina.iff', 0);
	else
		-- update existing
		update object_variables
		set value = 'datatables/spawning/building_spawns/mos_espa_cantina.iff',
			type = 4,
			detached = 0
		where object_id = 1256055
		and name_id = i;
	end if;

end;
/


commit;


