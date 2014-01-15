// ======================================================================
//
// GameServerUniverseLoadedMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GameServerUniverseLoadedMessage_H
#define INCLUDED_GameServerUniverseLoadedMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent from:  GameServer
 * Sent to:  Central
 * Action:  The GameServer has completed loading its universe objects.
 */
class GameServerUniverseLoadedMessage : public GameNetworkMessage
{
  public:
	GameServerUniverseLoadedMessage  (uint32 processId, uint32 sourceOfUniverseDataProcessId);
	GameServerUniverseLoadedMessage  (Archive::ReadIterator & source);
	~GameServerUniverseLoadedMessage ();

	uint32 getProcessId() const;
	uint32 getSourceOfUniverseDataProcessId() const;

  private:
	Archive::AutoVariable<uint32> m_processId;
	Archive::AutoVariable<uint32> m_sourceOfUniverseDataProcessId;

	GameServerUniverseLoadedMessage(const GameServerUniverseLoadedMessage&);
	GameServerUniverseLoadedMessage& operator= (const GameServerUniverseLoadedMessage&);
};

// ----------------------------------------------------------------------

inline uint32 GameServerUniverseLoadedMessage::getProcessId() const
{
	return m_processId.get();
}

// ----------------------------------------------------------------------

inline uint32 GameServerUniverseLoadedMessage::getSourceOfUniverseDataProcessId() const
{
	return m_sourceOfUniverseDataProcessId.get();
}

// ======================================================================

#endif
