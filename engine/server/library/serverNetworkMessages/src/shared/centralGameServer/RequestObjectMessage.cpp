//========================================================================
//
// RequestObjectMessage.cpp - tells Centralserver a Gameserver wants to create a
// proxy of an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RequestObjectMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id		id of object we want to create
 * @param process	process requesting the object
 */
RequestObjectMessage::RequestObjectMessage(const NetworkId& id, uint32 process) :
	GameNetworkMessage("RequestObjectMessage"),
	m_id(id),
	m_process(process)
{
	addVariable(m_id);
	addVariable(m_process);
}	// RequestObjectMessage::RequestObjectMessage

//-----------------------------------------------------------------------

	RequestObjectMessage::RequestObjectMessage(Archive::ReadIterator & source) :
GameNetworkMessage("RequestObjectMessage"),
m_id(),
m_process()
{
	addVariable(m_id);
	addVariable(m_process);
	unpack(source);
}	// RequestObjectMessage::RequestObjectMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
RequestObjectMessage::~RequestObjectMessage()
{
}	// RequestObjectMessage::~RequestObjectMessage

//-----------------------------------------------------------------------

