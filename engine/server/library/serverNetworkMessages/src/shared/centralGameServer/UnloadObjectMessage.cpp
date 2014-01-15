//========================================================================
//
// UnloadObjectMessage.cpp - tells Gameserver to unload an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id			id of object we want to unload
 */
UnloadObjectMessage::UnloadObjectMessage(const NetworkId& id) :
	GameNetworkMessage("UnloadObjectMessage"),
	m_id(id)
{
	addVariable(m_id);
}	// UnloadObjectMessage::UnloadObjectMessage

//-----------------------------------------------------------------------

	UnloadObjectMessage::UnloadObjectMessage(Archive::ReadIterator & source) :
GameNetworkMessage("UnloadObjectMessage"),
m_id()
{
	addVariable(m_id);
	unpack(source);
}	// UnloadObjectMessage::UnloadObjectMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
UnloadObjectMessage::~UnloadObjectMessage()
{
}	// UnloadObjectMessage::~UnloadObjectMessage

//-----------------------------------------------------------------------

