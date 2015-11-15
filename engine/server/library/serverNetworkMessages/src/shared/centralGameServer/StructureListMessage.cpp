// ======================================================================
//
// StructureListMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/StructureListMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

StructureListMessage::StructureListMessage(uint32 toolId, uint32 loginServerId, const NetworkId &characterId, const std::vector<StructureListMessageData> &data) :
		GameNetworkMessage("StructureListMessage"),
		m_data(),
		m_toolId(toolId),
		m_loginServerId(loginServerId),
		m_characterId(characterId)
{
	m_data.set(data);

	addVariable(m_data);
	addVariable(m_toolId);
	addVariable(m_loginServerId);
	addVariable(m_characterId);
}

//-----------------------------------------------------------------------

StructureListMessage::StructureListMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("StructureListMessage"),
		m_data(),
		m_toolId(),
		m_loginServerId(),
		m_characterId()
{
	addVariable(m_data);
	addVariable(m_toolId);
	addVariable(m_loginServerId);
	addVariable(m_characterId);
	
	unpack(source);
}

// ----------------------------------------------------------------------

StructureListMessage::~StructureListMessage()
{
}

// ----------------------------------------------------------------------

const std::vector<StructureListMessageData> &StructureListMessage::getData() const
{
	return m_data.get();
}		

// ----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, StructureListMessageData &c)
	{
		get(source,c.m_objectTemplate);
		get(source,c.m_structureId);
		get(source,c.m_location);
		get(source,c.m_coordinates);
		get(source,c.m_deleted);
	}

	void put(ByteStream & target, const StructureListMessageData &c)
	{
		put(target,c.m_objectTemplate);
		put(target,c.m_structureId);
		put(target,c.m_location);
		put(target,c.m_coordinates);
		put(target,c.m_deleted);
	}
}

// ----------------------------------------------------------------------

StructureListMessageData::StructureListMessageData(const std::string &objectTemplate, 
						   const NetworkId &structureId, 
						   const std::string &location, 
						   const Vector &coordinates, 
						   const int32 deleted) :
		m_objectTemplate(objectTemplate),
		m_structureId(structureId),
		m_location(location),
		m_coordinates(coordinates),
		m_deleted(deleted)
{
}

// ----------------------------------------------------------------------

StructureListMessageData::StructureListMessageData() :
		m_objectTemplate(),
		m_structureId(),
		m_location(),
		m_coordinates(),
		m_deleted()
{
}
	
// ======================================================================
