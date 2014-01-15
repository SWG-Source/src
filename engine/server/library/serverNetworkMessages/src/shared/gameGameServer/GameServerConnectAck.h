// ======================================================================
//
// GameServerConnectAck.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GameServerConnectAck_H
#define INCLUDED_GameServerConnectAck_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from: Gameserver
 * Sent to:   Gameserver
 * Action:  Sent in response to GameGameServerConnect.  Acknowledges
 * that we have recorded the processId of the game server.
 */
class GameServerConnectAck : public GameNetworkMessage
{
public:
	GameServerConnectAck  ();
	GameServerConnectAck  (Archive::ReadIterator & source);
	~GameServerConnectAck ();

private:
	GameServerConnectAck(const GameServerConnectAck&);
	GameServerConnectAck& operator= (const GameServerConnectAck&);
};

// ======================================================================

#endif
