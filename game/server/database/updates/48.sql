update objects set deleted = 1 where object_id in (select object_id from resource_class_objects);
update objects set deleted = 1 where object_id in (select object_id from resource_type_objects);
update objects set deleted = 1 where object_id in (select object_id from resource_pool_objects);
drop table resource_class_objects;
drop table resource_type_objects;
drop table resource_import;
drop table res_imp_oid_map;
drop table res_imp_object_variables;

create table resource_type_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	resource_name varchar(100),
	resource_class varchar(100),
	min_pools int,
	constraint pk_resource_type_objects primary key (object_id) using index tablespace indexes
);

grant select on resource_type_objects to public;

update version_number set version_number=48, min_version_number=48;
