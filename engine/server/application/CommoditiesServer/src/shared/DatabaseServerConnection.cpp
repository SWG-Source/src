// DatabaseServerConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstCommodityServer.h"
#include "DatabaseServerConnection.h"
#include "ConfigCommodityServer.h"
#include "AuctionMarket.h"
#include "sharedLog/Log.h"
#include "CommodityServer.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/DeleteCharacterMessage.h"
#include "serverNetworkMessages/CommoditiesLoadDoneMessage.h"
#include "serverNetworkMessages/GetAuctionLocationsMessage.h"
#include "serverNetworkMessages/GetMarketAuctionsMessage.h"
#include "serverNetworkMessages/GetMarketAuctionAttributesMessage.h"
#include "serverNetworkMessages/GetMarketAuctionBidsMessage.h"
#include "UnicodeUtils.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

DatabaseServerConnection::DatabaseServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
	LOG("DatabaseServerConnection", ("Connection with the database server created"));
}

//-----------------------------------------------------------------------

DatabaseServerConnection::~DatabaseServerConnection()
{
}

//-----------------------------------------------------------------------

void DatabaseServerConnection::onConnectionClosed()
{
	LOG("DatabaseServerConnection", ("Connection with the database server is closed"));
	static MessageConnectionCallback m("DatabaseServerConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void DatabaseServerConnection::onConnectionOpened()
{
	LOG("DatabaseServerConnection", ("Connection with the database server is open"));
	static MessageConnectionCallback m("DatabaseServerConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void DatabaseServerConnection::onReceive(const Archive::ByteStream & message)
{
	static int lastLocationCount = 0, currentLocationCount = 0;
	static int lastAuctionCount = 0, currentAuctionCount = 0;
	static int lastAuctionAttributeCount = 0, currentAuctionAttributeCount = 0;
	static int lastBidCount = 0, currentBidCount = 0;

	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage msg(ri);
	ri = message.begin();
	
	const uint32 messageType = msg.getType();

	switch(messageType) {
		case constcrc("CommoditiesLoadDone") :
		{
			if (currentLocationCount != lastLocationCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d AuctionLocations record received\n", currentLocationCount));
				LOG("CommoditiesServer", ("%d AuctionLocations record received\n", currentLocationCount));
				lastLocationCount = currentLocationCount;
			}
			if (currentAuctionCount != lastAuctionCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctions record received\n", currentAuctionCount));
				LOG("CommoditiesServer", ("%d MarketAuctions record received\n", currentAuctionCount));
				lastAuctionCount = currentAuctionCount;
			}
			if (currentAuctionAttributeCount != lastAuctionAttributeCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctionAttributes record received\n", currentAuctionAttributeCount));
				LOG("CommoditiesServer", ("%d MarketAuctionAttributes record received\n", currentAuctionAttributeCount));
				lastAuctionAttributeCount = currentAuctionAttributeCount;
			}
			if (currentBidCount != lastBidCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctionBids record received\n", currentBidCount));
				LOG("CommoditiesServer", ("%d MarketAuctionBids record received", currentBidCount));
				lastBidCount = currentBidCount;
			}
			REPORT_LOG("DatabaseServerConnection", ("CommoditiesLoadDone Message received on connection with the database server\n"));
			CommoditiesLoadDoneMessage m(ri);
			REPORT_LOG("DatabaseServerConnection", ("%d rows received on Auction_Locations\n", m.getAuctionLocationsCount()));
			REPORT_LOG("DatabaseServerConnection", ("%d rows received on Market_Auctions\n", m.getMarketAuctionsCount()));
			REPORT_LOG("DatabaseServerConnection", ("%d rows received on Market_Auction_Attributes\n", m.getMarketAuctionAttributesCount()));
			REPORT_LOG("DatabaseServerConnection", ("%d rows received on Market_Auction_Bids\n", m.getMarketAuctionBidsCount()));
			LOG("CommoditiesServer", ("CommoditiesLoadDone Message received, %d rows Auction_Locations, %d rows Market_Auctions, %d rows Market_Auction_Attributes, %d rows Market_Auction_Bids", m.getAuctionLocationsCount(), m.getMarketAuctionsCount(), m.getMarketAuctionAttributesCount(), m.getMarketAuctionBidsCount()));

			LOG("CommoditiesServer", ("Start build index for attribute search"));
			AuctionMarket::getInstance().BuildAuctionsSearchableAttributeList();
			LOG("CommoditiesServer", ("End build index for attribute search"));

			emitMessage(m);
			break;
		}
		case constcrc("GetAuctionLocationsMessage") :
		{
			GetAuctionLocationsMessage m(ri);
			const std::list<GetAuctionLocationsMessage::AuctionLocation> & auctionLocations = m.getAuctionLocations();
			for (std::list<GetAuctionLocationsMessage::AuctionLocation>::const_iterator iter = auctionLocations.begin(); iter != auctionLocations.end(); ++iter)
			{
				AuctionMarket::getInstance().onReceiveAuctionLocations(iter->locationId, iter->locationName, iter->ownerId, iter->salesTax, iter->salesTaxBankId, iter->emptyDate, iter->lastAccessDate, iter->inactiveDate, iter->status, iter->searchEnabled, iter->entranceCharge);
				currentLocationCount++;
				if (currentLocationCount - lastLocationCount >= 1000)
				{
					DEBUG_REPORT_LOG(true, ("  -- %d AuctionLocations record received\n", currentLocationCount));
					LOG("CommoditiesServer", ("%d AuctionLocations record received", currentLocationCount));
					lastLocationCount = currentLocationCount;
				}
			}
			break;
		}
		case constcrc("GetMarketAuctionsMessage") :
		{
			if (currentLocationCount != lastLocationCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d AuctionLocations record received\n", currentLocationCount));
				LOG("CommoditiesServer", ("%d AuctionLocations record received\n", currentLocationCount));
				lastLocationCount = currentLocationCount;
			}
			GetMarketAuctionsMessage m(ri);
			const std::list<GetMarketAuctionsMessage::MarketAuction> & auctions = m.getAuctions();
			for (std::list<GetMarketAuctionsMessage::MarketAuction>::const_iterator iter = auctions.begin(); iter != auctions.end(); ++iter)
			{
				AuctionMarket::getInstance().onReceiveMarketAuctions(iter->itemId, iter->ownerId, iter->creatorId, iter->locationId, iter->minBid, iter->buyNowPrice, iter->auctionTimer, iter->oob, iter->userDescription, iter->category, iter->itemTemplateId, iter->itemName, iter->itemTimer, iter->active, iter->itemSize);
				currentAuctionCount++;
				if (currentAuctionCount - lastAuctionCount >= 10000)
				{
					DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctions record received\n", currentAuctionCount));
					LOG("CommoditiesServer", ("%d MarketAuctions record received", currentAuctionCount));
					lastAuctionCount = currentAuctionCount;
				}
			}
			break;
		}
		case constcrc("GetMarketAuctionAttributesMessage") :
		{
			if (currentLocationCount != lastLocationCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d AuctionLocations record received\n", currentLocationCount));
				LOG("CommoditiesServer", ("%d AuctionLocations record received\n", currentLocationCount));
				lastLocationCount = currentLocationCount;
			}
			if (currentAuctionCount != lastAuctionCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctions record received\n", currentAuctionCount));
				LOG("CommoditiesServer", ("%d MarketAuctions record received\n", currentAuctionCount));
				lastAuctionCount = currentAuctionCount;
			}
			GetMarketAuctionAttributesMessage m(ri);
			const std::list<GetMarketAuctionAttributesMessage::MarketAuctionAttribute> & attributes = m.getAttributes();
			for (std::list<GetMarketAuctionAttributesMessage::MarketAuctionAttribute>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
			{
				AuctionMarket::getInstance().onReceiveMarketAuctionAttributes(iter->itemId, iter->attributeName, iter->attributeValue);
				currentAuctionAttributeCount++;
				if (currentAuctionAttributeCount - lastAuctionAttributeCount >= 10000)
				{
					DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctionAttributes record received\n", currentAuctionAttributeCount));
					LOG("CommoditiesServer", ("%d MarketAuctionAttributes record received", currentAuctionAttributeCount));
					lastAuctionAttributeCount = currentAuctionAttributeCount;
				}
			}
			break;
		}
		case constcrc("GetMarketAuctionBidsMessage") :
		{
			if (currentLocationCount != lastLocationCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d AuctionLocations record received\n", currentLocationCount));
				LOG("CommoditiesServer", ("%d AuctionLocations record received\n", currentLocationCount));
				lastLocationCount = currentLocationCount;
			}
			if (currentAuctionCount != lastAuctionCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctions record received\n", currentAuctionCount));
				LOG("CommoditiesServer", ("%d MarketAuctions record received\n", currentAuctionCount));
				lastAuctionCount = currentAuctionCount;
			}
			if (currentAuctionAttributeCount != lastAuctionAttributeCount)
			{
				DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctionAttributes record received\n", currentAuctionAttributeCount));
				LOG("CommoditiesServer", ("%d MarketAuctionAttributes record received\n", currentAuctionAttributeCount));
				lastAuctionAttributeCount = currentAuctionAttributeCount;
			}
			GetMarketAuctionBidsMessage m(ri);
			const std::list<GetMarketAuctionBidsMessage::MarketAuctionBid> & bids = m.getMarketAuctionBids();
			for (std::list<GetMarketAuctionBidsMessage::MarketAuctionBid>::const_iterator iter = bids.begin(); iter != bids.end(); ++iter)
			{
				AuctionMarket::getInstance().onReceiveMarketAuctionBids(iter->itemId, iter->bidderId, iter->bid, iter->maxProxyBid);
				currentBidCount++;
				if (currentBidCount - lastBidCount >= 1000)
				{
					DEBUG_REPORT_LOG(true, ("  -- %d MarketAuctionBids record received\n", currentBidCount));
					LOG("CommoditiesServer", ("%d MarketAuctionBids record received\n", currentBidCount));
					lastBidCount = currentBidCount;
				}
			}
			break;
		}
		case constcrc("DeleteCharacterMessage") :
		{
			DeleteCharacterMessage message(ri);
			AuctionMarket::getInstance().DeleteCharacter(message);
			break;
		}
	}
}

//-----------------------------------------------------------------------


