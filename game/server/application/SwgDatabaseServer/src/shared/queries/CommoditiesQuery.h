// CommoditiesQuery.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommoditiesQuery_H
#define INCLUDED_CommoditiesQuery_H

// ======================================================================

#include "serverDatabase/DatabaseProcessQuery.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedDatabaseInterface/DbModeQuery.h"
#include "sharedFoundation/NetworkId.h"
#include "SwgDatabaseServer/CommoditiesSchema.h"

#include <vector>

// ======================================================================

namespace DBQuery
{

class AuctionLocationsQuery : public DatabaseProcessQuery
{
	AuctionLocationsQuery(const AuctionLocationsQuery&);
	AuctionLocationsQuery& operator= (const AuctionLocationsQuery&);
public:
	AuctionLocationsQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString     m_location_ids;
	DB::BindableVarrayString     m_location_names;
	DB::BindableVarrayString     m_owner_ids;
	DB::BindableVarrayNumber     m_sales_taxes;
	DB::BindableVarrayString     m_sales_tax_bank_ids;
	DB::BindableVarrayNumber     m_empty_date;
	DB::BindableVarrayNumber     m_last_access_date;
	DB::BindableVarrayNumber     m_inactive_date;
	DB::BindableVarrayNumber     m_status;
	DB::BindableVarrayString     m_search_enabled;
	DB::BindableVarrayNumber     m_entrance_charge;
	DB::BindableLong             m_numItems;
};

class AuctionLocationsQuerySelect : public DB::Query
{
public:
	AuctionLocationsQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::AuctionLocationsRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::AuctionLocationsRow> m_data;
	const std::string m_schema;

	AuctionLocationsQuerySelect            (const AuctionLocationsQuerySelect&);
	AuctionLocationsQuerySelect& operator= (const AuctionLocationsQuerySelect&);
};


// ======================================================================

class MarketAuctionsQuery : public DatabaseProcessQuery
{
	MarketAuctionsQuery(const MarketAuctionsQuery&);
	MarketAuctionsQuery& operator= (const MarketAuctionsQuery&);
public:
	MarketAuctionsQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString    m_item_ids;
	DB::BindableVarrayString    m_owner_ids;
	DB::BindableVarrayString    m_creator_ids;
	DB::BindableVarrayString    m_location_ids;
	DB::BindableVarrayNumber    m_min_bids;
	DB::BindableVarrayNumber    m_buy_now_prices;
	DB::BindableVarrayNumber    m_auction_timers;
	DB::BindableVarrayString    m_oobs;
	DB::BindableVarrayString    m_user_descriptions;
	DB::BindableVarrayNumber    m_categories;
	DB::BindableVarrayString    m_item_names;
	DB::BindableVarrayNumber    m_item_timers;
	DB::BindableVarrayNumber    m_actives;
	DB::BindableVarrayNumber    m_item_sizes;
	DB::BindableVarrayNumber    m_object_template_ids;
	DB::BindableLong            m_numItems;
};

class MarketAuctionsQuerySelect : public DB::Query
{
public:
	MarketAuctionsQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::MarketAuctionsRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::MarketAuctionsRow> m_data;
	const std::string m_schema;

	MarketAuctionsQuerySelect            (const MarketAuctionsQuerySelect&);
	MarketAuctionsQuerySelect& operator= (const MarketAuctionsQuerySelect&);
};

// ======================================================================

class MarketAuctionBidsQuery : public DatabaseProcessQuery
{
	MarketAuctionBidsQuery(const MarketAuctionBidsQuery&);
	MarketAuctionBidsQuery& operator= (const MarketAuctionBidsQuery&);
public:
	MarketAuctionBidsQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString    m_item_ids;
	DB::BindableVarrayString    m_bidder_ids;
	DB::BindableVarrayNumber    m_bids;
	DB::BindableVarrayNumber    m_max_proxy_bids;
	DB::BindableLong            m_numItems;
};

class MarketAuctionBidsQuerySelect : public DB::Query
{
public:
	MarketAuctionBidsQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::MarketAuctionBidsRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::MarketAuctionBidsRow> m_data;
	const std::string m_schema;

	MarketAuctionBidsQuerySelect            (const MarketAuctionBidsQuerySelect&);
	MarketAuctionBidsQuerySelect& operator= (const MarketAuctionBidsQuerySelect&);
};

// ======================================================================

class MarketAuctionsAttributesQuery : public DatabaseProcessQuery
{
	MarketAuctionsAttributesQuery(const MarketAuctionsAttributesQuery&);
	MarketAuctionsAttributesQuery& operator= (const MarketAuctionsAttributesQuery&);
public:
	MarketAuctionsAttributesQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(NetworkId const & itemId, std::string const & attributeName, Unicode::String const & attributeValue);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString    m_item_ids;
	DB::BindableVarrayString    m_attribute_names;
	DB::BindableVarrayString    m_attribute_values;
	DB::BindableLong            m_numItems;
};

// ======================================================================

} // namespace

#endif
