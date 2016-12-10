// ======================================================================
//
// PlanetNodeStatusMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PlanetNodeStatusMessage.h"

#include <vector>

// ======================================================================

PlanetNodeStatusMessage::PlanetNodeStatusMessage(const std::vector<PlanetNodeStatusMessageData> &data) :
		GameNetworkMessage("PlanetNodeStatusMessage"),
		m_data()
{
	m_data.set(data);
	addVariable(m_data);
}

//-----------------------------------------------------------------------

PlanetNodeStatusMessage::PlanetNodeStatusMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("PlanetNodeStatusMessage"),
		m_data()
{
	addVariable(m_data);
	unpack(source);
}

// ----------------------------------------------------------------------

PlanetNodeStatusMessage::~PlanetNodeStatusMessage()
{
}

// ----------------------------------------------------------------------

void PlanetNodeStatusMessage::getData(std::vector<PlanetNodeStatusMessageData> &data)
{
	data=m_data.get();
}

// ======================================================================

PlanetNodeStatusMessageData::PlanetNodeStatusMessageData(int x, int z, bool loaded, const std::vector<uint32> &servers, const std::vector<int> &subscriptionCounts) :
		m_x(x),m_z(z),m_loaded(loaded),m_servers(servers),m_subscriptionCounts(subscriptionCounts)
{
}

// ----------------------------------------------------------------------

PlanetNodeStatusMessageData::PlanetNodeStatusMessageData()
{
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, PlanetNodeStatusMessageData &d)
	{
		get(source,d.m_x);
		get(source,d.m_z);
		get(source,d.m_loaded);

		AutoArray<uint32> temp;
		get(source,temp);
		d.m_servers=temp.get();

		AutoArray<int> temp2;
		get(source,temp2);
		d.m_subscriptionCounts=temp2.get();							
	}

// ----------------------------------------------------------------------

	void put(ByteStream & target, const PlanetNodeStatusMessageData &d)
	{
		put(target,d.m_x);
		put(target,d.m_z);
		put(target,d.m_loaded);

		AutoArray<uint32> temp;
		temp.set(d.m_servers);
		put(target,temp);

		AutoArray<int> temp2;
		temp2.set(d.m_subscriptionCounts);
		put(target,temp2);
	}
} //namespace

// ======================================================================
