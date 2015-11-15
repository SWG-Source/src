// ======================================================================
//
// CreateAuctionMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CreateAuctionMessage_H
#define	_CreateAuctionMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class CreateAuctionMessage : public GameNetworkMessage
{
public:

	CreateAuctionMessage (const NetworkId & itemId, const Unicode::String & itemLocalizedName, const NetworkId & containerId, int minimumBid, int auctionLength, const Unicode::String & userDescription, bool premium = false );
	explicit CreateAuctionMessage (Archive::ReadIterator &source);

	~CreateAuctionMessage();

public: // methods

	const NetworkId &                getItemId          () const;
	const Unicode::String &          getItemLocalizedName() const;
	const NetworkId &                getContainerId     () const;
	int                              getMinimumBid      () const;
	int                              getAuctionLength   () const;
	const Unicode::String &          getUserDescription () const;
	bool                             isPremium          () const;

public: // types

private: 
	Archive::AutoVariable<NetworkId>             m_itemId;          //the item to be auctioned
	Archive::AutoVariable<Unicode::String>       m_itemLocalizedName;
	Archive::AutoVariable<NetworkId>             m_containerId;     //the auction container
	Archive::AutoVariable<int>                   m_minimumBid;      //the minimum acceptable bid
	Archive::AutoVariable<int>                   m_auctionLength;   //in seconds
	Archive::AutoVariable<Unicode::String>       m_userDescription; //in seconds
	Archive::AutoVariable<bool>                  m_premium; 
};

// ----------------------------------------------------------------------

inline const NetworkId & CreateAuctionMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & CreateAuctionMessage::getItemLocalizedName() const
{
	return m_itemLocalizedName.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & CreateAuctionMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline int CreateAuctionMessage::getMinimumBid() const
{
	return m_minimumBid.get();
}

// ----------------------------------------------------------------------

inline int CreateAuctionMessage::getAuctionLength() const
{
	return m_auctionLength.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & CreateAuctionMessage::getUserDescription() const
{
	return m_userDescription.get();
}

// ----------------------------------------------------------------------

inline bool CreateAuctionMessage::isPremium() const
{
	return m_premium.get();
}

// ----------------------------------------------------------------------

#endif // _CreateAuctionMessage_H

