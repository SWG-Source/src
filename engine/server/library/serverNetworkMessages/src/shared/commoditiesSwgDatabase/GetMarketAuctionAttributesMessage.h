#ifndef GetMarketAuctionAttributesMessage_H
#define GetMarketAuctionAttributesMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "unicodeArchive/UnicodeArchive.h"

class GetMarketAuctionAttributesMessage : public GameNetworkMessage
{
public:
	struct MarketAuctionAttribute
	{
		NetworkId itemId;
		std::string attributeName;
		Unicode::String attributeValue;
	};

	GetMarketAuctionAttributesMessage();
	GetMarketAuctionAttributesMessage(Archive::ReadIterator & source);
	~GetMarketAuctionAttributesMessage();

	void addAttribute(
		const NetworkId &    itemId,
		const std::string &  attributeName,
		const Unicode::String & attributeValue
	);

	void clearAllAttributes() {m_attributes.get().clear();}

	const std::list<MarketAuctionAttribute> & getAttributes() const {return m_attributes.get();}

protected:

private:
	Archive::AutoList<MarketAuctionAttribute> m_attributes;
};

namespace Archive
{
	inline void get(ReadIterator & source, GetMarketAuctionAttributesMessage::MarketAuctionAttribute & target)
	{
		Archive::get(source, target.itemId);
		Archive::get(source, target.attributeName);
		Archive::get(source, target.attributeValue);
	}

	inline void put(ByteStream & target, const GetMarketAuctionAttributesMessage::MarketAuctionAttribute & source)
	{
		Archive::put(target, source.itemId);
		Archive::put(target, source.attributeName);
		Archive::put(target, source.attributeValue);
	}
}

#endif
