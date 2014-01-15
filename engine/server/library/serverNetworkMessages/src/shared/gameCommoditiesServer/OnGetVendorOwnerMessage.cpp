#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnGetVendorOwnerMessage.h"

OnGetVendorOwnerMessage::OnGetVendorOwnerMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnGetVendorOwnerMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_location(),
m_requesterId(),
m_resultCode()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_requesterId);
	AutoByteStream::addVariable(m_resultCode);
	unpack(source);
}

OnGetVendorOwnerMessage::OnGetVendorOwnerMessage(
		int responseId,
		int resultCode,
		const NetworkId & ownerId,
		const NetworkId & requesterId,
		const std::string & location
) :
GameNetworkMessage("OnGetVendorOwnerMessage"),
m_responseId(responseId),
m_trackId(0),
m_ownerId(ownerId),
m_location(location),
m_requesterId(requesterId),
m_resultCode(resultCode)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_requesterId);
	AutoByteStream::addVariable(m_resultCode);
}

OnGetVendorOwnerMessage::~OnGetVendorOwnerMessage()
{
}
