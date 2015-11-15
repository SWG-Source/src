// ======================================================================
//
// ObjectMenuSelectMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ObjectMenuSelectMessage.h"

//-----------------------------------------------------------------------

const char * const ObjectMenuSelectMessage::MESSAGE_TYPE = "ObjectMenuSelectMessage::MESSAGE_TYPE";

//-----------------------------------------------------------------

ObjectMenuSelectMessage::ObjectMenuSelectMessage (const NetworkId & playerId, uint16 selectedId) :
GameNetworkMessage (MESSAGE_TYPE),
m_playerId (playerId),
m_selectedItemId (selectedId)
{
	addVariable (m_playerId);
	addVariable (m_selectedItemId);
}

//-----------------------------------------------------------------------

ObjectMenuSelectMessage::ObjectMenuSelectMessage (Archive::ReadIterator & source) :
GameNetworkMessage (MESSAGE_TYPE),
m_playerId (),
m_selectedItemId ()
{
	addVariable (m_playerId);
	addVariable (m_selectedItemId);
	unpack (source);
}

//----------------------------------------------------------------------

ObjectMenuSelectMessage::~ObjectMenuSelectMessage ()
{
}

//-----------------------------------------------------------------------
