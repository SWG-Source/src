// ======================================================================
//
// TaskDailyCleanup.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskDailyCleanup.h"

#include "SwgDatabaseServer/ObjvarNameManager.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

bool TaskDailyCleanup::process(DB::Session *session)
{
	DataCleanupQuery qry;
	return (session->exec(&qry));
}

// ----------------------------------------------------------------------

void TaskDailyCleanup::onComplete()
{
	DEBUG_REPORT_LOG(true,("[DATABASE CLEANUP] : TaskDailyCleanup::onComplete().\n"));
}

// ======================================================================

TaskDailyCleanup::DataCleanupQuery::DataCleanupQuery()
{
}

// ----------------------------------------------------------------------
		
void TaskDailyCleanup::DataCleanupQuery::getSQL(std::string &sql)
{
	sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.run_cleanup(); end;";
}

// ----------------------------------------------------------------------

bool TaskDailyCleanup::DataCleanupQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool TaskDailyCleanup::DataCleanupQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskDailyCleanup::DataCleanupQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}


// ======================================================================
