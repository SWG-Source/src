whenever sqlerror exit rollback;

create table property_lists
(
	object_id number(20), -- BIND_AS(DB::BindableNetworkId)
	list_id int,
	sequence_no int,
	value varchar2(500),
	constraint pk_property_lists primary key (object_id, list_id, sequence_no) using index tablespace indexes
);

grant select on property_lists to public;

insert into property_lists select * from commands;
insert into property_lists select object_id,11,sequence_number,skill from skills;

drop table commands;
drop table skills;

update version_number set version_number=78, min_version_number=78;
