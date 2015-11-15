// ======================================================================
//
// AuctionLocation.h
// copyright (c) 2004 Sony Online Entertainment
//
// (refactorof original code only to the degree to make it work with new)
// (server & database framework ... i.e. none of the logic has changed)
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// ======================================================================

#ifndef AuctionLocation_h
#define AuctionLocation_h

#include "serverNetworkMessages/AuctionBase.h"
#include "Auction.h"

#include <map>
#include <string>
#include "sharedFoundation/NetworkId.h"

#define PUBLIC_OWNER           0

// ======================================================================

class AuctionLocation
{
private:
	AuctionLocation();
	AuctionLocation(const AuctionLocation&);
	AuctionLocation& operator= (const AuctionLocation&);

protected:
	const NetworkId                 m_locationId;
	NetworkId                       m_ownerId;
	std::string                     m_location;
	std::string                     m_locationPlanet;
	std::string                     m_locationRegion;
	int                             m_salesTax;
	NetworkId                       m_salesTaxBankId;
	int                             m_emptyDate;
	int                             m_lastAccessDate;
	int                             m_vendorFirstTimerExpiredAuctionDate; // time that first unsold auction expired by timer since vendor was last accessed
	int                             m_inactiveDate;
	int                             m_nextUpdateTime;
	int                             m_status;
	bool                            m_searchEnabled;
	int                             m_entranceCharge;
	int                             m_auctionItemCount;
	std::map<NetworkId, Auction *>  m_auctions;
	std::map<int, std::map<NetworkId, Auction *> > m_auctionsIndexedByType;
	std::map<int, std::map<NetworkId, Auction *> > m_auctionsIndexedByParentTypeExactMatch;
	std::map<std::pair<int, int>, std::map<NetworkId, Auction *> > m_auctionsIndexedByTemplate;
	std::map<NetworkId, Auction *>  m_auctionsResourceContainer;
	std::map<NetworkId, Auction *>  m_vendorOffers;

public:
	AuctionLocation(const NetworkId & locationId, const std::string & location, const NetworkId & ownerId, int salesTax, const NetworkId & salesTaxBankId, int emptyDate, int lastAccessDate, int inactiveDate, int status, bool searchEnabled, int entranceCharge);
	~AuctionLocation();

	void                            CancelVendorSale(Auction *auction);

	bool                            AddAuction(Auction *auction);
	bool                            RemoveAuction(const NetworkId & itemId);
	bool                            RemoveAuction(Auction *auction);
	Auction *                       GetAuction(const NetworkId & itemId);
	bool                            MatchLocation(const std::string & planet, const std::string & region, const NetworkId & locationId, bool checkLocationInfo, bool searchMyVendorsOnly, bool overrideVendorSearchFlag, const NetworkId & playerId, bool allowSearchVendors, const NetworkId & vendorId, const int searchType) const;

	bool                            IsVendorMarket() const {return m_ownerId.getValue() != PUBLIC_OWNER;}
	bool                            IsOwner(const NetworkId & ownerId) const {return (ownerId == m_ownerId);}
	int                             GetAuctionItemCount() const {return m_auctionItemCount;}

	const NetworkId &               GetLocationId() const {return m_locationId;}
	const NetworkId &               GetOwnerId() const {return m_ownerId;}
	int                             GetEmptyDate() const {return m_emptyDate;}
	int                             GetLastAccessDate() const {return m_lastAccessDate;}
	int                             GetVendorFirstTimerExpiredAuctionDate() const {return m_vendorFirstTimerExpiredAuctionDate;}
	void                            SetVendorFirstTimerExpiredAuctionDate(int vendorFirstTimerExpiredAuctionDate) {m_vendorFirstTimerExpiredAuctionDate = vendorFirstTimerExpiredAuctionDate;}
	int                             GetInactiveDate() const {return m_inactiveDate;}
	int                             GetNextUpdateTime() const {return m_nextUpdateTime;}
	void                            SetNextUpdateTime(int nextUpdateTime);
	bool                            GetSearchEnabled() const {return m_searchEnabled;}
	int                             GetEntranceCharge() const {return m_entranceCharge;}
	void                            Update(int emptyDate, int lastAccessDate, int inactiveDate, int status);

	int                             GetValue() const;

	void                            SetOwnerId(const NetworkId & ownerId);
	void                            SetLocationString( const std::string & newstr );
	void                            SetSalesTax(int salesTax, const NetworkId & bankId);
	void                            SetSearchedEnabled(bool enabled);
	void                            SetEntranceCharge(int entranceCharge) {m_entranceCharge = entranceCharge;}
	int                             GetSalesTax() const {return m_salesTax;};
	const NetworkId &               GetSalesTaxBankId() const {return m_salesTaxBankId;};
	const std::string &             GetLocationString() const {return m_location;}
	const std::string &             GetLocationPlanet() const {return m_locationPlanet;}
	const std::string &             GetLocationRegion() const {return m_locationRegion;}

	std::map<NetworkId, Auction *> & GetAuctions()     {return m_auctions;}
	std::map<NetworkId, Auction *> & GetAuctionsByType(int type);
	std::map<NetworkId, Auction *> & GetAuctionsByParentTypeExactMatch(int type);
	std::map<NetworkId, Auction *> & GetAuctionsByTemplate(int type, int templateId);
	std::map<NetworkId, Auction *> & GetAuctionsResourceContainer() {return m_auctionsResourceContainer;}
	std::map<NetworkId, Auction *> & GetVendorOffers() {return m_vendorOffers;}
	static void                     Initialization();

	int                             GetStatus() const;
	int                             GetFullStatus() const;

	void                            SetStatus(int const status);
	void                            SetFullStatus(int const status, bool const upperBitsOnly);

	bool                            IsPacked() const;
};

#endif

// ======================================================================
