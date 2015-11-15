// ======================================================================
//
// AuctionQueryHeadersMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, AuctionQueryHeadersMessage::SearchCondition & target)
	{
		get(source, target.attributeNameCrc);
		get(source, target.requiredAttribute);

		int8 temp;
		get(source, temp);
		target.comparison = static_cast<AuctionQueryHeadersMessage::SearchConditionComparison>(temp);

		if (target.comparison == AuctionQueryHeadersMessage::SCC_int)
		{
			get(source, target.intMin);
			get(source, target.intMax);
		}
		else if (target.comparison == AuctionQueryHeadersMessage::SCC_float)
		{
			get(source, target.floatMin);
			get(source, target.floatMax);
		}
		else if ((target.comparison == AuctionQueryHeadersMessage::SCC_string_equal) ||
		         (target.comparison == AuctionQueryHeadersMessage::SCC_string_not_equal) ||
		         (target.comparison == AuctionQueryHeadersMessage::SCC_string_contain) ||
		         (target.comparison == AuctionQueryHeadersMessage::SCC_string_not_contain))
		{
			get(source, target.stringValue);
		}
		else // error
		{
			target.comparison = AuctionQueryHeadersMessage::SCC_int;
		}
	}

	void put(ByteStream & target, const AuctionQueryHeadersMessage::SearchCondition & source)
	{
		put(target, source.attributeNameCrc);
		put(target, source.requiredAttribute);
		put(target, static_cast<int8>(source.comparison));

		if (source.comparison == AuctionQueryHeadersMessage::SCC_int)
		{
			put(target, source.intMin);
			put(target, source.intMax);
		}
		else if (source.comparison == AuctionQueryHeadersMessage::SCC_float)
		{
			put(target, source.floatMin);
			put(target, source.floatMax);
		}
		else if ((source.comparison == AuctionQueryHeadersMessage::SCC_string_equal) ||
		         (source.comparison == AuctionQueryHeadersMessage::SCC_string_not_equal) ||
		         (source.comparison == AuctionQueryHeadersMessage::SCC_string_contain) ||
		         (source.comparison == AuctionQueryHeadersMessage::SCC_string_not_contain))
		{
			put(target, source.stringValue);
		}
	}
}

// ======================================================================

AuctionQueryHeadersMessage::AuctionQueryHeadersMessage(
	AuctionLocationSearch locationSearchType,
	const NetworkId & container,
	int requestId,
	AuctionSearchType searchType,
	int itemType,
	bool itemTypeExactMatch,
	int itemTemplateId,
	const Unicode::String & textFilterAll,
	const Unicode::String & textFilterAny,
	int priceFilterMin,
	int priceFilterMax,
	bool priceFilterIncludesFee,
	const std::list<SearchCondition> & advancedSearch,
	AdvancedSearchMatchAllAny advancedSearchMatchAllAny,
	bool myVendorsOnly,
	uint16 queryOffset
):
	GameNetworkMessage("AuctionQueryHeadersMessage"),
	m_locationSearchType(static_cast<int>(locationSearchType)),
	m_requestId(requestId),
	m_searchType(static_cast<int>(searchType)),
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
	m_container(container),
	m_myVendorsOnly(myVendorsOnly),
	m_queryOffset(queryOffset)
{
	m_advancedSearch.set(advancedSearch);

	AutoByteStream::addVariable(m_locationSearchType);
	AutoByteStream::addVariable(m_requestId);
	AutoByteStream::addVariable(m_searchType);
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
	AutoByteStream::addVariable(m_container);
	AutoByteStream::addVariable(m_myVendorsOnly);
	AutoByteStream::addVariable(m_queryOffset);
}

// ======================================================================

AuctionQueryHeadersMessage::AuctionQueryHeadersMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("AuctionQueryHeadersMessage"),
	m_locationSearchType(),
	m_requestId(),
	m_searchType(),
	m_itemType(),
	m_itemTypeExactMatch(),
	m_itemTemplateId(),
	m_textFilterAll(),
	m_textFilterAny(),
	m_priceFilterMin(),
	m_priceFilterMax(),
	m_priceFilterIncludesFee(),
	m_advancedSearch(),
	m_advancedSearchMatchAllAny(static_cast<int8>(ASMAA_match_all)),
	m_container(),
	m_myVendorsOnly(),
	m_queryOffset(0)
{
	AutoByteStream::addVariable(m_locationSearchType);
	AutoByteStream::addVariable(m_requestId);
	AutoByteStream::addVariable(m_searchType);
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
	AutoByteStream::addVariable(m_container);
	AutoByteStream::addVariable(m_myVendorsOnly);
	AutoByteStream::addVariable(m_queryOffset);

	unpack(source);

	// error handling (against hacked client)
	if ((m_advancedSearchMatchAllAny.get() < 0) || (m_advancedSearchMatchAllAny.get() >= static_cast<int8>(ASMAA_LAST)))
		m_advancedSearchMatchAllAny.set(static_cast<int8>(ASMAA_match_all));
}

// ======================================================================

AuctionQueryHeadersMessage::~AuctionQueryHeadersMessage()
{
}
