#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "UpdateVendorSearchOptionMessage.h"

UpdateVendorSearchOptionMessage::UpdateVendorSearchOptionMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("UpdateVendorSearchOptionMessage"),
	m_responseId(),
	m_trackId(),
	m_vendorId(),
	m_enabled(),
	m_playerId()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_enabled);
	AutoByteStream::addVariable(m_playerId);
	unpack(source);
}

UpdateVendorSearchOptionMessage::UpdateVendorSearchOptionMessage(
	const int responseId,
	const int trackId,
	const NetworkId &vendorId,
	const bool enabled,
	const NetworkId & playerId
):
	GameNetworkMessage("UpdateVendorSearchOptionMessage"),
	m_responseId(responseId),
	m_trackId(trackId),
	m_vendorId(vendorId),
	m_enabled(enabled),
	m_playerId(playerId)
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_enabled);
	AutoByteStream::addVariable(m_playerId);
}

UpdateVendorSearchOptionMessage::~UpdateVendorSearchOptionMessage()
{
}
