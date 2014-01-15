#ifndef GetMarketAuctionsMessage_H
#define GetMarketAuctionsMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "unicodeArchive/UnicodeArchive.h"

class GetMarketAuctionsMessage : public GameNetworkMessage
{
public:
	struct MarketAuction
	{
		NetworkId       itemId;
		NetworkId       ownerId;
		NetworkId       creatorId;
		NetworkId       locationId;
		int             minBid;
		int             buyNowPrice;
		int             auctionTimer;
		int             oobLength;
		std::string     oob;
		int             userDescriptionLength;
		Unicode::String userDescription;
		int             category;
		int             itemTemplateId;
		int             itemNameLength;
		Unicode::String itemName;
		int             itemTimer;
		int             active;
		int             itemSize;
	};

	GetMarketAuctionsMessage();
	GetMarketAuctionsMessage(Archive::ReadIterator & source);
	~GetMarketAuctionsMessage();

	void addAuction(
		const NetworkId &    itemId,
		const NetworkId &    ownerId,
		const NetworkId &    creatorId,
		const NetworkId &    locationId,
		int                  minBid,
		int                  buyNowPrice,
		int                  auctionTimer,
		int                  oobLength,
		const std::string &  oob,
		int                  userDescriptionLength,
		const Unicode::String &userDescription,
		int                  category,
		int                  itemTemplateId,
		int                  itemNameLength,
		const Unicode::String &itemName,
		int                  itemTimer,
		int                  active,
		int                  itemSize
	);

	void clearAllAuctions() {m_auctions.get().clear();}

	const std::list<MarketAuction> & getAuctions() const {return m_auctions.get();}
		
protected:

private:
	Archive::AutoList<MarketAuction> m_auctions;
};

namespace Archive
{
	inline void get(ReadIterator & source, GetMarketAuctionsMessage::MarketAuction & target)
	{
		Archive::get(source, target.itemId);
		Archive::get(source, target.ownerId);
		Archive::get(source, target.creatorId);
		Archive::get(source, target.locationId);
		Archive::get(source, target.minBid);
		Archive::get(source, target.buyNowPrice);
		Archive::get(source, target.auctionTimer);
		Archive::get(source, target.oobLength);
		Archive::get(source, target.oob);
		Archive::get(source, target.userDescriptionLength);
		Archive::get(source, target.userDescription);
		Archive::get(source, target.category);
		Archive::get(source, target.itemTemplateId);
		Archive::get(source, target.itemNameLength);
		Archive::get(source, target.itemName);
		Archive::get(source, target.itemTimer);
		Archive::get(source, target.active);
		Archive::get(source, target.itemSize);
	}

	inline void put(ByteStream & target, const GetMarketAuctionsMessage::MarketAuction & source)
	{
		Archive::put(target, source.itemId);
		Archive::put(target, source.ownerId);
		Archive::put(target, source.creatorId);
		Archive::put(target, source.locationId);
		Archive::put(target, source.minBid);
		Archive::put(target, source.buyNowPrice);
		Archive::put(target, source.auctionTimer);
		Archive::put(target, source.oobLength);
		Archive::put(target, source.oob);
		Archive::put(target, source.userDescriptionLength);
		Archive::put(target, source.userDescription);
		Archive::put(target, source.category);
		Archive::put(target, source.itemTemplateId);
		Archive::put(target, source.itemNameLength);
		Archive::put(target, source.itemName);
		Archive::put(target, source.itemTimer);
		Archive::put(target, source.active);
		Archive::put(target, source.itemSize);
	}
}

#endif
