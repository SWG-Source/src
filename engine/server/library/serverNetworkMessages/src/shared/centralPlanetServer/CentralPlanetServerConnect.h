// ======================================================================
//
// CentralPlanetServerConnect.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CentralPlanetServerConnect_H
#define INCLUDED_CentralPlanetServerConnect_H

// ======================================================================

#include <string>

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Identify the Planet Server to Central
 * Sent From:  Planet Server
 * Sent To:  Central
 * Action:  Identifies which scene this Planet Server handles.
 */

class CentralPlanetServerConnect : public GameNetworkMessage
{
  public:
	CentralPlanetServerConnect (const std::string &sceneId, const std::string &connectionAddress, uint16 gameServerPort);
	CentralPlanetServerConnect (Archive::ReadIterator &source);

	const std::string &getSceneId() const;
	const std::string &getConnectionAddress() const;
	uint16 getGameServerPort() const;
	
  private:
	Archive::AutoVariable<std::string> m_sceneId;
	Archive::AutoVariable<std::string> m_connectionAddress;
	Archive::AutoVariable<uint16> m_gameServerPort;
	
  private:
	CentralPlanetServerConnect ();
	CentralPlanetServerConnect(const CentralPlanetServerConnect&);
	CentralPlanetServerConnect& operator= (const CentralPlanetServerConnect&);
};

// ----------------------------------------------------------------------

inline const std::string &CentralPlanetServerConnect::getSceneId() const
{
	return m_sceneId.get();
}

// ----------------------------------------------------------------------

inline const std::string &CentralPlanetServerConnect::getConnectionAddress() const
{
	return m_connectionAddress.get();
}

// ----------------------------------------------------------------------

inline uint16 CentralPlanetServerConnect::getGameServerPort() const
{
	return m_gameServerPort.get();
}

// ======================================================================

#endif
