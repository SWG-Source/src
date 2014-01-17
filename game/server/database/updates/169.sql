-- AUCTION_LOCATIONS

-- remove duplicate location
create table temp_locations(location_name varchar2(256), object_id number);
insert into temp_locations
select LOCATION_NAME, substr(LOCATION_NAME, instr(LOCATION_NAME, '.', -1) + 1,
                      instr(LOCATION_NAME, '#') - instr(LOCATION_NAME, '.', -1) - 1)
from auction_locations
where substr(LOCATION_NAME, instr(LOCATION_NAME, '.', -1) + 1,
      instr(LOCATION_NAME, '#') - instr(LOCATION_NAME, '.', -1) - 1) in
      (select substr(LOCATION_NAME, instr(LOCATION_NAME, '.', -1) + 1,
              instr(LOCATION_NAME, '#') - instr(LOCATION_NAME, '.', -1) - 1)
       from auction_locations
       group by substr(LOCATION_NAME, instr(LOCATION_NAME, '.', -1) + 1,
                instr(LOCATION_NAME, '#') - instr(LOCATION_NAME, '.', -1) - 1)
       having count(*) > 1);

delete temp_locations where location_name in
(select location from market_auctions);
commit;

delete temp_locations where location_name in
(select max(location_name) from temp_locations
 group by object_id
 having count(*) > 1);
commit;

delete auction_locations where location_name in (
select location_name from temp_locations);
commit;
drop table temp_locations;

-- drop the primary key from auction_locations
alter table auction_locations drop primary key;

-- add a location_id column
alter table auction_locations add (location_id number(20) NULL);

-- populate the location_id values from location
update auction_locations 
	set location_id = nvl (to_number(substr(location_name, instr(location_name, '.', 1, 3)+1, 
	instr(location_name, '#', -1)-instr(location_name, '.', 1, 3)-1)), 0);
commit;

-- add the primary key
alter table auction_locations add constraint 
pk_auction_locations primary key (location_id) using index tablespace indexes;

-- create the location_name index
create index auction_location_name_idx on auction_locations (location_name) tablespace indexes;


-- MARKET_AUCTIONS
-- rename old Market_Auctions
alter table market_auctions rename to market_auctions_old2;

-- create new Market_Auctions
create table market_auctions --NO_IMPORT
(
	item_id          number(20), -- BIND_AS(DB::BindableNetworkId)
	location_id      number(20), -- BIND_AS(DB::BindableNetworkId)
	creator_id       number(20), -- BIND_AS(DB::BindableNetworkId)
	owner_id         number(20), -- BIND_AS(DB::BindableNetworkId)
	min_bid          number(20), -- BIND_AS(DB::BindableNetworkId)
	buy_now_price    number(20), -- BIND_AS(DB::BindableNetworkId)
	auction_timer    number(20), -- BIND_AS(DB::BindableNetworkId)
	user_description varchar2(4000),
	item_name        varchar2(4000),
	oob              varchar2(4000),
	category         number(20), -- BIND_AS(DB::BindableNetworkId)
	item_timer       number(20), -- BIND_AS(DB::BindableNetworkId)
	active           number(20)  -- BIND_AS(DB::BindableNetworkId)
);
grant select on market_auctions to public;

-- populate Market_Acutions from old table
insert into market_auctions 
(select item_id, nvl (to_number(substr(location, instr(location, '.', 1, 3)+1,
	               instr(location, '#', -1)-instr(location, '.', 1, 3)-1)), 0) as location_id,
	creator_id, owner_id, min_bid, buy_now_price,
	auction_timer,	user_description, item_name, oob, category, item_timer, active
 from market_auctions_old2 );
commit;

-- rename the primary key index on old Market Auctions
alter table market_auctions_old2 drop primary key;
alter table market_auctions_old2 add constraint 
pk_market_auctions_old2 primary key (item_id) using index tablespace indexes;

-- add the primary key
alter table market_auctions add constraint 
pk_market_auctions primary key (item_id) using index tablespace indexes;

-- create index on location_id
create index market_auction_location_idx on market_auctions (location_id) tablespace indexes;

-- create dummy Auction_Locations record for FK violations
insert into auction_locations
(select 0, mo.location, 0, 0, ma.location_id
from market_auctions ma, market_auctions_old2 mo
where ma.location_id not in (select location_id from auction_locations)
and ma.item_id = mo.item_id);
commit;

-- add foreign keys to market_auctions
alter table market_auctions add constraint market_auctions_location_id_fk
  foreign key(location_id) references auction_locations(location_id) on delete cascade;

-- drop old Market_Auctions table
drop table market_auctions_old2;


-- MARKET_AUCTION_BIDS
-- update bid amount and delete duplicate item_id records
update market_auction_bids set max_proxy_bid = bid where max_proxy_bid < bid;
commit;

alter table market_auction_bids add (iswinning number DEFAULT 1);

update market_auction_bids set iswinning = 0 where bid_id in
(select distinct b1.bid_id from market_auction_bids b1, market_auction_bids b2
where b1.item_id = b2.item_id and (b1.max_proxy_bid < b2.max_proxy_bid
or (b1.max_proxy_bid = b2.max_proxy_bid and b1.bid_id > b2.bid_id)));
commit;

update market_auction_bids a
set a.bid = nvl((select max(b.max_proxy_bid) from market_auction_bids b
where b.item_id = a.item_id and b.bid_id > a.bid_id
and b.max_proxy_bid > a.bid), bid) where a.iswinning = 1;
commit;

update market_auction_bids a
set a.bid = nvl((select max(b.max_proxy_bid+1) from market_auction_bids b
where b.item_id = a.item_id and b.bid_id < a.bid_id
and b.max_proxy_bid > a.bid), bid) where a.iswinning = 1;
commit;

delete from market_auction_bids where iswinning=0;
commit;

-- drop primary key
alter table market_auction_bids drop primary key;

-- drop bid_id column
alter table market_auction_bids drop column iswinning;
alter table market_auction_bids drop column bid_id;

-- add primary key on item_id
declare
cnt integer;
begin
	select count(*) into cnt from user_indexes where index_name = 'MARKET_ITEM_INDEX';
	if (cnt > 0) then
		execute immediate 'drop index market_item_index';
	end if;
end;
/
alter table market_auction_bids add constraint 
pk_market_auction_bids primary key (item_id) using index tablespace indexes;

-- drop record with FK violations
delete from market_auction_bids where item_id not in
(select item_id from market_auctions);
commit;

-- add foreign keys to market_auction_bids 
alter table market_auction_bids add constraint market_auction_bids_item_id_fk 
  foreign key(item_id) references market_auctions on delete cascade;


-- drop auction_bids table and swg_commodities package
drop table auction_bids;
drop package swg_commodities;

-- update version number
update version_number set version_number=169, min_version_number=169;
commit;
