//========================================================================
//
// TeleportToMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_TeleportToMessage_H
#define	_INCLUDED_TeleportToMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  DBProcess
 * Action:  teleport a person to a target objectid
 */
class TeleportToMessage : public GameNetworkMessage
{
public:
	TeleportToMessage  (NetworkId const &actorId, NetworkId const &targetId, uint32 processId);
	TeleportToMessage  (Archive::ReadIterator & source);
	~TeleportToMessage ();

public:
	NetworkId const &  getActorId() const;
	NetworkId const &  getTargetId() const;
	uint32             getProcessId() const;
	
private:
	Archive::AutoVariable<NetworkId>  m_actorId;
	Archive::AutoVariable<NetworkId>  m_targetId;
	Archive::AutoVariable<uint32>     m_processId;

private:
	TeleportToMessage(const TeleportToMessage&);
	TeleportToMessage& operator= (const TeleportToMessage&);
};

// ======================================================================

inline NetworkId const &TeleportToMessage::getActorId() const
{
	return m_actorId.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &TeleportToMessage::getTargetId() const
{
	return m_targetId.get();
}

// ----------------------------------------------------------------------

inline uint32 TeleportToMessage::getProcessId() const
{
	return m_processId.get();
}

// ======================================================================

#endif	// _INCLUDED_TeleportToMessage_H
