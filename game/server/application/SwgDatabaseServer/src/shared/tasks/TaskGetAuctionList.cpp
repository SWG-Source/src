// ======================================================================
//
// TaskGetAuctionList.cpp
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// This task has a special decoding function for loading out of band
// data(OOB) from the database. This is the only place OOB is loaded.
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskGetAuctionList.h"
#include "utf8.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "serverDatabase/dBAuctionRecord.h"
#include "SwgDatabaseServer/CMLoader.h"

// ======================================================================

TaskGetAuctionList::TaskGetAuctionList () :
		TaskRequest(),
		count(0)
{

}

// ----------------------------------------------------------------------

bool TaskGetAuctionList::process(DB::Session *session)
{
	time_t startTime = ::time(0);
	GetAuctionsQuery qry;

	if (! (session->exec(&qry)))
		return false;

	int rowsFetched;
	int numRows;
	int currentCount;
	AuctionRecord temp;

	while ((rowsFetched = qry.fetch()) > 0)
	{
		numRows = qry.getNumRowsFetched();
		currentCount = 0;

		CMLoader::getInstance().lockMutex();
		for (std::vector<DBSchema::AuctionRow>::const_iterator i = qry.m_data.begin(); i != qry.m_data.end(); ++i)
		{
			if (++currentCount > numRows)
				break;

			count++;
			i->creatorId.getValue(temp.m_creatorId);
			i->minBid.getValue(temp.m_minBid);
			i->auctionTimer.getValue(temp.m_auctionTimer);
			i->buyNowPrice.getValue(temp.m_buyNowPrice);
			i->userDescription.getValue(temp.m_userDescription);
			i->oob.getValue(temp.m_oob);		
			i->locationId.getValue(temp.m_locationId);
			i->itemId.getValue(temp.m_itemId);
			i->category.getValue(temp.m_category);
			i->itemTimer.getValue(temp.m_itemTimer);
			i->itemName.getValue(temp.m_itemName);
			i->ownerId.getValue(temp.m_ownerId);
			i->active.getValue(temp.m_active);
			i->itemSize.getValue(temp.m_itemSize);
			i->itemTemplateId.getValue(temp.m_itemTemplateId);
			CMLoader::getInstance().addMarketAuctionsNoLockMutex(temp);
		}
		CMLoader::getInstance().unlockMutex();
	}

	qry.done();

	REPORT_LOG(true, ("[Commodities Server] : %d auctions loaded to memory from database in (%d) seconds.\n", count, static_cast<int>(::time(0) - startTime)));
	if (rowsFetched < 0)
		return false;

	return true;
}

// ----------------------------------------------------------------------

void TaskGetAuctionList::onComplete()
{
	CMLoader::getInstance().onLoadComplete("MARKET_AUCTIONS", count);
}

// ======================================================================

TaskGetAuctionList::GetAuctionsQuery::GetAuctionsQuery() :
	Query(),
	m_data(ConfigServerDatabase::getAuctionLoadBatchSize())
{
}

// ----------------------------------------------------------------------

void TaskGetAuctionList::GetAuctionsQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"cm_loader.get_auction_list(); end;";
}

// ----------------------------------------------------------------------

bool TaskGetAuctionList::GetAuctionsQuery::bindParameters()
{
	return true;	
}

// ----------------------------------------------------------------------

bool TaskGetAuctionList::GetAuctionsQuery::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ConfigServerDatabase::getAuctionLoadBatchSize());

	if (!bindCol(m_data[0].creatorId)) return false;
	if (!bindCol(m_data[0].minBid)) return false;
	if (!bindCol(m_data[0].auctionTimer)) return false;
	if (!bindCol(m_data[0].buyNowPrice)) return false;
	if (!bindCol(m_data[0].userDescription)) return false;
	if (!bindCol(m_data[0].oob)) return false;
	if (!bindCol(m_data[0].locationId)) return false;
	if (!bindCol(m_data[0].itemId)) return false;
	if (!bindCol(m_data[0].category)) return false;
	if (!bindCol(m_data[0].itemTimer)) return false;
	if (!bindCol(m_data[0].itemName)) return false;
	if (!bindCol(m_data[0].ownerId)) return false;
	if (!bindCol(m_data[0].active)) return false;
	if (!bindCol(m_data[0].itemSize)) return false;
	if (!bindCol(m_data[0].itemTemplateId)) return false;

	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetAuctionList::GetAuctionsQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
