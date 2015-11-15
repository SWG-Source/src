// copyright (c) 2001 Sony Online Entertainment
//
// Edit the file Schema_h.template.  Do not edit Schema.h.
//
// ======================================================================

#ifndef INCLUDED_CommoditiesSchema_H
#define INCLUDED_CommoditiesSchema_H

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbRow.h"

namespace DBSchema
{

struct AuctionLocationsRow : public DB::Row
{
	AuctionLocationsRow() { ++m_sAuctionRowsCreated; }
	~AuctionLocationsRow() { ++m_sAuctionRowsDeleted; }


	DB::BindableNetworkId location_id;
	DB::BindableString<256> location_name;
	DB::BindableNetworkId owner_id;
	DB::BindableLong sales_tax;
	DB::BindableNetworkId sales_tax_bank_id;
	DB::BindableLong empty_date;
	DB::BindableLong last_access_date;
	DB::BindableLong inactive_date;
	DB::BindableLong status;
	DB::BindableBool search_enabled;
	DB::BindableLong entrance_charge;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const AuctionLocationsRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		location_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return location_id.getValue();
	}

	static int getRowsCreated() { return m_sAuctionRowsCreated; }
	static int getRowsActive() 
	{ 
		int i_retval =  m_sAuctionRowsCreated - m_sAuctionRowsDeleted;
		//if ( i_retval < 0 )
		//	return 0;
		return i_retval;
	}

private:
	static volatile int m_sAuctionRowsCreated;
	static volatile int m_sAuctionRowsDeleted;

};

struct MarketAuctionsRow : public DB::Row
{
	MarketAuctionsRow() { ++m_sAuctionRowsCreated; }
	~MarketAuctionsRow() { ++m_sAuctionRowsDeleted; }

	DB::BindableNetworkId    item_id;
	DB::BindableNetworkId    owner_id;
	DB::BindableNetworkId    creator_id;
	DB::BindableNetworkId    location_id;
	DB::BindableLong         min_bid;
	DB::BindableLong         buy_now_price;
	DB::BindableLong         auction_timer;
	DB::BindableString<4000> oob;
	DB::BindableUnicode<1024>user_description;
	DB::BindableLong         category;
	DB::BindableUnicode<1024>item_name;
	DB::BindableLong         item_timer;
	DB::BindableLong         active;
	DB::BindableLong         item_size;
	DB::BindableLong         object_template_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const MarketAuctionsRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		item_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return item_id.getValue();
	}

	static int getRowsCreated() { return m_sAuctionRowsCreated; }
	static int getRowsActive() 
	{ 
		int i_retval =  m_sAuctionRowsCreated - m_sAuctionRowsDeleted;
		//if ( i_retval < 0 )
		//	return 0;
		return i_retval;
	}

private:
	static volatile int m_sAuctionRowsCreated;
	static volatile int m_sAuctionRowsDeleted;
};

struct MarketAuctionsRowDelete : public DB::Row
{
	MarketAuctionsRowDelete() { ++m_sAuctionRowsCreated; }
	~MarketAuctionsRowDelete() { ++m_sAuctionRowsDeleted; }

	DB::BindableNetworkId    item_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const MarketAuctionsRowDelete&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		item_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return item_id.getValue();
	}

	static int getRowsCreated() { return m_sAuctionRowsCreated; }
	static int getRowsActive() 
	{ 
		int i_retval =  m_sAuctionRowsCreated - m_sAuctionRowsDeleted;
		if ( i_retval < 0 )
			return 0;
		return i_retval;
	}

private:
	static volatile int m_sAuctionRowsCreated;
	static volatile int m_sAuctionRowsDeleted;
};

struct MarketAuctionsRowUpdate : public DB::Row
{
	MarketAuctionsRowUpdate() { ++m_sAuctionRowsCreated; }
	~MarketAuctionsRowUpdate() { ++m_sAuctionRowsDeleted; }

	DB::BindableNetworkId    item_id;
	DB::BindableNetworkId    owner_id;
	DB::BindableLong         active;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const MarketAuctionsRowUpdate&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		item_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return item_id.getValue();
	}

	static int getRowsCreated() { return m_sAuctionRowsCreated; }
	static int getRowsActive() 
	{ 
		int i_retval =  m_sAuctionRowsCreated - m_sAuctionRowsDeleted;
		if ( i_retval < 0 )
			return 0;
		return i_retval;
	}

private:
	static volatile int m_sAuctionRowsCreated;
	static volatile int m_sAuctionRowsDeleted;
};

struct MarketAuctionBidsRow : public DB::Row
{
	MarketAuctionBidsRow() { ++m_sAuctionRowsCreated; }
	~MarketAuctionBidsRow() { ++m_sAuctionRowsDeleted; }

	DB::BindableNetworkId item_id;
	DB::BindableNetworkId bidder_id;
	DB::BindableLong      bid;
	DB::BindableLong      max_proxy_bid;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const MarketAuctionBidsRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		item_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return item_id.getValue();
	}

	static int getRowsCreated() { return m_sAuctionRowsCreated; }
	static int getRowsActive() 
	{ 
		int i_retval =  m_sAuctionRowsCreated - m_sAuctionRowsDeleted;
		if ( i_retval < 0 )
			return 0;
		return i_retval;
	}

private:
	static volatile int m_sAuctionRowsCreated;
	static volatile int m_sAuctionRowsDeleted;
};

};

#endif
