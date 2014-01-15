#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "ItemMovedMessage.h"

ItemMovedMessage::ItemMovedMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("ItemMovedMessage"),
	m_playerId(),
	m_locationNameLength(),
	m_locationName()
{
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_locationNameLength);
	AutoByteStream::addVariable(m_locationName);
	unpack(source);
}

ItemMovedMessage::ItemMovedMessage(
	const NetworkId &    playerId,
	const int            locationNameLength,
	const std::string &  locationName
):
	GameNetworkMessage("ItemMovedMessage"),
	m_playerId(playerId),
	m_locationNameLength(locationNameLength),
	m_locationName(locationName)
{
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_locationNameLength);
	AutoByteStream::addVariable(m_locationName);
}

ItemMovedMessage::~ItemMovedMessage()
{
}
