// ======================================================================
//
// ResourceTypeMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AddResourceTypeMessage.h"

// ======================================================================

AddResourceTypeMessage::AddResourceTypeMessage(std::vector<AddResourceTypeMessageNamespace::ResourceTypeData> const &data) :
	GameNetworkMessage("AddResourceTypeMessage"),
	m_data()
{
	m_data.set(data);
	addVariable(m_data);
}

//-----------------------------------------------------------------------

AddResourceTypeMessage::AddResourceTypeMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("AddResourceTypeMessage"),
	m_data()
{
	addVariable(m_data);
	unpack(source); //lint !e1506 // virtual in constructor
}

//-----------------------------------------------------------------------

AddResourceTypeMessage::~AddResourceTypeMessage()
{
}

// ----------------------------------------------------------------------

std::vector<AddResourceTypeMessageNamespace::ResourceTypeData> const & AddResourceTypeMessage::getData() const
{
	return m_data.get();
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, AddResourceTypeMessageNamespace::ResourceTypeData & target)
	{
		get(source, target.m_networkId);
		get(source, target.m_name);
		get(source, target.m_depletedTimestamp);
		get(source, target.m_parentClass);
		get(source, target.m_attributes);
		get(source, target.m_fractalSeeds);
	}

	// ----------------------------------------------------------------------
	
	void put(ByteStream & target, const AddResourceTypeMessageNamespace::ResourceTypeData & source)
	{
		put(target, source.m_networkId);
		put(target, source.m_name);
		put(target, source.m_depletedTimestamp);
		put(target, source.m_parentClass);
		put(target, source.m_attributes);
		put(target, source.m_fractalSeeds);
	}
}

// ======================================================================
