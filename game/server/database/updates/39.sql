create table auction_bids
(
	item_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	bidder_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	bid_amount int,
	constraint pk_auction_bids primary key (item_id) using index tablespace indexes
);

update version_number set version_number=39, min_version_number=39;
