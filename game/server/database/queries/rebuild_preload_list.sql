delete preload_list;

insert into preload_list
select o.scene_id, o.object_id, o.node_x, o.node_z, t.interest_radius, l.server_id, l.server_id
from objects o, tangible_objects t, loadbeacon_server_map l
where deleted = 0
and o.object_id = t.object_id
and o.object_template = l.object_template;

update preload_list
set wilderness_server_id = (select value from object_variables where name = 'wildernessServerId' and object_variables.object_id = preload_list.object_id)
where object_id in (select object_id from object_variables where name = 'wildernessServerId');

update preload_list
set city_server_id = (select value from object_variables where name = 'cityServerId' and object_variables.object_id = preload_list.object_id)
where object_id in (select object_id from object_variables where name = 'cityServerId');
