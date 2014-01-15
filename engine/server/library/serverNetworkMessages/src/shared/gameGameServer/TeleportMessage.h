//========================================================================
//
// TeleportMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_TeleportMessage_H
#define	_INCLUDED_TeleportMessage_H

//-----------------------------------------------------------------------

#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  DBProcess
 * Action:  teleport a person to a target objectid
 */
class TeleportMessage: public GameNetworkMessage
{
public:
	TeleportMessage(NetworkId const &actorId, std::string const &sceneId, Vector const &position_w, NetworkId const &containerId, Vector const &position_p);
	TeleportMessage(Archive::ReadIterator & source);
	~TeleportMessage();

public:
	NetworkId const &    getActorId() const;
	std::string const &  getSceneId() const;
	Vector const &       getPosition_w() const;
	NetworkId const &    getContainerId() const;
	Vector const &       getPosition_p() const;
	
private:
	Archive::AutoVariable<NetworkId>    m_actorId;
	Archive::AutoVariable<std::string>  m_sceneId;
	Archive::AutoVariable<Vector>       m_position_w;
	Archive::AutoVariable<NetworkId>    m_containerId;
	Archive::AutoVariable<Vector>       m_position_p;

private:
	TeleportMessage(const TeleportMessage&);
	TeleportMessage& operator= (const TeleportMessage&);
};

// ======================================================================

inline NetworkId const &TeleportMessage::getActorId() const
{
	return m_actorId.get();
}

// ----------------------------------------------------------------------

inline std::string const &TeleportMessage::getSceneId() const
{
	return m_sceneId.get();
}

// ----------------------------------------------------------------------

inline Vector const &TeleportMessage::getPosition_w() const
{
	return m_position_w.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &TeleportMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline Vector const &TeleportMessage::getPosition_p() const
{
	return m_position_p.get();
}

// ======================================================================

#endif	// _INCLUDED_TeleportMessage_H

