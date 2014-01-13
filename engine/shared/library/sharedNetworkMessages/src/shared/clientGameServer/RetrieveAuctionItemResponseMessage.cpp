// ======================================================================
//
// RetrieveAuctionItemResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/RetrieveAuctionItemResponseMessage.h"

// ======================================================================

const char * const RetrieveAuctionItemResponseMessage::MessageType = "RetrieveAuctionItemResponseMessage";

//----------------------------------------------------------------------

RetrieveAuctionItemResponseMessage::RetrieveAuctionItemResponseMessage(NetworkId itemId, AuctionResult result) :
	GameNetworkMessage(MessageType),
	m_itemId(itemId),
	m_result(result)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
}

// ======================================================================

RetrieveAuctionItemResponseMessage::RetrieveAuctionItemResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(MessageType),
	m_itemId(),
	m_result()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
	unpack(source);
}

// ======================================================================

RetrieveAuctionItemResponseMessage::~RetrieveAuctionItemResponseMessage()
{
}
