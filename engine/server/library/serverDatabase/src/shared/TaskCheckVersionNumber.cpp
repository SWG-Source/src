// ======================================================================
//
// TaskCheckVersionNumber.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskCheckVersionNumber.h"

#include "serverDatabase/GetVersionNumberQuery.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskCheckVersionNumber::TaskCheckVersionNumber(int codeVersion, bool fatalOnMismatch) :
		m_codeVersion(codeVersion),
		m_databaseVersion(0),
		m_minVersion(0),
		m_fatalOnMismatch(fatalOnMismatch)
{
}

// ----------------------------------------------------------------------

TaskCheckVersionNumber::~TaskCheckVersionNumber()
{
}

// ----------------------------------------------------------------------

bool TaskCheckVersionNumber::process(DB::Session *session)
{
	int rowsFetched;
	DBQuery::GetVersionNumberQuery qry;

	if (! (session->exec(&qry)))
		return false;
	rowsFetched = qry.fetch();
	m_databaseVersion=qry.getVersionNumber();
	m_minVersion=qry.getMinVersionNumber();
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskCheckVersionNumber::onComplete()
{
	if ((m_codeVersion > m_databaseVersion) || (m_codeVersion < m_minVersion))
	{
		if (m_fatalOnMismatch)
		{
			FATAL(true,("***ERROR*** This version of the server requires database version %i, but the database is version %i. \nYou may need to run 'ant update_swg' to acquire the latest database updates.\n",m_codeVersion,m_databaseVersion));
		}
		else
		{
			WARNING(true,("***ERROR*** This version of the server requires database version %i, but the database is version %i. \nYou may need to run 'ant update_swg' to acquire the latest database updates.\n",m_codeVersion,m_databaseVersion));
		}
	}
}

// ======================================================================
