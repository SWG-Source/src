// ======================================================================
//
// TaskToggleCompletedTutorial.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskToggleCompletedTutorial.h"

#include "LoginServer.h"
#include "DatabaseConnection.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskToggleCompletedTutorial::TaskToggleCompletedTutorial(StationId stationId, bool completed) :
		TaskRequest(),
		m_stationId(stationId),
		m_completed(completed)
{
}

// ----------------------------------------------------------------------

bool TaskToggleCompletedTutorial::process(DB::Session *session)
{
	ToggleCompletedTutorialQuery qry;
	qry.station_id=static_cast<long>(m_stationId);
	qry.completed_flag=m_completed;

	bool rval = session->exec(&qry);

	qry.done();
	return rval;
}

// ----------------------------------------------------------------------

void TaskToggleCompletedTutorial::onComplete()
{
}

// ----------------------------------------------------------------------

TaskToggleCompletedTutorial::ToggleCompletedTutorialQuery::ToggleCompletedTutorialQuery() :
		Query(),
		station_id(),
		completed_flag()
{
}

// ----------------------------------------------------------------------

void TaskToggleCompletedTutorial::ToggleCompletedTutorialQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.toggle_completed_tutorial(:station_id, :completed_flag); end;";
	//DEBUG_REPORT_LOG(true, ("TaskToggleCompletedTutorial SQL: %s\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskToggleCompletedTutorial::ToggleCompletedTutorialQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(completed_flag)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool TaskToggleCompletedTutorial::ToggleCompletedTutorialQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskToggleCompletedTutorial::ToggleCompletedTutorialQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
