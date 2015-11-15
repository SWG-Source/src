// ======================================================================
//
// BidAuctionResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_BidAuctionResponseMessage_H
#define	_BidAuctionResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class BidAuctionResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	BidAuctionResponseMessage(NetworkId itemId, AuctionResult result);
	explicit BidAuctionResponseMessage(Archive::ReadIterator &source);

	~BidAuctionResponseMessage();

public: // methods

	NetworkId                        getItemId() const;
	AuctionResult                    getResult() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
	Archive::AutoVariable<int>       m_result;        //the auction container
};

// ----------------------------------------------------------------------

inline NetworkId BidAuctionResponseMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline AuctionResult BidAuctionResponseMessage::getResult() const
{
	return static_cast<AuctionResult>(m_result.get());
}


// ----------------------------------------------------------------------

#endif // _BidAuctionResponseMessage_H

