// ======================================================================
//
// AuctionBid.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// (refactorof original code only to the degree to make it work with new)
// (server & database framework ... i.e. none of the logic has changed)
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// ======================================================================

#include "FirstCommodityServer.h"

#include "AuctionBid.h"

#include <algorithm>


// ======================================================================

AuctionBid::AuctionBid(
	const NetworkId & bidderId,
	int bid,
	int maxProxyBid
) :
m_bidderId(bidderId),
m_bid(bid),
m_maxProxyBid(maxProxyBid)
{
}

// ----------------------------------------------------------------------

AuctionBid::~AuctionBid()
{
}

// ----------------------------------------------------------------------

const bool AuctionBid::operator > (const AuctionBid & rhs) const
{
	return (std::max(m_bid, m_maxProxyBid) > std::max(rhs.m_bid, rhs.m_maxProxyBid));
}

// ----------------------------------------------------------------------

const bool AuctionBid::operator >= (const AuctionBid & rhs) const
{
	return (std::max(m_bid, m_maxProxyBid) >= std::max(rhs.m_bid, rhs.m_maxProxyBid));
}

// ----------------------------------------------------------------------

const bool AuctionBid::operator == (const AuctionBid & rhs) const
{
	return (std::max(m_bid, m_maxProxyBid) == std::max(rhs.m_bid, rhs.m_maxProxyBid));
}

// ----------------------------------------------------------------------

const bool AuctionBid::operator < (const AuctionBid & rhs) const
{
	return (std::max(m_bid, m_maxProxyBid) < std::max(rhs.m_bid, rhs.m_maxProxyBid));
}

// ----------------------------------------------------------------------

const bool AuctionBid::operator <= (const AuctionBid & rhs) const
{
	return (std::max(m_bid, m_maxProxyBid) <= std::max(rhs.m_bid, rhs.m_maxProxyBid));
}

// ======================================================================
