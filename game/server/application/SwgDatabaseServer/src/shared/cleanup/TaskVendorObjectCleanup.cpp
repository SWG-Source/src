// ======================================================================
//
// TaskVendorObjectCleanup.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskVendorObjectCleanup.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

bool TaskVendorObjectCleanup::process(DB::Session *session)
{
	time_t t_start = time(0);
	while( (time(0) - t_start) < m_tSeconds )  // limit the run time
	{
		DataCleanupQuery qry;
		if ( session->exec(&qry) == false )
		{
			LOG("TaskVendorObjectCleanup::process",( "returned false!" ));
			return false;
		}
		LOG("TaskVendorObjectCleanup::process",("ROWS = %d", qry.result.getValue() ));
		if( qry.result.getValue() == 0 )   // cleanup done
			break;
	}
	LOG("TaskVendorObjectCleanup",("TIME = %d", time(0)-t_start ));
	return true;
}

// ----------------------------------------------------------------------

void TaskVendorObjectCleanup::onComplete()
{
}

// ======================================================================

TaskVendorObjectCleanup::DataCleanupQuery::DataCleanupQuery()
{
}

// ----------------------------------------------------------------------
		
void TaskVendorObjectCleanup::DataCleanupQuery::getSQL(std::string &sql)
{
	sql="begin :result := "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.vendor_object_cleanup(); end;";
}

// ----------------------------------------------------------------------

bool TaskVendorObjectCleanup::DataCleanupQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskVendorObjectCleanup::DataCleanupQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskVendorObjectCleanup::DataCleanupQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}


// ======================================================================
