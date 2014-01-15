#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "AcceptHighBidMessage.h"

AcceptHighBidMessage::AcceptHighBidMessage(Archive::ReadIterator & source) :
GameNetworkMessage("AcceptHighBidMessage"),
m_responseId(),
m_trackId(),
m_auctionId(),
m_playerId()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_auctionId);
	AutoByteStream::addVariable(m_playerId);
	unpack(source);

}

AcceptHighBidMessage::AcceptHighBidMessage(
		int requestId,
		const NetworkId & auctionId,
		const NetworkId & playerId
) :
GameNetworkMessage("AcceptHighBidMessage"),
m_responseId(requestId),
m_trackId(0),
m_auctionId(auctionId),
m_playerId(playerId)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_auctionId);
	AutoByteStream::addVariable(m_playerId);

}

AcceptHighBidMessage::~AcceptHighBidMessage()
{
}


