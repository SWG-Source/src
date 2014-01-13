// ======================================================================
//
// CreateImmediateAuctionMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateImmediateAuctionMessage.h"

// ======================================================================

CreateImmediateAuctionMessage::CreateImmediateAuctionMessage(const NetworkId & itemId, const Unicode::String & itemLocalizedName, const NetworkId & containerId, int price, int auctionLength, const Unicode::String & userDescription, bool premium, bool vendorTransfer) :
	GameNetworkMessage("CreateImmediateAuctionMessage"),
	m_itemId(itemId),
	m_itemLocalizedName(itemLocalizedName),
	m_containerId(containerId),
	m_price(price),
	m_auctionLength(auctionLength),
	m_userDescription(userDescription),
	m_premium(premium),
	m_vendorTransfer(vendorTransfer)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemLocalizedName);
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_price);
	AutoByteStream::addVariable(m_auctionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_premium);
	AutoByteStream::addVariable(m_vendorTransfer);
}

// ======================================================================

CreateImmediateAuctionMessage::CreateImmediateAuctionMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CreateImmediateAuctionMessage"),
	m_itemId(),
	m_itemLocalizedName(),
	m_containerId(),
	m_price(),
	m_auctionLength(),
	m_userDescription(),
	m_premium(),
	m_vendorTransfer()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemLocalizedName);
	AutoByteStream::addVariable(m_containerId);
	AutoByteStream::addVariable(m_price);
	AutoByteStream::addVariable(m_auctionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_premium);
	AutoByteStream::addVariable(m_vendorTransfer);
	unpack(source);
}

// ======================================================================

CreateImmediateAuctionMessage::~CreateImmediateAuctionMessage()
{
}
