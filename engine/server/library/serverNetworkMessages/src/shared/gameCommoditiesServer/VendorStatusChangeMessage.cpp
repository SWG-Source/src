#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "VendorStatusChangeMessage.h"

VendorStatusChangeMessage::VendorStatusChangeMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("VendorStatusChangeMessage"),
	m_vendorId(),
	m_status()
{
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_status);
	unpack(source);
}

VendorStatusChangeMessage::VendorStatusChangeMessage(
	const NetworkId &vendorId,
	const int status
):
	GameNetworkMessage("VendorStatusChangeMessage"),
	m_vendorId(vendorId),
	m_status(status)
{
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_status);
}

VendorStatusChangeMessage::~VendorStatusChangeMessage()
{
}
