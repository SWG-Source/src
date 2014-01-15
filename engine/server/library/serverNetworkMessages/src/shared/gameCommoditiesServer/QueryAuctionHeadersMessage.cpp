#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "QueryAuctionHeadersMessage.h"

QueryAuctionHeadersMessage::QueryAuctionHeadersMessage(Archive::ReadIterator & source) :
GameNetworkMessage("QueryAuctionHeadersMessage"),
m_responseId(),
m_trackId(),
m_playerId(),
m_vendorId(),
m_queryType(),
m_itemType(),
m_itemTypeExactMatch(),
m_itemTemplateId(),
m_textFilterAll(),
m_textFilterAny(),
m_priceFilterMin(),
m_priceFilterMax(),
m_priceFilterIncludesFee(),
m_advancedSearch(),
m_advancedSearchMatchAllAny(static_cast<int8>(AuctionQueryHeadersMessage::ASMAA_match_all)),
m_searchStringPlanet(),
m_searchStringRegion(),
m_searchAuctionLocationId(),
m_searchMyVendorsOnly(),
m_overrideVendorSearchFlag(),
m_queryOffset()
{
	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_queryType);
	AutoByteStream::addVariable(m_itemType);
	AutoByteStream::addVariable(m_itemTypeExactMatch);
	AutoByteStream::addVariable(m_itemTemplateId);
	AutoByteStream::addVariable(m_textFilterAll);
	AutoByteStream::addVariable(m_textFilterAny);
	AutoByteStream::addVariable(m_priceFilterMin);
	AutoByteStream::addVariable(m_priceFilterMax);
	AutoByteStream::addVariable(m_priceFilterIncludesFee);
	AutoByteStream::addVariable(m_advancedSearch);
	AutoByteStream::addVariable(m_advancedSearchMatchAllAny);
	AutoByteStream::addVariable(m_searchStringPlanet);
	AutoByteStream::addVariable(m_searchStringRegion);
	AutoByteStream::addVariable(m_searchAuctionLocationId);
	AutoByteStream::addVariable(m_searchMyVendorsOnly);
	AutoByteStream::addVariable(m_overrideVendorSearchFlag);
	AutoByteStream::addVariable(m_queryOffset);
	unpack(source);
}

QueryAuctionHeadersMessage::QueryAuctionHeadersMessage(
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
) :
GameNetworkMessage("QueryAuctionHeadersMessage"),
m_responseId(requestId),
m_trackId(0),
m_playerId(playerId),
m_vendorId(vendorId),
m_queryType(queryType),
m_itemType(itemType),
m_itemTypeExactMatch(itemTypeExactMatch),
m_itemTemplateId(itemTemplateId),
m_textFilterAll(textFilterAll),
m_textFilterAny(textFilterAny),
m_priceFilterMin(priceFilterMin),
m_priceFilterMax(priceFilterMax),
m_priceFilterIncludesFee(priceFilterIncludesFee),
m_advancedSearch(),
m_advancedSearchMatchAllAny(static_cast<int8>(advancedSearchMatchAllAny)),
m_searchStringPlanet(searchStringPlanet),
m_searchStringRegion(searchStringRegion),
m_searchAuctionLocationId(searchAuctionLocationId),
m_searchMyVendorsOnly(searchMyVendorsOnly),
m_overrideVendorSearchFlag(overrideVendorSearchFlag),
m_queryOffset(queryOffset)
{
	m_advancedSearch.set(advancedSearch);

	AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_playerId);
	AutoByteStream::addVariable(m_vendorId);
	AutoByteStream::addVariable(m_queryType);
	AutoByteStream::addVariable(m_itemType);
	AutoByteStream::addVariable(m_itemTypeExactMatch);
	AutoByteStream::addVariable(m_itemTemplateId);
	AutoByteStream::addVariable(m_textFilterAll);
	AutoByteStream::addVariable(m_textFilterAny);
	AutoByteStream::addVariable(m_priceFilterMin);
	AutoByteStream::addVariable(m_priceFilterMax);
	AutoByteStream::addVariable(m_priceFilterIncludesFee);
	AutoByteStream::addVariable(m_advancedSearch);
	AutoByteStream::addVariable(m_advancedSearchMatchAllAny);
	AutoByteStream::addVariable(m_searchStringPlanet);
	AutoByteStream::addVariable(m_searchStringRegion);
	AutoByteStream::addVariable(m_searchAuctionLocationId);
	AutoByteStream::addVariable(m_searchMyVendorsOnly);
	AutoByteStream::addVariable(m_overrideVendorSearchFlag);
	AutoByteStream::addVariable(m_queryOffset);
}

QueryAuctionHeadersMessage::~QueryAuctionHeadersMessage()
{
}
