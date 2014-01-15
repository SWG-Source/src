#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetMarketAuctionsMessage.h"

GetMarketAuctionsMessage::GetMarketAuctionsMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("GetMarketAuctionsMessage"),
	m_auctions()
{
	AutoByteStream::addVariable(m_auctions);
	unpack(source);
}

GetMarketAuctionsMessage::GetMarketAuctionsMessage() :
	GameNetworkMessage("GetMarketAuctionsMessage"),
	m_auctions()
{	
	AutoByteStream::addVariable(m_auctions);
}

GetMarketAuctionsMessage::~GetMarketAuctionsMessage()
{
}

void GetMarketAuctionsMessage::addAuction(
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
)
{
	GetMarketAuctionsMessage::MarketAuction auction;
	auction.itemId                = itemId;
	auction.ownerId               = ownerId;
	auction.creatorId             = creatorId;
	auction.locationId            = locationId;
	auction.minBid                = minBid;
	auction.buyNowPrice           = buyNowPrice;
	auction.auctionTimer          = auctionTimer;
	auction.oobLength             = oobLength;
	auction.oob                   = oob;
	auction.userDescriptionLength = userDescriptionLength;
	auction.userDescription       = userDescription;
	auction.category              = category;
	auction.itemTemplateId        = itemTemplateId;
	auction.itemNameLength        = itemNameLength;
	auction.itemName              = itemName;
	auction.itemTimer             = itemTimer;
	auction.active                = active;
	auction.itemSize              = itemSize;

	m_auctions.get().push_back(auction);
}
