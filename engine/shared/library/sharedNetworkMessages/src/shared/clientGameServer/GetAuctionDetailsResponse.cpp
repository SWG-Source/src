// ======================================================================
//
// GetAuctionDetailsResponse.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetAuctionDetailsResponse.h"

// ======================================================================

const char * const GetAuctionDetailsResponse::MessageType = "GetAuctionDetailsResponse";

//----------------------------------------------------------------------

GetAuctionDetailsResponse::GetAuctionDetailsResponse(const Auction::ItemDataDetails & details) :
	GameNetworkMessage(MessageType),
	m_details(details)
{
	AutoByteStream::addVariable(m_details);
}

// ----------------------------------------------------------------------

GetAuctionDetailsResponse::GetAuctionDetailsResponse(Archive::ReadIterator & source) :
	GameNetworkMessage(MessageType),
	m_details()
{
	AutoByteStream::addVariable(m_details);
	unpack(source);
}

// ----------------------------------------------------------------------

GetAuctionDetailsResponse::~GetAuctionDetailsResponse()
{
}

//----------------------------------------------------------------------

namespace Archive
{

	void get(ReadIterator &source, Auction::ItemDataDetails &target)
	{
		get(source, target.itemId);
		get(source, target.userDescription);
		get(source, target.propertyList);
		get(source, target.templateName);
		get(source, target.appearanceString);
	}

	void put(ByteStream &target, Auction::ItemDataDetails const &source)
	{
		put(target, source.itemId);
		put(target, source.userDescription);
		put(target, source.propertyList);
		put(target, source.templateName);
		put(target, source.appearanceString);
	}

}
// ======================================================================

