// ======================================================================
//
// UpdateConnectionServerStatus.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UpdateConnectionServerStatus_H
#define INCLUDED_UpdateConnectionServerStatus_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent from:  Connection Server
 * Sent to:  Central
 * Action:  Reports the number of players connected to that ConnectionServer
 */
class UpdateConnectionServerStatus : public GameNetworkMessage
{
  public:
	UpdateConnectionServerStatus(uint16 publicPort, uint16 privatePort);
	UpdateConnectionServerStatus(Archive::ReadIterator & source);

  public:
	uint16 getPublicPort() const;
	uint16 getPrivatePort() const;
	
  private:
	Archive::AutoVariable<uint16> m_publicPort;
	Archive::AutoVariable<uint16> m_privatePort;
};

// ======================================================================

inline uint16 UpdateConnectionServerStatus::getPublicPort() const
{
	return m_publicPort.get();
}

//----------------------------------------------------------------------

inline uint16 UpdateConnectionServerStatus::getPrivatePort() const
{
	return m_privatePort.get();
}

// ======================================================================

#endif
 
