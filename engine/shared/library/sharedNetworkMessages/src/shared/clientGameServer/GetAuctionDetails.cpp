// ======================================================================
//
// GetAuctionDetails.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetAuctionDetails.h"

// ======================================================================

GetAuctionDetails::GetAuctionDetails(const NetworkId & itemId):
	GameNetworkMessage("GetAuctionDetails"),
	m_itemId(itemId)
{
	AutoByteStream::addVariable(m_itemId);
}

// ======================================================================

GetAuctionDetails::GetAuctionDetails(Archive::ReadIterator &source) :
	GameNetworkMessage("GetAuctionDetails"),
	m_itemId()
{
	AutoByteStream::addVariable(m_itemId);

	unpack(source);
}

// ======================================================================

GetAuctionDetails::~GetAuctionDetails()
{
}
