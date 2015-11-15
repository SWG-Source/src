// ======================================================================
//
// PersistedPlayerMessage.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PersistedPlayerMessage_H
#define INCLUDED_PersistedPlayerMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent From:  DatabaseServer
 *      To:    GameServer
 * Action:     The specified player has been persisted.  The object can
 *             be deleted from memory.
 *
 * Sent From:  GameServer
 * Sent To:    PlanetServer
 * Action:     The specified player has been persisted and deleted.
 *             Reload him if he logs in again.
 */
class PersistedPlayerMessage: public GameNetworkMessage
{
public:
	PersistedPlayerMessage(NetworkId const &playerId);
	PersistedPlayerMessage(Archive::ReadIterator &source);
	~PersistedPlayerMessage();
	
public:
	NetworkId const &getPlayerId() const;

private:
	Archive::AutoVariable<NetworkId>  m_playerId;
	
private:
	//disabled functions:
	PersistedPlayerMessage            ();
	PersistedPlayerMessage            (PersistedPlayerMessage const &);
	PersistedPlayerMessage &operator= (PersistedPlayerMessage const &);

};

// ======================================================================

inline NetworkId const &PersistedPlayerMessage::getPlayerId() const
{
	return m_playerId.get();
} 

// ======================================================================

#endif
