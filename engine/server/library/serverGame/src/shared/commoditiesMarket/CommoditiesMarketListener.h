// ======================================================================
//
// CommoditiesMarketListener.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _CommoditiesMarketListener_H_
#define _CommoditiesMarketListener_H_

#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"
#include "serverNetworkMessages/AuctionData.h"

// ======================================================================

class CommoditiesMarketListener
{
public:

	CommoditiesMarketListener();
	~CommoditiesMarketListener();

	void OnAddAuction                (int requestId, int result, const NetworkId & auctionId, const NetworkId & auctionOwnerId, const std::string & ownerName, const NetworkId & itemId, const NetworkId & vendorId, const std::string & vendorName, const std::string & location);
	void OnAddBid                    (int requestId, int result, const NetworkId & auctionOwnerId, const NetworkId & itemId, const NetworkId & bidPlayerId, const NetworkId & previousBidderId, int bidAmount, int previousBidderAmount, int maxProxyBidAmount, const std::string & location, const std::string & ownerName, const std::string & previousBidderName, int itemNameLength, const Unicode::String &itemName, int salesTaxAmount, const NetworkId & salesTaxBankId);
	void OnItemExpired               (const NetworkId & itemOwner, const NetworkId & itemId, const std::string & ownerName, int itemNameLength, const Unicode::String &itemName, const std::string &locationName, const NetworkId & locationId);
	void OnAuctionExpired            (const NetworkId & auctionId, const NetworkId & auctionOwnerId, bool sold, const NetworkId & buyerId, int highBidAmount, const NetworkId & itemId, int highBidMaxProxy, const std::string & location, bool immediate, const std::string & ownerName, const std::string & buyerName, int itemNameLength, const Unicode::String &itemName, bool sendSellerMail);
	void OnQueryAuctionHeadersReply  (int requestId, int result, const NetworkId & playerId, int typeFlag, int numAuctions, const std::vector<AuctionDataHeader> & auctionData, unsigned int queryAuctions, bool hasMorePages);
	void OnGetItemDetailsReply       (int requestId, int result, const NetworkId & playerId, const NetworkId & itemId, int userDescriptionLength, const Unicode::String &userDescription, int oobLength, const Unicode::String & oobData, std::vector<std::pair<std::string, Unicode::String> > const & attributes);

	void OnCancelAuction             (int requestId, int result, const NetworkId & itemId, const NetworkId & playerId, const NetworkId & highBidderId, int highBid, const std::string & location);
	void OnVendorRefuseItem          (int requestId, int result, const NetworkId & itemId, const NetworkId & vendorId, const NetworkId & itemOwnerId);
	void OnAcceptHighBid             (int requestId, int result, const NetworkId & itemId, const NetworkId & playerId);
	void OnGetItemReply              (int requestId, int result, const NetworkId & itemId, const NetworkId & playerId, const std::string & location);
	void OnCreateVendorMarket        (int requestId, int result, const NetworkId & ownerId, const std::string & location);
	void OnGetVendorOwner            (int requestId, int result, const NetworkId & ownerId, const std::string & ownerName, const std::string & location, const NetworkId & requesterId);
	void OnCleanupInvalidItemRetrieval (int requestId, const NetworkId & itemId, const NetworkId & playerId, const NetworkId & creatorId, int reimburseAmt);
    void OnPermanentAuctionPurchased (const NetworkId & ownerId, const NetworkId & buyerId, int price, const NetworkId & itemId, const std::string & location, const std::string & ownerName, const std::string & buyerName, int itemNameLength, const Unicode::String &itemName, std::vector<std::pair<std::string, Unicode::String> > const & attributes);
	void OnGetVendorValue            (int requestId, const std::string & location, int value);

private:
	CommoditiesMarketListener(const CommoditiesMarketListener&);
	CommoditiesMarketListener& operator= (const CommoditiesMarketListener&);

};

// ======================================================================

#endif // _CommoditiesMarketListener_H_

