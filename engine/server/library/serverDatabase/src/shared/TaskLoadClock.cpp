// ======================================================================
//
// TaskLoadClock.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskLoadClock.h"

#include "serverDatabase/GetTimestampQuery.h"
#include "serverDatabase/Loader.h"
#include "serverUtility/ServerClock.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskLoadClock::TaskLoadClock() :
		m_timestamp(-1)
{
}

// ======================================================================

bool TaskLoadClock::process(DB::Session *session)
{
	int rowsFetched;
	DBQuery::GetTimestampQuery qry;

	if (! (session->exec(&qry)))
		return false;
	rowsFetched = qry.fetch();
	m_timestamp=qry.getTimestamp();
	return (rowsFetched >= 0);
}

// ======================================================================

void TaskLoadClock::onComplete()
{
	DEBUG_REPORT_LOG(true,("Setting clock to %i\n",m_timestamp));
	DEBUG_FATAL(m_timestamp==-1,("Suspicious timestamp.\n"));
	
	ServerClock::getInstance().setGameTimeSeconds(m_timestamp);
	Loader::getInstance().loadClockCompleted();
}

// ======================================================================
