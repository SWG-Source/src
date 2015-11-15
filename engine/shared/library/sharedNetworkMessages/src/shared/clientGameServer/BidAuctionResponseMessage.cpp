// ======================================================================
//
// BidAuctionResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/BidAuctionResponseMessage.h"

// ======================================================================

const char * const BidAuctionResponseMessage::MessageType = "BidAuctionResponseMessage";

//----------------------------------------------------------------------

BidAuctionResponseMessage::BidAuctionResponseMessage(NetworkId itemId, AuctionResult result) :
	GameNetworkMessage(MessageType),
	m_itemId(itemId),
	m_result(result)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
}

// ======================================================================

BidAuctionResponseMessage::BidAuctionResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(MessageType),
	m_itemId(),
	m_result()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
	unpack(source);
}

// ======================================================================

BidAuctionResponseMessage::~BidAuctionResponseMessage()
{
}
