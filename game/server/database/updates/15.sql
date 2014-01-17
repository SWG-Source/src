create table player_objects  -- NO_IMPORT
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	house_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	station_id int,
	constraint pk_player_objects primary key (object_id) using index tablespace indexes
)
storage (initial 4k next 4k);

create table accounts  -- NO_IMPORT
(
        station_id int,
	num_lots int,
        is_outcast char(1),
	constraint pk_accounts primary key (station_id) using index tablespace indexes
)
storage (initial 4k next 4k);

update version_number set version_number = 15, min_version_number=13;
