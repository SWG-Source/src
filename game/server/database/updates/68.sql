create table auction_locations
(
    owner_id      number(20), -- BIND_AS(DB::BindableNetworkId)
    owner_name    varchar2(64),
    location_name varchar2(256),
    constraint pk_auction_locations primary key (location_name) using index tablespace indexes
);
grant select on auction_locations to public;

update version_number set version_number=68, min_version_number=68;
