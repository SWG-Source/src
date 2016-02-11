// ======================================================================
//
// GameServerData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "GameServerData.h"

#include "ConfigPlanetServer.h"
#include "GameServerConnection.h"
#include "PlanetServer.h"
#include "sharedLog/Log.h"

// ======================================================================

GameServerData::GameServerData(GameServerConnection *connection) :
		m_connection(connection), 
		m_objectCount(0),
		m_interestObjectCount(0),
		m_interestCreatureObjectCount(0),
		m_serverStatus(SS_unready)
{
}

// ----------------------------------------------------------------------

GameServerData::GameServerData(const GameServerData &rhs) :
		m_connection(nullptr),  // connection pointer is not copied when we copy GameServerDatas
		m_objectCount(rhs.m_objectCount),
		m_interestObjectCount(rhs.m_interestObjectCount),
		m_interestCreatureObjectCount(rhs.m_interestCreatureObjectCount),
		m_serverStatus(rhs.m_serverStatus)		
{
}

// ----------------------------------------------------------------------

GameServerData::GameServerData() :
		m_connection(nullptr), 
		m_objectCount(0),
		m_interestObjectCount(0),
		m_interestCreatureObjectCount(0),
		m_serverStatus(SS_unready)
{
}

// ----------------------------------------------------------------------

GameServerData &GameServerData::operator=(const GameServerData &rhs)
{
	if (&rhs == this)
		return *this;

	m_connection=nullptr;  // connection pointer is not copied when we copy GameServerDatas
	m_objectCount=rhs.m_objectCount;
	m_interestObjectCount=rhs.m_interestObjectCount;
	m_interestCreatureObjectCount=rhs.m_interestCreatureObjectCount;
	m_serverStatus=rhs.m_serverStatus;
	return *this;
}

// ----------------------------------------------------------------------

void GameServerData::universeLoaded()
{
	WARNING_DEBUG_FATAL(m_serverStatus != SS_readyForObjects,("Got UniverseLoaded for server %d when it was not in the \"readyForObjects\" state.",getProcessId()));
	m_serverStatus = SS_loadedUniverseObjects;
}

// ----------------------------------------------------------------------

void GameServerData::ready()
{
	WARNING_DEBUG_FATAL(m_serverStatus != SS_unready,("Got GameServerReady for server %d when it was not in the \"unready\" state.",getProcessId()));
	m_serverStatus = SS_readyForObjects;
}

// ----------------------------------------------------------------------

void GameServerData::preloadComplete()
{
	WARNING_DEBUG_FATAL(PlanetServer::getInstance().getEnablePreload() && (m_serverStatus != SS_loadedUniverseObjects),("Got prelaod complete for server %d when it was not in the \"loadedUniverseObjects\" state.",getProcessId()));
	m_serverStatus = SS_running;
}

// ----------------------------------------------------------------------

uint32 GameServerData::getProcessId() const
{
	NOT_NULL(m_connection);
	return m_connection->getProcessId();
}

// ----------------------------------------------------------------------

void GameServerData::debugOutputData() const
{
	LOG("LoadBalancing", ("Server %d:  %i objects, %i interest objects",getProcessId(),getObjectCount(),getInterestObjectCount()));
}

// ======================================================================
