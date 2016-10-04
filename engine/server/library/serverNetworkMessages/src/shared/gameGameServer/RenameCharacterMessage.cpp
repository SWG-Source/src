//========================================================================
//
// RenameCharacterMessage.cpp
//
// copyright 2002 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"

#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

RenameCharacterMessage::RenameCharacterMessage(const NetworkId &characterId, const Unicode::String &newName, const NetworkId &requestedBy) :
	GameNetworkMessage("RenameCharacterMessage"),
	m_characterId(characterId),
	m_newName(newName),
	m_newNormalizedName(),
	m_requestedBy(requestedBy)
{
	addVariable(m_characterId);
	addVariable(m_newName);
	addVariable(m_newNormalizedName);
	addVariable(m_requestedBy);
}

//-----------------------------------------------------------------------

RenameCharacterMessage::RenameCharacterMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("RenameCharacterMessage"),
		m_characterId(),
		m_newName(),
		m_newNormalizedName(),
		m_requestedBy()
{
	addVariable(m_characterId);
	addVariable(m_newName);
	addVariable(m_newNormalizedName);
	addVariable(m_requestedBy);
	unpack(source);
}

//-----------------------------------------------------------------------

RenameCharacterMessage::~RenameCharacterMessage()
{
}

//========================================================================

RenameCharacterMessageEx::RenameCharacterMessageEx(RenameCharacterMessageSource renameCharacterMessageSource, uint32 stationId, const NetworkId &characterId, const Unicode::String &newName, const Unicode::String &oldName, const NetworkId &requestedBy) :
	GameNetworkMessage("RenameCharacterMessageEx"),
	m_stationId(stationId),
	m_characterId(characterId),
	m_newName(newName),
	m_oldName(oldName),
	m_requestedBy(requestedBy),
	m_renameCharacterMessageSource(static_cast<int8>(renameCharacterMessageSource)),
	m_lastNameChangeOnly(false)
{
	{
		static Unicode::String const delimiters(Unicode::narrowToWide(" "));

		Unicode::UnicodeStringVector newNameTokens;
		if (!Unicode::tokenize(newName, newNameTokens, &delimiters, nullptr))
			newNameTokens.clear();

		Unicode::UnicodeStringVector oldNameTokens;
		if (!Unicode::tokenize(oldName, oldNameTokens, &delimiters, nullptr))
			oldNameTokens.clear();

		m_lastNameChangeOnly.set(((newNameTokens.size() >= 1) && (oldNameTokens.size() >= 1) && Unicode::caseInsensitiveCompare(newNameTokens[0], oldNameTokens[0])));
	}

	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_newName);
	addVariable(m_oldName);
	addVariable(m_requestedBy);
	addVariable(m_renameCharacterMessageSource);
	addVariable(m_lastNameChangeOnly);
}

//-----------------------------------------------------------------------

RenameCharacterMessageEx::RenameCharacterMessageEx(Archive::ReadIterator & source) :
		GameNetworkMessage("RenameCharacterMessageEx"),
		m_stationId(),
		m_characterId(),
		m_newName(),
		m_oldName(),
		m_requestedBy(),
		m_renameCharacterMessageSource(),
		m_lastNameChangeOnly(false)
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_newName);
	addVariable(m_oldName);
	addVariable(m_requestedBy);
	addVariable(m_renameCharacterMessageSource);
	addVariable(m_lastNameChangeOnly);
	unpack(source);
}

//-----------------------------------------------------------------------

RenameCharacterMessageEx::~RenameCharacterMessageEx()
{
}

//-----------------------------------------------------------------------
