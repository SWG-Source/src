
// ======================================================================
//
// ObjectMenuSelectMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectMenuSelectMessage_H
#define INCLUDED_ObjectMenuSelectMessage_H

//-----------------------------------------------------------------------

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

//-----------------------------------------------------------------------

class ObjectMenuSelectMessage : public GameNetworkMessage
{
public:

	static const char * const MESSAGE_TYPE;

	           ObjectMenuSelectMessage (const NetworkId & playerId, uint16 selectedId);
	explicit   ObjectMenuSelectMessage (Archive::ReadIterator & source);
	virtual   ~ObjectMenuSelectMessage ();

	const NetworkId &                       getNetworkId () const;
	const int                               getSelectedItemId () const;

private:

	Archive::AutoVariable<NetworkId>        m_playerId;
	Archive::AutoVariable<uint16>           m_selectedItemId;

private:
	ObjectMenuSelectMessage(const ObjectMenuSelectMessage&);
	ObjectMenuSelectMessage& operator= (const ObjectMenuSelectMessage&);
};

//-----------------------------------------------------------------------

inline const NetworkId & ObjectMenuSelectMessage::getNetworkId () const
{
	return m_playerId.get ();
}

//----------------------------------------------------------------------

inline const int ObjectMenuSelectMessage::getSelectedItemId () const
{
	return static_cast<int>(m_selectedItemId.get ());
}

//-----------------------------------------------------------------

#endif
