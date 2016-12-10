// ======================================================================
//
// AuctionMarket.h (refactor of original Commodities Market code)
// copyright (c) 2004 Sony Online Entertainment
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// All the Commodities Market logic is part of this class. Changes were
// made to this class only to the degree necessary to make it work with
// the new server infrastructure. In other words ... the logic is
// estentially unchanged and there are still many issues with the logic
// code itself.
//
// ======================================================================

#ifndef AuctionMarket_h
#define AuctionMarket_h

#include "sharedFoundation/NetworkId.h"
#include "Singleton/Singleton.h"

#include "serverNetworkMessages/AcceptHighBidMessage.h"
#include "serverNetworkMessages/AddAuctionMessage.h"
#include "serverNetworkMessages/AddBidMessage.h"
#include "serverNetworkMessages/AddImmediateAuctionMessage.h"
#include "serverNetworkMessages/AuctionBase.h"
#include "serverNetworkMessages/CancelAuctionMessage.h"
#include "serverNetworkMessages/CleanupInvalidItemRetrievalMessage.h"
#include "serverNetworkMessages/CreateVendorMarketMessage.h"
#include "serverNetworkMessages/DeleteAuctionLocationMessage.h"
#include "serverNetworkMessages/DestroyVendorMarketMessage.h"
#include "serverNetworkMessages/GetItemDetailsMessage.h"
#include "serverNetworkMessages/GetItemMessage.h"
#include "serverNetworkMessages/GetPlayerVendorCountMessage.h"
#include "serverNetworkMessages/SetSalesTaxMessage.h"
#include "serverNetworkMessages/GetVendorOwnerMessage.h"
#include "serverNetworkMessages/GetVendorValueMessage.h"
#include "serverNetworkMessages/QueryAuctionHeadersMessage.h"
#include "sharedNetworkMessages/DeleteCharacterMessage.h"
#include "serverNetworkMessages/SetEntranceChargeMessage.h"
#include "serverNetworkMessages/SetGameTimeMessage.h"
#include "serverNetworkMessages/QueryVendorItemCountMessage.h"
#include "serverNetworkMessages/UpdateVendorSearchOptionMessage.h"
#include "serverNetworkMessages/UpdateVendorStatusMessage.h"

#include "Auction.h"
#include "AuctionItem.h"
#include "AuctionLocation.h"

#include <map>
#include <list>
#include <set>
#include <string>
#include <vector>
#include "Unicode.h"

class GameServerConnection;

class AuctionMarket  : public Singleton<AuctionMarket>
{
private:

protected:
	std::map<NetworkId, AuctionLocation *> m_locationIdMap;
	std::map<NetworkId, std::map<NetworkId, AuctionLocation *> > m_playerVendorListMap;

	std::map<NetworkId, AuctionLocation *> m_allBazaar;
	std::map<std::string, std::map<NetworkId, AuctionLocation *> > m_bazaarByPlanet;
	std::map<std::pair<std::string, std::string>, std::map<NetworkId, AuctionLocation *> > m_bazaarByRegion;

	std::map<NetworkId, AuctionLocation *> m_allSearchableVendor;
	std::map<std::string, std::map<NetworkId, AuctionLocation *> > m_searchableVendorByPlanet;
	std::map<std::pair<std::string, std::string>, std::map<NetworkId, AuctionLocation *> > m_searchableVendorByRegion;

	std::map<NetworkId, Auction *>             m_auctions;
	std::map<std::string, int>                 m_auctionsCountByGameObjectType;
	std::set<std::string>                      m_auctionsCountByGameObjectTypeChanged; // clear() by CommodityServerMetricsData
	std::map<NetworkId, int>                   m_auctionCountMap;

	std::map<int, std::set<int> >              m_resourceTreeHierarchy;
	std::map<int, std::set<int> >              m_itemTypeMap;
	int                                        m_itemTypeMapVersionNumber;
	std::map<int, std::set<std::string> >      m_resourceTypeMap;
	int                                        m_resourceTypeMapVersionNumber;
	int                                        m_gameTime;
	bool                                       m_showAllDebugInfo;

	// auctions that are completed during this frame
	// and needs to be processes at the start of the next frame
	// (typically buy now auctions bought during this frame)
	std::vector<NetworkId>                     m_completedAuctions;

	// priority queue of auctions ordered by
	// the auction expiration time
	std::set<std::pair<int, NetworkId> >        m_priorityQueueAuctionTimer;

	// priority queue of auctions ordered by
	// the item's expiration time
	std::set<std::pair<int, NetworkId> >        m_priorityQueueItemTimer;

	// priority queue of auction locations ordered by the time
	// the auction location needs to be checked for status change
	std::set<std::pair<int, NetworkId> >        m_priorityQueueAuctionLocation;

	void              AddAuction(Auction *auction);
	void              DestroyAuction(std::map<NetworkId, Auction *>::iterator & iter);
	void              UpdateLiveAuctions(int gameTime);
	void              DestroyExpiredItems(int gameTime);
	AuctionLocation * CreateLocation(const NetworkId & ownerId, const std::string & locationString, int entranceCharge);
	void              InitializeFromDB();
	bool              FixVendorLocation( const std::string& );
	bool              GetContainerIdString( const std::string&, std::string& );
	NetworkId         GetLocationId(const std::string&);
	void              ModifyAuctionCount  (const NetworkId & playerId, int delta);
	bool              HasOpenAuctionSlots (const NetworkId & playerId);
	void              AddPlayerVendor(const NetworkId & playerId, const NetworkId & vendorId, AuctionLocation * auctionLocation);
	void              RemovePlayerVendor(const NetworkId & playerId, const NetworkId & vendorId);
	int               GetVendorCount(const NetworkId & playerId);
	int               GetItemCount(const NetworkId & playerId);
	void              AddAuctionToPriorityQueue(const Auction & auction);

	void              SingletonInialize();

public:
	AuctionMarket();
	~AuctionMarket();

enum VendorStatusCode
{
	ACTIVE,
	EMPTY,
	UNACCESSED,
	EMPTY_AND_UNACCESSED,
	ENDANGERED,
	REMOVED
};

	void    Update                        (int gameTime);
	void    AddAuction                    (const AddAuctionMessage &message);
	void    AddImmediateAuction           (const AddImmediateAuctionMessage &message);
	void    AddBid                        (const AddBidMessage &message);
	void    CancelAuction                 (const CancelAuctionMessage &message);
	void    AcceptHighBid                 (const AcceptHighBidMessage &message);
	void    QueryAuctionHeaders           (const QueryAuctionHeadersMessage &message);
	void    SetGameTime                   (const SetGameTimeMessage &message);
	void    GetItemDetails                (const GetItemDetailsMessage &message);
	void    GetItem                       (const GetItemMessage &message);
	void    CreateVendorMarket            (const CreateVendorMarketMessage &message);
	void    DestroyVendorMarket           (const DestroyVendorMarketMessage &message);
	void    DeleteAuctionLocation         (const DeleteAuctionLocationMessage &message);
	void    GetVendorOwner                (const GetVendorOwnerMessage &message);
	void    GetVendorValue                (const GetVendorValueMessage &message);
	void    CleanupInvalidItemRetrieval   (const CleanupInvalidItemRetrievalMessage &message);
	void    SetSalesTax                   (const SetSalesTaxMessage &message);
	void    QueryVendorItemCount          (const QueryVendorItemCountMessage &message);
	void    GetPlayerVendorCount          (const GetPlayerVendorCountMessage &message);
	void    UpdateVendorSearchOption      (const UpdateVendorSearchOptionMessage &message);
	void    SetEntranceCharge             (const SetEntranceChargeMessage &message);
	void    DeleteCharacter               (const DeleteCharacterMessage &message);
	void    UpdateVendorStatus            (const UpdateVendorStatusMessage &message);
	void    UpdateVendorLocation          (const NetworkId &locationId, const std::string &locationString);

	void    SendItemTypeMap               (GameServerConnection &gameServerConnection);

	void    AddResourceType               (int resourceClassCrc, const std::string & resourceName);
	void    SendResourceTypeMap           (GameServerConnection &gameServerConnection);

	void    RemoveFromAuctionTimerPriorityQueue(int timer, const NetworkId & item);
	void    AddAuctionToCompletedAuctionsList(const Auction & auction);

	void    onReceiveAuctionLocations     (const NetworkId &locationId, const std::string &locationName, const NetworkId &ownerId, const int salesTax, const NetworkId &salesTaxBankId, const int emptyDate, const int lastAccessDate, const int inactiveDate, const int status, const bool searchEnabled, const int entranceCharge);
	void    onReceiveMarketAuctions       (const NetworkId &itemId, const NetworkId &ownerId, const NetworkId &creatorId, const NetworkId &locationId, const int minBid, const int buyNowPrice, const int auctionTimer, const std::string &oob, const Unicode::String &userDescription, const int category, const int itemTemplateId, const Unicode::String &itemName, const int itemTimer, const int active, const int itemSize);
	void    onReceiveMarketAuctionAttributes(const NetworkId &itemId, const std::string &attributeName, const Unicode::String &attributeValue);
	void    onReceiveMarketAuctionBids    (const NetworkId &itemId,  const NetworkId &bidderId, const int bid, const int maxProxyBid);
	void    printAuctionTables();

	int     getAuctionCount               () const { return m_auctions.size(); }
	int     getLocationCount              () const { return m_locationIdMap.size(); }

	void    OnAddAuction                  (int trackId, int result, int responseId, const NetworkId & itemId, const NetworkId & ownerId, const std::string & ownerName, const NetworkId & vendorId, const std::string & location);
	void    OnAddBid                      (int trackId, int result, int responseId, const NetworkId & ownerId, const NetworkId & itemId, const NetworkId & bidderId, const NetworkId & previousBidderId, int bidAmount, int previousBidAmount, int maxProxyBid, const std::string & location, int itemNameLength, const Unicode::String & itemName, int salesTaxAmount, const NetworkId & salesTaxBankId);
	void    OnCancelAuction               (int trackId, int result, int responseId, const NetworkId & itemId, const NetworkId & playerId, const NetworkId & highBidderId, int highBid, const std::string & location);
	void    OnAcceptHighBid               (int trackId, int result, int responseId, const NetworkId & itemId, const NetworkId & playerId);
	void    OnQueryAuctionHeaders         (int trackId, int result, int responseId, const NetworkId & playerId, int queryType, std::vector<AuctionDataHeader> &auctions, unsigned int queryOffset, bool hasMorePages);
	void    OnGetItemDetails              (int trackId, int result, int responseId, const NetworkId & itemId, const NetworkId & playerId, int userDescriptionLength, const Unicode::String & userDescription, int oobDataLength, const Unicode::String & oobData, std::vector<std::pair<std::string, Unicode::String> > const & attributes);
	void    OnAuctionExpired              (const NetworkId & ownerId, bool sold, int flags, const NetworkId & buyerId, int highBidAmount, const NetworkId & itemId, int highBidMaxProxy, const std::string & location, bool immediate, int itemNameLength, const Unicode::String & itemName, const NetworkId & itemOwnerId, int track_id, bool sendSellerMail);
	void    OnItemExpired                 (const NetworkId & ownerId, const NetworkId & itemId, int itemNameLength, const Unicode::String & itemName, const std::string & locationName, const NetworkId & locationId);
	void    OnGetItem                     (int trackId, int result, int responseId, const NetworkId & itemId, const NetworkId & playerId, const std::string & location);
	void    OnCreateVendorMarket          (int trackId, int result, int responseId, const NetworkId & ownerId, const std::string & location);
	void    OnVendorRefuseItem            (int trackId, int result, int responseId, const NetworkId & itemId, const NetworkId & vendorId, const NetworkId & itemOwnerId);
	void    OnGetVendorOwner              (int trackId, int result, int responseId, const NetworkId & ownerId, const NetworkId & requesterId, const std::string & location);
	void    OnGetVendorValue              (int trackId, int responseId, const std::string & location, int value);
	void    OnPermanentAuctionPurchased   (int trackId, const NetworkId & ownerId, const NetworkId & buyerId, int price, const NetworkId & itemId, const std::string & location, int itemNameLength, const Unicode::String & itemName, std::vector<std::pair<std::string, Unicode::String> > const & attributes);
	void    OnCleanupInvalidItemRetrieval (int trackId, int responseId, const NetworkId & itemId, const NetworkId & playerId, const NetworkId & creatorId, int reimburseAmt);
	void    OnQueryVendorItemCount        (const int responseId, const int trackId, const NetworkId &vendorId, const int itemCount, const bool searchEnabled);
	void    OnGetPlayerVendorCount        (const int responseId, const int trackId, const NetworkId &playerId, const int vendorCount, std::vector<NetworkId> vendorList);
	void    OnVendorStatusChange          (int trackId, const NetworkId &vendorId, int newStatus);

	AuctionLocation & GetLocation         (const std::string & location);
	AuctionLocation & GetLocation         (const NetworkId & location);

	void    VerifyExcludedGotTypes        (std::map<int, std::string> const & excludedGotTypes);
	void    VerifyExcludedResourceClasses (std::set<std::string> const & excludedResourceClasses);
	void    SetResourceTreeHierarchy      (std::map<int, std::set<int>> const & resourceTreeHierarchy);
	bool    HasReceivedResourceTreeHierarchy() const {return !m_resourceTreeHierarchy.empty();}

	std::map<std::string, int> const & getAuctionsCountByGameObjectType() const { return m_auctionsCountByGameObjectType; }
	std::set<std::string> & getAuctionsCountByGameObjectTypeChanged() { return m_auctionsCountByGameObjectTypeChanged; }

	void    getItemAttributeData          (int requestingGameServerId, const NetworkId & requester, const std::string & outputFileName, int gameObjectType, bool exactGameObjectTypeMatch, bool ignoreSearchableAttribute, int throttle) const;
	void    getItemAttributeDataValues    (int requestingGameServerId, const NetworkId & requester, int gameObjectType, bool exactGameObjectTypeMatch, const std::string & attributeName, int throttle) const;
	void    getItemAttribute              (int requestingGameServerId, const NetworkId & requester, const NetworkId & item) const;
	void    getVendorInfoForPlayer        (int requestingGameServerId, const NetworkId & requester, const NetworkId & player, bool godMode);
	void    getAuctionLocationPriorityQueue(int requestingGameServerId, const NetworkId & requester, int count) const;

	void    addAuctionLocationToLocationIndex(const AuctionLocation * auctionLocation);
	void    removeAuctionLocationFromLocationIndex(const AuctionLocation * auctionLocation);

	void    sanityCheckAuctionLocationBeingDestroyed(const AuctionLocation * auctionLocation);

	void    BuildAuctionsSearchableAttributeList();

	void    AddAuctionLocationToPriorityQueue(const AuctionLocation & auctionLocation);
	void    RemoveAuctionLocationFromPriorityQueue(const AuctionLocation & auctionLocation);

	static void getPlanetAndRegionFromLocationString(const std::string &locationName, std::string &planet, std::string &region);

	friend class CommodityServer;

private:
	bool    IsResourceClassDerivedFrom    (int resourceClassCrc, int parentResourceClassCrc);
};

#endif
