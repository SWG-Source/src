drop package auction;

create index market_item_index on market_auction_bids (item_id) tablespace indexes;

update version_number set version_number=106, min_version_number=106;
