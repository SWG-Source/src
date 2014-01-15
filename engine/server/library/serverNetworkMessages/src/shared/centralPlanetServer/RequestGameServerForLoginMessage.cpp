// ======================================================================
//
// RequestGameServerForLoginMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RequestGameServerForLoginMessage.h"

//#include "PlanetGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

RequestGameServerForLoginMessage::RequestGameServerForLoginMessage(uint32 stationId, const NetworkId &characterId, const NetworkId &containerId, const std::string &scene, const Vector &coordinates, bool forCtsSourceCharacter) :
		GameNetworkMessage("RequestGameServerForLoginMessage"),
		m_stationId(stationId),
		m_characterId(characterId),
		m_containerId(containerId),
		m_scene(scene),
		m_coordinates(coordinates),
		m_forCtsSourceCharacter(forCtsSourceCharacter)
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_containerId);
	addVariable(m_scene);
	addVariable(m_coordinates);
	addVariable(m_forCtsSourceCharacter);
}

// ----------------------------------------------------------------------

RequestGameServerForLoginMessage::RequestGameServerForLoginMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("RequestGameServerForLoginMessage"),
		m_stationId(),
		m_characterId(),
		m_containerId(),
		m_scene(),
		m_coordinates(),
		m_forCtsSourceCharacter()
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_containerId);
	addVariable(m_scene);
	addVariable(m_coordinates);
	addVariable(m_forCtsSourceCharacter);

	unpack(source);
}

//-----------------------------------------------------------------------

RequestGameServerForLoginMessage::~RequestGameServerForLoginMessage()
{
}

// ======================================================================
