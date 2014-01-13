// ======================================================================
//
// AcceptAuctionResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AcceptAuctionResponseMessage.h"

// ======================================================================

const char * const AcceptAuctionResponseMessage::MessageType = "AcceptAuctionResponseMessage";

//----------------------------------------------------------------------

AcceptAuctionResponseMessage::AcceptAuctionResponseMessage(NetworkId itemId, AuctionResult result) :
	GameNetworkMessage(MessageType),
	m_itemId(itemId),
	m_result(result)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
}

// ======================================================================

AcceptAuctionResponseMessage::AcceptAuctionResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(MessageType),
	m_itemId(),
	m_result()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
	unpack(source);
}

// ======================================================================

AcceptAuctionResponseMessage::~AcceptAuctionResponseMessage()
{
}
