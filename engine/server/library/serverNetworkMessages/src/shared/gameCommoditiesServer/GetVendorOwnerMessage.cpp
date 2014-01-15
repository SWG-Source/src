#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetVendorOwnerMessage.h"

GetVendorOwnerMessage::GetVendorOwnerMessage(Archive::ReadIterator & source) :
GameNetworkMessage("GetVendorOwnerMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_location()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_location);
	unpack(source);
}

GetVendorOwnerMessage::GetVendorOwnerMessage(
		int responseId,
		const NetworkId & ownerId,
		const std::string & location
) :
GameNetworkMessage("GetVendorOwnerMessage"),
m_responseId(responseId),
m_trackId(0),
m_ownerId(ownerId),
m_location(location)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_location);
}

GetVendorOwnerMessage::~GetVendorOwnerMessage()
{
}

