alter table market_auctions add (item_size number(20) DEFAULT 1 not null);

update version_number set version_number=178, min_version_number=178;

