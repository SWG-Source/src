// ======================================================================
//
// AuctionInfo.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _AuctionInfo_H_
#define _AuctionInfo_H_

// ======================================================================

#include "sharedFoundation/NetworkId.h"

// ======================================================================

struct AuctionInfo
{
	NetworkId   ownerId;
	NetworkId   itemId;
	int         minBid;
	int         timer;
	NetworkId   auctionContainerId;
};

// ======================================================================

#endif // _AuctionInfo_H_

