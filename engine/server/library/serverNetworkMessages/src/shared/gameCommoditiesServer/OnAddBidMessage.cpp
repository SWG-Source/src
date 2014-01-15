#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnAddBidMessage.h"

OnAddBidMessage::OnAddBidMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnAddBidMessage"),
m_responseId(),
m_trackId(),
m_itemId(),
m_ownerId(),
m_bidderId(),
m_previousBidderId(),
m_bidAmount(),
m_previousBidAmount(),
m_maxProxyBid(),
m_location(),
m_resultCode(),
m_itemNameLength(),
m_itemName(),
m_salesTaxAmount(),
m_salesTaxBankId()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_bidderId);
	AutoByteStream::addVariable(m_previousBidderId);
	AutoByteStream::addVariable(m_bidAmount);
	AutoByteStream::addVariable(m_previousBidAmount);
	AutoByteStream::addVariable(m_maxProxyBid);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_resultCode);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_salesTaxAmount);
	AutoByteStream::addVariable(m_salesTaxBankId);
	unpack(source);
}

OnAddBidMessage::OnAddBidMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & ownerId,
		const NetworkId & bidderId,
		const NetworkId & previousBidderId,
		int bidAmount,
		int previousBidAmount,
		int maxProxyBid,
		const std::string &location,
		int itemNameLength,
		const Unicode::String &itemName,
		int salesTaxAmount,
		const NetworkId & salesTaxBankId
) :
GameNetworkMessage("OnAddBidMessage"),
m_responseId(responseId),
m_trackId(0),
m_itemId(itemId),
m_ownerId(ownerId),
m_bidderId(bidderId),
m_previousBidderId(previousBidderId),
m_bidAmount(bidAmount),
m_previousBidAmount(previousBidAmount),
m_maxProxyBid(maxProxyBid),
m_location(location),
m_resultCode(resultCode),
m_itemNameLength(itemNameLength),
m_itemName(itemName),
m_salesTaxAmount(salesTaxAmount),
m_salesTaxBankId(salesTaxBankId)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_bidderId);
	AutoByteStream::addVariable(m_previousBidderId);
	AutoByteStream::addVariable(m_bidAmount);
	AutoByteStream::addVariable(m_previousBidAmount);
	AutoByteStream::addVariable(m_maxProxyBid);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_resultCode);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_salesTaxAmount);
	AutoByteStream::addVariable(m_salesTaxBankId);
}


OnAddBidMessage::~OnAddBidMessage()
{
}
