#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CMUpdateAuctionMessage.h"

CMUpdateAuctionMessage::CMUpdateAuctionMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CMUpdateAuctionMessage"),
	m_itemId(),
	m_ownerId(),
	m_flags()
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_flags);
	unpack(source);
}

CMUpdateAuctionMessage::CMUpdateAuctionMessage(
	const NetworkId &    itemId,
	const NetworkId &    ownerId,
	int                  flags
):
	GameNetworkMessage("CMUpdateAuctionMessage"),
	m_itemId(itemId),
	m_ownerId(ownerId),
	m_flags(flags)
{
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_flags);
}

CMUpdateAuctionMessage::~CMUpdateAuctionMessage()
{
}
