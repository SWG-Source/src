#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CMDeleteLocationMessage.h"

CMDeleteLocationMessage::CMDeleteLocationMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CMDeleteLocationMessage"),
	m_locationId()
{
	AutoByteStream::addVariable(m_locationId);
	unpack(source);
}

CMDeleteLocationMessage::CMDeleteLocationMessage(
	const NetworkId &    locationId
):
	GameNetworkMessage("CMDeleteLocationMessage"),
	m_locationId(locationId)
{
	AutoByteStream::addVariable(m_locationId);
}

CMDeleteLocationMessage::~CMDeleteLocationMessage()
{
}
