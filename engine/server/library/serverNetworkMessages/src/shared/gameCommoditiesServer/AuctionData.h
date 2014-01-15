// AuctionData.h
#ifndef AUCTION_DATA_H
#define AUCTION_DATA_H

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

/// length of static memory used for storing location data
const int LOCATION_LENGTH  = 256;
const int NAME_LENGTH  = 64;
const int DESCRIPTION_LENGTH = 512;

// ----------------------------------------------------------------------

struct AuctionDataHeader {
    AuctionDataHeader();

    /// copy constructor
    AuctionDataHeader(const AuctionDataHeader & original);
	virtual ~AuctionDataHeader();

	int             type;                      /// was char
	NetworkId       auctionId;                 ///< identifies the auction in both client and server
	NetworkId       itemId;                    ///< identifies the item for sale
	int             itemNameLength;
	Unicode::String itemName;
	int             minBid;                    ///< the minimum bid allowed for this auction
	int             highBid;                   ///< if non-zero, the highest current valid bid
	int             timer;                     ///< timeout for auction expiration
	int             buyNowPrice;
	std::string     location;                  ///< which auction house/spaceport is this item stored
	NetworkId       ownerId;                   ///< the identifier for the player that created this auction
	NetworkId       highBidderId;              ///< identifies the player with the current high bid
	int             maxProxyBid;               ///< if non-zero, the highest bid the current bidder will place
	int             myBid;
	int             itemType;
	int             resourceContainerClassCrc;
	int             flags;
	int             entranceCharge;
};

// ----------------------------------------------------------------------

#endif //AUCTION_DATA_H
