CREATE OR REPLACE PACKAGE BODY cm_loader is

  FUNCTION get_location_list RETURN swg_cur
  IS

  swg_cur_out swg_cur;

  BEGIN
	-- Clean up orphaned vendors
	DELETE from Auction_Locations
	WHERE owner_id > 0 and not exists
	(select 1 from objects where object_id = location_id and deleted = 0);
	commit;
	
    OPEN swg_cur_out FOR
    SELECT location_id,
    	   owner_id,
           location_name,
           sales_tax,
           sales_tax_bank_id,
	   empty_date,
	   last_access_date,
	   inactive_date,
	   status,
	   search_enabled,
	   entrance_charge
    FROM auction_locations;

    RETURN swg_cur_out;

  END get_location_list;

  FUNCTION get_bid_list RETURN swg_cur
  IS

  swg_cur_out swg_cur;

  BEGIN
    OPEN swg_cur_out FOR
    SELECT item_id,
           bidder_id,
           bid,
           max_proxy_bid
    FROM market_auction_bids;

    RETURN swg_cur_out;

  END get_bid_list;

  FUNCTION get_auction_list RETURN swg_cur
  IS

  swg_cur_out swg_cur;

  BEGIN
    OPEN swg_cur_out FOR
    SELECT creator_id,
           min_bid,
           auction_timer,
           buy_now_price,
           user_description,
           oob,
           location_id,
           item_id,
           category,
           item_timer,
           item_name,
           owner_id,
           active,
           item_size,
           object_template_id
    FROM market_auctions WHERE object_template_id IS NOT NULL;

    RETURN swg_cur_out;

  END get_auction_list;

  FUNCTION get_auction_attributes RETURN swg_cur
  IS

  swg_cur_out swg_cur;

  BEGIN
    OPEN swg_cur_out FOR
    SELECT item_id,
           attribute_name,
           attribute_value
    FROM market_auction_attributes;

    RETURN swg_cur_out;

  END get_auction_attributes;
end cm_loader;
/
