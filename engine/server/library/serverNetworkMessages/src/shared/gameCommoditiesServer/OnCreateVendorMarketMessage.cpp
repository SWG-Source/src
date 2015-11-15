#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnCreateVendorMarketMessage.h"

OnCreateVendorMarketMessage::OnCreateVendorMarketMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnCreateVendorMarketMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_location(),
m_resultCode()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_resultCode);
	unpack(source);
}

OnCreateVendorMarketMessage::OnCreateVendorMarketMessage(
		int responseId,
		int resultCode,
		const NetworkId & ownerId,
		const std::string & location
) :
GameNetworkMessage("OnCreateVendorMarketMessage"),
m_responseId(responseId),
m_trackId(0),
m_ownerId(ownerId),
m_location(location),
m_resultCode(resultCode)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_resultCode);
}

OnCreateVendorMarketMessage::~OnCreateVendorMarketMessage()
{
}
