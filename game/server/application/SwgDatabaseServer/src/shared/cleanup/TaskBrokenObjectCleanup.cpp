// ======================================================================
//
// TaskBrokenObjectCleanup.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskBrokenObjectCleanup.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

bool TaskBrokenObjectCleanup::process(DB::Session *session)
{
	time_t t_start = time(0);
	while( (time(0) - t_start) < m_tSeconds )  // limit the run time
	{
		DataCleanupQuery qry;
		if ( session->exec(&qry) == false )
		{
			LOG("TaskBrokenObjectCleanup::process",( "returned false!" ));
			return false;
		}
		LOG("TaskBrokenObjectCleanup::process",("ROWS = %d", qry.result.getValue() ));
		if( qry.result.getValue() == 0 )   // cleanup done
			break;
	}
	LOG("TaskBrokenObjectCleanup",("TIME = %d", time(0)-t_start ));
	return true;
}

// ----------------------------------------------------------------------

void TaskBrokenObjectCleanup::onComplete()
{
}

// ======================================================================

TaskBrokenObjectCleanup::DataCleanupQuery::DataCleanupQuery()
{
}

// ----------------------------------------------------------------------
		
void TaskBrokenObjectCleanup::DataCleanupQuery::getSQL(std::string &sql)
{
	sql="begin :result := "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.broken_object_cleanup(); end;";
}

// ----------------------------------------------------------------------

bool TaskBrokenObjectCleanup::DataCleanupQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskBrokenObjectCleanup::DataCleanupQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskBrokenObjectCleanup::DataCleanupQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}


// ======================================================================
