create or replace view object_variables_view as
select v.object_id, n.name, v.type, v.value, -1 packed
from object_variables v, object_variable_names n
where v.name_id = n.id
and v.detached = 0
union all
select o.object_id, o.objvar_0_name, o.objvar_0_type, o.objvar_0_value, 0
from objects o
where o.objvar_0_type >= 0
union all
select o.object_id, o.objvar_1_name, o.objvar_1_type, o.objvar_1_value, 1
from objects o
where o.objvar_1_type >= 0
union all
select o.object_id, o.objvar_2_name, o.objvar_2_type, o.objvar_2_value, 2
from objects o
where o.objvar_2_type >= 0
union all
select o.object_id, o.objvar_3_name, o.objvar_3_type, o.objvar_3_value, 3
from objects o
where o.objvar_3_type >= 0
union all
select o.object_id, o.objvar_4_name, o.objvar_4_type, o.objvar_4_value, 4
from objects o
where o.objvar_4_type >= 0
union all
select o.object_id, o.objvar_5_name, o.objvar_5_type, o.objvar_5_value, 5
from objects o
where o.objvar_5_type >= 0
union all
select o.object_id, o.objvar_6_name, o.objvar_6_type, o.objvar_6_value, 6
from objects o
where o.objvar_6_type >= 0
union all
select o.object_id, o.objvar_7_name, o.objvar_7_type, o.objvar_7_value, 7
from objects o
where o.objvar_7_type >= 0
union all
select o.object_id, o.objvar_8_name, o.objvar_8_type, o.objvar_8_value, 8
from objects o
where o.objvar_8_type >= 0
union all
select o.object_id, o.objvar_9_name, o.objvar_9_type, o.objvar_9_value, 9
from objects o
where o.objvar_9_type >= 0
union all
select o.object_id, o.objvar_10_name, o.objvar_10_type, o.objvar_10_value, 10
from objects o
where o.objvar_10_type >= 0
union all
select o.object_id, o.objvar_11_name, o.objvar_11_type, o.objvar_11_value, 11
from objects o
where o.objvar_11_type >= 0
union all
select o.object_id, o.objvar_12_name, o.objvar_12_type, o.objvar_12_value, 12
from objects o
where o.objvar_12_type >= 0
union all
select o.object_id, o.objvar_13_name, o.objvar_13_type, o.objvar_13_value, 13
from objects o
where o.objvar_13_type >= 0
union all
select o.object_id, o.objvar_14_name, o.objvar_14_type, o.objvar_14_value, 14
from objects o
where o.objvar_14_type >= 0
union all
select o.object_id, o.objvar_15_name, o.objvar_15_type, o.objvar_15_value, 15
from objects o
where o.objvar_15_type >= 0
union all
select o.object_id, o.objvar_16_name, o.objvar_16_type, o.objvar_16_value, 16
from objects o
where o.objvar_16_type >= 0
union all
select o.object_id, o.objvar_17_name, o.objvar_17_type, o.objvar_17_value, 17
from objects o
where o.objvar_17_type >= 0
union all
select o.object_id, o.objvar_18_name, o.objvar_18_type, o.objvar_18_value, 18
from objects o
where o.objvar_18_type >= 0
union all
select o.object_id, o.objvar_19_name, o.objvar_19_type, o.objvar_19_value, 19
from objects o
where o.objvar_19_type >= 0;

alter table harvester_installation_objects add resource_type number(20);

update harvester_installation_objects
set resource_type = nvl((select resource_type from resource_pool_objects p where p.object_id = harvester_installation_objects.resource_pool),0);

alter table harvester_installation_objects drop column resource_pool;

create table resource_types
(
	resource_id number(20), -- BIND_AS(DB::BindableNetworkId)
	resource_name varchar2(100),
	resource_class varchar2(100),
	attributes varchar2(1024),
	fractal_seeds varchar2(1024),
	depleted_timestamp int,
	constraint pk_resource_types primary key (resource_id) using index tablespace indexes
);

grant select on resource_types to public;

insert into resource_types (resource_id, resource_name, resource_class)
select t.object_id, t.resource_name, t.resource_class
from resource_type_objects t, objects o
where t.object_id = o.object_id
and o.deleted = 0;

commit;
analyze table resource_types estimate statistics;

create table temp_depleted_timestamps
as select resource_type, max(nvl(depleted_timestamp,0)) max_timestamp 
from resource_pool_objects
where resource_type is not null
group by resource_type;

alter table temp_depleted_timestamps add constraint pk_tdt primary key (resource_type);
analyze table temp_depleted_timestamps estimate statistics;

update resource_types
set depleted_timestamp = nvl((select max_timestamp from temp_depleted_timestamps p where p.resource_type = resource_id),0);

drop table temp_depleted_timestamps;

-- build list of fractal seeds
declare
	last_type number;
	seed_string varchar2(1024);
begin
	last_type := 0;
	for x in (select planet, map_seed, resource_type from resource_pool_objects where planet <> 0 and resource_type <> 0 order by resource_type, planet)
	loop
		if (x.resource_type <> last_type and last_type <> 0) then
			update resource_types
			set fractal_seeds = seed_string
			where resource_types.resource_id = last_type;

			seed_string := NULL;
		end if;

		seed_string := seed_string || x.planet || ' ' || x.map_seed || ':';
		last_type := x.resource_type;
	end loop;

	if (last_type <> 0) then
		update resource_types
		set fractal_seeds = seed_string
		where resource_types.resource_id = last_type;
	end if;
end;
/

commit;

-- copy resource attributes

declare
	last_type number := 0;
	attribute_string varchar2(1024);
begin
	for x in (select v.object_id, substr(name,12) ||  ' ' || value || ':' value_string from object_variables_view v, resource_type_objects r where r.object_id = v.object_id and name like 'attributes.%' order by r.object_id, packed)
	loop
		if (x.object_id <> last_type and last_type <> 0) then
			update resource_types
			set attributes = attribute_string
			where resource_types.resource_id = last_type;

			attribute_string := NULL;
		end if;

		attribute_string := attribute_string || x.value_string;
		last_type := x.object_id;
	end loop;

	if (last_type <> 0) then
		update resource_types
		set attributes = attribute_string
		where resource_types.resource_id = last_type;
	end if;
end;
/

commit;

-- delete objects

update objects
set deleted = 8, deleted_date = sysdate, load_with = null
where exists (select 1 from resource_pool_objects p where p.object_id = objects.object_id) and deleted = 0;

commit;

update objects
set deleted = 8, deleted_date = sysdate, load_with = null
where exists (select 1 from resource_type_objects t where t.object_id = objects.object_id) and deleted = 0;

commit;

update version_number set version_number=167, min_version_number=167;
