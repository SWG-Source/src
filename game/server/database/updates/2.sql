create table skills
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	sequence_number number,
	skill varchar2(200),
	constraint pk_skills primary key (object_id, sequence_number)
);

create table armor
(
	object_id number(20), -- BIND_AS(DB::BindableNetworkId)
	layer int,
	effectiveness int,
	integrity int,
	special_protections varchar2(1000),
	encumberance_0 int,
	encumberance_1 int,
	encumberance_2 int,
	encumberance_3 int,
	encumberance_4 int,
	encumberance_5 int,
	encumberance_6 int,
	encumberance_7 int,
	encumberance_8 int,
	constraint pk_armor primary key (object_id, layer)
);

update version_number set version_number = 2;
