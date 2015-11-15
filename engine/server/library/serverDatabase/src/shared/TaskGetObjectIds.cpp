// ======================================================================
// 
// TaskGetObjectIds.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/TaskGetObjectIds.h"

#include "serverNetworkMessages/AddObjectIdBlockMessage.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GetObjectIdsQuery.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

TaskGetObjectIds::TaskGetObjectIds(int processId, int howMany, bool logRequest) :
		m_processId(processId),
		m_howMany(howMany),
		m_logRequest(logRequest)
{
}

// ----------------------------------------------------------------------

bool TaskGetObjectIds::process(DB::Session *session)
{
	if (m_logRequest)
		LOG("ObjectIdManager", ("TaskGetObjectIds::process() for %d more object ids for pid %d", m_howMany, m_processId));

	int numLeft=m_howMany;
	DBQuery::GetOIDsQuery qry;
	
	while (numLeft > 0)
	{
		qry.setHowMany(numLeft);
	
		if (! session->exec(&qry))
			return false;

		m_blockList.push_back(ObjectIdBlock(qry.getStartId(),qry.getEndId()));

		numLeft -= qry.getHowMany();
	}
	return true;
}

// ----------------------------------------------------------------------
/**
 * Send the object ID's to central.
 *
 * We send the block(s) of ID's to the central server.  It is central's
 * job to forward them to whatever game server needs them.
 * In theory, we could send them directly to the game server.  However,
 * getting ID's happens during the startup process and it is simpler if
 * we assume we don't have a connection to the game server yet.
 */
void TaskGetObjectIds::onComplete()
{
	if (m_logRequest)
		LOG("ObjectIdManager", ("TaskGetObjectIds::onComplete() for %d more object ids for pid %d, returning %d blocks", m_howMany, m_processId, m_blockList.size()));

	bool logRequest = m_logRequest;
	for (BlockListType::iterator i=m_blockList.begin(); i!=m_blockList.end(); ++i)
	{
		// Send one message for each block
		AddOIDBlockMessage msg(m_processId,(*i).m_startId,(*i).m_endId, logRequest);
		DatabaseProcess::getInstance().sendToCentralServer(msg,true);

		// only need to do logging, if requested, for the first block sent
		logRequest = false;
	}
}

// ======================================================================
