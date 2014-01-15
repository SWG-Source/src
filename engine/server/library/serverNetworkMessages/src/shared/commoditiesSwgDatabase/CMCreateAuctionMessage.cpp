#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "CMCreateAuctionMessage.h"

CMCreateAuctionMessage::CMCreateAuctionMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CMCreateAuctionMessage"),
	m_creatorId(),
	m_minimumBid(),
	m_auctionTimer(),
	m_buyNowPrice(),
	m_userDescriptionLength(),
	m_userDescription(),
	m_attributes(),
	m_locationId(),
	m_itemId(),
	m_itemType(),
	m_itemTemplateId(),
	m_expireTimer(),
	m_itemNameLength(),
	m_itemName(),
	m_ownerId(),
	m_flags(),
	m_itemSize()
{
	AutoByteStream::addVariable(m_creatorId);
	AutoByteStream::addVariable(m_minimumBid);
	AutoByteStream::addVariable(m_auctionTimer);
	AutoByteStream::addVariable(m_buyNowPrice);
	AutoByteStream::addVariable(m_userDescriptionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_attributes);
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemType);
	AutoByteStream::addVariable(m_itemTemplateId);
	AutoByteStream::addVariable(m_expireTimer);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_flags);
	AutoByteStream::addVariable(m_itemSize);
	unpack(source);
}

CMCreateAuctionMessage::CMCreateAuctionMessage(
	const NetworkId &       creatorId,
	int                     minimumBid,
	int                     auctionTimer,
	int                     buyNowPrice,
	int                     userDescriptionLength,
	const Unicode::String & userDescription,
	std::vector<std::pair<std::string, Unicode::String> > const & attributes,
	const NetworkId &       locationId,
	const NetworkId &       itemId,
	int                     itemType,
	int                     itemTemplateId,
	int                     expireTimer,
	int                     itemNameLength,
	const Unicode::String & itemName,
	const NetworkId &       ownerId,
	int                     flags,
	int                     itemSize
):
	GameNetworkMessage("CMCreateAuctionMessage"),
	m_creatorId(creatorId),
	m_minimumBid(minimumBid),
	m_auctionTimer(auctionTimer),
	m_buyNowPrice(buyNowPrice),
	m_userDescriptionLength(userDescriptionLength),
	m_userDescription(userDescription),
	m_attributes(),
	m_locationId(locationId),
	m_itemId(itemId),
	m_itemType(itemType),
	m_itemTemplateId(itemTemplateId),
	m_expireTimer(expireTimer),
	m_itemNameLength(itemNameLength),
	m_itemName(itemName),
	m_ownerId(ownerId),
	m_flags(flags),
	m_itemSize(itemSize)
{
	m_attributes.set(attributes);
	
	AutoByteStream::addVariable(m_creatorId);
	AutoByteStream::addVariable(m_minimumBid);
	AutoByteStream::addVariable(m_auctionTimer);
	AutoByteStream::addVariable(m_buyNowPrice);
	AutoByteStream::addVariable(m_userDescriptionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_attributes);
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemType);
	AutoByteStream::addVariable(m_itemTemplateId);
	AutoByteStream::addVariable(m_expireTimer);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_flags);
	AutoByteStream::addVariable(m_itemSize);
}

CMCreateAuctionMessage::~CMCreateAuctionMessage()
{
}
