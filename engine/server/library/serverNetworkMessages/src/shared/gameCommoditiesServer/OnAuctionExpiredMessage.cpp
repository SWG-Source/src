#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnAuctionExpiredMessage.h"

OnAuctionExpiredMessage::OnAuctionExpiredMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnAuctionExpiredMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_sold(),
m_buyerId(),
m_bidAmount(),
m_itemId(),
m_maxProxyBid(),
m_location(),
m_immediate(),
m_itemNameLength(),
m_itemName(),
m_sendSellerMail()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_sold);
	AutoByteStream::addVariable(m_buyerId);
	AutoByteStream::addVariable(m_bidAmount);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_maxProxyBid);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_immediate);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_sendSellerMail);
	unpack(source);
}

OnAuctionExpiredMessage::OnAuctionExpiredMessage(
		const NetworkId & ownerId,
		bool  sold,
		const NetworkId & buyerId,
		int bidAmount,
		const NetworkId & itemId,
		int maxProxyBid,
		const std::string & location,
		bool immediate,
		int itemNameLength,
		const Unicode::String & itemName,
		bool sendSellerMail
) :
GameNetworkMessage("OnAuctionExpiredMessage"),
m_responseId(0),
m_trackId(0),
m_ownerId(ownerId),
m_sold(sold),
m_buyerId(buyerId),
m_bidAmount(bidAmount),
m_itemId(itemId),
m_maxProxyBid(maxProxyBid),
m_location(location),
m_immediate(immediate),
m_itemNameLength(itemNameLength),
m_itemName(itemName),
m_sendSellerMail(sendSellerMail)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_sold);
	AutoByteStream::addVariable(m_buyerId);
	AutoByteStream::addVariable(m_bidAmount);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_maxProxyBid);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_immediate);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_sendSellerMail);
}

OnAuctionExpiredMessage::~OnAuctionExpiredMessage()
{
}
