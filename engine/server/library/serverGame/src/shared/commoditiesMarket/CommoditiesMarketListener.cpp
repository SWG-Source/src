// ======================================================================
//
// CommoditiesMarketListener.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CommoditiesMarketListener.h"

#include "serverGame/Chat.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/GameServer.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"
#include "UnicodeUtils.h"
#include "serverNetworkMessages/AuctionBase.h"

#ifdef _WIN32
#define atoll(a) _atoi64(a)
#endif

// ======================================================================

static const char *findNetworkId(const char *locationString)
{
	static char str[1024];
	strcpy(str, locationString);
	char * poundPos = strstr(str, "#");
	if (poundPos)
	{
		*poundPos = 0;	
	}
	int loc = strlen(str) - 1;
	while (loc > 0)
	{
		if (str[loc] == '.')
		{
			return (&(str[loc + 1]));
		}
		--loc;
	}
	return str;
}

//-----------------------------------------------------------------------

CommoditiesMarketListener::CommoditiesMarketListener()
{
}

//-----------------------------------------------------------------------

CommoditiesMarketListener::~CommoditiesMarketListener()
{
}

//-----------------------------------------------------------------------

void CommoditiesMarketListener::OnAddAuction(int requestId, int result, const NetworkId & auctionId, const NetworkId & auctionOwnerId, const std::string & ownerName, const NetworkId & itemId, const NetworkId & vendorId, const std::string & vendorName, const std::string & location)
{
	UNREF(auctionId);
	

	if (vendorId.getValue() != -1 && (vendorId != auctionOwnerId))
	{
		Unicode::String message = Unicode::narrowToWide(ownerName + " has offered an item at your vendor market.");
		Unicode::String oob;
		Chat::sendSystemMessage(std::string(vendorName), message, oob); 
			
	}

	CommoditiesMarket::onAddAuction(requestId, result, auctionOwnerId, ownerName, itemId, vendorId, vendorName, NetworkId(atoll(findNetworkId(location.c_str()))));
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnAddBid(int requestId, int result, const NetworkId & auctionOwnerId, const NetworkId & itemId, const NetworkId & bidPlayerId, const NetworkId & previousBidderId, int bidAmount, int previousBid, int maxProxyBidAmount, const std::string & location, const std::string & ownerName, const std::string & previousBidderName, int itemNameLength, const Unicode::String &  itemName, int salesTaxAmount, const NetworkId & salesTaxBankId)
{
	UNREF(requestId);

	bool success = ((result != ARC_BidTooLow) && (bidAmount > 0));

// Warning: not all state accounted for!

/// success
//  SuccessPermanentAuction
//  expired
//  toohigh
//  toolow

	
	CommoditiesMarket::onAddBid(result, itemId, bidPlayerId, previousBidderId, maxProxyBidAmount, previousBid, NetworkId(atoll(findNetworkId(location.c_str()))), success, auctionOwnerId, ownerName, previousBidderName, itemNameLength, itemName, salesTaxAmount, salesTaxBankId);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnItemExpired(const NetworkId & itemOwner, const NetworkId & itemId, const std::string & ownerName, int itemNameLength, const Unicode::String &  itemName, const std::string &locationName, const NetworkId &locationId)
{
	CommoditiesMarket::onItemExpired(itemOwner, itemId, ownerName, itemNameLength, itemName, locationName, locationId);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnAuctionExpired(const NetworkId & auctionId, const NetworkId & auctionOwnerId, bool sold, const NetworkId & buyerId, int highBidAmount, const NetworkId & itemId, int highBidMaxProxy, const std::string & location, bool immediate, const std::string & ownerName, const std::string & buyerName, int itemNameLength, const Unicode::String & itemName, bool sendSellerMail)
{
	CommoditiesMarket::onAuctionComplete(itemId, auctionOwnerId, buyerId, highBidAmount, highBidMaxProxy, NetworkId(findNetworkId(location.c_str())), sold, immediate, location, ownerName, buyerName, itemNameLength, itemName, auctionId.getValue(), sendSellerMail);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnQueryAuctionHeadersReply(int requestId, int result, const NetworkId & playerId, int typeFlag, int numAuctions, const std::vector<AuctionDataHeader> & auctionData, unsigned int queryOffset, bool hasMorePages)
{
	UNREF(playerId);

	CommoditiesMarket::onQueryHeadersReply(result, requestId, typeFlag, numAuctions, auctionData, queryOffset, hasMorePages);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnGetItemDetailsReply(int requestId, int result, const NetworkId & playerId, const NetworkId & itemId, int userDescriptionLength, const Unicode::String &userDescription, int oobLength, const Unicode::String & oobData, std::vector<std::pair<std::string, Unicode::String> > const & attributes)
{
	CommoditiesMarket::onGetItemDetailsReply(result, requestId, playerId, itemId, userDescriptionLength, userDescription, oobLength, oobData, attributes);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnGetItemReply(int requestId, int result, const NetworkId & itemId, const NetworkId & playerId, const std::string & location)
{
	CommoditiesMarket::onGetItemReply(result, requestId, playerId, itemId, NetworkId(findNetworkId(location.c_str())));
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnGetVendorOwner(int requestId, int result, const NetworkId & ownerId, const std::string & ownerName, const std::string & location, const NetworkId & requesterId)
{
	UNREF(ownerName);
	UNREF(result);
	UNREF(requestId);

	CommoditiesMarket::onIsVendorOwner(requesterId, ownerId, NetworkId(findNetworkId(location.c_str())));
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnAcceptHighBid(int requestId, int result, const NetworkId & itemId, const NetworkId & playerId)
{
	UNREF(requestId);

	CommoditiesMarket::onAcceptHighBid(result, itemId, playerId);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnPermanentAuctionPurchased(
	const NetworkId & ownerId,
	const NetworkId & buyerId,
	int price,
	const NetworkId & itemId,
	const std::string & location,
	const std::string & ownerName,
	const std::string & buyerName,
	int itemNameLength,
	const Unicode::String &  itemName,
	std::vector<std::pair<std::string, Unicode::String> > const & attributes
)
{
	CommoditiesMarket::onPermanentAuctionPurchased(
		ownerId,
		buyerId,
		price,
		itemId,
		NetworkId(findNetworkId(location.c_str())),
		ownerName, buyerName, itemNameLength, itemName,
		attributes);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnGetVendorValue(int requestId, const std::string & location, int value)
{
	UNREF(requestId);

	CommoditiesMarket::onGetVendorValue(NetworkId(findNetworkId(location.c_str())), value);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnVendorRefuseItem(int requestId, int result, const NetworkId & itemId, const NetworkId & vendorId, const NetworkId & itemOwnerId)
{
	UNREF(requestId);

	CommoditiesMarket::onVendorRefuseItem(result, itemId, vendorId, itemOwnerId);
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnCancelAuction(int requestId, int result, const NetworkId & itemId, const NetworkId & playerId, const NetworkId & highBidderId, int highBid, const std::string & location)
{
	UNREF(requestId); 
	UNREF(highBidderId); 
	UNREF(highBid); 
	UNREF(location);

	if (result != 0)
	{
		CommoditiesMarket::onCancelAuction(result, playerId, itemId);
	}
}

// ----------------------------------------------------------------------


void CommoditiesMarketListener::OnCreateVendorMarket(int requestId, int result, const NetworkId & ownerId, const std::string & location)
{
	UNREF(requestId); 
	UNREF(ownerId);

	CommoditiesMarket::onCreateVendorMarket(result, ownerId, NetworkId(atoll(findNetworkId(location.c_str()))));
}

// ----------------------------------------------------------------------

void CommoditiesMarketListener::OnCleanupInvalidItemRetrieval(int requestId, const NetworkId & itemId, const NetworkId & playerId, const NetworkId & creatorId, int reimburseAmt)
{
	UNREF(requestId);

	CommoditiesMarket::onCleanupInvalidItemRetrieval(itemId, playerId, creatorId, reimburseAmt);
}

// ======================================================================
