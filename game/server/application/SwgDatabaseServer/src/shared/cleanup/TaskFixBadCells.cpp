// ======================================================================
//
// TaskFixBadCells.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskFixBadCells.h"

#include "serverDatabase/DatabaseProcess.h"
//#include "serverDatabase/Loader.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

bool TaskFixBadCells::process(DB::Session *session)
{
	FixBadCellsQuery qry;
	DEBUG_REPORT_LOG(true,("[DATABASE CLEANUP] : Processing Bad Cells Query.\n"));
	return (session->exec(&qry));
}

// ----------------------------------------------------------------------

void TaskFixBadCells::onComplete()
{
	DEBUG_REPORT_LOG(true,("[DATABASE CLEANUP] : TaskFixBadCells::onComplete().\n"));
}

// ======================================================================

TaskFixBadCells::FixBadCellsQuery::FixBadCellsQuery()
{
	DEBUG_REPORT_LOG(true,("[DATABASE CLEANUP] : TaskFixBadCells::FixBadCellsQuery::FixBadCellsQuery().\n"));
}

// ----------------------------------------------------------------------
		
void TaskFixBadCells::FixBadCellsQuery::getSQL(std::string &sql)
{
	sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"data_cleanup.run_fix_bad_cells(); end;";
	DEBUG_REPORT_LOG(true,("[DATABASE CLEANUP] : Query : %s.\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskFixBadCells::FixBadCellsQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool TaskFixBadCells::FixBadCellsQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskFixBadCells::FixBadCellsQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}


// ======================================================================
