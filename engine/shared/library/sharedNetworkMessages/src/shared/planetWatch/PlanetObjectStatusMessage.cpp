// ======================================================================
//
// PlanetObjectStatusMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PlanetObjectStatusMessage.h"

// ======================================================================

PlanetObjectStatusMessage::PlanetObjectStatusMessage(const std::vector<PlanetObjectStatusMessageData> &data) :
		GameNetworkMessage("PlanetObjectStatusMessage"),
		m_data()
{
	m_data.set(data);
	addVariable(m_data);
}

//-----------------------------------------------------------------------

PlanetObjectStatusMessage::PlanetObjectStatusMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("PlanetObjectStatusMessage"),
		m_data()
{
	addVariable(m_data);
	
	unpack(source);
}

// ----------------------------------------------------------------------

PlanetObjectStatusMessage::~PlanetObjectStatusMessage()
{
}

// ----------------------------------------------------------------------

void PlanetObjectStatusMessage::getData(std::vector<PlanetObjectStatusMessageData> &data)
{
	data = m_data.get();
}

// ----------------------------------------------------------------------

PlanetObjectStatusMessageData::PlanetObjectStatusMessageData(NetworkId const & objectId, int const x, int const z, uint32 const authoritativeServer, int const interestRadius, int const deleteObject, int const objectTypeTag, int const level, bool const hibernating, uint32 const templateCrc, int const aiActivity, int const creationType) :
		m_objectId(objectId), 
		m_x(x), m_z(z), 
		m_authoritativeServer(authoritativeServer), 
		m_interestRadius(interestRadius), 
		m_deleteObject(deleteObject),
		m_objectTypeTag(objectTypeTag),
		m_level(level),
		m_hibernating(hibernating),
		m_templateCrc(templateCrc),
		m_aiActivity(aiActivity),
		m_creationType(creationType)
{
}

// ----------------------------------------------------------------------

PlanetObjectStatusMessageData::PlanetObjectStatusMessageData()
{
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, PlanetObjectStatusMessageData &d)
	{
		get(source,d.m_objectId);
		get(source,d.m_x);
		get(source,d.m_z);
		get(source,d.m_authoritativeServer);
		get(source,d.m_interestRadius);
		get(source,d.m_deleteObject);
		get(source,d.m_objectTypeTag);
		get(source,d.m_level);
		get(source,d.m_hibernating);
		get(source,d.m_templateCrc);
		get(source,d.m_aiActivity);
		get(source,d.m_creationType);
	}

// ----------------------------------------------------------------------

	void put(ByteStream & target, const PlanetObjectStatusMessageData &d)
	{
		put(target,d.m_objectId);
		put(target,d.m_x);
		put(target,d.m_z);
		put(target,d.m_authoritativeServer);
		put(target,d.m_interestRadius);
		put(target,d.m_deleteObject);
		put(target,d.m_objectTypeTag);
		put(target,d.m_level);
		put(target,d.m_hibernating);
		put(target,d.m_templateCrc);
		put(target,d.m_aiActivity);
		put(target,d.m_creationType);
	}

} //namespace

// ======================================================================
