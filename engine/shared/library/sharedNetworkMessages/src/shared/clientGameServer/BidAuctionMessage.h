// ======================================================================
//
// BidAuctionMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_BidAuctionMessage_H
#define	_BidAuctionMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class BidAuctionMessage : public GameNetworkMessage
{
public:

	BidAuctionMessage(NetworkId itemId, int bid, int maxProxyBid );
	explicit BidAuctionMessage(Archive::ReadIterator &source);

	~BidAuctionMessage();

public: // methods

	NetworkId                        getItemId() const;
	int                              getBid() const;
	int                              getMaxProxyBid() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
	Archive::AutoVariable<int>       m_bid;           //the player bid
	Archive::AutoVariable<int>       m_maxProxyBid;   //the maximum proxy bid
};

// ----------------------------------------------------------------------

inline NetworkId BidAuctionMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline int BidAuctionMessage::getBid() const
{
	return m_bid.get();
}

// ----------------------------------------------------------------------

inline int BidAuctionMessage::getMaxProxyBid() const
{
	return m_maxProxyBid.get();
}

// ----------------------------------------------------------------------

#endif // _BidAuctionMessage_H

