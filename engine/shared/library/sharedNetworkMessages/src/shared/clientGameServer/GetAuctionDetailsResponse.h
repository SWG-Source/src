// ======================================================================
//
// GetAuctionDetailsResponse.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetAuctionDetailsResponse_H
#define	_GetAuctionDetailsResponse_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/AuctionData.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/AuctionData.h"

//-----------------------------------------------------------------------

class GetAuctionDetailsResponse : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	GetAuctionDetailsResponse(const Auction::ItemDataDetails & details);
	explicit GetAuctionDetailsResponse(Archive::ReadIterator &source);

	~GetAuctionDetailsResponse();

public: // methods

	Auction::ItemDataDetails    getDetails() const;
public: // types

private: 
	Archive::AutoVariable<Auction::ItemDataDetails>  m_details;
};

// ----------------------------------------------------------------------

inline Auction::ItemDataDetails GetAuctionDetailsResponse::getDetails() const
{
	return m_details.get();
}

// ----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator &source, Auction::ItemDataDetails &target);
	void put(ByteStream &target, Auction::ItemDataDetails const &source);
}

// ----------------------------------------------------------------------

#endif // _GetAuctionDetailsResponse_H

