// ======================================================================
//
// TaskGetBidList.cpp
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskGetBidList.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "serverDatabase/dBBidRecord.h"
#include "serverNetworkMessages/GetMarketAuctionBidsMessage.h"
#include "SwgDatabaseServer/CMLoader.h"

// ======================================================================

TaskGetBidList::TaskGetBidList () :
		TaskRequest(),
		count(0)
{

}

// ----------------------------------------------------------------------

bool TaskGetBidList::process(DB::Session *session)
{
	time_t startTime = ::time(0);
	GetBidsQuery qry;

	if (! (session->exec(&qry)))
		return false;

	int rowsFetched;
	int numRows;
	int currentCount;
	BidRecord temp;
	
	while ((rowsFetched = qry.fetch()) > 0)
	{
		numRows = qry.getNumRowsFetched();
		currentCount = 0;

		CMLoader::getInstance().lockMutex();
		for (std::vector<DBSchema::AuctionBidRow>::const_iterator i = qry.m_data.begin(); i != qry.m_data.end(); ++i)
		{
			if (++currentCount > numRows)
				break;

			count++;
			i->itemId.getValue(temp.m_itemId);
			i->bidderId.getValue(temp.m_bidderId);
			i->bid.getValue(temp.m_bid);
			i->maxProxyBid.getValue(temp.m_maxProxyBid);
			CMLoader::getInstance().addMarketAuctionBidNoLockMutex(temp);
	    }
		CMLoader::getInstance().unlockMutex();
	}

	qry.done();

	REPORT_LOG(true, ("[Commodities Server] : %d bids loaded to memory from database in (%d) seconds.\n", count, static_cast<int>(::time(0) - startTime)));
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskGetBidList::onComplete()
{
	CMLoader::getInstance().onLoadComplete("MARKET_AUCTION_BIDS", count);
}

// ======================================================================

TaskGetBidList::GetBidsQuery::GetBidsQuery() :
	Query(),
	m_data(ConfigServerDatabase::getAuctionBidLoadBatchSize())
{
}

// ----------------------------------------------------------------------

void TaskGetBidList::GetBidsQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"cm_loader.get_bid_list(); end;";
}

// ----------------------------------------------------------------------

bool TaskGetBidList::GetBidsQuery::bindParameters()
{
		return true;	
}

// ----------------------------------------------------------------------

bool TaskGetBidList::GetBidsQuery::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ConfigServerDatabase::getAuctionBidLoadBatchSize());

	if (!bindCol(m_data[0].itemId)) return false;
	if (!bindCol(m_data[0].bidderId)) return false;
	if (!bindCol(m_data[0].bid)) return false;
	if (!bindCol(m_data[0].maxProxyBid)) return false;

	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetBidList::GetBidsQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
