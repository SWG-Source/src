#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnItemExpiredMessage.h"

OnItemExpiredMessage::OnItemExpiredMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnItemExpiredMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_itemId(),
m_itemNameLength(),
m_itemName(),
m_locationName(),
m_locationId()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_locationName);
	AutoByteStream::addVariable(m_locationId);
	unpack(source);
}

OnItemExpiredMessage::OnItemExpiredMessage(
		const NetworkId & ownerId,
		const NetworkId & itemId,
		int itemNameLength,
		const Unicode::String & itemName,
		const std::string & locationName,
		const NetworkId & locationId
) :
GameNetworkMessage("OnItemExpiredMessage"),
m_responseId(0),
m_trackId(0),
m_ownerId(ownerId),
m_itemId(itemId),
m_itemNameLength(itemNameLength),
m_itemName(itemName),
m_locationName(locationName),
m_locationId(locationId)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_locationName);
	AutoByteStream::addVariable(m_locationId);
}

OnItemExpiredMessage::~OnItemExpiredMessage()
{
}
