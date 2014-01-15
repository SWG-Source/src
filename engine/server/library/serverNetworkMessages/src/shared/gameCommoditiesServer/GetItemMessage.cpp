#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetItemMessage.h"

GetItemMessage::GetItemMessage(Archive::ReadIterator & source) :
GameNetworkMessage("GetItemMessage"),
m_responseId(),
m_trackId(),
m_itemId(),
m_playerId(),
m_location()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_location);
	unpack(source);
}

GetItemMessage::GetItemMessage(
		int requestId,
		const NetworkId & itemId,
		const NetworkId & playerId,
		const std::string & location
) : 
GameNetworkMessage("GetItemMessage"),
m_responseId(requestId),
m_trackId(0),
m_itemId(itemId),
m_playerId(playerId),
m_location(location)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_location);
}

GetItemMessage::~GetItemMessage()
{
}
