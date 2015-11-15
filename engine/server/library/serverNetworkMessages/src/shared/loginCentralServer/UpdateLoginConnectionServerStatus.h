// ======================================================================
//
// UpdateLoginConnectionServerStatus.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UpdateLoginConnectionServerStatus_H
#define INCLUDED_UpdateLoginConnectionServerStatus_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent from:  Connection Server
 * Sent to:  Central
 * Action:  Reports the number of players connected to that ConnectionServer
 */
class UpdateLoginConnectionServerStatus : public GameNetworkMessage
{
  public:
	UpdateLoginConnectionServerStatus(int id, uint16 publicPort, uint16 privatePort, int playerCount);
	UpdateLoginConnectionServerStatus(Archive::ReadIterator & source);

  public:
	int    getId() const;
	uint16 getPublicPort() const;
	uint16 getPrivatePort() const;
	int    getPlayerCount() const;
	
  private:
	Archive::AutoVariable<int> m_id;
	Archive::AutoVariable<uint16> m_publicPort;
	Archive::AutoVariable<uint16> m_privatePort;
	Archive::AutoVariable<int>    m_playerCount;
};

// ======================================================================
inline int UpdateLoginConnectionServerStatus::getId() const
{
	return m_id.get();
}

//----------------------------------------------------------------------

inline uint16 UpdateLoginConnectionServerStatus::getPublicPort() const
{
	return m_publicPort.get();
}

//----------------------------------------------------------------------

inline uint16 UpdateLoginConnectionServerStatus::getPrivatePort() const
{
	return m_privatePort.get();
}

//----------------------------------------------------------------------

inline int UpdateLoginConnectionServerStatus::getPlayerCount() const
{
	return m_playerCount.get();
}

// ======================================================================

#endif
 
