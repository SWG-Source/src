-- objects

alter table objects add object_template_id int;

update objects set object_template_id = object_template where
object_template like '-%'
or object_template like '1%'
or object_template like '2%'
or object_template like '3%'
or object_template like '4%'
or object_template like '5%'
or object_template like '6%'
or object_template like '7%'
or object_template like '8%'
or object_template like '9%'
or object_template like '0%';

update objects set object_template_id = (select id from object_templates t where t.name=objects.object_template)
where object_template_id is null;

delete objects where object_template_id is null;

alter table objects drop column object_template;

create or replace view character_view as
select p.station_id, p.character_object, o.object_template_id, o.scene_id, o.object_name,
(select object_id from objects where contained_by=0 start with object_id=character_object connect by prior contained_by=object_id) container,
(select x from objects where contained_by=0 start with object_id=character_object connect by prior contained_by=object_id) x,
(select y from objects where contained_by=0 start with object_id=character_object connect by prior contained_by=object_id) y,
(select z from objects where contained_by=0 start with object_id=character_object connect by prior contained_by=object_id) z
from players p, objects o
where p.character_object = o.object_id
  and o.deleted = 0;

-- swg_characters

alter table swg_characters add template_id number;

update swg_characters set template_id = template_name where
template_name like '-%'
or template_name like '1%'
or template_name like '2%'
or template_name like '3%'
or template_name like '4%'
or template_name like '5%'
or template_name like '6%'
or template_name like '7%'
or template_name like '8%'
or template_name like '9%'
or template_name like '0%';

update swg_characters set template_id = (select id from object_templates t where t.name=template_name)
where template_id is null;

alter table swg_characters drop column template_name;

-- swg_login_methods

drop package swg_login_methods;

-- version number

update version_number set version_number=107, min_version_number=107;
