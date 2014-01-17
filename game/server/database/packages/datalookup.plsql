create or replace package body datalookup
as

	function check_character_name(p_name varchar2) return number as
		obj_id number;
	begin
		select character_object
		into obj_id
		from players
		where uc_character_name = p_name;

		if (obj_id is null) then
			return 0;
		else
			return 1;
		end if;

	exception
		when no_data_found then
			return 0;

		when too_many_rows then
			return 1;
	end;

	function get_structures_for_purge(p_station_id number) return refcursor
	as
		result_cursor refcursor;
	begin
		open result_cursor for
		select o.object_id, t.owner_id 
		from players p, tangible_objects t, objects o
		where p.station_id = p_station_id 
		and p.character_object = t.owner_id
		and t.object_id = o.object_id
		and o.contained_by = 0
		and o.deleted = 0
		and o.type_id in (1112885583, 1212763727, 1296649807) --house, harvester, factory
		-- exclude city halls and faction hqs
		and o.object_template_id not in
		(2104917241, -1402078881, -1650739949, 1078805016, -1085193189, -2036447549,
		-1503538066, 1083153409, -1889839602, 2103485856, -1318727266, -447776542,
		-278088733, 456016075, 2103180392, -1782254683, 1221154416, 943757712,
		344955127, -1651881206, -1428516812, 121334480, -1391658348, 585958560,
		1568866170, -970221632, 1490696786, -274776875, 1419804026, -1907246, 
		-714325274, -2102543938, -382786695);

		return result_cursor;
	end;

	function get_vendors_for_purge(p_station_id number) return refcursor
	as
		result_cursor refcursor;
	begin
		open result_cursor for
		-- get all vendors owned by this station_id
		select o.object_id, t.owner_id, o.object_name
		from players p, tangible_objects t, objects o
		where p.station_id = p_station_id 
		and p.character_object = t.owner_id
		and t.object_id = o.object_id
		and o.deleted = 0
		and o.script_list like '%terminal.vendor:%'

		union
		-- plus all vendors in purge structures owned by this station_id
		select t.object_id, t.owner_id, o.object_name
		from tangible_objects t, objects o
		where t.object_id in
		(
		select object_id
		from objects
		where deleted = 0
		and script_list like '%terminal.vendor:%'
		start with object_id in 
		(
			select o.object_id
			from players p, tangible_objects t, objects o
		where p.station_id = p_station_id 
		and p.character_object = t.owner_id
		and t.object_id = o.object_id
		and o.deleted = 0
			and o.type_id in (1112885583, 1212763727, 1296649807) --house, harvester, factory
			-- exclude city halls and faction hqs
			and o.object_template_id not in
			(2104917241, -1402078881, -1650739949, 1078805016, -1085193189, -2036447549,
			-1503538066, 1083153409, -1889839602, 2103485856, -1318727266, -447776542,
			-278088733, 456016075, 2103180392, -1782254683, 1221154416, 943757712,
			344955127, -1651881206, -1428516812, 121334480, -1391658348, 585958560,
			1568866170, -970221632, 1490696786, -274776875, 1419804026, -1907246, 
			-714325274, -2102543938, -382786695)
		) 
			connect by contained_by = prior object_id
		) and t.object_id = o.object_id;

		return result_cursor;
	end;
end;
/
