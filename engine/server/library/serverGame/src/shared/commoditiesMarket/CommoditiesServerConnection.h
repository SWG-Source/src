// ======================================================================
//
// CommoditiesServerConnection.h (refactor of original Commodities Market code)
// The old CM API was depricated and all code for comminicating with the
// CM Server now uses this class
//
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CommoditiesServerConnection_H
#define	_CommoditiesServerConnection_H

#include "serverGame/CommoditiesMarketListener.h"
#include "serverUtility/ServerConnection.h"
#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"

#include <list>

// ======================================================================

class CommoditiesServerConnection : public ServerConnection
{
public:
	CommoditiesServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~CommoditiesServerConnection();

	void  onConnectionClosed();
	void  onConnectionOpened();
	void  onReceive(const Archive::ByteStream & message);

	int   SetGameTime(time_t gameTime);
	int   CancelAuction(const NetworkId & auctionId, const NetworkId & playerId);
	int   AddAuction(const NetworkId &  ownerId, const std::string & ownerName, int  minBid, time_t timer, const NetworkId &  itemId, int itemNameLength, const Unicode::String & itemName, int  itemType, int  itemTemplateId, time_t itemTimer, const NetworkId &  locationId,  const std::string &  location, int flags, int userDescriptionLength, const Unicode::String & userDescription, std::vector<std::pair<std::string, Unicode::String> > const & attributes, int itemSize, int vendorLimit, int vendorItemLimit);
	int   AddImmediateAuction(const NetworkId &  ownerId, const std::string & ownerName, int  price, time_t timer, const NetworkId &  itemId, int itemNameLength, const Unicode::String & itemName, int  itemType, int  itemTemplateId, time_t itemTimer, const NetworkId &  locationId, const std::string &  location, int  flags, int userDescriptionLength, const Unicode::String & userDescription, std::vector<std::pair<std::string, Unicode::String> > const & attributes, int itemSize, int vendorLimit, int vendorItemLimit);
	int   AddBid(const NetworkId & auctionId, const NetworkId & playerId, const std::string & playerName, int bidAmount, int maxProxyBid);
	int   AcceptHighBid(const NetworkId & auctionId, const NetworkId & playerId);
	int   QueryAuctionHeaders(int requestId, const NetworkId & playerId, const NetworkId & vendorId, int queryType, int itemType, bool itemTypeExactMatch, int itemTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, const std::string & searchStringPlanet, const std::string & searchStringRegion, const NetworkId & searchAuctionLocationId, bool searchMyVendorsOnly, bool overrideVendorSearchFlag, int queryOffset);
	int   GetItemDetails(int requestId, const NetworkId & playerId, const NetworkId & itemId);
	int   GetVendorValue(const std::string & location);
	int   CreateVendorMarket(const NetworkId & ownerId, const std::string & location, int playerVendorLimit, int entranceCharge);
	int   DestroyVendorMarket(const NetworkId & ownerId, const std::string & ownerName, const std::string & location);
	int   deleteAuctionLocation(const NetworkId & locationId, const std::string & whoRequested);
	int   GetVendorOwner(const NetworkId & requesterId, const std::string & location);
	int   GetItem(const NetworkId & itemId, const NetworkId & playerId, const std::string & location);
	int   CleanupInvalidItemRetrieval( const NetworkId & itemId);
	int   SetSalesTax(int salesTax, const NetworkId & bankId, const std::string & location);
	int   GetPlayerVendorCount(const NetworkId & playerId);
	int   GiveTime();
	

private:
    // request tracking
	int                             m_nextRequest;
	CommoditiesMarketListener       listener;
	bool                            m_commoditiesShowAllDebugInfo;


	CommoditiesServerConnection(const CommoditiesServerConnection&);
	CommoditiesServerConnection& operator= (const CommoditiesServerConnection&);

}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _CommoditiesServerConnection_H


