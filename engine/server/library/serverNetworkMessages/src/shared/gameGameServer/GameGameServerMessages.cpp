//========================================================================
//
// GameGameServerMessages.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameGameServerMessages.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param processId		id of process that connected
 */
GameGameServerConnect::GameGameServerConnect(const uint32 processId, bool isDbProcess, int spawnCookie, int preloadNumber) :
	GameNetworkMessage("GameGameServerConnect"),
	m_isDbProcess(isDbProcess),
	m_processId(processId),
	m_spawnCookie(spawnCookie),
	m_preloadNumber(preloadNumber)
{
	addVariable(m_isDbProcess);
	addVariable(m_processId);
	addVariable(m_spawnCookie);
	addVariable(m_preloadNumber);
}

//-----------------------------------------------------------------------

GameGameServerConnect::GameGameServerConnect(Archive::ReadIterator & source) :
GameNetworkMessage("GameGameServerConnect"),
m_isDbProcess(0),
m_processId(),
m_spawnCookie(0),
m_preloadNumber(0)
{
	addVariable(m_isDbProcess);
	addVariable(m_processId);
	addVariable(m_spawnCookie);
	addVariable(m_preloadNumber);
	unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
GameGameServerConnect::~GameGameServerConnect()
{
}	

//-----------------------------------------------------------------------
