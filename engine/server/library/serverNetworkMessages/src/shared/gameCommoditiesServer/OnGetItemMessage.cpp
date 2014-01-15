#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnGetItemMessage.h"

OnGetItemMessage::OnGetItemMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnGetItemMessage"),
m_responseId(),
m_trackId(),
m_itemId(),
m_playerId(),
m_resultCode(),
m_location()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_resultCode);
	AutoByteStream::addVariable(m_location);
	unpack(source);
}

OnGetItemMessage::OnGetItemMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & playerId,
		const std::string & location
) :
GameNetworkMessage("OnGetItemMessage"),
m_responseId(responseId),
m_trackId(0),
m_itemId(itemId),
m_playerId(playerId),
m_resultCode(resultCode),
m_location(location)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_resultCode);
	AutoByteStream::addVariable(m_location);
}

OnGetItemMessage::~OnGetItemMessage()
{
}
