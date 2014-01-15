// ======================================================================
//
// ValidateCharacterForLoginReplyMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ValidateCharacterForLoginReplyMessage.h"

#include "sharedMathArchive/VectorArchive.h"

// ======================================================================

ValidateCharacterForLoginReplyMessage::ValidateCharacterForLoginReplyMessage(bool approved, StationId suid, const NetworkId &characterId, const NetworkId &containerId, const std::string &scene, const Vector &coordinates, const Unicode::String &characterName) :
		GameNetworkMessage("ValidateCharacterForLoginReplyMessage"),
		m_approved(approved),
		m_suid(suid),
		m_characterId(characterId),
		m_containerId(containerId),
		m_scene(scene),
		m_coordinates(coordinates),
		m_characterName(characterName)
{
	addVariable(m_approved);
	addVariable(m_suid);
	addVariable(m_characterId);
	addVariable(m_containerId);
	addVariable(m_scene);
	addVariable(m_coordinates);
	addVariable(m_characterName);
}

// ----------------------------------------------------------------------

ValidateCharacterForLoginReplyMessage::ValidateCharacterForLoginReplyMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ValidateCharacterForLoginMessage")
{
	addVariable(m_approved);
	addVariable(m_suid);
	addVariable(m_characterId);
	addVariable(m_containerId);
	addVariable(m_scene);
	addVariable(m_coordinates);
	addVariable(m_characterName);

	unpack(source);
}

// ======================================================================
