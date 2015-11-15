// ======================================================================
//
// AcceptAuctionMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AcceptAuctionMessage.h"

// ======================================================================

AcceptAuctionMessage::AcceptAuctionMessage(NetworkId itemId) :
	GameNetworkMessage("AcceptAuctionMessage"),
	m_itemId(itemId)
{
	AutoByteStream::addVariable(m_itemId);
}

// ======================================================================

AcceptAuctionMessage::AcceptAuctionMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("AcceptAuctionMessage"),
	m_itemId()
{
	AutoByteStream::addVariable(m_itemId);
	unpack(source);
}

// ======================================================================

AcceptAuctionMessage::~AcceptAuctionMessage()
{
}
