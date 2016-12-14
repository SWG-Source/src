// ======================================================================
//
// CommoditiesMarket.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _CommoditiesMarket_H_
#define _CommoditiesMarket_H_

#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"

#include <list>

// ======================================================================

struct AuctionData;
struct AuctionDataHeader;
struct AuctionInfo;
class CreatureObject;
class NetworkId;
class ServerObject;
class StringId;
class TangibleObject;
class CommoditiesServerConnection;
class GameNetworkMessage;

// ======================================================================

class CommoditiesMarket // static class
{
public:

	static const std::string OBJVAR_VENDOR_REINITIALIZING;

	typedef int64 AuctionId;
	typedef int32 BidAmount;

	static void install               ();
	static void remove                ();
	static void closeCommoditiesServerConnection();

	static void getCommoditiesServerConnection ();

	static int  getCommoditiesServerConnectionAgeSeconds();
	static bool isCommoditiesServerAvailable();

	static void giveTime              ();
	static void checkPendingLoads     (const NetworkId &id);

	static void auctionCreate         (CreatureObject &owner, ServerObject &item, const Unicode::String &itemLocalizedName, ServerObject &auctionContainer, BidAmount minBid, time_t timer, const Unicode::String &userDescription, bool premium = false);
	static void auctionCreate         (CreatureObject &owner, NetworkId &item, const Unicode::String &itemLocalizedName, NetworkId &auctionContainer, BidAmount minBid, time_t timer, const Unicode::String &userDescription, bool premium = false);

	static void auctionCreatePermanent(const std::string &ownerName, const ServerObject &objectTemplate, const ServerObject &auctionContainer, BidAmount cost, const Unicode::String &userDescription, bool premium = false);

	static void auctionCreateImmediate         (CreatureObject &owner, ServerObject &item, const Unicode::String &itemLocalizedName, ServerObject &auctionContainer, BidAmount price, time_t timer, const Unicode::String &userDescription, bool premium = false);
	static void auctionCreateImmediate         (CreatureObject &owner, NetworkId &item, const Unicode::String &itemLocalizedName, NetworkId &auctionContainer, BidAmount price, time_t timer, const Unicode::String &userDescription, bool premium = false);
	static void transferVendorItemFromStockroom(CreatureObject &owner, NetworkId &item, const Unicode::String &itemLocalizedName, BidAmount price, time_t timer, const Unicode::String &userDescription);

	static void auctionBid            (CreatureObject &bidder, AuctionId auctionId, BidAmount bidAmount, BidAmount maxProxyBid);
	static void auctionCancel         (CreatureObject &who, AuctionId auctionId);
	static void auctionCancel         (const NetworkId &playerId, AuctionId auctionId);
	static void auctionAccept         (CreatureObject &who, AuctionId auctionId);
	static void auctionQueryHeaders   (CreatureObject &who, int requestId, int type, int category, bool categoryExactMatch, int itemTemplateId, const Unicode::String &textFilterAll, const Unicode::String &textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, const NetworkId &container, int locatioNSearchType, bool searchMyVendorsOnly, uint16 queryOffset);
	static void auctionRetrieve       (CreatureObject &who, AuctionId auctionId, const NetworkId &itemId, ServerObject &auctionContainer);
	static void getAuctionDetails     (CreatureObject &who, const NetworkId &itemId);
	static void createVendorMarket    (const CreatureObject &who, ServerObject &container, int entranceCharge);
	static void destroyVendorMarket   (const NetworkId &playerId, ServerObject &container);
	static void isVendorOwner         (CreatureObject &who, const NetworkId &containerId);
	static void getVendorValue        (ServerObject &containerId);
	static void setSalesTax           (int32 salesTax, const NetworkId &bankId, ServerObject &auctionContainer);

	static void reinitializeVendor    (ServerObject &vendor, const NetworkId &player);
	static void reinitializeVendorContents(ServerObject &vendor);
	static bool restoreItem           (ServerObject& item, TangibleObject & vendor);

	static void requestVendorItemCount(const NetworkId& vendorId);
	static void vendorStatusChange(const NetworkId& vendorId, int status);
	static void vendorItemCountReply(const NetworkId& vendorId, int count, bool searchEnabled);
	static void requestPlayerVendorCount(const NetworkId& playerId);
	static void playerVendorCountReply(const NetworkId& playerId, int vendorCount, const std::vector<NetworkId> vendorList);
	static void updateVendorSearchOption(const NetworkId& vendorId, bool enabled);
	static void deleteAuctionLocation(const NetworkId& locationId, const std::string& whoRequested);
	static void setEntranceCharge(const NetworkId& vendorId, int entranceCharge);
	static void updateVendorStatus(const NetworkId & vendorId, int status);
	static void updateVendorLocation(const ServerObject & vendor);

	static void updateItemTypeMap(int itemTypeMapVersionNumber, const std::map<int, std::set<int> > & itemTypeMap);
	static void updateItemTypeMap(int itemTypeMapVersionNumber, int itemType, int itemTemplateId);
	static const std::map<int, std::map<int, std::pair<int, StringId> > > & getItemTypeMap();
	static int  getItemTypeMapVersionNumber();

	static void updateResourceTypeMap(int resourceTypeMapVersionNumber, const std::map<int, std::set<std::string> > & resourceTypeMap);
	static void updateResourceTypeMap(int resourceTypeMapVersionNumber, int resourceClassCrc, const std::string & resourceName);
	static const std::map<int, std::set<std::string> > & getResourceTypeMap();
	static int  getResourceTypeMapVersionNumber();

	static void sendToCommoditiesServer(const GameNetworkMessage & message);

	static const std::string & getCommoditiesNamedEscrowAccount();

protected:
	static void onAddBid              (int32 result, const NetworkId &itemId, const NetworkId &newBidPlayerId, const NetworkId &previousBidder, BidAmount newBidMaxProxy, BidAmount previousBid, const NetworkId &location, bool success, const NetworkId &auctionOwnerId, const std::string & ownerName, const std::string & previousBidderName, int itemNameLength, const Unicode::String & itemName, int32 salesTaxAmount, const NetworkId &salesTaxBankId);
	static void onAuctionComplete     (const NetworkId &itemId, const NetworkId &sellerId, const NetworkId &buyerId, BidAmount purchaseAmount, BidAmount bidMaxProxy, const NetworkId &location, bool success, bool immediate, const std::string & locationString, const std::string & ownerName, const std::string & buyerName, int32 itemNameLength, const Unicode::String & itemName, const AuctionId& auctionId, bool sendSellerMail);
	static void onItemExpired         (const NetworkId &itemOwnerId, const NetworkId &itemId, const std::string & ownerName, int itemNameLength, const Unicode::String & itemName, const std::string &locationName, const NetworkId &locationId);
	static void onQueryHeadersReply   (int32 result, int requestId, int typeFlag, int numAuctions, const std::vector<AuctionDataHeader> & auctionData, uint32 queryOffset, bool hasMorePages);
	static void onGetItemDetailsReply (int32 result, int32 requestId, NetworkId playerId, NetworkId itemId, int userDescriptionLength, const Unicode::String & userDescription, int oobLength, const Unicode::String & oobData, std::vector<std::pair<std::string, Unicode::String> > const & attributes);
	static void onGetItemReply        (int32 result, int32 requestId, NetworkId playerId, NetworkId itemId, NetworkId location);

	static void onAddAuction          (int sequence, int32 result, const NetworkId &auctionOwnerId, const std::string &ownerName, const NetworkId &itemId, const NetworkId &vendorId, const std::string &vendorName, const NetworkId &location);
	static void onIsVendorOwner       (const NetworkId & requesterId, const NetworkId & ownerId, const NetworkId &container);
	static void onAcceptHighBid       (int32 result, NetworkId itemId, NetworkId playerId);
	static void onPermanentAuctionPurchased (const NetworkId &ownerId, const NetworkId &buyerID, BidAmount purchaseAmount, const NetworkId &itemId, const NetworkId &location, const std::string & ownerName, const std::string & buyerName, int32 itemNameLength, const Unicode::String & itemName, std::vector<std::pair<std::string, Unicode::String> > const & attributes);
	static void onGetVendorValue      (const NetworkId &location, int32 value);
	static void onVendorRefuseItem    (int32 result, const NetworkId & itemId, const NetworkId & vendorId, const NetworkId & itemOwnerId);
	static void onCancelAuction       (int32 result, const NetworkId & playerId, const NetworkId & itemId);
	static void onCreateVendorMarket  (int32 result, const NetworkId & playerId, const NetworkId & locationId);
	static void onCleanupInvalidItemRetrieval(const NetworkId &itemId, const NetworkId & playerId, const NetworkId &creatorId, int32 reimburseAmt);

	friend class CommoditiesMarketListener;

};

// ======================================================================

#endif // _CommoditiesMarket_H_

