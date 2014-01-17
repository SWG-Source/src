create table location_lists -- NO_IMPORT
(
	object_id number,
	list_id number,
	sequence_number number,
	name varchar2(255),
	scene varchar2(50),
	x number,
	y number,
	z number,
	radius number,
	constraint pk_location_lists primary key (object_id, list_id, sequence_number)
);

update version_number set version_number = 28, min_version_number=28;
