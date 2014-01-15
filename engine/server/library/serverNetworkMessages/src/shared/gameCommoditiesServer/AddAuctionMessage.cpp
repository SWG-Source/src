#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "AddAuctionMessage.h"

AddAuctionMessage::AddAuctionMessage(Archive::ReadIterator & source) :
GameNetworkMessage("AddAuctionMessage"),
m_responseId(),
m_trackId(),
m_ownerId(),
m_ownerName(),
m_minimumBid(),
m_auctionTimer(),
m_itemId(),
m_itemNameLength(),
m_itemName(),
m_itemType(),
m_itemTemplateId(),
m_expireTimer(),
m_locationId(),
m_location(),
m_flags(),
m_userDescriptionLength(),
m_userDescription(),
m_itemSize(),
m_vendorLimit(),
m_vendorItemLimit()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_ownerName);
	AutoByteStream::addVariable(m_minimumBid);
	AutoByteStream::addVariable(m_auctionTimer);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_itemType);
	AutoByteStream::addVariable(m_itemTemplateId);
	AutoByteStream::addVariable(m_expireTimer);
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_flags);
	AutoByteStream::addVariable(m_userDescriptionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_attributes);
	AutoByteStream::addVariable(m_itemSize);
	AutoByteStream::addVariable(m_vendorLimit);
	AutoByteStream::addVariable(m_vendorItemLimit);
	unpack(source);
}

AddAuctionMessage::AddAuctionMessage(
		int responseId,
		const NetworkId & ownerId,
		const std::string & ownerName,
		int minimumBid,
		int auctionTimer,
		const NetworkId & itemId,
		int itemNameLength,
		const Unicode::String & itemName,
		int itemType,
		int itemTemplateId,
		int expireTimer,
		const NetworkId & locationId,
		const std::string & location,
		int flags,
		int userDescriptionLength,
		const Unicode::String & userDescription,
		std::vector<std::pair<std::string, Unicode::String> > const & attributes,
		int itemSize,
		int vendorLimit,
		int vendorItemLimit) :
GameNetworkMessage("AddAuctionMessage"),
m_responseId(responseId),
m_trackId(0),
m_ownerId(ownerId),
m_ownerName(ownerName),
m_minimumBid(minimumBid),
m_auctionTimer(auctionTimer),
m_itemId(itemId),
m_itemNameLength(itemNameLength),
m_itemName(itemName),
m_itemType(itemType),
m_itemTemplateId(itemTemplateId),
m_expireTimer(expireTimer),
m_locationId(locationId),
m_location(location),
m_flags(flags),
m_userDescriptionLength(userDescriptionLength),
m_userDescription(userDescription),
m_attributes(),
m_itemSize(itemSize),
m_vendorLimit(vendorLimit),
m_vendorItemLimit(vendorItemLimit)
{
	m_attributes.set(attributes);
	
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_ownerId);
	AutoByteStream::addVariable(m_ownerName);
	AutoByteStream::addVariable(m_minimumBid);
	AutoByteStream::addVariable(m_auctionTimer);
	AutoByteStream::addVariable(m_itemId);
	AutoByteStream::addVariable(m_itemNameLength);
	AutoByteStream::addVariable(m_itemName);
	AutoByteStream::addVariable(m_itemType);
	AutoByteStream::addVariable(m_itemTemplateId);
	AutoByteStream::addVariable(m_expireTimer);
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_location);
	AutoByteStream::addVariable(m_flags);
	AutoByteStream::addVariable(m_userDescriptionLength);
	AutoByteStream::addVariable(m_userDescription);
	AutoByteStream::addVariable(m_attributes);
	AutoByteStream::addVariable(m_itemSize);
	AutoByteStream::addVariable(m_vendorLimit);
	AutoByteStream::addVariable(m_vendorItemLimit);
}

AddAuctionMessage::~AddAuctionMessage()
{
}

