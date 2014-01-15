//========================================================================
//
// UnloadObjectMessage.h - tells Gameserver to unload an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_UnloadObjectMessage_H
#define	_INCLUDED_UnloadObjectMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class UnloadObjectMessage : public GameNetworkMessage
{
public:
	explicit UnloadObjectMessage  (const NetworkId& id);
	UnloadObjectMessage           (Archive::ReadIterator & source);
	~UnloadObjectMessage ();

	const NetworkId&          getId(void) const;

private:
	Archive::AutoVariable<NetworkId>     m_id;			// id of object we want to unload

	UnloadObjectMessage();
	UnloadObjectMessage(const UnloadObjectMessage&);
	UnloadObjectMessage& operator= (const UnloadObjectMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& UnloadObjectMessage::getId(void) const
{
	return m_id.get();
}	// UnloadObjectMessage::getId

// ----------------------------------------------------------------------

#endif	// _INCLUDED_UnloadObjectMessage_H
