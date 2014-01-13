// ======================================================================
//
// CreateAuctionMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateAuctionMessage.h"

// ======================================================================

CreateAuctionMessage::CreateAuctionMessage(const NetworkId & itemId, const Unicode::String & itemLocalizedName, const NetworkId & containerId, int minimumBid, int auctionLength, const Unicode::String & userDescription, bool premium) :
	GameNetworkMessage("CreateAuctionMessage"),
	m_itemId(itemId),
	m_itemLocalizedName(itemLocalizedName),
	m_containerId(containerId),
	m_minimumBid(minimumBid),
	m_auctionLength(auctionLength),
	m_userDescription(userDescription),
	m_premium(premium)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemLocalizedName);
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_minimumBid);
	AutoByteStream::addVariable(m_auctionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_premium);
}

// ======================================================================

CreateAuctionMessage::CreateAuctionMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CreateAuctionMessage"),
	m_itemId(),
	m_itemLocalizedName(),
	m_containerId(),
	m_minimumBid(),
	m_auctionLength(),
	m_userDescription(),
	m_premium()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemLocalizedName);
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_minimumBid);
	AutoByteStream::addVariable(m_auctionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_premium);
	unpack(source);
}

// ======================================================================

CreateAuctionMessage::~CreateAuctionMessage()
{
}
