#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "OnPermanentAuctionPurchasedMessage.h"

OnPermanentAuctionPurchasedMessage::OnPermanentAuctionPurchasedMessage(Archive::ReadIterator & source) :
GameNetworkMessage("OnPermanentAuctionPurchasedMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_buyerId(),
m_price(),
m_itemId(),
m_location(),
m_itemNameLength(),
m_itemName(),
m_attributes()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_buyerId);
	AutoByteStream::addVariable(m_price);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_attributes);
	unpack(source);
}

OnPermanentAuctionPurchasedMessage::OnPermanentAuctionPurchasedMessage(
		const NetworkId & ownerId,
		const NetworkId & buyerId,
		int price,
		const NetworkId & itemId,
		const std::string & location,
		int itemNameLength,
		const Unicode::String & itemName,
		std::vector<std::pair<std::string, Unicode::String> > const & attributes
) :
GameNetworkMessage("OnPermanentAuctionPurchasedMessage"),
m_responseId(0),
m_trackId(0),
m_ownerId(ownerId),
m_buyerId(buyerId),
m_price(price),
m_itemId(itemId),
m_location(location),
m_itemNameLength(itemNameLength),
m_itemName(itemName),
m_attributes()
{
	m_attributes.set(attributes);
	
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_buyerId);
	AutoByteStream::addVariable(m_price);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_attributes);
}

OnPermanentAuctionPurchasedMessage::~OnPermanentAuctionPurchasedMessage()
{
}
