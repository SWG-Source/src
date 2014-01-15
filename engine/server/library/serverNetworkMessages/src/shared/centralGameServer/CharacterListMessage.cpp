// ======================================================================
//
// CharacterListMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CharacterListMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

CharacterListMessage::CharacterListMessage(uint32 accountNumber, const std::vector<CharacterListMessageData> &data) :
		GameNetworkMessage("CharacterListMessage"),
		m_data(),
		m_accountNumber(accountNumber)
{
	m_data.set(data);

	addVariable(m_data);
	addVariable(m_accountNumber);
}

//-----------------------------------------------------------------------

CharacterListMessage::CharacterListMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("CharacterListMessage"),
		m_data(),
		m_accountNumber()
{
	addVariable(m_data);
	addVariable(m_accountNumber);
	
	unpack(source);
}

// ----------------------------------------------------------------------

CharacterListMessage::~CharacterListMessage()
{
}

// ----------------------------------------------------------------------

const std::vector<CharacterListMessageData> &CharacterListMessage::getData() const
{
	return m_data.get();
}		

// ----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, CharacterListMessageData &c)
	{
		get(source,c.m_name);
		get(source,c.m_objectTemplate);
		get(source,c.m_characterId);
		get(source,c.m_containerId);
		get(source,c.m_location);
		get(source,c.m_coordinates);
	}

	void put(ByteStream & target, const CharacterListMessageData &c)
	{
		put(target,c.m_name);
		put(target,c.m_objectTemplate);
		put(target,c.m_characterId);
		put(target,c.m_containerId);
		put(target,c.m_location);
		put(target,c.m_coordinates);
	}
}

// ----------------------------------------------------------------------

CharacterListMessageData::CharacterListMessageData(const Unicode::String &name, const std::string &objectTemplate, const NetworkId &characterId, const NetworkId &containerId, const std::string &location, const Vector &coordinates) :
		m_name(name),
		m_objectTemplate(objectTemplate),
		m_characterId(characterId),
		m_containerId(containerId),
		m_location(location),
		m_coordinates(coordinates)
{
}

// ----------------------------------------------------------------------

CharacterListMessageData::CharacterListMessageData() :
		m_name(),
		m_objectTemplate(),
		m_characterId(),
		m_containerId(),
		m_location(),
		m_coordinates()
{
}
	
// ======================================================================
