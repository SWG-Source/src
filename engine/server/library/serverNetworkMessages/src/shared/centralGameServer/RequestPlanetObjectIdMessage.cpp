//========================================================================
//
// RequestPlanetObjectIdMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RequestPlanetObjectIdMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param process		gameserver id that should receive the loaded objects
 */
RequestPlanetObjectIdMessage::RequestPlanetObjectIdMessage  (const std::string &sceneId) :
	GameNetworkMessage("RequestPlanetObjectIdMessage"),
	m_sceneId(sceneId)
{
	addVariable(m_sceneId);
}	

//-----------------------------------------------------------------------

RequestPlanetObjectIdMessage::RequestPlanetObjectIdMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("RequestPlanetObjectIdMessage"),
		m_sceneId()
{
	addVariable(m_sceneId);
	
	unpack(source);
}

//-----------------------------------------------------------------------

RequestPlanetObjectIdMessage::~RequestPlanetObjectIdMessage()
{
}	

//-----------------------------------------------------------------------
