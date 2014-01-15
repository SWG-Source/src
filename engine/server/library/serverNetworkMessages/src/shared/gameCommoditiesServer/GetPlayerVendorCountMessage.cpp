#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetPlayerVendorCountMessage.h"

GetPlayerVendorCountMessage::GetPlayerVendorCountMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("GetPlayerVendorCountMessage"),
	m_responseId(),
	m_trackId(),
	m_playerId()
{
 	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_playerId);
	unpack(source);
}

GetPlayerVendorCountMessage::GetPlayerVendorCountMessage(
	int responseId,
	const NetworkId & playerId
) :
	GameNetworkMessage("GetPlayerVendorCountMessage"),
	m_responseId(responseId),
	m_trackId(0),
	m_playerId(playerId)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_playerId);
}

GetPlayerVendorCountMessage::~GetPlayerVendorCountMessage()
{
}

