// ======================================================================
//
// UniverseLocator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/UniverseLocator.h"

#include <vector>

#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/GetUniverseQuery.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/SetUniverseAuthoritativeMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

bool UniverseLocator::locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated)
{
	NOT_NULL(session);
	DEBUG_REPORT_LOG(true,("Loading Universe Objects.\n"));
	
	DBQuery::GetUniverseQuery qry(schema);
	bool rval = session->exec(&qry);
	qry.done();

	objectsLocated = 1; //TODO:  Counts
	return rval;
}

// ----------------------------------------------------------------------

void UniverseLocator::sendPostBaselinesCustomData(GameServerConnection &conn) const
{
	DEBUG_REPORT_LOG(true,("Sending UniverseComplete.\n"));
	GenericValueTypeMessage<unsigned long> const universeCompleteMessage("UniverseCompleteMessage", DatabaseProcess::getInstance().getProcessId());
	conn.send(universeCompleteMessage, true);

	SetUniverseAuthoritativeMessage const setUniverseAuthoritativeMessage(conn.getProcessId());
	conn.send(setUniverseAuthoritativeMessage, true);
}

// ======================================================================
