#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "AddBidMessage.h"

AddBidMessage::AddBidMessage(Archive::ReadIterator & source) :
GameNetworkMessage("AddBidMessage"),
m_responseId(),
m_trackId(),
m_auctionId(),
m_playerId(),
m_bid(),
m_maxProxyBid(),
m_playerName()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
  AutoByteStream::addVariable(m_auctionId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_bid);
	AutoByteStream::addVariable(m_maxProxyBid);
	AutoByteStream::addVariable(m_playerName);
	unpack(source);
}

AddBidMessage::AddBidMessage(
		int requestId,
		const NetworkId & auctionId,
		const NetworkId & playerId,
		int bid,
		int maxProxyBid,
		const std::string & playerName
) :
GameNetworkMessage("AddBidMessage"),
m_responseId(requestId),
m_trackId(0),
m_auctionId(auctionId),
m_playerId(playerId),
m_bid(bid),
m_maxProxyBid(maxProxyBid),
m_playerName(playerName)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
  AutoByteStream::addVariable(m_auctionId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_bid);
	AutoByteStream::addVariable(m_maxProxyBid);
	AutoByteStream::addVariable(m_playerName);
}

AddBidMessage::~AddBidMessage()
{
}

