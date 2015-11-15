#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnAddAuctionMessage.h"

OnAddAuctionMessage::OnAddAuctionMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnAddAuctionMessage"),
m_responseId(),
m_trackId(),
m_itemId(),
m_ownerId(),
m_resultCode(),
m_vendorId(),
m_ownerName(),
m_location()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_resultCode);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_ownerName);
	AutoByteStream::addVariable(m_location);
	unpack(source);
}

OnAddAuctionMessage::OnAddAuctionMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & ownerId,
		const std::string & ownerName,
		const NetworkId & vendorId,
		const std::string & location
) :
GameNetworkMessage("OnAddAuctionMessage"),
m_responseId(responseId),
m_trackId(0),
m_itemId(itemId),
m_ownerId(ownerId),
m_resultCode(resultCode),
m_vendorId(vendorId),
m_ownerName(ownerName),
m_location(location)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_resultCode);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_ownerName);
	AutoByteStream::addVariable(m_location);
}

OnAddAuctionMessage::~OnAddAuctionMessage()
{
}

