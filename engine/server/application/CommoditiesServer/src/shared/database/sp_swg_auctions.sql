-------------------------------------------------
-- Export file for user AVALDES                --
-- Created by avaldes on 09-Jun-03, 2:15:33 PM --
-------------------------------------------------

spool sp_swg_auctions.log

prompt
prompt Creating package SWG_COMMODITIES
prompt ================================
prompt
create or replace package swg_commodities is

  -- Author  : AVALDES
  -- Created : 05-Jun-03 4:28:39 PM
  -- Purpose : Stored Procedures for Star Wars Galaxies Commodities
  
  TYPE swg_cur IS REF CURSOR;
  
  FUNCTION create_auction
    (creator_id_in         IN market_auctions.creator_id%TYPE, 
     min_bid_in            IN market_auctions.min_bid%TYPE, 
     auction_timer_in      IN market_auctions.auction_timer%TYPE, 
     buy_now_price_in      IN market_auctions.buy_now_price%TYPE, 
     location_in           IN market_auctions.location%TYPE, 
     item_id_in            IN market_auctions.item_id%TYPE, 
     category_in           IN market_auctions.category%TYPE, 
     item_timer_in         IN market_auctions.item_timer%TYPE, 
     owner_id_in           IN market_auctions.owner_id%TYPE, 
     active_in             IN market_auctions.active%TYPE,
     auction_cur_out       OUT swg_cur)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION create_character
    (character_id_in       IN auction_characters.character_id%TYPE,
     character_name_in     IN auction_characters.character_name%TYPE)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION delete_auction
    (item_id_in           IN market_auctions.item_id%TYPE)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION delete_location
    (location_in           IN auction_locations.location_name%TYPE)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION create_bid 
    (bid_id_in            IN market_auction_bids.bid_id%TYPE,
     item_id_in           IN market_auction_bids.item_id%TYPE,
     bidder_id_in         IN market_auction_bids.bidder_id%TYPE,
     bid_in               IN market_auction_bids.bid%TYPE,
     max_proxy_bid_in     IN market_auction_bids.max_proxy_bid%TYPE)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION create_location
    (owner_id_in          IN auction_locations.owner_id%TYPE,
     location_name_in     IN auction_locations.location_name%TYPE)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION update_auction
    (item_id_in           IN market_auctions.item_id%TYPE,
     active_in            IN market_auctions.active%TYPE,
     owner_id_in          IN market_auctions.owner_id%TYPE)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION update_location
    (oldloc_in            IN auction_locations.location_name%TYPE,
     newloc_in            IN auction_locations.location_name%TYPE,
     sales_tax_in         IN auction_locations.sales_tax%TYPE,
     sales_tax_bank_id_in IN auction_locations.sales_tax_bank_id%TYPE)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION update_item_location
    (oldloc_in            IN market_auctions.location%TYPE,
     newloc_in            IN market_auctions.location%TYPE)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION get_auction_characters
    (auction_cur_out       OUT swg_cur)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION get_auction_locations
      (auction_cur_out       OUT swg_cur)
  RETURN INTEGER;
  --
  --
  --
  FUNCTION get_auction_bids
    (auction_cur_out       OUT swg_cur)
  RETURN INTEGER; 
  --
  --
  --
  FUNCTION get_market_auctions  
    (auction_cur_out       OUT swg_cur)
  RETURN INTEGER;
  
end swg_commodities;
/

prompt
prompt Creating package body SWG_COMMODITIES
prompt =====================================
prompt
create or replace package body swg_commodities is

  FUNCTION create_auction
    (creator_id_in         IN market_auctions.creator_id%TYPE, 
     min_bid_in            IN market_auctions.min_bid%TYPE, 
     auction_timer_in      IN market_auctions.auction_timer%TYPE, 
     buy_now_price_in      IN market_auctions.buy_now_price%TYPE, 
     location_in           IN market_auctions.location%TYPE, 
     item_id_in            IN market_auctions.item_id%TYPE, 
     category_in           IN market_auctions.category%TYPE, 
     item_timer_in         IN market_auctions.item_timer%TYPE, 
     owner_id_in           IN market_auctions.owner_id%TYPE, 
     active_in             IN market_auctions.active%TYPE,
     auction_cur_out       OUT swg_cur)
  RETURN INTEGER IS
  BEGIN
    INSERT INTO market_auctions
      (creator_id, 
       min_bid, 
       auction_timer, 
       buy_now_price, 
       user_description, 
       oob, 
       location, 
       item_id, 
       category, 
       item_timer, 
       item_name, 
       owner_id, 
       active)
    VALUES  
      (creator_id_in, 
       min_bid_in, 
       auction_timer_in, 
       buy_now_price_in, 
       empty_blob(), 
       empty_blob(), 
       location_in, 
       item_id_in, 
       category_in, 
       item_timer_in, 
       empty_blob(), 
       owner_id_in, 
       active_in);
     

    OPEN auction_cur_out FOR
    SELECT user_description, oob, item_name
    FROM market_auctions
    WHERE item_id = item_id_in
    FOR UPDATE;
       
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END create_auction;
  --
  --
  --
  FUNCTION create_character
    (character_id_in       IN auction_characters.character_id%TYPE,
     character_name_in     IN auction_characters.character_name%TYPE)
  RETURN INTEGER IS
  BEGIN
    INSERT INTO auction_characters
      (character_id,
       character_name)
    VALUES
      (character_id_in,
       character_name_in);
       
    COMMIT;
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
      
  END create_character;
  --
  --
  --
  FUNCTION delete_auction
    (item_id_in           IN market_auctions.item_id%TYPE)
  RETURN INTEGER IS
  BEGIN
    DELETE FROM market_auctions
    WHERE item_id = item_id_in;
    
    DELETE FROM market_auction_bids
    WHERE item_id = item_id_in;
    
    COMMIT;
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END delete_auction;
  --
  --
  --
  FUNCTION delete_location
    (location_in           IN auction_locations.location_name%TYPE)
  RETURN INTEGER IS
  BEGIN
    DELETE FROM auction_locations
    WHERE location_name = location_in;
    
    COMMIT;
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END delete_location;
  --
  --
  --
  FUNCTION create_bid 
    (bid_id_in            IN market_auction_bids.bid_id%TYPE,
     item_id_in           IN market_auction_bids.item_id%TYPE,
     bidder_id_in         IN market_auction_bids.bidder_id%TYPE,
     bid_in               IN market_auction_bids.bid%TYPE,
     max_proxy_bid_in     IN market_auction_bids.max_proxy_bid%TYPE)
  RETURN INTEGER IS
  BEGIN
    INSERT INTO market_auction_bids
      (bid_id, 
       item_id, 
       bidder_id, 
       bid, 
       max_proxy_bid)
    VALUES
      (bid_id_in, 
       item_id_in, 
       bidder_id_in, 
       bid_in, 
       max_proxy_bid_in);
       
    COMMIT;
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END create_bid;
  --
  --
  --
  FUNCTION create_location
    (owner_id_in          IN auction_locations.owner_id%TYPE,
     location_name_in     IN auction_locations.location_name%TYPE)
  RETURN INTEGER IS
  BEGIN
    INSERT INTO auction_locations
      (owner_id,
       location_name)
    VALUES  
      (owner_id_in,
       location_name_in);
       
    COMMIT;
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END create_location;
  --
  --
  --
  FUNCTION update_auction
    (item_id_in           IN market_auctions.item_id%TYPE,
     active_in            IN market_auctions.active%TYPE,
     owner_id_in          IN market_auctions.owner_id%TYPE)
  RETURN INTEGER IS
  BEGIN
    UPDATE market_auctions
    SET active = active_in,
        owner_id = owner_id_in
    WHERE item_id = item_id_in;
    
    COMMIT;
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END update_auction;
  --
  --
  --
  FUNCTION update_location
    (oldloc_in            IN auction_locations.location_name%TYPE,
     newloc_in            IN auction_locations.location_name%TYPE,
     sales_tax_in         IN auction_locations.sales_tax%TYPE,
     sales_tax_bank_id_in IN auction_locations.sales_tax_bank_id%TYPE)
  RETURN INTEGER IS
  BEGIN
    UPDATE auction_locations
    SET location_name = newloc_in, sales_tax = sales_tax_in, sales_tax_bank_id = sales_tax_bank_id_in
    WHERE location_name = oldloc_in;

    COMMIT;
    RETURN 0;

    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END update_location;
  --
  --
  --
  FUNCTION update_item_location
    (oldloc_in            IN market_auctions.location%TYPE,
     newloc_in            IN market_auctions.location%TYPE)
  RETURN INTEGER IS
  BEGIN
    UPDATE market_auctions
    SET location = newloc_in
    WHERE location = oldloc_in;

    COMMIT;
    RETURN 0;

    EXCEPTION
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END update_item_location;
  --
  --
  --
  FUNCTION get_auction_characters
    (auction_cur_out       OUT swg_cur)
  RETURN INTEGER IS
  BEGIN
    OPEN auction_cur_out FOR
    SELECT character_id, 
           character_name
    FROM auction_characters;
    
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      RETURN SQLCODE;
  END get_auction_characters;
  --
  --
  --
  FUNCTION get_auction_locations
    (auction_cur_out       OUT swg_cur)
  RETURN INTEGER IS
  BEGIN
    OPEN auction_cur_out FOR
    SELECT owner_id,
           location_name,
           sales_tax,
           sales_tax_bank_id
    FROM auction_locations;
    
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      RETURN SQLCODE;
  END get_auction_locations;
  --
  --
  --
  FUNCTION get_auction_bids
    (auction_cur_out       OUT swg_cur)
  RETURN INTEGER IS
  BEGIN
    OPEN auction_cur_out FOR
    SELECT bid_id, 
           item_id, 
           bidder_id, 
           bid, 
           max_proxy_bid
    FROM market_auction_bids;
    
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      RETURN SQLCODE;
  END get_auction_bids; 
  --
  --
  --
  FUNCTION get_market_auctions  
    (auction_cur_out       OUT swg_cur)
  RETURN INTEGER IS
  BEGIN
    OPEN auction_cur_out FOR
    SELECT creator_id, 
           min_bid, 
           auction_timer, 
           buy_now_price, 
           user_description, 
           oob, 
           location, 
           item_id, 
           category, 
           item_timer, 
           item_name, 
           owner_id, 
           active
    FROM market_auctions
    FOR UPDATE;
    
    RETURN 0;
    
    EXCEPTION
      WHEN OTHERS THEN
      RETURN SQLCODE;
  END get_market_auctions;
  
end swg_commodities;
/


spool off
