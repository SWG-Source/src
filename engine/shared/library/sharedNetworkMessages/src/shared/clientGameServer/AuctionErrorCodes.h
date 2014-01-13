// ======================================================================
//
// AuctionErrorCodes.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionErrorCodes_H
#define	_AuctionErrorCodes_H

//-----------------------------------------------------------------------

enum AuctionResult
{
	ar_OK,
	ar_INVALID_AUCTIONER,
	ar_INVALID_ITEM_ID,
	ar_INVALID_CONTAINER_ID,
	ar_INVALID_MINIMUM_BID,
	ar_INVALID_AUCTION_LENGTH,
	ar_ITEM_ALREADY_AUCTIONED,
	ar_ITEM_NOT_IN_CONTAINER,
	ar_NOT_ITEM_OWNER,
	ar_NOT_ENOUGH_MONEY,
	ar_INVALID_BID,
	ar_BID_REJECTED,
	ar_INVENTORY_FULL,
	ar_TOO_MANY_AUCTIONS,
	ar_BID_TOO_HIGH,
	ar_AUCTION_ALREADY_COMPLETED,
	ar_VENDOR_DEACTIVATED,
	ar_ITEM_NOLONGER_EXISTS,
	ar_INVALID_ITEM_REIMBURSAL,
	ar_IN_TRADE,
	ar_IN_CRATE,
	ar_NOT_ALLOWED,
	ar_NOT_EMPTY,
	ar_BID_OUTBID,
	ar_TOO_MANY_VENDORS,
	ar_TOO_MANY_VENDOR_ITEMS,
	ar_IS_BIOLINKED,
	ar_ITEM_EQUIPPED,
	ar_ITEM_RESTRICTED,
	ar_PRICE_TOO_HIGH
};


// ----------------------------------------------------------------------

#endif // _AuctionErrorCodes_H

