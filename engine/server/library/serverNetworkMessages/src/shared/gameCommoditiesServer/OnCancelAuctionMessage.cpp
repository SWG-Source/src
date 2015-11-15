#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnCancelAuctionMessage.h"

OnCancelAuctionMessage::OnCancelAuctionMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnCancelAuctionMessage"),
m_responseId(),
m_trackId(),
m_itemId(),
m_playerId(),
m_highBidderId(),
m_highBidAmount(),
m_location(),
m_resultCode()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_highBidderId);
	AutoByteStream::addVariable(m_highBidAmount);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_resultCode);
	unpack(source);
}

OnCancelAuctionMessage::OnCancelAuctionMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & playerId,
		const NetworkId & highBidderId,
		int highBidAmount,
		const std::string & location
) :
GameNetworkMessage("OnCancelAuctionMessage"),
m_responseId(responseId),
m_trackId(0),
m_itemId(itemId),
m_playerId(playerId),
m_highBidderId(highBidderId),
m_highBidAmount(highBidAmount),
m_location(location),
m_resultCode(resultCode)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_highBidderId);
	AutoByteStream::addVariable(m_highBidAmount);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_resultCode);
}

OnCancelAuctionMessage::~OnCancelAuctionMessage()
{
}

