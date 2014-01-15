#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnUpdateVendorSearchOptionMessage.h"

OnUpdateVendorSearchOptionMessage::OnUpdateVendorSearchOptionMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("OnUpdateVendorSearchOptionMessage"),
	m_playerId(),
	m_enabled()
{
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_enabled);
	unpack(source);
}

OnUpdateVendorSearchOptionMessage::OnUpdateVendorSearchOptionMessage(
	const NetworkId & playerId,
	const bool enabled
):
	GameNetworkMessage("OnUpdateVendorSearchOptionMessage"),
	m_playerId(playerId),
	m_enabled(enabled)
{
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_enabled);
}

OnUpdateVendorSearchOptionMessage::~OnUpdateVendorSearchOptionMessage()
{
}
