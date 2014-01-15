// ======================================================================
//
// CharacterNamesMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CharacterNamesMessage.h"

// ======================================================================

CharacterNamesMessage::CharacterNamesMessage(const std::vector<NetworkId> &ids, const std::vector<int> &stationIds, const std::vector<std::string> &characterNames, const std::vector<std::string> &characterFullNames, const std::vector<int> &createTimes, const std::vector<int> &loginTimes) :
		GameNetworkMessage("CharacterNamesMessage"),
		m_ids(),
		m_stationIds(),
		m_names(),
		m_fullNames(),
		m_createTimes(),
		m_loginTimes()
{
	m_ids.set(ids);
	m_stationIds.set(stationIds);
	m_names.set(characterNames);
	m_fullNames.set(characterFullNames);
	m_createTimes.set(createTimes);
	m_loginTimes.set(loginTimes);
	
	addVariable(m_ids);
	addVariable(m_stationIds);
	addVariable(m_names);
	addVariable(m_fullNames);
	addVariable(m_createTimes);
	addVariable(m_loginTimes);
}

// ----------------------------------------------------------------------

CharacterNamesMessage::CharacterNamesMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("CharacterNamesMessage"),
		m_ids(),
		m_stationIds(),
		m_names(),
		m_fullNames(),
		m_createTimes(),
		m_loginTimes()
{
	addVariable(m_ids);
	addVariable(m_stationIds);
	addVariable(m_names);
	addVariable(m_fullNames);
	addVariable(m_createTimes);
	addVariable(m_loginTimes);
	unpack(source);
}

// ----------------------------------------------------------------------

CharacterNamesMessage::~CharacterNamesMessage()
{
}

// ======================================================================
