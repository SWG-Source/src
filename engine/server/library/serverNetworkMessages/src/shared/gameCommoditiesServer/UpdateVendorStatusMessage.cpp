#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "UpdateVendorStatusMessage.h"

UpdateVendorStatusMessage::UpdateVendorStatusMessage(Archive::ReadIterator & source) :
GameNetworkMessage("UpdateVendorStatusMessage"),
m_vendorId(),
m_location(),
m_status()
{
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_status);
	unpack(source);
}

UpdateVendorStatusMessage::UpdateVendorStatusMessage(
	const NetworkId &   vendorId,
	const std::string & location,
	int                 status
) :
GameNetworkMessage("UpdateVendorStatusMessage"),
m_vendorId(vendorId),
m_location(location),
m_status(status)
{
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_status);
}

UpdateVendorStatusMessage::~UpdateVendorStatusMessage()
{
}
