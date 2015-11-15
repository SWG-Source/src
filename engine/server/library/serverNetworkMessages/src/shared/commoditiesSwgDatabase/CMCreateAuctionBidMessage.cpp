#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CMCreateAuctionBidMessage.h"

CMCreateAuctionBidMessage::CMCreateAuctionBidMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CMCreateAuctionBidMessage"),
	m_itemId(),
	m_bidderId(),
	m_bid(),
	m_maxProxyBid()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_bidderId);
	AutoByteStream::addVariable(m_bid);
	AutoByteStream::addVariable(m_maxProxyBid);
	unpack(source);
}

CMCreateAuctionBidMessage::CMCreateAuctionBidMessage(
	const NetworkId &    itemId,
	const NetworkId &    bidderId,
	int                  bid,
	int                  maxProxyBid
):
	GameNetworkMessage("CMCreateAuctionBidMessage"),
	m_itemId(itemId),
	m_bidderId(bidderId),
	m_bid(bid),
	m_maxProxyBid(maxProxyBid)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_bidderId);
	AutoByteStream::addVariable(m_bid);
	AutoByteStream::addVariable(m_maxProxyBid);
}

CMCreateAuctionBidMessage::~CMCreateAuctionBidMessage()
{
}
