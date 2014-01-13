// ======================================================================
//
// ClientOpenContainerMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClientOpenContainerMessage.h"

// ======================================================================

ClientOpenContainerMessage::ClientOpenContainerMessage(NetworkId containerId, std::string slot) :
	GameNetworkMessage("ClientOpenContainerMessage"),
	m_containerId(containerId),
	m_slot(slot)
{
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_slot);
}

// ======================================================================

ClientOpenContainerMessage::ClientOpenContainerMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("ClientOpenContainerMessage"),
	m_containerId(),
	m_slot()
{
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_slot);
	unpack(source);
}

// ======================================================================

ClientOpenContainerMessage::~ClientOpenContainerMessage()
{
}
