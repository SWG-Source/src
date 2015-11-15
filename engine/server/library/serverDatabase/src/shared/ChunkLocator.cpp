// ======================================================================
//
// ChunkLocator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/ChunkLocator.h"

#include <string>
#include <vector>

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/GetChunkQuery.h"
#include "serverNetworkMessages/ChunkCompleteMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/Clock.h"
#include "sharedLog/Log.h"

// ======================================================================

ChunkLocator::ChunkLocator(int nodeX, int nodeZ, const std::string &sceneId, uint32 serverId, bool wantChunkCompleteMessage) :
		ObjectLocator(),
		m_nodeX(nodeX),
		m_nodeZ(nodeZ),
		m_sceneId(new std::string(sceneId)),
		m_objectCount(0),
		m_queryTime(0),
		m_serverId(serverId),
		m_wantChunkCompleteMessage(wantChunkCompleteMessage)
{
}

// ----------------------------------------------------------------------

ChunkLocator::~ChunkLocator()
{
	if (ConfigServerDatabase::getLogChunkLoading())
		LOG("ChunkLocator",("Chunk %s (%i,%i) (server %lu) -- %i objects %i ms",m_sceneId->c_str(), m_nodeX, m_nodeZ, m_serverId, m_objectCount,m_queryTime));

	delete m_sceneId;
	m_sceneId=0;
}

// ======================================================================

bool ChunkLocator::locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated)
{
	NOT_NULL(session);
	
	DBQuery::GetChunkQuery qry(schema);
	qry.setChunk(*m_sceneId, m_nodeX, m_nodeZ);
	int startTime = Clock::timeMs();

	bool rval = session->exec(&qry);
	qry.done();

	m_objectCount = qry.getObjectCount();
	m_queryTime = Clock::timeMs()-startTime;

	objectsLocated = m_objectCount;
	return rval;
}

// ----------------------------------------------------------------------

void ChunkLocator::sendPostBaselinesCustomData(GameServerConnection &conn) const
{
	if (m_wantChunkCompleteMessage)
		conn.queueCompletedChunk(m_nodeX, m_nodeZ);
}

// ======================================================================
