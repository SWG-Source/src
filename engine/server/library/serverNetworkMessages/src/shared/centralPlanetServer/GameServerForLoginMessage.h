// ======================================================================
//
// GameServerForLoginMessage.h
//
// Copyright 2001-2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GameServerForLoginMessage_H
#define INCLUDED_GameServerForLoginMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
// ======================================================================

/** 
 * Reply to RequestGameServerForLoginMessage.  Identifies the game server
 * to use to log in.
 *
 * Sent from:  PlanetServer 
 * Sent to:  ConnectionServer (via Central)
 * Action:  Connect the client to the specified game server
 */
class GameServerForLoginMessage : public GameNetworkMessage
{
public:
	GameServerForLoginMessage(uint32 stationId, uint32 server, const NetworkId & characterId);
	GameServerForLoginMessage(Archive::ReadIterator &source);
	~GameServerForLoginMessage();

public:
	uint32 getStationId() const;
	uint32 getServer() const;
	const NetworkId& getCharacterId() const;

private:
	Archive::AutoVariable<uint32> m_stationId;
	Archive::AutoVariable<uint32> m_server;
	Archive::AutoVariable<NetworkId> m_characterId;

private:
	GameServerForLoginMessage();
	GameServerForLoginMessage(const GameServerForLoginMessage&);
	GameServerForLoginMessage& operator= (const GameServerForLoginMessage&);
};

// ======================================================================

inline uint32 GameServerForLoginMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline uint32 GameServerForLoginMessage::getServer() const
{
	return m_server.get();
}

inline const NetworkId & GameServerForLoginMessage::getCharacterId() const
{
	return m_characterId.get();
}
// ======================================================================

#endif

