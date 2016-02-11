// ======================================================================
//
// TaskGetBiography.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskGetBiography.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/GetBiographyQuery.h"
#include "serverNetworkMessages/BiographyMessage.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskGetBiography::TaskGetBiography(const NetworkId &owner, uint32 requestingProcess) :
		m_owner(owner),
		m_requestingProcess(requestingProcess),
		m_bio(nullptr)
{
}

// ----------------------------------------------------------------------

TaskGetBiography::~TaskGetBiography()
{
	delete m_bio;
	m_bio=nullptr;
}

// ----------------------------------------------------------------------

bool TaskGetBiography::process(DB::Session *session)
{
	int rowsFetched;
	DBQuery::GetBiographyQuery qry(m_owner);
	if (! (session->exec(&qry)))
		return false;

	m_bio = new Unicode::String();
	
	if ((rowsFetched = qry.fetch()) > 0)
		qry.getBio(*m_bio);
	
	qry.done();
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskGetBiography::onComplete()
{
	WARNING_STRICT_FATAL(!m_bio,("In TaskGetBiography::onComplete, but m_bio is nullptr\n"));
	if (m_bio)
	{
		BiographyMessage msg(m_owner, *m_bio);
		GameServerConnection *conn = DatabaseProcess::getInstance().getConnectionByProcess(m_requestingProcess);
		if (conn)
			conn->send(msg,true);
		else
			DEBUG_REPORT_LOG(true,("Discarded loaded biography because Game Server has disconnected.\n"));
	}

}

// ======================================================================
