#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "SetEntranceChargeMessage.h"

SetEntranceChargeMessage::SetEntranceChargeMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("SetEntranceChargeMessage"),
	m_responseId(),
	m_trackId(),
	m_vendorId(),
	m_entranceCharge()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_entranceCharge);
	unpack(source);
}

SetEntranceChargeMessage::SetEntranceChargeMessage(
	const int responseId,
	const int trackId,
	const NetworkId &vendorId,
	const int entranceCharge
):
	GameNetworkMessage("SetEntranceChargeMessage"),
	m_responseId(responseId),
	m_trackId(trackId),
	m_vendorId(vendorId),
	m_entranceCharge(entranceCharge)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_entranceCharge);
}

SetEntranceChargeMessage::~SetEntranceChargeMessage()
{
}
