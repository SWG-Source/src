// ======================================================================
//
// dBBidRecord.h
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef INCLUDED_dBBidRecord_H
#define INCLUDED_dBBidRecord_H

// ======================================================================

#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Data structures for querying bids.
 */
struct BidRecord
{
 	NetworkId                m_itemId;
 	NetworkId                m_bidderId;
 	int                      m_bid;
 	int                      m_maxProxyBid;
};

// ======================================================================

typedef std::vector<BidRecord> BidList;

// ======================================================================

#endif
