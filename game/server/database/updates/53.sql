create table mission_list_entry_objects
(
    object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	description_table varchar2(500),
	description_text varchar2(500),
	due_date int,
	mission_data_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	quantity int,
	reward int,
	title_table varchar2(500),
	title_text varchar2(500),
	type varchar2(500),
	constraint pk_mission_list_entry_objects primary key (object_id)
);

grant select on mission_list_entry_objects to public;

update version_number set version_number=53, min_version_number=52;
