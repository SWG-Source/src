alter table preload_list add preload_range int;
alter table preload_list add city_server_id int;
alter table preload_list add wilderness_server_id int;

update preload_list set city_server_id = server_id;
update preload_list set preload_range = interest_radius;
update preload_list set wilderness_server_id = server_id;

alter table preload_list drop column interest_radius;
alter table preload_list drop column server_id;
alter table preload_list drop column secondary_server_id;
alter table preload_list drop column use_for_wilderness;

insert into object_variables
select object_id, 'wildernessServerId',0,server_id,0
from objects o, loadbeacon_server_map l
where o.object_template = l.object_template;

insert into object_variables
select object_id, 'cityServerId',0,server_id,0
from objects o, loadbeacon_server_map l
where o.object_template = l.object_template;

update version_number set version_number=92, min_version_number=92;
