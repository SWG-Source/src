#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CancelAuctionMessage.h"

// namespace CommoditiesNamespace {

CancelAuctionMessage::CancelAuctionMessage(Archive::ReadIterator & source) :
GameNetworkMessage("CancelAuctionMessage"),
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

CancelAuctionMessage::CancelAuctionMessage(
		int requestId,
		const NetworkId & auctionId,
		const NetworkId & playerId
) :
GameNetworkMessage("CancelAuctionMessage"),
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

CancelAuctionMessage::~CancelAuctionMessage()
{
}

// }
