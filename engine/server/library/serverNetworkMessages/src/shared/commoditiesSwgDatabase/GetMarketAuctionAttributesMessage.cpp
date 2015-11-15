#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetMarketAuctionAttributesMessage.h"

GetMarketAuctionAttributesMessage::GetMarketAuctionAttributesMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("GetMarketAuctionAttributesMessage"),
	m_attributes()
{
	AutoByteStream::addVariable(m_attributes);
	unpack(source);
}

GetMarketAuctionAttributesMessage::GetMarketAuctionAttributesMessage() :
	GameNetworkMessage("GetMarketAuctionAttributesMessage"),
	m_attributes()
{
	AutoByteStream::addVariable(m_attributes);
}

GetMarketAuctionAttributesMessage::~GetMarketAuctionAttributesMessage()
{
}

void GetMarketAuctionAttributesMessage::addAttribute(
	const NetworkId &    itemId,
	const std::string &  attributeName,
	const Unicode::String & attributeValue
)
{
	GetMarketAuctionAttributesMessage::MarketAuctionAttribute attribute;
	attribute.itemId = itemId;
	attribute.attributeName = attributeName;
	attribute.attributeValue = attributeValue;

	m_attributes.get().push_back(attribute);
}
