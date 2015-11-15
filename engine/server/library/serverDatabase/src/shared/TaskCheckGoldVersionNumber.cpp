// ======================================================================
//
// TaskCheckGoldVersionNumber.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskCheckGoldVersionNumber.h"

#include "serverDatabase/GetGoldVersionNumberQuery.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskCheckGoldVersionNumber::TaskCheckGoldVersionNumber(int codeVersion, bool fatalOnMismatch) :
		m_codeVersion(codeVersion),
		m_databaseVersion(0),
		m_minVersion(0),
		m_fatalOnMismatch(fatalOnMismatch)
{
}

// ----------------------------------------------------------------------

TaskCheckGoldVersionNumber::~TaskCheckGoldVersionNumber()
{
}

// ----------------------------------------------------------------------

bool TaskCheckGoldVersionNumber::process(DB::Session *session)
{
	int rowsFetched;
	DBQuery::GetGoldVersionNumberQuery qry;

	if (! (session->exec(&qry)))
		return false;
	rowsFetched = qry.fetch();
	m_databaseVersion=qry.getVersionNumber();
	m_minVersion=qry.getMinVersionNumber();
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskCheckGoldVersionNumber::onComplete()
{
	if ((m_codeVersion > m_databaseVersion) || (m_codeVersion < m_minVersion))
	{
		if (m_fatalOnMismatch)
		{
			FATAL(true,("This version of the server requires GOLD database version %i, but the GOLD database is version %i.\n",m_codeVersion,m_databaseVersion));
		}
		else
		{
			WARNING(true,("This version of the server requires GOLD database version %i, but the GOLD database is version %i.\n",m_codeVersion,m_databaseVersion));
		}
	}
}

// ======================================================================
