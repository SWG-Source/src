//========================================================================
//
// LoadContentsMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_LoadContentsMessage_H
#define	_INCLUDED_LoadContentsMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent From:  GameServer
 * Sent To:    DatabaseProcess
 * Action:     Load the contents of the specified container and send them
 *             to the GameServer that sent this message.
 *
 * Sent From:  DatabaseProcess
 * Sent To:    GameServer
 * Action:     Inform the GameServer that the requested load has been completed.
 */
class LoadContentsMessage : public GameNetworkMessage
{
public:
	LoadContentsMessage            (const NetworkId & containerId);
	LoadContentsMessage            (Archive::ReadIterator & source);
	~LoadContentsMessage           ();

	const NetworkId& getContainerId       (void) const;
        
private:
	Archive::AutoVariable<NetworkId>      m_containerId;

	//disabled functions:
	LoadContentsMessage            ();
	LoadContentsMessage            (const LoadContentsMessage&);
	LoadContentsMessage& operator= (const LoadContentsMessage&);
};

//-----------------------------------------------------------------------

inline const NetworkId &LoadContentsMessage::getContainerId(void) const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

#endif

