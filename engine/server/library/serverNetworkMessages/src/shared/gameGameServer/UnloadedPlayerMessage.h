// ======================================================================
//
// UnloadedPlayerMessage.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UnloadedPlayerMessage_H
#define INCLUDED_UnloadedPlayerMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent From:  GameServer
 * Sent To:    DatabaseProcess
 * Action:     The specified player is unloading.  Reply us when all
 *             changes have been saved.
 *
 * Sent From:  GameServer
 * Sent To:    PlanetServer
 * Action:     The specified player is unloading.  If the player tries to
 *             log in again before his data is saved, send him to the GameServer.
 */
class UnloadedPlayerMessage : public GameNetworkMessage
{
  public:
	UnloadedPlayerMessage(const NetworkId &playerId);
	UnloadedPlayerMessage(Archive::ReadIterator & source);
	~UnloadedPlayerMessage();
	
  public:
	const NetworkId &getPlayerId();

  private:
	Archive::AutoVariable<NetworkId>  m_playerId;
	
  private:
	//disabled functions:
	UnloadedPlayerMessage            ();
	UnloadedPlayerMessage            (const UnloadedPlayerMessage&);
	UnloadedPlayerMessage& operator= (const UnloadedPlayerMessage&);

};

// ======================================================================

inline const NetworkId &UnloadedPlayerMessage::getPlayerId()
{
	return m_playerId.get();
} 

// ======================================================================

#endif
