// ======================================================================
//
// ValidateCharacterForLoginMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ValidateCharacterForLoginMessage.h"

// ======================================================================

ValidateCharacterForLoginMessage::ValidateCharacterForLoginMessage(StationId suid, const NetworkId &characterId) :
		GameNetworkMessage("ValidateCharacterForLoginMessage"),
		m_suid(suid),
		m_characterId(characterId)
{
	addVariable(m_suid);
	addVariable(m_characterId);
}

// ----------------------------------------------------------------------

ValidateCharacterForLoginMessage::ValidateCharacterForLoginMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ValidateCharacterForLoginMessage")
{
	addVariable(m_suid);
	addVariable(m_characterId);

	unpack(source);
}

// ======================================================================
