// ======================================================================
//
// FirstPlanetGameServerIdMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FirstPlanetGameServerIdMessage_H
#define INCLUDED_FirstPlanetGameServerIdMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent from:  PlanetServer 
 * Sent to:  GameServer
 * Action:  Identifies the "first" GameServer for a particular planet.
 *   This GameServer will be used by certain systems that require an
 *   arbitrary GameServer or that are only supposed to run on one server
 *   per planet.
 */
class FirstPlanetGameServerIdMessage : public GameNetworkMessage
{
  public:
	FirstPlanetGameServerIdMessage  (uint32 gameServerId);
	FirstPlanetGameServerIdMessage  (Archive::ReadIterator & source);
	~FirstPlanetGameServerIdMessage ();

	uint32 getGameServerId() const;

  private:
	Archive::AutoVariable<uint32> m_gameServerId;

	FirstPlanetGameServerIdMessage();
	FirstPlanetGameServerIdMessage(const FirstPlanetGameServerIdMessage&);
	FirstPlanetGameServerIdMessage& operator= (const FirstPlanetGameServerIdMessage&);
};

// ----------------------------------------------------------------------

inline uint32 FirstPlanetGameServerIdMessage::getGameServerId() const
{
	return m_gameServerId.get();
}

// ======================================================================

#endif
