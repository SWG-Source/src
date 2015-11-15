#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetMarketAuctionBidsMessage.h"

GetMarketAuctionBidsMessage::GetMarketAuctionBidsMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("GetMarketAuctionBidsMessage"),
	m_marketAuctionBids()
{
	AutoByteStream::addVariable(m_marketAuctionBids);
	unpack(source);
}

GetMarketAuctionBidsMessage::GetMarketAuctionBidsMessage() :
	GameNetworkMessage("GetMarketAuctionBidsMessage"),
	m_marketAuctionBids()
{
	AutoByteStream::addVariable(m_marketAuctionBids);
}

GetMarketAuctionBidsMessage::~GetMarketAuctionBidsMessage()
{
}

void GetMarketAuctionBidsMessage::addMarketAuctionBid(
	const NetworkId & itemId,
	const NetworkId & bidderId,
	int               bid,
	int               maxProxyBid
)
{
	GetMarketAuctionBidsMessage::MarketAuctionBid marketAuctionBid;
	marketAuctionBid.itemId      = itemId;
	marketAuctionBid.bidderId    = bidderId;
	marketAuctionBid.bid         = bid;
	marketAuctionBid.maxProxyBid = maxProxyBid;

	m_marketAuctionBids.get().push_back(marketAuctionBid);
}
