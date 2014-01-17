create table character_types -- NO_IMPORT
(
	id           number not null,
	description  varchar2(1000) not null,
	constraint pk_character_types primary key (id) using index tablespace indexes
);
grant select on character_types to public;

create table default_char_limits -- NO_IMPORT
(
	account_limit  number not null,
	cluster_limit  number not null
);
grant select on default_char_limits to public;

create table extra_character_slots -- NO_IMPORT
(
	station_id	   number not null,
	cluster_id	   number not null,
	character_type_id  number not null,
	num_extra_slots    number not null,
	constraint pk_extra_character_slots primary key (station_id,cluster_id,character_type_id) using index tablespace indexes
);
grant select on extra_character_slots to public;

create table default_character_slots -- NO_IMPORT
(
	character_type_id  number not null,
	num_slots          number not null,
	constraint pk_default_character_slots primary key (character_type_id) using index tablespace indexes
);
grant select on default_character_slots to public;

alter table swg_characters add character_type number;
update swg_characters set character_type = 1;



insert into character_types values (1,'Normal');
insert into character_types values (2,'Jedi');
insert into character_types values (3,'Spectral');

insert into default_char_limits values (30,10000);

insert into default_character_slots values (1,5);
insert into default_character_slots values (2,0);
insert into default_character_slots values (3,8);


update version_number set version_number=83, min_version_number=83;
