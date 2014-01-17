create table commands
(
	object_id number(20), -- BIND_AS(DB::BindableNetworkId)
	command_type int,
	sequence_no int,
	command varchar2(500),
	constraint pk_commands primary key (object_id, command_type, sequence_no) using index tablespace indexes
) storage (initial 4k next 4k);

update version_number set version_number = 3;
