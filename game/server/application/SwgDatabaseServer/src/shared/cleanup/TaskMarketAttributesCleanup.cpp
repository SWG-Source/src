// ======================================================================
//
// TaskMarketAttributesCleanup.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskMarketAttributesCleanup.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

bool TaskMarketAttributesCleanup::process(DB::Session *session)
{
	time_t t_start = time(0);
	while( (time(0) - t_start) < m_tSeconds )  // limit the run time
	{
		DataCleanupQuery qry;
		if ( session->exec(&qry) == false )
		{
			LOG("TaskMarketAttributesCleanup::process",( "returned false!" ));
			return false;
		}
		LOG("TaskMarketAttributesCleanup::process",("ROWS = %d", qry.result.getValue() ));
		if( qry.result.getValue() == 0 )   // cleanup done
			break;
	}
	LOG("TaskMarketAttributesCleanup",("TIME = %d", time(0)-t_start ));
	return true;
}

// ----------------------------------------------------------------------

void TaskMarketAttributesCleanup::onComplete()
{
}

// ======================================================================

TaskMarketAttributesCleanup::DataCleanupQuery::DataCleanupQuery()
{
}

// ----------------------------------------------------------------------
		
void TaskMarketAttributesCleanup::DataCleanupQuery::getSQL(std::string &sql)
{
	sql="begin :result := "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.market_attributes_cleanup(); end;";
}

// ----------------------------------------------------------------------

bool TaskMarketAttributesCleanup::DataCleanupQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskMarketAttributesCleanup::DataCleanupQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskMarketAttributesCleanup::DataCleanupQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}


// ======================================================================
