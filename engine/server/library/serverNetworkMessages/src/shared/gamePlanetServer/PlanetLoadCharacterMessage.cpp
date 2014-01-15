// ======================================================================
//
// PlanetLoadCharacterMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/PlanetLoadCharacterMessage.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

PlanetLoadCharacterMessage::PlanetLoadCharacterMessage  (const NetworkId &characterId, uint32 gameServerId) :
		GameNetworkMessage("PlanetLoadCharacterMessage"),
		m_characterId(characterId),
		m_gameServerId(gameServerId)
{
	addVariable(m_characterId);
	addVariable(m_gameServerId);
}

// ----------------------------------------------------------------------

PlanetLoadCharacterMessage::PlanetLoadCharacterMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("PlanetLoadCharacterMessage"),
		m_characterId(),
		m_gameServerId()
{
	addVariable(m_characterId);
	addVariable(m_gameServerId);

	unpack(source);
}

//-----------------------------------------------------------------------

PlanetLoadCharacterMessage::~PlanetLoadCharacterMessage()
{
}

// ======================================================================
