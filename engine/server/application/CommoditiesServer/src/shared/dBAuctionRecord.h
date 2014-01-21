// ======================================================================
//
// dBAuctionRecord.h
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================


#ifndef INCLUDED_dBAuctionRecord_H
#define INCLUDED_dBAuctionRecord_H

// ======================================================================

#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Data structures for querying auctions.
 */
struct AuctionRecord
{
  NetworkId               m_creatorId;
  int                     m_minBid;
  int                     m_auctionTimer;
  int                     m_buyNowPrice;
  Unicode::String         m_userDescription;
  std::string             m_oob;
  NetworkId               m_locationId;
  NetworkId               m_itemId;
  int                     m_category;
  int                     m_itemTimer;
  Unicode::String         m_itemName;
  NetworkId               m_ownerId;
  int                     m_active;
};

// ======================================================================

#endif
