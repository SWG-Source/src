// ======================================================================
//
// TaskGetLocationList.cpp
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskGetLocationList.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "serverDatabase/dBLocationRecord.h"
#include "serverNetworkMessages/GetAuctionLocationsMessage.h"
#include "SwgDatabaseServer/CMLoader.h"

// ======================================================================

TaskGetLocationList::TaskGetLocationList () :
		TaskRequest(),
		count(0)
{

}

// ----------------------------------------------------------------------

bool TaskGetLocationList::process(DB::Session *session)
{
	time_t startTime = ::time(0);
	GetLocationsQuery qry;
	
	if (! (session->exec(&qry)))
		return false;

	int rowsFetched;
	int numRows;
	int currentCount;
	LocationRecord temp;

	while ((rowsFetched = qry.fetch()) > 0)
	{
		numRows = qry.getNumRowsFetched();
		currentCount = 0;

		CMLoader::getInstance().lockMutex();
		for (std::vector<DBSchema::AuctionLocationRow>::const_iterator i = qry.m_data.begin(); i != qry.m_data.end(); ++i)
		{
			if (++currentCount > numRows)
				break;

			count++;
			i->locationId.getValue(temp.m_locationId);
			i->ownerId.getValue(temp.m_ownerId);
			i->locationString.getValue(temp.m_locationString);
			i->salesTax.getValue(temp.m_salesTax);
			i->salesTaxBankId.getValue(temp.m_salesTaxBankId);
			i->emptyDate.getValue(temp.m_emptyDate);
			i->lastAccessDate.getValue(temp.m_lastAccessDate);
			i->inactiveDate.getValue(temp.m_inactiveDate);
			i->status.getValue(temp.m_status);
			i->searchEnabled.getValue(temp.m_searchEnabled);
			i->entranceCharge.getValue(temp.m_entranceCharge);
			CMLoader::getInstance().addAuctionLocationNoLockMutex(temp);
		}
		CMLoader::getInstance().unlockMutex();
	}

	qry.done();

	REPORT_LOG(true, ("[Commodities Server] : %d locations loaded to memory from database in (%d) seconds.\n", count, static_cast<int>(::time(0) - startTime)));
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskGetLocationList::onComplete()
{
	CMLoader::getInstance().onLoadComplete("AUCTION_LOCATIONS", count);
}

// ======================================================================

TaskGetLocationList::GetLocationsQuery::GetLocationsQuery() :
	Query(),
	m_data(ConfigServerDatabase::getAuctionLocationLoadBatchSize())
{
}

// ----------------------------------------------------------------------

void TaskGetLocationList::GetLocationsQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"cm_loader.get_location_list(); end;";
}

// ----------------------------------------------------------------------

bool TaskGetLocationList::GetLocationsQuery::bindParameters()
{
		return true;	
}

// ----------------------------------------------------------------------

bool TaskGetLocationList::GetLocationsQuery::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ConfigServerDatabase::getAuctionLocationLoadBatchSize());

	if (!bindCol(m_data[0].locationId)) return false;
	if (!bindCol(m_data[0].ownerId)) return false;
	if (!bindCol(m_data[0].locationString)) return false;
	if (!bindCol(m_data[0].salesTax)) return false;
	if (!bindCol(m_data[0].salesTaxBankId)) return false;
	if (!bindCol(m_data[0].emptyDate)) return false;
	if (!bindCol(m_data[0].lastAccessDate)) return false;
	if (!bindCol(m_data[0].inactiveDate)) return false;
	if (!bindCol(m_data[0].status)) return false;
	if (!bindCol(m_data[0].searchEnabled)) return false;
	if (!bindCol(m_data[0].entranceCharge)) return false;

	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetLocationList::GetLocationsQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
