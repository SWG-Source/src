//========================================================================
//
// PlanetObjectIdMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/PlanetObjectIdMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param process		gameserver id that should receive the loaded objects
 */
PlanetObjectIdMessage::PlanetObjectIdMessage  (const std::string &sceneId, NetworkId planetObject) :
	GameNetworkMessage("PlanetObjectIdMessage"),
	m_sceneId(sceneId),
	m_planetObject(planetObject)
{
	addVariable(m_sceneId);
	addVariable(m_planetObject);
}	

//-----------------------------------------------------------------------

PlanetObjectIdMessage::PlanetObjectIdMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("PlanetObjectIdMessage"),
		m_sceneId(),
		m_planetObject()
{
	addVariable(m_sceneId);
	addVariable(m_planetObject);
	
	unpack(source);
}

//-----------------------------------------------------------------------

PlanetObjectIdMessage::~PlanetObjectIdMessage()
{
}	

//-----------------------------------------------------------------------
