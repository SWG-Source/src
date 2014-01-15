#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "LoadCommoditiesMessage.h"

LoadCommoditiesMessage::LoadCommoditiesMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("LoadCommodities"),
	m_payload()
{
	AutoByteStream::addVariable(m_payload);
	unpack(source);
}

LoadCommoditiesMessage::LoadCommoditiesMessage(
	const int            payload
):
	GameNetworkMessage("LoadCommodities"),
	m_payload(payload)
{
	AutoByteStream::addVariable(m_payload);
}

LoadCommoditiesMessage::~LoadCommoditiesMessage()
{
}
