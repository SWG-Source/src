create table bounty_hunter_targets
(
	object_id number(20) NOT NULL,  -- BIND_AS(DB::BindableNetworkId)
	target_id number(20) NOT NULL,  -- BIND_AS(DB::BindableNetworkId)
	constraint pk_bounty_hunter_targets primary key (object_id)
);
grant select on bounty_hunter_targets to public;

update version_number set version_number=180, min_version_number=180;

