// ======================================================================
//
// RetrieveAuctionItemMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/RetrieveAuctionItemMessage.h"

// ======================================================================

RetrieveAuctionItemMessage::RetrieveAuctionItemMessage(NetworkId itemId, NetworkId containerId) :
	GameNetworkMessage("RetrieveAuctionItemMessage"),
	m_itemId(itemId),
	m_containerId(containerId)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_containerId);
}

// ======================================================================

RetrieveAuctionItemMessage::RetrieveAuctionItemMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("RetrieveAuctionItemMessage"),
	m_itemId(),
	m_containerId()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_containerId);
	unpack(source);
}

// ======================================================================

RetrieveAuctionItemMessage::~RetrieveAuctionItemMessage()
{
}
