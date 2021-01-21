#ifndef AUCTIONBASE_H
#define AUCTIONBASE_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// These ***MUST*** be kept in sync in src/engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/AuctionData.h and src/engine/server/library/serverNetworkMessages/src/shared/gameCommoditiesServer/AuctionBase.h
#define AUCTION_ALWAYS_PRESENT    (1 << 5)
#define AUCTION_PREMIUM_AUCTION   (1 << 10)
#define AUCTION_ACTIVE            (1 << 11)
#define AUCTION_VENDOR_TRANSFER   (1 << 12)
#define AUCTION_OFFERED_ITEM      (1 << 14)

#include "AuctionData.h"

//#ifndef max
//	#define max(a, b) ((a > b) ? a : b)
//#endif
//
//#ifndef min
//	#define min(a, b) ((a < b) ? a : b)
//#endif

enum AuctionSearchType
{
	AST_ByCategory,
	AST_ByLocation,
	AST_ByAll,
	AST_ByPlayerSales,
	AST_ByPlayerBids,
	AST_ByPlayerStockroom,
	AST_ByVendorOffers,
	AST_ByVendorSelling,
	AST_ByVendorStockroom,
	AST_ByPlayerOffersToVendor
};

// DANGER!!!!
// If adding any values to this enumeration, add the values to the
// end of the list.  There are hard-coded values in script that
// rely upon these values not to change.
enum AuctionResultCode
{
	ARC_Success = 0,
	ARC_AuctionAlreadyExists,
	ARC_AuctionExpired,
	ARC_OwnerBidOnOwnItem,
	ARC_BidTooLow,
	ARC_AuctionDoesNotExist,
	ARC_NotItemOwner,
	ARC_NoBids,
	ARC_LocationAlreadyExists,
	ARC_AuctionAtVendorMarket,
	ARC_NotVendorOwner,
	ARC_SuccessPermanentAuction,
	ARC_TooManyAuctions,
	ARC_AuctionAlreadyCompleted,
	ARC_VendorOwnerCanceledCompletedAuction,
	ARC_BidTooHigh,
	ARC_BidOutbid,
	ARC_LocationVendorLimitExceeded,
	ARC_AuctionVendorLimitExceeded,
	ARC_AuctionVendorItemLimitExceeded,
	ARC_VendorStatusEmpty,    // vendor status must be consecutive and appear between ARC_VendorStatusEmpty and ARC_VendorRemoved
	ARC_VendorStatusNotEmpty,
	ARC_VendorUnaccessed,
	ARC_VendorEndangered,
	ARC_VendorRemoved,
	ARC_InvalidBid
};


#endif
