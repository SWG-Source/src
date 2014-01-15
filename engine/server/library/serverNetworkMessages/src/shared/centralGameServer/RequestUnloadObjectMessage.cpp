//========================================================================
//
// RequestUnloadObjectMessage.cpp - tells CentralServer we want to unload an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RequestUnloadObjectMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id			id of object we want to unload
 * @param process		process id of Gameserver that wants to unload the object
 */
RequestUnloadObjectMessage::RequestUnloadObjectMessage(const NetworkId& id, uint32 process) :
	GameNetworkMessage("RequestUnloadObjectMessage"),
	m_id(id),
	m_process(process)
{
	addVariable(m_id);
	addVariable(m_process);
}	// RequestUnloadObjectMessage::RequestUnloadObjectMessage

//-----------------------------------------------------------------------

	RequestUnloadObjectMessage::RequestUnloadObjectMessage(Archive::ReadIterator & source) :
GameNetworkMessage("RequestUnloadObjectMessage"),
m_id(),
m_process()
{
	addVariable(m_id);
	addVariable(m_process);
	unpack(source);
}	// RequestUnloadObjectMessage::RequestUnloadObjectMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
RequestUnloadObjectMessage::~RequestUnloadObjectMessage()
{
}	// RequestUnloadObjectMessage::~RequestUnloadObjectMessage

//-----------------------------------------------------------------------

