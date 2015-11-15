// ======================================================================
//
// TaskMessagesCleanup.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskMessagesCleanup.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

bool TaskMessagesCleanup::process(DB::Session *session)
{
	time_t t_start = time(0);
	while( (time(0) - t_start) < m_tSeconds )  // limit the run time
	{
		DataCleanupQuery qry;
		if ( session->exec(&qry) == false )
		{
			LOG("TaskMessagesCleanup::process",( "returned false!" ));
			return false;
		}
		LOG("TaskMessagesCleanup::process",("ROWS = %d", qry.result.getValue() ));
		if( qry.result.getValue() == 0 )   // cleanup done
			break;
	}
 	LOG("TaskMessagesCleanup",("TIME = %d", time(0)-t_start ));
	return true;
}

// ----------------------------------------------------------------------

void TaskMessagesCleanup::onComplete()
{
}

// ======================================================================

TaskMessagesCleanup::DataCleanupQuery::DataCleanupQuery()
{
}

// ----------------------------------------------------------------------
		
void TaskMessagesCleanup::DataCleanupQuery::getSQL(std::string &sql)
{
	sql="begin :result := "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.messages_cleanup(); end;";
}

// ----------------------------------------------------------------------

bool TaskMessagesCleanup::DataCleanupQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskMessagesCleanup::DataCleanupQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskMessagesCleanup::DataCleanupQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}


// ======================================================================
