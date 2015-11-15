// ======================================================================
//
// CancelLiveAuctionResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CancelLiveAuctionResponseMessage.h"

// ======================================================================

const char * const CancelLiveAuctionResponseMessage::MessageType = "CancelLiveAuctionResponseMessage";

//----------------------------------------------------------------------

CancelLiveAuctionResponseMessage::CancelLiveAuctionResponseMessage(NetworkId itemId, AuctionResult result, bool vendorRefusal) :
	GameNetworkMessage(MessageType),
	m_itemId(itemId),
	m_result(result),
	m_vendorRefusal(vendorRefusal)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_vendorRefusal);
}

// ======================================================================

CancelLiveAuctionResponseMessage::CancelLiveAuctionResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(MessageType),
	m_itemId(),
	m_result(),
	m_vendorRefusal()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_vendorRefusal);
	unpack(source);
}

// ======================================================================

CancelLiveAuctionResponseMessage::~CancelLiveAuctionResponseMessage()
{
}
