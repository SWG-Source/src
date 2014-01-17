create table commands_temp as select * from commands;
create table accounts_temp as select * from accounts;
create table cluster_list_temp as select * from cluster_list;
create table player_objects_temp as select * from player_objects;

drop table commands;
drop table accounts;
drop table cluster_list;
drop table player_objects;

create table commands
(
	object_id number(20), -- BIND_AS(DB::BindableNetworkId)
	command_type int,
	sequence_no int,
	command varchar2(500),
	constraint pk_commands primary key (object_id, command_type, sequence_no) using index tablespace indexes
);
grant select on commands to public;
create table accounts  -- NO_IMPORT
(
	station_id int,
	num_lots int,
	is_outcast char(1),
	move_check_time_ms int,
	constraint pk_accounts primary key (station_id) using index tablespace indexes
);
grant select on accounts to public;
create table cluster_list --NO_IMPORT
(
	id number, 
	name varchar2(255),
	num_characters number
);
alter table cluster_list
add constraint pk_cluster_list PRIMARY KEY (id)
using index tablespace indexes;
grant select on cluster_list to public;
create table player_objects  -- NO_IMPORT
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	house_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	station_id int,
	constraint pk_player_objects primary key (object_id) using index tablespace indexes
);
grant select on player_objects to public;

insert into commands select * from commands_temp;
insert into accounts select * from accounts_temp;
insert into cluster_list select * from cluster_list_temp;
insert into player_objects select * from player_objects_temp;

drop table commands_temp;
drop table accounts_temp;
drop table cluster_list_temp;
drop table player_objects_temp;

update version_number set version_number=59, min_version_number=56;
