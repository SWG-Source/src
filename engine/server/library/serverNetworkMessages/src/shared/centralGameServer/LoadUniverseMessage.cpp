//========================================================================
//
// LoadUniverseMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoadUniverseMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param process		gameserver id that should receive the loaded objects
 */
LoadUniverseMessage::LoadUniverseMessage  (uint32 process) :
		GameNetworkMessage("LoadUniverseMessage"),
		m_process(process)
{
	addVariable(m_process);
}	

//-----------------------------------------------------------------------

LoadUniverseMessage::LoadUniverseMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("LoadUniverseMessage"),
		m_process()
{
	addVariable(m_process);

	unpack(source);
}

//-----------------------------------------------------------------------

LoadUniverseMessage::~LoadUniverseMessage()
{
}	

//-----------------------------------------------------------------------
