create or replace view object_variables_view as
select v.object_id, n.name, v.type, v.value
from object_variables v, object_variable_names n
where v.name_id = n.id
and v.detached = 0
union all
select o.object_id, o.objvar_0_name, o.objvar_0_type, o.objvar_0_value
from objects o
where o.objvar_0_type >= 0
union all
select o.object_id, o.objvar_1_name, o.objvar_1_type, o.objvar_1_value
from objects o
where o.objvar_1_type >= 0
union all
select o.object_id, o.objvar_2_name, o.objvar_2_type, o.objvar_2_value
from objects o
where o.objvar_2_type >= 0
union all
select o.object_id, o.objvar_3_name, o.objvar_3_type, o.objvar_3_value
from objects o
where o.objvar_3_type >= 0
union all
select o.object_id, o.objvar_4_name, o.objvar_4_type, o.objvar_4_value
from objects o
where o.objvar_4_type >= 0
union all
select o.object_id, o.objvar_5_name, o.objvar_5_type, o.objvar_5_value
from objects o
where o.objvar_5_type >= 0
union all
select o.object_id, o.objvar_6_name, o.objvar_6_type, o.objvar_6_value
from objects o
where o.objvar_6_type >= 0
union all
select o.object_id, o.objvar_7_name, o.objvar_7_type, o.objvar_7_value
from objects o
where o.objvar_7_type >= 0
union all
select o.object_id, o.objvar_8_name, o.objvar_8_type, o.objvar_8_value
from objects o
where o.objvar_8_type >= 0
union all
select o.object_id, o.objvar_9_name, o.objvar_9_type, o.objvar_9_value
from objects o
where o.objvar_9_type >= 0;

update version_number set version_number=134, min_version_number=134;
