#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetItemDetailsMessage.h"

GetItemDetailsMessage::GetItemDetailsMessage(Archive::ReadIterator & source) :
GameNetworkMessage("GetItemDetailsMessage"),
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

GetItemDetailsMessage::GetItemDetailsMessage(
		int requestId,
		const NetworkId & auctionId,
		const NetworkId & playerId
) :
GameNetworkMessage("GetItemDetailsMessage"),
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

GetItemDetailsMessage::~GetItemDetailsMessage()
{
}
