whenever sqlerror exit failure rollback

create table temp_names_in_use
as select distinct name_id from object_variables where detached = 0;

alter table temp_names_in_use add primary key (name_id);

analyze table temp_names_in_use compute statistics;

delete object_variable_names 
where id not in (select * from temp_names_in_use);

drop table temp_names_in_use;
