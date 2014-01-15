// ======================================================================
//
// GameServerReadyMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GameServerReadyMessage_H
#define INCLUDED_GameServerReadyMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent from:  GameServer
 * Sent to:  Central and Planet Servers
 * Action:  The GameServer has finished initialization and is ready to
 * handle requests.
 */
class GameServerReadyMessage : public GameNetworkMessage
{
  public:
	GameServerReadyMessage  (int mapWidth);
	GameServerReadyMessage  (Archive::ReadIterator & source);
	~GameServerReadyMessage ();

	int getMapWidth() const;
	
  private:
	Archive::AutoVariable<int> m_mapWidth;
	
  private:

	GameServerReadyMessage(const GameServerReadyMessage&);
	GameServerReadyMessage& operator= (const GameServerReadyMessage&);
};

// ======================================================================

#endif
