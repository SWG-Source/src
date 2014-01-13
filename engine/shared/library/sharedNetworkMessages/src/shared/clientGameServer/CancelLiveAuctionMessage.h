// ======================================================================
//
// CancelLiveAuctionMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CancelLiveAuctionMessage_H
#define	_CancelLiveAuctionMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class CancelLiveAuctionMessage : public GameNetworkMessage
{
public:

	CancelLiveAuctionMessage(NetworkId itemId);
	explicit CancelLiveAuctionMessage(Archive::ReadIterator &source);

	~CancelLiveAuctionMessage();

public: // methods

	NetworkId                        getItemId() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
};

// ----------------------------------------------------------------------

inline NetworkId CancelLiveAuctionMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

#endif // _CancelLiveAuctionMessage_H

