create table market_auctions_temp --NO_IMPORT
(
	creator_id       number(20), -- BIND_AS(DB::BindableNetworkId)
	min_bid          number(20), -- BIND_AS(DB::BindableNetworkId)
	auction_timer    number(20), -- BIND_AS(DB::BindableNetworkId)
	buy_now_price    number(20), -- BIND_AS(DB::BindableNetworkId)
	user_description varchar2(4000),
	oob              varchar2(4000),
	location         varchar2(256),
	item_id          number(20), -- BIND_AS(DB::BindableNetworkId)
	category         number(20), -- BIND_AS(DB::BindableNetworkId)
	item_timer       number(20), -- BIND_AS(DB::BindableNetworkId)
	item_name        varchar2(4000),
	owner_id         number(20), -- BIND_AS(DB::BindableNetworkId)
	active           number(20), -- BIND_AS(DB::BindableNetworkId)
	constraint pk_market_auctions_temp primary key (item_id) using index tablespace indexes
);

insert into market_auctions_temp select
creator_id,	
min_bid, 
auction_timer, 
buy_now_price, 
dbms_lob.substr(user_description,dbms_lob.getlength(user_description),1), 
dbms_lob.substr(oob,dbms_lob.getlength(oob),1), 
location, 
item_id, 
category, 
item_timer, 
dbms_lob.substr(item_name,dbms_lob.getlength(item_name),1), 
owner_id, 
active
from market_auctions;

rename market_auctions to market_auctions_old;

rename market_auctions_temp to market_auctions;

grant select on market_auctions to public;

/

