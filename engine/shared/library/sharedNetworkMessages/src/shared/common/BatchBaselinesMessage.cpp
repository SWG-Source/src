// ======================================================================
//
// BatchBaselinesMessage.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/BatchBaselinesMessage.h"

// ======================================================================

BatchBaselinesMessage::BatchBaselinesMessage(std::vector<BatchBaselinesMessageData> const & data) :
		GameNetworkMessage("BatchBaselinesMessage"),
		m_data()
{
	m_data.set(data);

	addVariable(m_data);
}

// ----------------------------------------------------------------------

BatchBaselinesMessage::BatchBaselinesMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("BatchBaselinesMessage"),
		m_data()
{
	addVariable(m_data);

	unpack(source);
}

// ----------------------------------------------------------------------

BatchBaselinesMessage::~BatchBaselinesMessage()
{
}

// ----------------------------------------------------------------------

std::vector<BatchBaselinesMessageData> const & BatchBaselinesMessage::getData() const
{
	return m_data.get();
}

// ======================================================================

BatchBaselinesMessageData::BatchBaselinesMessageData(NetworkId const & networkId, Tag objectType, char packageId, Archive::ByteStream const & package) :
		m_networkId(networkId),
		m_objectType(objectType),
		m_packageId(packageId),
		m_package(package)
{
}

// ----------------------------------------------------------------------

BatchBaselinesMessageData::BatchBaselinesMessageData() :
		m_networkId(NetworkId::cms_invalid),
		m_objectType(),
		m_packageId(0),
		m_package()
{
}

// ----------------------------------------------------------------------

void Archive::put(Archive::ByteStream & target, BatchBaselinesMessageData const & source)
{
	put(target,source.m_networkId);
	put(target,source.m_objectType);
	put(target,source.m_packageId);
	put(target,source.m_package);
}

// ----------------------------------------------------------------------

void Archive::get(Archive::ReadIterator & source, BatchBaselinesMessageData & target)
{
	get(source,target.m_networkId);
	get(source,target.m_objectType);
	get(source,target.m_packageId);
	ByteStream temp;
	get(source,temp);
	target.m_package=temp;
}

// ======================================================================
