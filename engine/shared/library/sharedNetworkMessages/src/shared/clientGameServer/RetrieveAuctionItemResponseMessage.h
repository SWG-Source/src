// ======================================================================
//
// RetrieveAuctionItemResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_RetrieveAuctionItemResponseMessage_H
#define	_RetrieveAuctionItemResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class RetrieveAuctionItemResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	RetrieveAuctionItemResponseMessage(NetworkId itemId, AuctionResult result);
	explicit RetrieveAuctionItemResponseMessage(Archive::ReadIterator &source);

	~RetrieveAuctionItemResponseMessage();

public: // methods

	NetworkId                        getItemId() const;
	AuctionResult                    getResult() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
	Archive::AutoVariable<int>       m_result;        //the auction container
};

// ----------------------------------------------------------------------

inline NetworkId RetrieveAuctionItemResponseMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline AuctionResult RetrieveAuctionItemResponseMessage::getResult() const
{
	return static_cast<AuctionResult>(m_result.get());
}


// ----------------------------------------------------------------------

#endif // _RetrieveAuctionItemResponseMessage_H

