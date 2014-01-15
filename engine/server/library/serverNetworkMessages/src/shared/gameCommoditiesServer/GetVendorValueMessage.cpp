#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetVendorValueMessage.h"

GetVendorValueMessage::GetVendorValueMessage(Archive::ReadIterator & source) :
GameNetworkMessage("GetVendorValueMessage"),
m_responseId(),
m_trackId(),
m_location()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_location);
	unpack(source);
}

GetVendorValueMessage::GetVendorValueMessage(
		int responseId,
		const std::string & location
) :
GameNetworkMessage("GetVendorValueMessage"),
m_responseId(responseId),
m_trackId(0),
m_location(location)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_location);
}

GetVendorValueMessage::~GetVendorValueMessage()
{
}

