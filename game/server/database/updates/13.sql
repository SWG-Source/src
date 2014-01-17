create table cluster_list
(
	id number, --NO_BIND
	name varchar2(255) --NO_BIND
);

create table swg_characters -- NO_IMPORT
(
	STATION_ID      NUMBER NOT NULL,
	CLUSTER_ID      NUMBER NOT NULL,
	CHARACTER_NAME  VARCHAR2(127) NOT NULL,
	OBJECT_ID       NUMBER,
	TEMPLATE_NAME   VARCHAR2(255)
);

alter table SWG_CHARACTERS
add constraint pk_swg_character PRIMARY KEY (station_id, cluster_id, character_name)
using index tablespace indexes;

create table temp_characters -- NO_IMPORT
(
TRANSACTION_TYPE                                            NUMBER,
STATION_ID                                                  NUMBER,
CLUSTER_NAME                                                VARCHAR2(120),
CHARACTER_NAME                                              VARCHAR2(30),
OBJECT_ID                                                   NUMBER,
TEMPLATE_NAME                                               VARCHAR2(120)
);

update version_number set version_number = 13, min_version_number=13;
