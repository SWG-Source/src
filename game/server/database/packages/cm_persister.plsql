CREATE OR REPLACE package body cm_persister
as

	procedure update_auction_locations (p_location_id VAOFSTRING, 
					 p_location_name VAOFSTRING, 
					 p_owner_id VAOFSTRING, 
					 p_sales_tax VAOFNUMBER, 
					 p_sales_tax_bank_id VAOFSTRING, 
					 p_empty_date VAOFNUMBER, 
					 p_last_access_date VAOFNUMBER, 
					 p_inactive_date VAOFNUMBER, 
					 p_status VAOFNUMBER, 
					 p_search_enabled VAOFSTRING, 
					 p_entrance_charge VAOFNUMBER, 
					 p_chunk_size number)
	as
		errors number;
		dml_errors EXCEPTION;
		PRAGMA exception_init(dml_errors, -24381);
	begin
		FORALL i in 1..p_chunk_size save exceptions
			UPDATE AUCTION_LOCATIONS Set
				location_name = DECODE(p_location_name(i), NULL, location_name, p_location_name(i)),
				owner_id = DECODE(p_owner_id(i), NULL, owner_id, p_owner_id(i)),
				sales_tax = DECODE(p_sales_tax(i), NULL, sales_tax, p_sales_tax(i)),
				sales_tax_bank_id = DECODE(p_sales_tax_bank_id(i), NULL, sales_tax_bank_id, p_sales_tax_bank_id(i)),
				empty_date = DECODE(p_empty_date(i), NULL, empty_date, p_empty_date(i)),
				last_access_date = DECODE(p_last_access_date(i), NULL, last_access_date, p_last_access_date(i)),
				inactive_date = DECODE(p_inactive_date(i), NULL, inactive_date, p_inactive_date(i)),
				status = DECODE(p_status(i), NULL, status, p_status(i)),
				search_enabled = DECODE(p_search_enabled(i), NULL, search_enabled, p_search_enabled(i)),
				entrance_charge = DECODE(p_entrance_charge(i), NULL, entrance_charge, p_entrance_charge(i))
			WHERE
				location_id = p_location_id(i);
		exception
			when dml_errors then
				errors := SQL%BULK_EXCEPTIONS.count;
	end;

	procedure insert_auction_locations (p_location_id VAOFSTRING, 
					 p_location_name VAOFSTRING, 
					 p_owner_id VAOFSTRING, 
					 p_sales_tax VAOFNUMBER, 
					 p_sales_tax_bank_id VAOFSTRING, 
					 p_empty_date VAOFNUMBER, 
					 p_last_access_date VAOFNUMBER, 
					 p_inactive_date VAOFNUMBER, 
					 p_status VAOFNUMBER, 
					 p_search_enabled VAOFSTRING, 
					 p_entrance_charge VAOFNUMBER, 
					 p_chunk_size number)
	as
		errors number;
		k number;
		dml_errors EXCEPTION;
		PRAGMA exception_init(dml_errors, -24381);
	begin
		FORALL i in 1..p_chunk_size save exceptions
			INSERT into auction_locations (
				location_id,
				location_name,
				owner_id,
				sales_tax,
				sales_tax_bank_id,
				empty_date,
				last_access_date,
				inactive_date,
				status,
				search_enabled,
				entrance_charge )
			VALUES (
				p_location_id(i),
				p_location_name(i),
				p_owner_id(i),
				p_sales_tax(i),
				p_sales_tax_bank_id(i),
				p_empty_date(i),
				p_last_access_date(i),
				p_inactive_date(i),
				p_status(i),
				p_search_enabled(i),
				p_entrance_charge(i) );
		exception
			when dml_errors then
				errors := SQL%BULK_EXCEPTIONS.count;
				for j in 1..errors loop
					k := SQL%BULK_EXCEPTIONS(j).ERROR_INDEX;
					UPDATE auction_locations Set
						location_name = p_location_name(k),
						owner_id = p_owner_id(k),
						sales_tax = p_sales_tax(k),
						sales_tax_bank_id = p_sales_tax_bank_id(k),
						empty_date = p_empty_date(k),
						last_access_date = p_last_access_date(k),
						inactive_date = p_inactive_date(k),
						status = p_status(k),
						search_enabled = p_search_enabled(k),
						entrance_charge = p_entrance_charge(k)
					WHERE
						location_id = p_location_id(k);
				end loop;
	end;

	procedure delete_auction_locations (p_location_id VAOFSTRING, p_chunk_size number)
	as
		errors number;
		dml_errors EXCEPTION;
		PRAGMA exception_init(dml_errors, -24381);
	begin
		FORALL i in 1..p_chunk_size save exceptions
			delete from auction_locations
			where location_id = p_location_id(i);
		exception
			when dml_errors then
				errors := SQL%BULK_EXCEPTIONS.count;
	end;

	procedure insert_market_auction_bids (p_item_id VAOFSTRING, 
					 p_bidder_id VAOFSTRING, 
					 p_bid VAOFNUMBER, 
					 p_max_proxy_bid VAOFNUMBER, 
					 p_chunk_size number)
	as
		errors number;
		k number;
		dml_errors EXCEPTION;
		PRAGMA exception_init(dml_errors, -24381);
	begin
		FORALL i in 1..p_chunk_size save exceptions
			INSERT into market_auction_bids (
				item_id,
				bidder_id,
				bid,
				max_proxy_bid )
			VALUES (
				p_item_id(i),
				p_bidder_id(i),
				p_bid(i),
				p_max_proxy_bid(i) );
		exception
			when dml_errors then
				errors := SQL%BULK_EXCEPTIONS.count;
				for j in 1..errors loop
					k := SQL%BULK_EXCEPTIONS(j).ERROR_INDEX;
					UPDATE market_auction_bids Set
						bidder_id = p_bidder_id(k),
						bid = p_bid(k),
						max_proxy_bid = p_max_proxy_bid(k)
					WHERE
						item_id = p_item_id(k);
				end loop;
	end;

	procedure update_market_auctions (p_item_id VAOFSTRING, 
					 p_owner_id VAOFSTRING, 
					 p_active VAOFNUMBER, 
					 p_chunk_size number)
	as
		errors number;
		dml_errors EXCEPTION;
		PRAGMA exception_init(dml_errors, -24381);
	begin
		FORALL i in 1..p_chunk_size save exceptions
			UPDATE market_auctions Set
				owner_id = DECODE(p_owner_id(i), NULL, owner_id, p_owner_id(i)),
				active = DECODE(p_active(i), NULL, active, p_active(i))
			WHERE
				item_id = p_item_id(i);
		exception
			when dml_errors then
				errors := SQL%BULK_EXCEPTIONS.count;
	end;

	procedure insert_market_auctions (p_item_id VAOFSTRING, 
					 p_owner_id VAOFSTRING, 
					 p_creator_id VAOFSTRING,
					 p_location_id VAOFSTRING, 
					 p_min_bid VAOFNUMBER, 
					 p_buy_now_price VAOFNUMBER,
					 p_auction_timer VAOFNUMBER, 
					 p_oob VAOFLONGSTRING, 
					 p_user_description VAOFLONGSTRING,
					 p_category VAOFNUMBER, 
					 p_item_name VAOFLONGSTRING, 
					 p_item_timer VAOFNUMBER,
					 p_active VAOFNUMBER,
					 p_item_size VAOFNUMBER, 
					 p_object_template_id VAOFNUMBER, 
					 p_chunk_size number)
	as
		errors number;
		k number;
		dml_errors EXCEPTION;
		PRAGMA exception_init(dml_errors, -24381);
	begin
		FORALL i in 1..p_chunk_size save exceptions
			INSERT into market_auctions (
				item_id,
				owner_id,
				creator_id,
				location_id,
				min_bid,
				buy_now_price,
				auction_timer,
				oob,
				user_description,
				category,
				item_name,
				item_timer,
				active,
				item_size,
				object_template_id )
			VALUES (
				p_item_id(i),
				p_owner_id(i),
				p_creator_id(i),
				p_location_id(i),
				p_min_bid(i),
				p_buy_now_price(i),
				p_auction_timer(i),
				p_oob(i),
				p_user_description(i),
				p_category(i),
				p_item_name(i),
				p_item_timer(i),
				p_active(i),
				p_item_size(i),
				p_object_template_id(i) );
	exception
		when dml_errors then
			errors := SQL%BULK_EXCEPTIONS.count;
			for j in 1..errors loop
				k := SQL%BULK_EXCEPTIONS(j).ERROR_INDEX;
				UPDATE market_auctions Set
					owner_id = p_owner_id(k),
					creator_id = p_creator_id(k),
					location_id = p_location_id(k),
					min_bid = p_min_bid(k),
					buy_now_price = p_buy_now_price(k),
					auction_timer = p_auction_timer(k),
					oob = p_oob(k),
					user_description = p_user_description(k),
					category = p_category(k),
					item_name = p_item_name(k),
					item_timer = p_item_timer(k),
					active = p_active(k),
					item_size = p_item_size(k),
					object_template_id = p_object_template_id(k)
				WHERE
					item_id = p_item_id(k);
			end loop;
	end;

	procedure delete_market_auctions (p_item_id VAOFSTRING, p_chunk_size number)
	as
		errors number;
		dml_errors EXCEPTION;
		PRAGMA exception_init(dml_errors, -24381);
	begin
		FORALL i in 1..p_chunk_size save exceptions
			delete from market_auction_attributes
			where item_id = p_item_id(i);

		FORALL i in 1..p_chunk_size save exceptions
			delete from market_auctions
			where item_id = p_item_id(i);
		--exception
			--when dml_errors then
				--errors := SQL%BULK_EXCEPTIONS.count;
	end;

	procedure insert_auction_attributes (p_item_id VAOFSTRING, p_attribute_name VAOFSTRING, p_attribute_value VAOFSTRING, p_chunk_size number)
	as
		dml_errors EXCEPTION;
		PRAGMA exception_init(dml_errors, -24381);
	begin
		FORALL i in 1..p_chunk_size save exceptions
			insert into market_auction_attributes (item_id, attribute_name, attribute_value)
			values (p_item_id(i), p_attribute_name(i), p_attribute_value(i));
		exception when dml_errors then
			null;
	end;
end;
/

