// ======================================================================
//
// MarketAuctionsBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/MarketAuctionsBuffer.h"

#include "SwgDatabaseServer/CommoditiesQuery.h"
#include "SwgDatabaseServer/CommoditiesSchema.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include <vector>

// ======================================================================

MarketAuctionsBufferCreate::MarketAuctionsBufferCreate() :
		AbstractTableBuffer(),
		m_rows(),
		m_attributes()
{
}

// ----------------------------------------------------------------------

MarketAuctionsBufferCreate::~MarketAuctionsBufferCreate(void)
{
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------
	
bool MarketAuctionsBufferCreate::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase)
{
	return true;
}
	
// ----------------------------------------------------------------------
	
void MarketAuctionsBufferCreate::removeObject(const NetworkId &object)
{
	removeMarketAuctions(object);
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferCreate::removeMarketAuctions(const NetworkId &itemId)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferCreate Removing Market Auctions Buffer Row. rowcount:%d\n", m_rows.size()));
	DBSchema::MarketAuctionsRow *row=findRowByIndex(itemId);
	if (row)
	{
		delete row;
		IGNORE_RETURN(m_rows.erase(itemId));
		IGNORE_RETURN(m_attributes.erase(itemId));
		DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferCreate Removing Market Auctions Buffer Row. rowcount:%d\n", m_rows.size()));
	}
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferCreate::setMarketAuctions(const NetworkId &itemId, const NetworkId &ownerId, const int active)
{
	DBSchema::MarketAuctionsRow *row=findRowByIndex(itemId);
	if (!row)
	{
		row=new DBSchema::MarketAuctionsRow;
		addRowToIndex(itemId, row);
	}
	row->item_id = itemId;
	row->owner_id = ownerId;
	row->active = active;
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferCreate::setMarketAuctions(const NetworkId &itemId, const NetworkId &ownerId, const NetworkId &creatorId, const NetworkId &locationId, const int minBid, const int buyNowPrice, const int auctionTimer, std::vector<std::pair<std::string, Unicode::String> > const & attributes, const Unicode::String &userDescription, const int category, const int itemTemplateId, const Unicode::String &itemName, const int itemTimer, const int active, const int itemSize)
{
	DBSchema::MarketAuctionsRow *row=findRowByIndex(itemId);
	if (!row)
	{
		row=new DBSchema::MarketAuctionsRow;
		addRowToIndex(itemId, row);
	}
	row->item_id = itemId;
	row->owner_id = ownerId;
	row->creator_id = creatorId;
	row->location_id = locationId;
	row->min_bid = minBid;
	row->buy_now_price = buyNowPrice;
	row->auction_timer = auctionTimer;
//	row->oob = oob;
	row->user_description = userDescription;
	row->category = category;
	row->item_name = itemName;
	row->item_timer = itemTimer;
	row->active = active;
	row->item_size = itemSize;
	row->object_template_id = itemTemplateId;
	LOG("CommoditiesServer", ("MarketAuctionsBufferCreate Added MarketAuctionsBuffer row for item %s", row->item_id.getValueASCII().c_str()));

	m_attributes[itemId] = attributes;
}

// ----------------------------------------------------------------------

DBSchema::MarketAuctionsRow * MarketAuctionsBufferCreate::findRowByIndex(const NetworkId &itemId)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferCreate Finding row by Index. ItemId : %s.\n", itemId.getValueString().c_str()));
	IndexType::iterator i=m_rows.find(itemId);
	if (i==m_rows.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferCreate::addRowToIndex (const NetworkId &itemId, DBSchema::MarketAuctionsRow *row)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferCreate Adding row to Index. ItemId : %s.\n", itemId.getValueString().c_str()));
	m_rows[itemId]=row;
}

// ----------------------------------------------------------------------

bool MarketAuctionsBufferCreate::save(DB::Session *session)
{
	{
		// count how many attributes we're saving
		size_t numAttributes = 0;

		for (AttributesType::const_iterator i=m_attributes.begin(); i!=m_attributes.end(); ++i)
			numAttributes += i->second.size();

		LOG("SaveCounts",("MarketAuctionsBufferCreate: %i auctions, %i auction attributes saved to db",m_rows.size(), numAttributes));
		DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferCreate Saving Market Auctions Buffer. rowcount:%d auctions, %d auction attributes\n", m_rows.size(), numAttributes));
	}

	{
		DBQuery::MarketAuctionsQuery qry;
		qry.insertMode();

		if (!qry.setupData(session))
			return false;
		for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
		{
			if (!qry.addData(i->second))
				return false;
			LOG("CommoditiesServer", ("MarketAuctionsBufferCreate Added row to MarketAuctionsQuery VARRAY for item %s", (i->second)->item_id.getValueASCII().c_str()));
			if (qry.getNumItems() >= MarketAuctionsTableBufferNamespace::ms_maxItemsPerExec)
			{
				if (! (session->exec(&qry)))
					return false;
				qry.clearData();
			}
		}
		if (qry.getNumItems() != 0)
			if (! (session->exec(&qry)))
				return false;
	
		qry.done();
		qry.freeData();
	}


	{
		DBQuery::MarketAuctionsAttributesQuery qry;
		qry.insertMode();

		if (!qry.setupData(session))
			return false;
		for (AttributesType::const_iterator i=m_attributes.begin(); i!=m_attributes.end(); ++i)
		{
			for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator j=i->second.begin(); j!=i->second.end(); ++j)
			{
				if (!qry.addData(i->first, j->first, j->second))
					return false;
				if (qry.getNumItems() >= MarketAuctionsTableBufferNamespace::ms_maxItemsPerExec)
				{
					if (! (session->exec(&qry)))
						return false;
					qry.clearData();
				}
			}
		}
		if (qry.getNumItems() != 0)
			if (! (session->exec(&qry)))
				return false;
		
		qry.done();
		qry.freeData();
	}
	
	return true;
}

// ======================================================================

MarketAuctionsBufferDelete::MarketAuctionsBufferDelete() :
		AbstractTableBuffer(),
		m_rows()
{
}

// ----------------------------------------------------------------------

MarketAuctionsBufferDelete::~MarketAuctionsBufferDelete(void)
{
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------
	
bool MarketAuctionsBufferDelete::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase)
{
	return true;
}
	
// ----------------------------------------------------------------------
	
void MarketAuctionsBufferDelete::removeObject(const NetworkId &object)
{
	removeMarketAuctions(object);
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferDelete::removeMarketAuctions(const NetworkId &itemId)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferDelete Removing Market Auctions Buffer Row. rowcount:%d\n", m_rows.size()));
	DBSchema::MarketAuctionsRowDelete *row=findRowByIndex(itemId);
	if (row)
	{
		delete row;
		IGNORE_RETURN(m_rows.erase(itemId));
		DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferDelete Removing Market Auctions Buffer Row. rowcount:%d\n", m_rows.size()));
	}
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferDelete::setMarketAuctions(const NetworkId &itemId)
{
	DBSchema::MarketAuctionsRowDelete *row=findRowByIndex(itemId);
	if (!row)
	{
		row=new DBSchema::MarketAuctionsRowDelete;
		addRowToIndex(itemId, row);
	}
	row->item_id = itemId;
}

// ----------------------------------------------------------------------

DBSchema::MarketAuctionsRowDelete * MarketAuctionsBufferDelete::findRowByIndex(const NetworkId &itemId)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferDelete Finding row by Index. ItemId : %s.\n", itemId.getValueString().c_str()));
	IndexType::iterator i=m_rows.find(itemId);
	if (i==m_rows.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferDelete::addRowToIndex (const NetworkId &itemId, DBSchema::MarketAuctionsRowDelete *row)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferDelete Adding row to Index. ItemId : %s.\n", itemId.getValueString().c_str()));
	m_rows[itemId]=row;
}

// ----------------------------------------------------------------------

bool MarketAuctionsBufferDelete::save(DB::Session *session)
{
	LOG("SaveCounts",("MarketAuctionsBufferDelete: %i saved to db",m_rows.size()));
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferDelete Saving Market Auctions Buffer. rowcount:%d\n", m_rows.size()));

	DBQuery::MarketAuctionsQuery qry;
	qry.deleteMode();

	if (!qry.setupData(session))
		return false;
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		if (!qry.addData(i->second))
			return false;
		LOG("CommoditiesServer", ("MarketAuctionsBufferDelete Added row to MarketAuctionsQuery VARRAY for item %s", (i->second)->item_id.getValueASCII().c_str()));
		if (qry.getNumItems() >= MarketAuctionsTableBufferNamespace::ms_maxItemsPerExec)
		{
			if (! (session->exec(&qry)))
				return false;
			qry.clearData();
		}
	}
	if (qry.getNumItems() != 0)
		if (! (session->exec(&qry)))
			return false;

	qry.done();
	qry.freeData();
	return true;
}

// ======================================================================

MarketAuctionsBufferUpdate::MarketAuctionsBufferUpdate() :
		AbstractTableBuffer(),
		m_rows()
{
}

// ----------------------------------------------------------------------

MarketAuctionsBufferUpdate::~MarketAuctionsBufferUpdate(void)
{
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------
	
bool MarketAuctionsBufferUpdate::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase)
{
	return true;
}
	
// ----------------------------------------------------------------------
	
void MarketAuctionsBufferUpdate::removeObject(const NetworkId &object)
{
	removeMarketAuctions(object);
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferUpdate::removeMarketAuctions(const NetworkId &itemId)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferUpdate Removing Market Auctions Buffer Row. rowcount:%d\n", m_rows.size()));
	DBSchema::MarketAuctionsRowUpdate *row=findRowByIndex(itemId);
	if (row)
	{
		delete row;
		IGNORE_RETURN(m_rows.erase(itemId));
		DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferUpdate Removing Market Auctions Buffer Row. rowcount:%d\n", m_rows.size()));
	}
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferUpdate::setMarketAuctions(const NetworkId &itemId, const NetworkId &ownerId, const int active)
{
	DBSchema::MarketAuctionsRowUpdate *row=findRowByIndex(itemId);
	if (!row)
	{
		row=new DBSchema::MarketAuctionsRowUpdate;
		addRowToIndex(itemId, row);
	}
	row->item_id = itemId;
	row->owner_id = ownerId;
	row->active = active;
}

// ----------------------------------------------------------------------

DBSchema::MarketAuctionsRowUpdate * MarketAuctionsBufferUpdate::findRowByIndex(const NetworkId &itemId)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferUpdate Finding row by Index. ItemId : %s.\n", itemId.getValueString().c_str()));
	IndexType::iterator i=m_rows.find(itemId);
	if (i==m_rows.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void MarketAuctionsBufferUpdate::addRowToIndex (const NetworkId &itemId, DBSchema::MarketAuctionsRowUpdate *row)
{
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferUpdate Adding row to Index. ItemId : %s.\n", itemId.getValueString().c_str()));
	m_rows[itemId]=row;
}

// ----------------------------------------------------------------------

bool MarketAuctionsBufferUpdate::save(DB::Session *session)
{
	LOG("SaveCounts",("MarketAuctionsBufferUpdate: %i saved to db",m_rows.size()));
	DEBUG_REPORT_LOG(true, ("MarketAuctionsBufferUpdate Saving Market Auctions Buffer. rowcount:%d\n", m_rows.size()));

	DBQuery::MarketAuctionsQuery qry;
	qry.updateMode();

	if (!qry.setupData(session))
		return false;
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		if (!qry.addData(i->second))
			return false;
		LOG("CommoditiesServer", ("MarketAuctionsBufferUpdate Added row to MarketAuctionsQuery VARRAY for item %s", (i->second)->item_id.getValueASCII().c_str()));
		if (qry.getNumItems() >= MarketAuctionsTableBufferNamespace::ms_maxItemsPerExec)
		{
			if (! (session->exec(&qry)))
				return false;
			qry.clearData();
		}
	}
	if (qry.getNumItems() != 0)
		if (! (session->exec(&qry)))
			return false;

	qry.done();
	qry.freeData();
	return true;
}

// ======================================================================
