//========================================================================
//
// LoadContainedObjectMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_LoadContainedObjectMessage_H
#define	_INCLUDED_LoadContainedObjectMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent From:  GameServer
 * Sent To:    DatabaseProcess
 * Action:     Load the specified object (which must be in the specified
 *             container), and send it to the GameServer that sent this message.
 *
 * Sent From:  DatabaseProcess
 * Sent To:    GameServer
 * Action:     Inform the GameServer that the requested load has been completed.
 */
class LoadContainedObjectMessage : public GameNetworkMessage
{
public:
	LoadContainedObjectMessage            (const NetworkId & containerId, const NetworkId &objectId);
	LoadContainedObjectMessage            (Archive::ReadIterator & source);
	~LoadContainedObjectMessage           ();

	const NetworkId& getContainerId       (void) const;
	const NetworkId& getObjectId          (void) const;
        
private:
	Archive::AutoVariable<NetworkId>      m_containerId;
	Archive::AutoVariable<NetworkId>      m_objectId;

	//disabled functions:
	LoadContainedObjectMessage            ();
	LoadContainedObjectMessage            (const LoadContainedObjectMessage&);
	LoadContainedObjectMessage& operator= (const LoadContainedObjectMessage&);
};

//-----------------------------------------------------------------------

inline const NetworkId &LoadContainedObjectMessage::getContainerId(void) const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &LoadContainedObjectMessage::getObjectId(void) const
{
	return m_objectId.get();
}

// ----------------------------------------------------------------------

#endif	// _INCLUDED_LoadContainedObjectMessage_H

