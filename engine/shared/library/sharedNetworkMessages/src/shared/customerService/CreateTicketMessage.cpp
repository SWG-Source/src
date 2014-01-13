// ======================================================================
//
// CreateTicketMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateTicketMessage.h"

// ======================================================================

CreateTicketMessage::CreateTicketMessage(
	const std::string &characterName, 
	unsigned category, 
	unsigned subCategory, 
	const Unicode::String &details, 
	const Unicode::String &hiddenDetails, 
	const Unicode::String &harassingPlayerName,
	const std::string &language,
	bool isBug
) :
GameNetworkMessage("CreateTicketMessage"),
m_characterName(characterName),
m_category(category),
m_subCategory(subCategory),
m_details(details),
m_hiddenDetails(hiddenDetails),
m_harassingPlayerName(harassingPlayerName),
m_language(language),
m_stationId(0),
m_isBug(isBug)
{
	addVariable(m_characterName);
	addVariable(m_category);
	addVariable(m_subCategory);
	addVariable(m_details);
	addVariable(m_hiddenDetails);
	addVariable(m_harassingPlayerName);
	addVariable(m_language);
	addVariable(m_stationId);
	addVariable(m_isBug);
}

// ======================================================================

CreateTicketMessage::CreateTicketMessage(Archive::ReadIterator &source) :
GameNetworkMessage("CreateTicketMessage"),
m_category(0),
m_subCategory(0),
m_details(),
m_hiddenDetails(),
m_harassingPlayerName(),
m_language(),
m_stationId(0),
m_isBug(false)
{
	addVariable(m_characterName);
	addVariable(m_category);
	addVariable(m_subCategory);
	addVariable(m_details);
	addVariable(m_hiddenDetails);
	addVariable(m_harassingPlayerName);
	addVariable(m_language);
	addVariable(m_stationId);
	addVariable(m_isBug);
	unpack(source);
}

// ======================================================================

CreateTicketMessage::~CreateTicketMessage()
{
}
