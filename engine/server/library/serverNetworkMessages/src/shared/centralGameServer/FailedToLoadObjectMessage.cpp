//========================================================================
//
// FailedToLoadObjectMessage.cpp - tells Centralserver an object couldn't be loaded.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/FailedToLoadObjectMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id					id of object we didn't load
 * @param requestingProcess		process that originally requested the load
 */
FailedToLoadObjectMessage::FailedToLoadObjectMessage(const NetworkId& id,
	uint32 requestingProcess) :
	GameNetworkMessage("FailedToLoadObjectMessage"),
	m_id(id),
	m_process(requestingProcess)
{
	addVariable(m_id);
	addVariable(m_process);
}	// FailedToLoadObjectMessage::FailedToLoadObjectMessage

//-----------------------------------------------------------------------

FailedToLoadObjectMessage::FailedToLoadObjectMessage(Archive::ReadIterator & source) :
GameNetworkMessage("FailedToLoadObjectMessage"),
m_id(),
m_process()
{
	addVariable(m_id);
	addVariable(m_process);
	unpack(source);
}	// FailedToLoadObjectMessage::FailedToLoadObjectMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
FailedToLoadObjectMessage::~FailedToLoadObjectMessage()
{
}	// FailedToLoadObjectMessage::~FailedToLoadObjectMessage

//-----------------------------------------------------------------------
