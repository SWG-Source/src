// ======================================================================
//
// MarketAuctionBidsBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/MarketAuctionBidsBuffer.h"

#include "SwgDatabaseServer/CommoditiesQuery.h"
#include "SwgDatabaseServer/CommoditiesSchema.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include <vector>

// ======================================================================

MarketAuctionBidsBuffer::MarketAuctionBidsBuffer(DB::ModeQuery::Mode mode) :
		AbstractTableBuffer(),
		m_mode(mode),
		m_rows()
{
}

// ----------------------------------------------------------------------

MarketAuctionBidsBuffer::~MarketAuctionBidsBuffer(void)
{
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------
	
bool MarketAuctionBidsBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase)
{
	return true;
}
	
// ----------------------------------------------------------------------
	
void MarketAuctionBidsBuffer::removeObject(const NetworkId &object)
{
	removeMarketAuctionBids(object);
}

// ----------------------------------------------------------------------

void MarketAuctionBidsBuffer::removeMarketAuctionBids(const NetworkId &itemId)
{
	IndexType::iterator i=m_rows.find(itemId);
	if (i!=m_rows.end())
	{
		delete i->second;
		i->second=nullptr;
		m_rows.erase(i);
	}
}

// ----------------------------------------------------------------------

void MarketAuctionBidsBuffer::setMarketAuctionBids(const NetworkId &itemId, const NetworkId &bidderId, const int bid, const int maxProxyBid)
{
	DBSchema::MarketAuctionBidsRow *row=findRowByIndex(itemId);
	if (!row)
	{
		row=new DBSchema::MarketAuctionBidsRow;
		addRowToIndex(itemId, row);
	}
	row->item_id = itemId;
	row->bidder_id = bidderId;
	row->bid = bid;
	row->max_proxy_bid = maxProxyBid;
}

// ----------------------------------------------------------------------

DBSchema::MarketAuctionBidsRow * MarketAuctionBidsBuffer::findRowByIndex(const NetworkId &itemId)
{
	IndexType::iterator i=m_rows.find(itemId);
	if (i==m_rows.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void MarketAuctionBidsBuffer::addRowToIndex (const NetworkId &itemId, DBSchema::MarketAuctionBidsRow *row)
{
	m_rows[itemId]=row;
}

// ----------------------------------------------------------------------

bool MarketAuctionBidsBuffer::save(DB::Session *session)
{
	LOG("SaveCounts",("MarketAuctionBids:  %i saved to db",m_rows.size()));
	DEBUG_REPORT_LOG(true, ("Saving Market Auction Bids Buffer. mode:%d rowcount:%d\n", m_mode, m_rows.size()));
	
	DBQuery::MarketAuctionBidsQuery qry;
	
	if (m_mode==DB::ModeQuery::mode_INSERT)
		qry.insertMode();
	else if (m_mode == DB::ModeQuery::mode_UPDATE)
		qry.updateMode();
	else
		qry.deleteMode();
	if (!qry.setupData(session))
		return false;
	
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		if (!qry.addData(i->second))
			return false;
		if (qry.getNumItems() >= MarketAuctionBidsTableBufferNamespace::ms_maxItemsPerExec)
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
