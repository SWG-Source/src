// ======================================================================
//
// CancelLiveAuctionMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CancelLiveAuctionMessage.h"

// ======================================================================

CancelLiveAuctionMessage::CancelLiveAuctionMessage(NetworkId itemId) :
	GameNetworkMessage("CancelLiveAuctionMessage"),
	m_itemId(itemId)
{
	AutoByteStream::addVariable(m_itemId);
}

// ======================================================================

CancelLiveAuctionMessage::CancelLiveAuctionMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CancelLiveAuctionMessage"),
	m_itemId()
{
	AutoByteStream::addVariable(m_itemId);
	unpack(source);
}

// ======================================================================

CancelLiveAuctionMessage::~CancelLiveAuctionMessage()
{
}
