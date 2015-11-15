// ======================================================================
//
// CentralPlanetServerConnect.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CentralPlanetServerConnect.h"

// ======================================================================

CentralPlanetServerConnect::CentralPlanetServerConnect (const std::string &sceneId, const std::string &connectionAddress, uint16 gameServerPort) :
		GameNetworkMessage("CentralPlanetServerConnect"),
		m_sceneId(sceneId),
		m_connectionAddress(connectionAddress),
		m_gameServerPort(gameServerPort)
{
	addVariable(m_sceneId);
	addVariable(m_connectionAddress);
	addVariable(m_gameServerPort);
}

// ----------------------------------------------------------------------

CentralPlanetServerConnect::CentralPlanetServerConnect (Archive::ReadIterator &source) :
		GameNetworkMessage("CentralPlanetServerConnect"),
		m_sceneId()
{
	addVariable(m_sceneId);
	addVariable(m_connectionAddress);
	addVariable(m_gameServerPort);
	
	unpack(source);
}

// ======================================================================
