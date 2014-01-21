// ======================================================================
//
// AuctionBid.h
// copyright (c) 2004 Sony Online Entertainment
//
// (refactorof original code only to the degree to make it work with new)
// (server & database framework ... i.e. none of the logic has changed)
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// ======================================================================

#ifndef AuctionBid_h
#define AuctionBid_h

#include <string>
#include "sharedFoundation/NetworkId.h"
#include "serverNetworkMessages/AuctionBase.h"

// ======================================================================

class AuctionBid
{
private:
	AuctionBid();
	AuctionBid(const AuctionBid&);
	AuctionBid& operator= (const AuctionBid&);

protected:
	const NetworkId        m_bidderId;
	int                    m_bid;
	const int              m_maxProxyBid;

public:
	AuctionBid(const NetworkId & bidderId, int bid, int maxProxyBid);
	~AuctionBid();

	const NetworkId &    GetBidderId() const {return m_bidderId;}
	int                  GetBid() const {return m_bid;}
	int                  GetMaxProxyBid() const {return m_maxProxyBid;}

	void                 SetBid(int bid) {m_bid = bid;}
	const bool  operator>(const AuctionBid &rhs) const;
	const bool  operator>=(const AuctionBid &rhs) const;
	const bool  operator==(const AuctionBid &rhs) const;
	const bool  operator<(const AuctionBid &rhs) const;
	const bool  operator<=(const AuctionBid &rhs) const;
};

#endif

// ======================================================================
