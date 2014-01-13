// ======================================================================
//
// AcceptAuctionMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AcceptAuctionMessage_H
#define	_AcceptAuctionMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class AcceptAuctionMessage : public GameNetworkMessage
{
public:

	AcceptAuctionMessage(NetworkId itemId);
	explicit AcceptAuctionMessage(Archive::ReadIterator &source);

	~AcceptAuctionMessage();

public: // methods

	NetworkId                        getItemId() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
};

// ----------------------------------------------------------------------

inline NetworkId AcceptAuctionMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

#endif // _AcceptAuctionMessage_H

