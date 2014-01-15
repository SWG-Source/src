#ifndef GetMarketAuctionBidsMessage_H
#define GetMarketAuctionBidsMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class GetMarketAuctionBidsMessage : public GameNetworkMessage
{
public:
	struct MarketAuctionBid
	{
		NetworkId itemId;
		NetworkId bidderId;
		int       bid;
		int       maxProxyBid;
	};

	GetMarketAuctionBidsMessage();
	GetMarketAuctionBidsMessage(Archive::ReadIterator & source);
	~GetMarketAuctionBidsMessage();

	void addMarketAuctionBid(
		const NetworkId & itemId,
		const NetworkId & bidderId,
		int               bid,
		int               maxProxyBid
	);

	void clearAllMarketAuctionBids() {m_marketAuctionBids.get().clear();}

	const std::list<MarketAuctionBid> & getMarketAuctionBids() const {return m_marketAuctionBids.get();}
		
protected:

private:
	Archive::AutoList<MarketAuctionBid> m_marketAuctionBids;
};

namespace Archive
{
	inline void get(ReadIterator & source, GetMarketAuctionBidsMessage::MarketAuctionBid & target)
	{
		Archive::get(source, target.itemId);
		Archive::get(source, target.bidderId);
		Archive::get(source, target.bid);
		Archive::get(source, target.maxProxyBid);
	}

	inline void put(ByteStream & target, const GetMarketAuctionBidsMessage::MarketAuctionBid & source)
	{
		Archive::put(target, source.itemId);
		Archive::put(target, source.bidderId);
		Archive::put(target, source.bid);
		Archive::put(target, source.maxProxyBid);
	}
}

#endif
