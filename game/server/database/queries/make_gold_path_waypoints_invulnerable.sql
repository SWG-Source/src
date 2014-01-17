-- this makes all path waypoints with bad condition values (negative or null) invulnerable 
update tangible_objects set condition = 256
where object_id in (
	select t.object_id
	from tangible_objects t, objects o, object_templates ot 
	where (t.condition < 0 or t.condition is null)
	and t.object_id = o.object_id
	and o.object_template_id = ot.id
	and ot.name like '%path_waypoint%'
);

