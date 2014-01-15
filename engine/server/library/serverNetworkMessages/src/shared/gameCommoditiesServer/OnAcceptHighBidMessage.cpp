#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnAcceptHighBidMessage.h"

OnAcceptHighBidMessage::OnAcceptHighBidMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnAcceptHighBidMessage"),
m_responseId(),
m_trackId(),
m_itemId(),
m_playerId(),
m_resultCode()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_resultCode);
	unpack(source);
}

OnAcceptHighBidMessage::OnAcceptHighBidMessage(
		int responseId,
		int resultCode,
		const NetworkId &  itemId,
		const NetworkId &  playerId
) :
GameNetworkMessage("OnAcceptHighBidMessage"),
m_responseId(responseId),
m_trackId(0),
m_itemId(itemId),
m_playerId(playerId),
m_resultCode(resultCode)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_resultCode);
}

OnAcceptHighBidMessage::~OnAcceptHighBidMessage()
{
}

