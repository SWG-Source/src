// ======================================================================
//
// BidAuctionMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/BidAuctionMessage.h"

// ======================================================================

BidAuctionMessage::BidAuctionMessage(NetworkId itemId, int bid, int maxProxyBid) :
	GameNetworkMessage("BidAuctionMessage"),
	m_itemId(itemId),
	m_bid(bid),
	m_maxProxyBid(maxProxyBid)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_bid);
	AutoByteStream::addVariable(m_maxProxyBid);
}

// ======================================================================

BidAuctionMessage::BidAuctionMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("BidAuctionMessage"),
	m_itemId(),
	m_bid(),
	m_maxProxyBid()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_bid);
	AutoByteStream::addVariable(m_maxProxyBid);
	unpack(source);
}

// ======================================================================

BidAuctionMessage::~BidAuctionMessage()
{
}
