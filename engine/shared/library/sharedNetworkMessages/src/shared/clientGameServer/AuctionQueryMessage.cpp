// ======================================================================
//
// AuctionQueryMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AuctionQueryMessage.h"

// ======================================================================

AuctionQueryMessage::AuctionQueryMessage(NetworkId containerId, int requestId, int itemType):
	GameNetworkMessage("AuctionQueryMessage"),
	m_containerId(containerId),
	m_requestId(requestId),
	m_itemType(itemType)
{
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_requestId);
	AutoByteStream::addVariable(m_itemType);
}

// ======================================================================

AuctionQueryMessage::AuctionQueryMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("AuctionQueryMessage"),
	m_containerId(),
	m_requestId(),
	m_itemType()
{
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_requestId);
	AutoByteStream::addVariable(m_itemType);

	unpack(source);
}

// ======================================================================

AuctionQueryMessage::~AuctionQueryMessage()
{
}
