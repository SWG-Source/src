col x format 99999.99
col y format 99999.99
col z format 99999.99
col scene_id format a20
col object_id format 999999999999
col object_template_id format 999999999999 heading template
set heading off
set feedback off
set trimspool on
set linesize 200
select o.object_id, o.x, o.y, o.z, o.scene_id, o.object_template_id
from objects o, tangible_objects t, building_objects b
where o.object_id = t.object_id and t.object_id = b.object_id and
      o.deleted = 0;
 
select o.object_id, o.x, o.y, o.z, o.scene_id, o.object_template_id
from objects o, tangible_objects t, installation_objects i
where o.object_id = t.object_id and t.object_id = i.object_id and
      o.deleted = 0;

