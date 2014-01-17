alter table creature_objects add scale_factor float;

update creature_objects set scale_factor=1;

create or replace view character_view as
select p.station_id, p.character_object, o.object_template, o.scene_id, o.object_name,
(select object_id from objects where contained_by=0 start with object_id=character_object connect by prior contained_by=object_id) container,
(select x from objects where contained_by=0 start with object_id=character_object connect by prior contained_by=object_id) x,
(select y from objects where contained_by=0 start with object_id=character_object connect by prior contained_by=object_id) y,
(select z from objects where contained_by=0 start with object_id=character_object connect by prior contained_by=object_id) z
from players p, objects o
where p.character_object = o.object_id
  and o.deleted = 0;

update version_number set version_number = 9, min_version_number=9;
