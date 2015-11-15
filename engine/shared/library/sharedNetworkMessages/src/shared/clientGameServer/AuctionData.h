// ======================================================================
//
// AuctionData.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionData_H
#define	_AuctionData_H

#include <string>
#include <Unicode.h>
#include <vector>

#include "sharedFoundation/NetworkId.h"

// These ***MUST*** be kept in sync in src/engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/AuctionData.h and src/engine/server/library/serverNetworkMessages/src/shared/gameCommoditiesServer/AuctionBase.h
#ifndef AUCTION_PREMIUM_AUCTION
#define AUCTION_PREMIUM_AUCTION (1 << 10)
#endif

#ifndef AUCTION_ACTIVE
#define AUCTION_ACTIVE (1 << 11)
#endif

#ifndef AUCTION_VENDOR_TRANSFER
#define AUCTION_VENDOR_TRANSFER (1 << 12)
#endif

#ifndef AUCTION_MAGIC_ITEM
#define AUCTION_MAGIC_ITEM (1 << 13)
#endif

#ifndef AUCTION_OFFERED_ITEM
#define AUCTION_OFFERED_ITEM (1 << 14)
#endif

//-----------------------------------------------------------------------

namespace Auction 
{
	//----------------------------------------------------------------------

	struct PalettizedItemDataHeader
	{
		NetworkId                     itemId;
		uint8                         itemNameKey;
		int32                         highBid;
		int32                         timer;
		int32                         buyNowPrice;
		uint16                        locationKey;
		NetworkId                     ownerId;
		uint16                        ownerNameKey;
		NetworkId                     highBidderId;
		uint16                        highBidderNameKey;
		int32                         maxProxyBid;
		int32                         myBid;
		long                          itemType;
		int                           resourceContainerClassCrc;
		int32                         flags;
		int32                         entranceCharge;

		PalettizedItemDataHeader () :
		itemId         (),
		itemNameKey    (0),
		highBid        (0),
		timer          (0),
		buyNowPrice    (0),
		locationKey    (0),
		ownerId        (),
		ownerNameKey   (0),
		highBidderId   (),
		highBidderNameKey (0),
		maxProxyBid    (0),
		myBid          (0),
		itemType       (0),
		resourceContainerClassCrc(0),
		flags          (0),
		entranceCharge (0)
		{
		}
	};

	struct ItemDataHeader
	{
		NetworkId                     itemId;
		Unicode::String               itemName;
		int32                         minBid;
		int32                         highBid;
		int32                         timer;
		int32                         buyNowPrice;
		std::string                   location;
		NetworkId                     ownerId;
		std::string                   ownerName;
		NetworkId                     highBidderId;
		std::string                   highBidderName;
		int32                         maxProxyBid;
		int32                         myBid;
		long                          itemType;
		int                           resourceContainerClassCrc;
		int32                         flags;
		int32                         entranceCharge;

		ItemDataHeader () :
		itemId         (),
		itemName       (),
		minBid         (0),
		highBid        (0),
		timer          (0),
		buyNowPrice    (0),
		location       (),
		ownerId        (),
		ownerName      (),
		highBidderId   (),
		highBidderName (),
		maxProxyBid    (0),
		myBid          (0),
		itemType       (0),
		resourceContainerClassCrc(0),
		flags          (0),
		entranceCharge (0)
		{
		}

		bool isPremium   () const;
		bool isActive    () const;
		bool isMagicItem () const;
	};
	
	//----------------------------------------------------------------------

	struct ItemDataDetails
	{
		NetworkId                                             itemId;
		Unicode::String                                       userDescription;
		std::vector<std::pair<std::string, Unicode::String> > propertyList;
		std::string                                           templateName;
		std::string                                           appearanceString;
		
		ItemDataDetails () :
			itemId           (),
			userDescription  (),
			propertyList     (),
			templateName     (),
			appearanceString ()
		{
		}
	};

	//----------------------------------------------------------------------

	inline bool ItemDataHeader::isPremium () const
	{
		return (flags & AUCTION_PREMIUM_AUCTION) != 0;
	}

	//----------------------------------------------------------------------

	inline bool ItemDataHeader::isActive  () const
	{
		return (flags & AUCTION_ACTIVE) != 0;
	}

	//----------------------------------------------------------------------

	inline bool ItemDataHeader::isMagicItem  () const
	{
		return (flags & AUCTION_MAGIC_ITEM) != 0;
	}

	//----------------------------------------------------------------------
}

//----------------------------------------------------------------------


#endif // _AuctionData_H

