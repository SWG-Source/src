// ======================================================================
//
// RetrieveAuctionItemMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_RetrieveAuctionItemMessage_H
#define	_RetrieveAuctionItemMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class RetrieveAuctionItemMessage : public GameNetworkMessage
{
public:

	RetrieveAuctionItemMessage(NetworkId itemId, NetworkId containerId);
	explicit RetrieveAuctionItemMessage(Archive::ReadIterator &source);

	~RetrieveAuctionItemMessage();

public: // methods

	NetworkId                        getItemId() const;
	NetworkId                        getContainerId() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
	Archive::AutoVariable<NetworkId> m_containerId;   //the auction container
};

// ----------------------------------------------------------------------

inline NetworkId RetrieveAuctionItemMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline NetworkId RetrieveAuctionItemMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

#endif // _RetrieveAuctionItemMessage_H

