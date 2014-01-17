alter table auction_locations add (status number(20) DEFAULT 0 not null);

update version_number set version_number=172, min_version_number=172;
