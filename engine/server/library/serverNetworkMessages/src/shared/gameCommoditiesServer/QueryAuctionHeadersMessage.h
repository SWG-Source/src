#ifndef QueryAuctionHeadersMessage_h
#define QueryAuctionHeadersMessage_h

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

class QueryAuctionHeadersMessage : public GameNetworkMessage
{

public:
	QueryAuctionHeadersMessage(
		int requestId,
		const NetworkId & playerId,
		const NetworkId & vendorId,
		int queryType,
		int itemType,
		bool itemTypeExactMatch,
		int itemTemplateId,
		const Unicode::String & textFilterAll,
		const Unicode::String & textFilterAny,
		int priceFilterMin,
		int priceFilterMax,
		bool priceFilterIncludesFee,
		const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch,
		AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny,
		const std::string & searchStringPlanet,
		const std::string & searchStringRegion,
		const NetworkId & searchAuctionLocationId,
		bool searchMyVendorsOnly,
		bool overrideVendorSearchFlag,
		unsigned int queryOffset
	);
	QueryAuctionHeadersMessage(Archive::ReadIterator & source);
	~QueryAuctionHeadersMessage();

	const NetworkId &          GetPlayerId() const {return m_playerId.get();}
	const NetworkId &          GetVendorId() const {return m_vendorId.get();}
	int                        GetQueryType() const {return m_queryType.get();}
	int                        GetItemType() const {return m_itemType.get();}
	bool                       GetItemTypeExactMatch() const {return m_itemTypeExactMatch.get();}
	int                        GetItemTemplateId() const {return m_itemTemplateId.get();}

	const Unicode::String &    GetTextFilterAll() const {return m_textFilterAll.get();}
	const Unicode::String &    GetTextFilterAny() const {return m_textFilterAny.get();}

	int                        GetPriceFilterMin() const {return m_priceFilterMin.get();}
	int                        GetPriceFilterMax() const {return m_priceFilterMax.get();}
	bool                       GetPriceFilterIncludesFee() const {return m_priceFilterIncludesFee.get();}
	const std::list<AuctionQueryHeadersMessage::SearchCondition> & getAdvancedSearch() const {return m_advancedSearch.get();}
	AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny getAdvancedSearchMatchAllAny() const {return static_cast<AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny>(m_advancedSearchMatchAllAny.get());}

	const std::string &        GetSearchStringPlanet() const {return m_searchStringPlanet.get();}
	const std::string &        GetSearchStringRegion() const {return m_searchStringRegion.get();}
	const NetworkId &          GetSearchAuctionLocationId() const {return m_searchAuctionLocationId.get();}

	bool                       GetSearchMyVendorsOnly() const {return m_searchMyVendorsOnly.get();}
	bool                       GetOverrideVendorSearchFlag() const {return m_overrideVendorSearchFlag.get();}
	unsigned int               GetQueryOffset() const {return m_queryOffset.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>                    m_responseId;
	Archive::AutoVariable<int>                    m_trackId;
	Archive::AutoVariable<NetworkId>              m_playerId;
	Archive::AutoVariable<NetworkId>              m_vendorId;
	Archive::AutoVariable<int>                    m_queryType;
	Archive::AutoVariable<int>                    m_itemType;
	Archive::AutoVariable<bool>                   m_itemTypeExactMatch;
	Archive::AutoVariable<int>                    m_itemTemplateId;
	Archive::AutoVariable<Unicode::String>        m_textFilterAll;
	Archive::AutoVariable<Unicode::String>        m_textFilterAny;
	Archive::AutoVariable<int>                    m_priceFilterMin;
	Archive::AutoVariable<int>                    m_priceFilterMax;
	Archive::AutoVariable<bool>                   m_priceFilterIncludesFee;
	Archive::AutoList<AuctionQueryHeadersMessage::SearchCondition> m_advancedSearch;
	Archive::AutoVariable<int8>                   m_advancedSearchMatchAllAny;
	Archive::AutoVariable<std::string>            m_searchStringPlanet;
	Archive::AutoVariable<std::string>            m_searchStringRegion;
	Archive::AutoVariable<NetworkId>              m_searchAuctionLocationId;
	Archive::AutoVariable<bool>                   m_searchMyVendorsOnly;
	Archive::AutoVariable<bool>                   m_overrideVendorSearchFlag;
	Archive::AutoVariable<unsigned int>           m_queryOffset;
};

#endif
