create or replace view zero_mass AS
select v.object_id, n.name, v.type, v.value, -1 packed
from object_variables v, object_variable_names n
where v.name_id = n.id
and v.detached = 0 
and name_id in
(
        select id from object_variable_names
        where name like '%ship_comp.mass%'
)
and value = '0.000000'
union all
select o.object_id, o.objvar_0_name, o.objvar_0_type, o.objvar_0_value, 0
from objects o
where o.objvar_0_type >= 0
and objvar_0_name like '%ship_comp.mass%'
and objvar_0_value = '0.000000'
union all
select o.object_id, o.objvar_1_name, o.objvar_1_type, o.objvar_1_value, 1
from objects o
where o.objvar_1_type >= 0
and objvar_1_name like '%ship_comp.mass%'
and objvar_1_value = '0.000000'
union all
select o.object_id, o.objvar_2_name, o.objvar_2_type, o.objvar_2_value, 2
from objects o
where o.objvar_2_type >= 0
and objvar_2_name like '%ship_comp.mass%'
and objvar_2_value = '0.000000'
union all
select o.object_id, o.objvar_3_name, o.objvar_3_type, o.objvar_3_value, 3
from objects o
where o.objvar_3_type >= 0
and objvar_3_name like '%ship_comp.mass%'
and objvar_3_value = '0.000000'
union all
select o.object_id, o.objvar_4_name, o.objvar_4_type, o.objvar_4_value, 4
from objects o
where o.objvar_4_type >= 0
and objvar_4_name like '%ship_comp.mass%'
and objvar_4_value = '0.000000'
union all
select o.object_id, o.objvar_5_name, o.objvar_5_type, o.objvar_5_value, 5
from objects o
where o.objvar_5_type >= 0
and objvar_5_name like '%ship_comp.mass%'
and objvar_5_value = '0.000000'
union all
select o.object_id, o.objvar_6_name, o.objvar_6_type, o.objvar_6_value, 6
from objects o
where o.objvar_6_type >= 0
and objvar_6_name like '%ship_comp.mass%'
and objvar_6_value = '0.000000'
union all
select o.object_id, o.objvar_7_name, o.objvar_7_type, o.objvar_7_value, 7
from objects o
where o.objvar_7_type >= 0
and objvar_7_name like '%ship_comp.mass%'
and objvar_7_value = '0.000000'
union all
select o.object_id, o.objvar_8_name, o.objvar_8_type, o.objvar_8_value, 8
from objects o
where o.objvar_8_type >= 0
and objvar_8_name like '%ship_comp.mass%'
and objvar_8_value = '0.000000'
union all
select o.object_id, o.objvar_9_name, o.objvar_9_type, o.objvar_9_value, 9
from objects o
where o.objvar_9_type >= 0
and objvar_9_name like '%ship_comp.mass%'
and objvar_9_value = '0.000000'
union all
select o.object_id, o.objvar_10_name, o.objvar_10_type, o.objvar_10_value, 10
from objects o
where o.objvar_10_type >= 0
and objvar_10_name like '%ship_comp.mass%'
and objvar_10_value = '0.000000'
union all
select o.object_id, o.objvar_11_name, o.objvar_11_type, o.objvar_11_value, 11
from objects o
where o.objvar_11_type >= 0
and objvar_11_name like '%ship_comp.mass%'
and objvar_11_value = '0.000000'
union all
select o.object_id, o.objvar_12_name, o.objvar_12_type, o.objvar_12_value, 12
from objects o
where o.objvar_12_type >= 0
and objvar_12_name like '%ship_comp.mass%'
and objvar_12_value = '0.000000'
union all
select o.object_id, o.objvar_13_name, o.objvar_13_type, o.objvar_13_value, 13
from objects o
where o.objvar_13_type >= 0
and objvar_13_name like '%ship_comp.mass%'
and objvar_13_value = '0.000000'
union all
select o.object_id, o.objvar_14_name, o.objvar_14_type, o.objvar_14_value, 14
from objects o
where o.objvar_14_type >= 0
and objvar_14_name like '%ship_comp.mass%'
and objvar_14_value = '0.000000'
union all
select o.object_id, o.objvar_15_name, o.objvar_15_type, o.objvar_15_value, 15
from objects o
where o.objvar_15_type >= 0
and objvar_15_name like '%ship_comp.mass%'
and objvar_15_value = '0.000000'
union all
select o.object_id, o.objvar_16_name, o.objvar_16_type, o.objvar_16_value, 16
from objects o
where o.objvar_16_type >= 0
and objvar_16_name like '%ship_comp.mass%'
and objvar_16_value = '0.000000'
union all
select o.object_id, o.objvar_17_name, o.objvar_17_type, o.objvar_17_value, 17
from objects o
where o.objvar_17_type >= 0
and objvar_17_name like '%ship_comp.mass%'
and objvar_17_value = '0.000000'
union all
select o.object_id, o.objvar_18_name, o.objvar_18_type, o.objvar_18_value, 18
from objects o
where o.objvar_18_type >= 0
and objvar_18_name like '%ship_comp.mass%'
and objvar_18_value = '0.000000'
union all
select o.object_id, o.objvar_19_name, o.objvar_19_type, o.objvar_19_value, 19
from objects o
where o.objvar_19_type >= 0
and objvar_19_name like '%ship_comp.mass%'
and objvar_19_value = '0.000000';

select c.name, count(*) from 
zero_mass a, objects b, object_templates c
where a.object_id = b.object_id 
and b.object_template_id = c.id (+)
group by c.name; 

