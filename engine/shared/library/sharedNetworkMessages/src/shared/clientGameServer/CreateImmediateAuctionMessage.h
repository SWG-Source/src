// ======================================================================
//
// CreateImmediateAuctionMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CreateImmediateAuctionMessage_H
#define	_CreateImmediateAuctionMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class CreateImmediateAuctionMessage : public GameNetworkMessage
{
public:

	CreateImmediateAuctionMessage(const NetworkId & itemId, const Unicode::String & itemLocalizedName, const NetworkId & containerId, int price, int auctionLength, const Unicode::String & userDescription, bool premium = false, bool vendorTransfer = false );
	explicit CreateImmediateAuctionMessage(Archive::ReadIterator &source);

	~CreateImmediateAuctionMessage();

public: // methods

	const NetworkId &                getItemId() const;
	const Unicode::String &          getItemLocalizedName() const;
	const NetworkId &                getContainerId() const;
	int                              getPrice() const;
	int                              getAuctionLength() const;
	const Unicode::String &          getUserDescription() const;
	bool                             isPremium() const;
	bool                             isVendorTransfer() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId>             m_itemId;          //the item to be auctioned
	Archive::AutoVariable<Unicode::String>       m_itemLocalizedName;
	Archive::AutoVariable<NetworkId>             m_containerId;     //the auction container
	Archive::AutoVariable<int>                   m_price;           //the price of the item
	Archive::AutoVariable<int>                   m_auctionLength;   //in seconds
	Archive::AutoVariable<Unicode::String>       m_userDescription; //in seconds
	Archive::AutoVariable<bool>                  m_premium;
	Archive::AutoVariable<bool>                  m_vendorTransfer;
};

// ----------------------------------------------------------------------

inline const NetworkId & CreateImmediateAuctionMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & CreateImmediateAuctionMessage::getItemLocalizedName() const
{
	return m_itemLocalizedName.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & CreateImmediateAuctionMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline int CreateImmediateAuctionMessage::getPrice() const
{
	return m_price.get();
}

// ----------------------------------------------------------------------

inline int CreateImmediateAuctionMessage::getAuctionLength() const
{
	return m_auctionLength.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & CreateImmediateAuctionMessage::getUserDescription() const
{
	return m_userDescription.get();
}

// ----------------------------------------------------------------------

inline bool CreateImmediateAuctionMessage::isPremium() const
{
	return m_premium.get();
}

// ----------------------------------------------------------------------

inline bool CreateImmediateAuctionMessage::isVendorTransfer() const
{
	return m_vendorTransfer.get();
}

// ----------------------------------------------------------------------

#endif // _CreateImmediateAuctionMessage_H

