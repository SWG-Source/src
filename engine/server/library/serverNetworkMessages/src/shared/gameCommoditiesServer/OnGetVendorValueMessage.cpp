#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnGetVendorValueMessage.h"

OnGetVendorValueMessage::OnGetVendorValueMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnGetVendorValueMessage"),
m_responseId(),
m_trackId(),
m_location(),
m_value()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_value);
	unpack(source);
}

OnGetVendorValueMessage::OnGetVendorValueMessage(int responseId, const std::string & location, int value ) :
GameNetworkMessage("OnGetVendorValueMessage"),
m_responseId(responseId),
m_trackId(0),
m_location(location),
m_value(value)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_value);
}

OnGetVendorValueMessage::~OnGetVendorValueMessage()
{
}
