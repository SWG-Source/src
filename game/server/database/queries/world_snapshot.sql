set head off
set feedback off
set pagesize 0
set linesize 300

spool /tmp/swg_object.txt

select scene_id||chr(9)||
       o.object_id||chr(9)||
       x||chr(9)||
       y||chr(9)||
       z||chr(9)|| 
       quaternion_w||chr(9)||
       quaternion_x||chr(9)||
       quaternion_y||chr(9)||
       quaternion_z||chr(9)||
       contained_by||chr(9)||  
       object_template_id||chr(9) ||
       (select cell_number from cell_objects c where c.object_id = o.object_id)  || chr(9) ||
       (select value from object_variables_view v where o.object_id = v.object_id and v.name = 'portalProperty.crc') || chr(10)
from objects o
where not exists (select * from tangible_objects t where o.object_id = t.object_id and visible='N')
and not exists (select * from universe_objects t where o.object_id = t.object_id)
start with deleted = 0 and contained_by = 0
and type_id <> 1129465167 -- creature
and type_id <> 1380271939 -- universe
connect by prior object_id = contained_by
and deleted = 0
and type_id <> 1129465167
order by level,object_id;

spool off
set head on
set feedback on

update objects
set cache_version = 1
where not exists (select * from tangible_objects t where objects.object_id = t.object_id and visible='N');

update objects
set deleted = (select reason_code from delete_reasons where tag = 'Publish'), deleted_date = sysdate, load_with = null
where contained_by <> 0
and deleted = 0
and (type_id = 1413566031 or type_id = 1398030671) -- static or tangible
and not exists (select * from scripts where objects.object_id = scripts.object_id)
and script_list is null;

exit

