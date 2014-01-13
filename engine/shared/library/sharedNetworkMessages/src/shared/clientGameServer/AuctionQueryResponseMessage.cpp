// ======================================================================
//
// AuctionQueryResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AuctionQueryResponseMessage.h"

// ======================================================================

AuctionQueryResponseMessage::AuctionQueryResponseMessage(int requestId, int typeFlag, AuctionDataVector const &auctionData) :
	GameNetworkMessage("AuctionQueryResponseMessage"),
	m_requestId(requestId),
	m_typeFlag(typeFlag),
	m_auctionData()
{
	m_auctionData.set(auctionData);
	AutoByteStream::addVariable(m_requestId);
	AutoByteStream::addVariable(m_typeFlag);
	AutoByteStream::addVariable(m_auctionData);
}

// ----------------------------------------------------------------------

AuctionQueryResponseMessage::AuctionQueryResponseMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("AuctionQueryResponseMessage"),
	m_requestId(),
	m_typeFlag(),
	m_auctionData()
{
	AutoByteStream::addVariable(m_requestId);
	AutoByteStream::addVariable(m_typeFlag);
	AutoByteStream::addVariable(m_auctionData);
	unpack(source);
}

// ----------------------------------------------------------------------

AuctionQueryResponseMessage::~AuctionQueryResponseMessage()
{
}

// ======================================================================

