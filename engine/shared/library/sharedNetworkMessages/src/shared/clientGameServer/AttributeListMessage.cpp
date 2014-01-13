// ======================================================================
//
// AttributeListMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AttributeListMessage.h"

// ======================================================================

const char * const AttributeListMessage::MessageType = "AttributeListMessage";

//----------------------------------------------------------------------

AttributeListMessage::AttributeListMessage(NetworkId const & networkId, AttributeVector const & data, int const revision) :
GameNetworkMessage(MessageType),
m_networkId(networkId),
m_staticItemName(),
m_data(),
m_revision(revision)
{
	m_data.set(data);

	addVariable(m_networkId);
	addVariable(m_staticItemName);
	addVariable(m_data);
	addVariable(m_revision);
}

//----------------------------------------------------------------------

AttributeListMessage::AttributeListMessage(std::string const & staticItemName, AttributeVector const & data, int const revision) :
GameNetworkMessage(MessageType),
m_networkId(),
m_staticItemName(staticItemName),
m_data(),
m_revision(revision)
{
	m_data.set(data);

	addVariable(m_networkId);
	addVariable(m_staticItemName);
	addVariable(m_data);
	addVariable(m_revision);
}

//----------------------------------------------------------------------

AttributeListMessage::AttributeListMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_networkId(),
m_staticItemName(),
m_data(),
m_revision()
{
	addVariable(m_networkId);
	addVariable(m_staticItemName);
	addVariable(m_data);
	addVariable(m_revision);

	unpack(source);
}

//----------------------------------------------------------------------

AttributeListMessage::~AttributeListMessage()
{
}

//----------------------------------------------------------------------
