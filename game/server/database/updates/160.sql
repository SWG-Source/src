create table ship_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	primary key (object_id)
);

grant select on ship_objects to public;

update version_number set version_number=160, min_version_number=160;
