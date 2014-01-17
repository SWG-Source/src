create table biographies -- NO_IMPORT
(
	object_id number(20),
	biography varchar2(1024),
	constraint pk_biographies primary key (object_id)
);

update version_number set version_number = 31, min_version_number=30;
