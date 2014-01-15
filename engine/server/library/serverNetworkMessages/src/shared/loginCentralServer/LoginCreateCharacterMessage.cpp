// ======================================================================
//
// LoginCreateCharacterMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoginCreateCharacterMessage.h"

// ======================================================================

LoginCreateCharacterMessage::LoginCreateCharacterMessage (const StationId &stationId, const Unicode::String &characterName, const NetworkId &characterObjectId, int templateId, bool jedi) :
		GameNetworkMessage("LoginCreateCharacterMessage"),
		m_stationId(stationId),
		m_characterName(characterName),
		m_characterObjectId(characterObjectId),
		m_templateId(templateId),
		m_jedi(jedi)
{
	addVariable(m_stationId);
	addVariable(m_characterName);
	addVariable(m_characterObjectId);
	addVariable(m_templateId);
	addVariable(m_jedi);
}

// ----------------------------------------------------------------------

LoginCreateCharacterMessage::LoginCreateCharacterMessage (Archive::ReadIterator & source) :
		GameNetworkMessage("LoginCreateCharacterMessage"),
		m_stationId(),
		m_characterName(),
		m_characterObjectId(),
		m_templateId(),
		m_jedi()
{
	addVariable(m_stationId);
	addVariable(m_characterName);
	addVariable(m_characterObjectId);
	addVariable(m_templateId);
	addVariable(m_jedi);

	unpack(source);
}

// ----------------------------------------------------------------------

LoginCreateCharacterMessage::~LoginCreateCharacterMessage ()
{
}

// ======================================================================
