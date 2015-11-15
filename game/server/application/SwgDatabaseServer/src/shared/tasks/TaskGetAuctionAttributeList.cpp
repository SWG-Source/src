// ======================================================================
//
// TaskGetAuctionAttributeList.cpp
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskGetAuctionAttributeList.h"
#include "utf8.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "SwgDatabaseServer/CMLoader.h"

// ======================================================================

TaskGetAuctionAttributeList::TaskGetAuctionAttributeList () :
		TaskRequest(),
		count(0)
{

}

// ----------------------------------------------------------------------

bool TaskGetAuctionAttributeList::process(DB::Session *session)
{
	time_t startTime = ::time(0);
	GetAuctionAttributesQuery qry;
	
	if (! (session->exec(&qry)))
		return false;

	int rowsFetched;
	int numRows;
	int currentCount;
	NetworkId item;
	std::string attributeName;
	Unicode::String attributeValue;

	while ((rowsFetched = qry.fetch()) > 0)
	{
		numRows = qry.getNumRowsFetched();
		currentCount = 0;

		CMLoader::getInstance().lockMutex();
		for (std::vector<DBSchema::AuctionAttributeRow>::const_iterator i = qry.m_data.begin(); i != qry.m_data.end(); ++i)
		{		
			if (++currentCount > numRows)
				break;

			count++;
			i->item_id.getValue(item);
			i->attribute_name.getValue(attributeName);
			i->attribute_value.getValue(attributeValue);
			CMLoader::getInstance().addMarketAuctionAttributesNoLockMutex(item,attributeName,attributeValue);
		}
		CMLoader::getInstance().unlockMutex();
	}

	qry.done();

	REPORT_LOG(true, ("[Commodities Server] : %d auction attributes loaded to memory from database in (%d) seconds.\n", count, static_cast<int>(::time(0) - startTime)));
	if (rowsFetched < 0)
		return false;

	return true;
}

// ----------------------------------------------------------------------

void TaskGetAuctionAttributeList::onComplete()
{
	CMLoader::getInstance().onLoadComplete("MARKET_AUCTION_ATTRIBUTES", count);
}

// ======================================================================

TaskGetAuctionAttributeList::GetAuctionAttributesQuery::GetAuctionAttributesQuery() :
	Query(),
	m_data(ConfigServerDatabase::getAuctionAttributeLoadBatchSize())
{
}

// ----------------------------------------------------------------------

void TaskGetAuctionAttributeList::GetAuctionAttributesQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"cm_loader.get_auction_attributes(); end;";
}

// ----------------------------------------------------------------------

bool TaskGetAuctionAttributeList::GetAuctionAttributesQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool TaskGetAuctionAttributeList::GetAuctionAttributesQuery::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ConfigServerDatabase::getAuctionAttributeLoadBatchSize());

	if (!bindCol(m_data[0].item_id)) return false;
	if (!bindCol(m_data[0].attribute_name)) return false;
	if (!bindCol(m_data[0].attribute_value)) return false;

	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetAuctionAttributeList::GetAuctionAttributesQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
