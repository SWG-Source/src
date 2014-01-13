// ======================================================================
//
// AuctionQueryMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionQueryMessage_H
#define	_AuctionQueryMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class AuctionQueryMessage : public GameNetworkMessage
{
public:

	AuctionQueryMessage(NetworkId containerId, int requestId, int type);
	explicit AuctionQueryMessage(Archive::ReadIterator &source);

	~AuctionQueryMessage();

public: // methods

	NetworkId                        getContainerId() const;
	int                              getRequestId() const;
	int                              getItemType() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_containerId;   //the container to query
	Archive::AutoVariable<int>       m_requestId;     //sequence number
	Archive::AutoVariable<int>       m_itemType;      //item category
};

// ----------------------------------------------------------------------

inline NetworkId AuctionQueryMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryMessage::getRequestId() const
{
	return m_requestId.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryMessage::getItemType() const
{
	return m_itemType.get();
}

// ----------------------------------------------------------------------

#endif // _AuctionQueryMessage_H

