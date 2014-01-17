create table battlefield_participants
(
	region_object_id number, -- BIND_AS(DB::BindableNetworkId)
	character_object_id number, -- BIND_AS(DB::BindableNetworkId)
	faction_id number,
	constraint pk_battlefield_participants primary key (region_object_id, character_object_id) using index tablespace indexes
);

update version_number set version_number=44, min_version_number=44;
