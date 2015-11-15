// ======================================================================
//
// Auction.h
// copyright (c) 2004 Sony Online Entertainment
//
// (refactorof original code only to the degree to make it work with new)
// (server & database framework ... i.e. none of the logic has changed)
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// ======================================================================
#ifndef Auction_h
#define Auction_h

#include "serverNetworkMessages/AuctionBase.h"
#include "AuctionItem.h"
#include "AuctionBid.h"

#include <map>
#include <string>
#include <vector>
#include "Unicode.h"

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"

class AuctionLocation;

// ======================================================================

class Auction
{
private:
	Auction();
	Auction(const Auction&);
	Auction& operator= (const Auction&);

protected:
	const NetworkId             m_creatorId;
	const int                   m_minBid;
	int                         m_auctionTimer;
	const int                   m_buyNowPrice;
	const int                   m_userDescriptionLength;
	const Unicode::String       m_userDescription;
	const int                   m_oobLength;
	const Unicode::String       m_oobData;
	std::vector<std::pair<std::string, Unicode::String> > m_attributes;
	std::map<uint32, int> *     m_searchableAttributeInt;
	std::map<uint32, double> *  m_searchableAttributeFloat;
	std::map<uint32, std::string> * m_searchableAttributeString;
	AuctionBid *                m_highBid;
	AuctionItem * const         m_item;

	bool                            m_sold;                    
	bool                            m_active;
	std::vector<AuctionBid *>       m_bids;
	AuctionLocation &               m_location; 
	const int                       m_flags;

	int                             m_trackId;

	int                         GetActualBid(AuctionBid *bid);

public:
	Auction(
		const NetworkId &        creatorId,
		int                      minBid,
		int                      auctionTimer,
		const NetworkId &        itemId,
		int                      itemNameLength,
		const Unicode::String &  itemName,
		int                      itemType,
		int                      itemTemplateId,
		int                      itemTimer,
		int                      itemSize,
		const AuctionLocation &  location,
		int                      flags,
		int                      buyNowPrice,
		int                      userDescriptionLength,
		const Unicode::String &  userDescription,
		int                      oobLength,
		const Unicode::String &  oobData,
		std::vector<std::pair<std::string, Unicode::String> > const & attributes
	);

	Auction(
		const NetworkId &        creatorId,
		int                      minBid,
		int                      auctionTimer,
		const NetworkId &        itemId,
		int                      itemNameLength,
		const Unicode::String &  itemName,
		int                      itemType,
		int                      itemTemplateId,
		int                      itemTimer,
		int                      itemSize,
		const AuctionLocation &  location,
		int                      flags,
		int                      buyNowPrice,
		int                      userDescriptionLength,
		const Unicode::String &  userDescription,
		int                      oobLength,
		const Unicode::String &  oobData,
		std::vector<std::pair<std::string, Unicode::String> > const & attributes,
		bool                     isActive,
		bool                     isSold
	);

	~Auction();

	static void                Initialization();

	const NetworkId &          GetCreatorId() const {return m_creatorId;}
	int                        GetMinBid() const {return m_minBid;}
	int                        GetAuctionTimer() const {return m_auctionTimer;}
	int                        GetBuyNowPrice() const {return m_buyNowPrice;}
	int                        GetBuyNowPriceWithSalesTax() const;
	int                        GetUserDescriptionLength() const {return m_userDescriptionLength;}
	const Unicode::String &    GetUserDescription() const {return m_userDescription;}
	int                        GetOobLength() const {return m_oobLength;}
	const Unicode::String &    GetOobData() const {return m_oobData;}
	std::vector<std::pair<std::string, Unicode::String> > const & GetAttributes() const {return m_attributes;}
	void                       GetAttributes(std::string & output) const;
	void                       AddAttributes(const std::string & attributeName, const Unicode::String & attributeValue);
	const AuctionBid *         GetHighBid() const {return m_highBid;}
	int                        GetHighBidAmount() const;
	const AuctionBid *         GetPreviousBid() const;
	const AuctionBid *         GetPlayerBid(const NetworkId & playerId) const;
	AuctionItem &              GetItem() const {return *m_item;}
	bool                       IsSold() const {return m_sold;}
	bool                       IsActive() const {return m_active;}
	bool                       IsImmediate() const {return m_buyNowPrice > 0;}
	int                        GetFlags() const {return m_flags;}

	AuctionResultCode          AddBid(const NetworkId & bidderId, int bid, int maxProxyBid);
	void                       AddLoadedBid(const NetworkId & bidderId, int bid, int maxProxyBid);

	bool                       Update(int gameTime);
	void                       Expire(bool sold, bool expiredBasedOnTimer, int track_id = -1);

	AuctionLocation &          GetLocation() const {return m_location;}

	void                       SetTrackId(int trackId) {m_trackId = trackId;}

	void                       BuildSearchableAttributeList();

	bool                       IsAdvancedFilterMatch(std::list<AuctionQueryHeadersMessage::SearchCondition> const & advancedSearch, int matchAllAny) const;

private:
	void                       SetItemResourceContainerClassCrc();
	void                       SetItemResourceContainerClassCrc(const std::string & attributeName, const Unicode::String & attributeValue);
};

#endif

// ======================================================================
