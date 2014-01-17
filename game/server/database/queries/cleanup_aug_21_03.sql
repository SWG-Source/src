whenever sqlerror exit failure rollback

-- tangibles

create table tangible_objects_temp as select t.* from tangible_objects t, objects o where t.object_id = o.object_id;
drop table tangible_objects;
rename tangible_objects_temp to tangible_objects;
alter table tangible_objects add constraint pk_tangible_objects primary key (object_id) using index tablespace &&index_tablespace;
create index owner_id_idx on tangible_objects(owner_id) tablespace &&index_tablespace;
analyze table tangible_objects compute statistics;

-- resource containers

create table res_cont_objects_temp as select t.* from resource_container_objects t, objects o where t.object_id = o.object_id;
drop table resource_container_objects;
rename res_cont_objects_temp to resource_container_objects;
alter table resource_container_objects add constraint pk_resource_container_objects primary key (object_id) using index tablespace &&index_tablespace;
analyze table resource_container_objects compute statistics;

-- object variable names

create table temp_names_in_use
as select distinct name_id from object_variables where detached = 0;

alter table temp_names_in_use add primary key (name_id);

analyze table temp_names_in_use compute statistics;

delete object_variable_names 
where id not in (select * from temp_names_in_use);

drop table temp_names_in_use;
