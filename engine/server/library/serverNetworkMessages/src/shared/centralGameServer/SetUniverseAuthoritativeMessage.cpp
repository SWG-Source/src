//========================================================================
//
// SetUniverseAuthoritativeMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/SetUniverseAuthoritativeMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param process		gameserver id that should receive the loaded objects
 */
SetUniverseAuthoritativeMessage::SetUniverseAuthoritativeMessage  (uint32 process) :
		GameNetworkMessage("SetUniverseAuthoritativeMessage"),
		m_process(process)
{
	addVariable(m_process);
}	

//-----------------------------------------------------------------------

SetUniverseAuthoritativeMessage::SetUniverseAuthoritativeMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("SetUniverseAuthoritativeMessage"),
		m_process()
{
	addVariable(m_process);

	unpack(source);
}

//-----------------------------------------------------------------------

SetUniverseAuthoritativeMessage::~SetUniverseAuthoritativeMessage()
{
}	

//-----------------------------------------------------------------------
