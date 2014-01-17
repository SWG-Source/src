create table object_variable_names
(
	id number,
	name varchar2(500),
	constraint pk_object_variable_names primary key (id) using index tablespace indexes
);
create index object_variable_name_index on object_variable_names(name);
grant select on object_variable_names to public;

insert into object_variable_names
select rownum,name from
	(select distinct name from object_variables);

create table object_variables_2 as select object_id,id name_id,type,value,detached from object_variables v, object_variable_names n where v.name = n.name and detached=0;

drop table object_variables;

create table object_variables
(
	object_id number(20),
	name_id number,
	type int,
	value varchar(1000),
	detached int,
	constraint pk_object_variables primary key (object_id,name_id) using index tablespace indexes
);
grant select on object_variables to public;

insert into object_variables (select * from object_variables_2);

drop table object_variables_2;

declare 
	max_name number;
begin
	select nvl(max(id)+1,1) into max_name from object_variable_names;
	execute immediate 'create sequence next_object_variable_name start with ' || max_name;
end;
/

update version_number set version_number=108, min_version_number=108;
