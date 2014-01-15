#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CMDeleteAuctionMessage.h"

CMDeleteAuctionMessage::CMDeleteAuctionMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CMDeleteAuctionMessage"),
	m_itemId()
{
	AutoByteStream::addVariable(m_itemId);
	unpack(source);
}

CMDeleteAuctionMessage::CMDeleteAuctionMessage(
	const NetworkId &    itemId
):
	GameNetworkMessage("CMDeleteAuctionMessage"),
	m_itemId(itemId)
{
	AutoByteStream::addVariable(m_itemId);
}

CMDeleteAuctionMessage::~CMDeleteAuctionMessage()
{
}
