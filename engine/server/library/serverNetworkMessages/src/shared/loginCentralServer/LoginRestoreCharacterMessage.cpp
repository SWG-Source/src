// ======================================================================
//
// LoginRestoreCharacterMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoginRestoreCharacterMessage.h"

// ======================================================================

LoginRestoreCharacterMessage::LoginRestoreCharacterMessage(const std::string &whoRequested, const NetworkId &characterId, StationId account, const Unicode::String &characterName, int templateId, bool jedi) :
		GameNetworkMessage("LoginRestoreCharacterMessage"),
		m_whoRequested(whoRequested),
		m_characterId(characterId),
		m_account(account),
		m_characterName(characterName),
		m_templateId(templateId),
		m_jedi(jedi)
{
	addVariable(m_whoRequested);
	addVariable(m_characterId);
	addVariable(m_account);
	addVariable(m_characterName);
	addVariable(m_templateId);
	addVariable(m_jedi);
}

// ----------------------------------------------------------------------

LoginRestoreCharacterMessage::LoginRestoreCharacterMessage (Archive::ReadIterator & source) :
		GameNetworkMessage("LoginRestoreCharacterMessage"),
		m_whoRequested(),
		m_characterId(),
		m_account(),
		m_characterName(),
		m_templateId(),
		m_jedi()
{
	addVariable(m_whoRequested);
	addVariable(m_characterId);
	addVariable(m_account);
	addVariable(m_characterName);
	addVariable(m_templateId);
	addVariable(m_jedi);

	unpack(source);
}

// ----------------------------------------------------------------------

LoginRestoreCharacterMessage::~LoginRestoreCharacterMessage ()
{
}

// ======================================================================
