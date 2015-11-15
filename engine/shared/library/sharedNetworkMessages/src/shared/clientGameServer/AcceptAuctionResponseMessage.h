// ======================================================================
//
// AcceptAuctionResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AcceptAuctionResponseMessage_H
#define	_AcceptAuctionResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class AcceptAuctionResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	AcceptAuctionResponseMessage(NetworkId itemId, AuctionResult result);
	explicit AcceptAuctionResponseMessage(Archive::ReadIterator &source);

	~AcceptAuctionResponseMessage();

public: // methods

	NetworkId                        getItemId() const;
	AuctionResult                    getResult() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
	Archive::AutoVariable<int>       m_result;        //the auction container
};

// ----------------------------------------------------------------------

inline NetworkId AcceptAuctionResponseMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline AuctionResult AcceptAuctionResponseMessage::getResult() const
{
	return static_cast<AuctionResult>(m_result.get());
}


// ----------------------------------------------------------------------

#endif // _AcceptAuctionResponseMessage_H

