// ======================================================================
//
// CancelLiveAuctionResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CancelLiveAuctionResponseMessage_H
#define	_CancelLiveAuctionResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class CancelLiveAuctionResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	CancelLiveAuctionResponseMessage(NetworkId itemId, AuctionResult result, bool vendorRefusal = false);
	explicit CancelLiveAuctionResponseMessage(Archive::ReadIterator &source);

	~CancelLiveAuctionResponseMessage();

public: // methods

	NetworkId                        getItemId() const;
	AuctionResult                    getResult() const;
	bool                             isVendorRefusal() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
	Archive::AutoVariable<int>       m_result;        //the auction container
	Archive::AutoVariable<bool>      m_vendorRefusal; 
};

// ----------------------------------------------------------------------

inline NetworkId CancelLiveAuctionResponseMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline AuctionResult CancelLiveAuctionResponseMessage::getResult() const
{
	return static_cast<AuctionResult>(m_result.get());
}

// ----------------------------------------------------------------------

inline bool CancelLiveAuctionResponseMessage::isVendorRefusal() const
{
	return m_vendorRefusal.get();
}


// ----------------------------------------------------------------------

#endif // _CancelLiveAuctionResponseMessage_H

