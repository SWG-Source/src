drop table auction_locations;

create table auction_characters
(
	character_id      number(20), 
	character_name    varchar2(64),
	constraint pk_auction_characters primary key (character_id) using index tablespace indexes
);
grant select on auction_characters to public;
create table auction_locations
(
	owner_id      number(20), -- BIND_AS(DB::BindableNetworkId)
	location_name varchar2(256),
	constraint pk_auction_locations primary key (location_name) using index tablespace indexes
);
grant select on auction_locations to public;
create table market_auction_bids
(
	bid_id           number(20),
	item_id          number(20),
	bidder_id        number(20),
	bid              number(20),
	max_proxy_bid    number(20),
	constraint pk_market_auction_bids primary key (bid_id) using index tablespace indexes
);
grant select on market_auction_bids to public;
create table market_auctions
(
	creator_id       number(20),
	min_bid          number(20),
	auction_timer    number(20),
	buy_now_price    number(20),
	user_description BLOB,
	oob              BLOB,
	location         varchar2(256),
	item_id          number(20),
	category         number(20),
	item_timer       number(20),
	item_name        BLOB,
	owner_id         number(20),
	active           number(20),
	constraint pk_market_auctions primary key (item_id) using index tablespace indexes
);
grant select on market_auctions to public;

update version_number set version_number=74, min_version_number=74;
