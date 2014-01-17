// ======================================================================
//
// TaskOrphanedObjectCleanup.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskOrphanedObjectCleanup.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

bool TaskOrphanedObjectCleanup::process(DB::Session *session)
{
	time_t t_start = time(0);
	while( (time(0) - t_start) < m_tSeconds )  // limit the run time
	{
		DataCleanupQuery qry;
		if ( session->exec(&qry) == false )
		{
			LOG("TaskOrphanedObjectCleanup::process",( "returned false!" ));
			return false;
		}
		LOG("TaskOrphanedObjectCleanup::process",("ROWS = %d", qry.result.getValue() ));
		if( qry.result.getValue() == 0 )   // cleanup done
			break;
	}
	LOG("TaskOrphanedObjectCleanup",("TIME = %d", time(0)-t_start ));
	return true;
}

// ----------------------------------------------------------------------

void TaskOrphanedObjectCleanup::onComplete()
{
}

// ======================================================================

TaskOrphanedObjectCleanup::DataCleanupQuery::DataCleanupQuery()
{
}

// ----------------------------------------------------------------------
		
void TaskOrphanedObjectCleanup::DataCleanupQuery::getSQL(std::string &sql)
{
	sql="begin :result := "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.orphaned_object_cleanup(); end;";
}

// ----------------------------------------------------------------------

bool TaskOrphanedObjectCleanup::DataCleanupQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskOrphanedObjectCleanup::DataCleanupQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskOrphanedObjectCleanup::DataCleanupQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}


// ======================================================================
