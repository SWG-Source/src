//========================================================================
//
// ReleaseAuthoritativeMessage.cpp - tells Centralserver an authoritative object
// wants to release it's authority to a proxy.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ReleaseAuthoritativeMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id			id of object that is releasing it's authority
 * @param process		process we would like to release the authority to (0 = any)
 */
ReleaseAuthoritativeMessage::ReleaseAuthoritativeMessage(const NetworkId& id, uint32 process) :
	GameNetworkMessage("ReleaseAuthoritativeMessage"),
	m_id(id),
	m_process(process)
{
	addVariable(m_id);
	addVariable(m_process);
}	// ReleaseAuthoritativeMessage::ReleaseAuthoritativeMessage

//-----------------------------------------------------------------------

	ReleaseAuthoritativeMessage::ReleaseAuthoritativeMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ReleaseAuthoritativeMessage"),
m_id(),
m_process()
{
	addVariable(m_id);
	addVariable(m_process);
	unpack(source);
}	// ReleaseAuthoritativeMessage::ReleaseAuthoritativeMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
ReleaseAuthoritativeMessage::~ReleaseAuthoritativeMessage()
{
}	// ReleaseAuthoritativeMessage::~ReleaseAuthoritativeMessage

//-----------------------------------------------------------------------

