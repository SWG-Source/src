#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "DestroyVendorMarketMessage.h"

DestroyVendorMarketMessage::DestroyVendorMarketMessage(Archive::ReadIterator & source) :
GameNetworkMessage("DestroyVendorMarketMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_ownerName(),
m_location()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_ownerName);
	AutoByteStream::addVariable(m_location);
	unpack(source);
}

DestroyVendorMarketMessage::DestroyVendorMarketMessage(
		int responseId,
		const NetworkId & ownerId,
		const std::string & ownerName,
		const std::string & location
) :
GameNetworkMessage("DestroyVendorMarketMessage"),
m_responseId(responseId),
m_trackId(0),
m_ownerId(ownerId),
m_ownerName(ownerName),
m_location(location)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_ownerName);
	AutoByteStream::addVariable(m_location);
}

DestroyVendorMarketMessage::~DestroyVendorMarketMessage()
{
}

