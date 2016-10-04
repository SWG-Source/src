// ======================================================================
//
// Auction.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// (refactor of original code only to the degree to make it work with new)
// (server & database framework ... i.e. none of the logic has changed)
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// ======================================================================

#include "FirstCommodityServer.h"
#include "Auction.h"
#include "AuctionMarket.h"
#include "CommodityServer.h"
#include "ConfigCommodityServer.h"
#include "StringId.h"
#include "UnicodeUtils.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedGame/CommoditiesAdvancedSearchAttribute.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedLog/Log.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <algorithm>

// src/engine/client/library/clientGame/src/shared/core/AuctionManagerClient.cpp (s_maxBid)
// src/engine/server/application/CommoditiesServer/src/shared/Auction.cpp (MAX_BID)
// src/engine/server/library/serverGame/src/shared/commoditiesMarket/CommoditiesMarket.cpp (MAX_BID)
#define MAX_BID 10000000
#define MAX_VENDOR_PRICE 833333333

namespace AuctionNamespace
{
	typedef bool (*SearchConditionComparisonFn) (AuctionQueryHeadersMessage::SearchCondition const & /*searchCondition*/,
		std::map<uint32, int> const * /*searchableAttributeInt*/,
		std::map<uint32, double> const * /*searchableAttributeFloat*/,
		std::map<uint32, std::string> const * /*searchableAttributeString*/,
		bool & /*applicableSearchCondition*/);

	SearchConditionComparisonFn s_searchConditionComparisonFn[static_cast<int>(AuctionQueryHeadersMessage::SCC_LAST)];

	// ----------------------------------------------------------------------

	bool searchConditionComparisonFnInt(AuctionQueryHeadersMessage::SearchCondition const & searchCondition,
		std::map<uint32, int> const * searchableAttributeInt,
		std::map<uint32, double> const * searchableAttributeFloat,
		std::map<uint32, std::string> const * searchableAttributeString,
		bool & applicableSearchCondition)
	{
#ifdef _DEBUG
		DEBUG_FATAL((searchCondition.comparison != AuctionQueryHeadersMessage::SCC_int), ("searchConditionComparisonFnInt() called for attributeNameCrc=(%lu), comparison=(%d) that is not AuctionQueryHeadersMessage::SCC_int", searchCondition.attributeNameCrc, static_cast<int>(searchCondition.comparison)));
		DEBUG_REPORT_LOG(ConfigCommodityServer::getShowAllDebugInfo(), ("[Commodities Server QueryAuctionHeadersMessage] searchConditionComparisonFnInt\n"));
#endif
		UNREF(searchableAttributeFloat);
		UNREF(searchableAttributeString);

		// if the filter condition requires the attribute,
		// it will always be an applicable filter condition;
		// otherwise, it will only be applicable if the item
		// has the attribute
		applicableSearchCondition = searchCondition.requiredAttribute;

		if (searchableAttributeInt)
		{
			std::map<uint32, int>::const_iterator iterAttr = searchableAttributeInt->find(searchCondition.attributeNameCrc);
			if (iterAttr != searchableAttributeInt->end())
			{
				applicableSearchCondition = true;
				return ((iterAttr->second >= searchCondition.intMin) && (iterAttr->second <= searchCondition.intMax));
			}
		}

		return false;
	}

	// ----------------------------------------------------------------------

	bool searchConditionComparisonFnFloat(AuctionQueryHeadersMessage::SearchCondition const & searchCondition,
		std::map<uint32, int> const * searchableAttributeInt,
		std::map<uint32, double> const * searchableAttributeFloat,
		std::map<uint32, std::string> const * searchableAttributeString,
		bool & applicableSearchCondition)
	{
#ifdef _DEBUG
		DEBUG_FATAL((searchCondition.comparison != AuctionQueryHeadersMessage::SCC_float), ("searchConditionComparisonFnFloat() called for attributeNameCrc=(%lu), comparison=(%d) that is not AuctionQueryHeadersMessage::SCC_float", searchCondition.attributeNameCrc, static_cast<int>(searchCondition.comparison)));
		DEBUG_REPORT_LOG(ConfigCommodityServer::getShowAllDebugInfo(), ("[Commodities Server QueryAuctionHeadersMessage] searchConditionComparisonFnFloat\n"));
#endif
		UNREF(searchableAttributeInt);
		UNREF(searchableAttributeString);

		// if the filter condition requires the attribute,
		// it will always be an applicable filter condition;
		// otherwise, it will only be applicable if the item
		// has the attribute
		applicableSearchCondition = searchCondition.requiredAttribute;

		if (searchableAttributeFloat)
		{
			std::map<uint32, double>::const_iterator iterAttr = searchableAttributeFloat->find(searchCondition.attributeNameCrc);
			if (iterAttr != searchableAttributeFloat->end())
			{
				applicableSearchCondition = true;
				return ((iterAttr->second >= searchCondition.floatMin) && (iterAttr->second <= searchCondition.floatMax));
			}
		}

		return false;
	}

	// ----------------------------------------------------------------------

	bool searchConditionComparisonFnStringEqual(AuctionQueryHeadersMessage::SearchCondition const & searchCondition,
		std::map<uint32, int> const * searchableAttributeInt,
		std::map<uint32, double> const * searchableAttributeFloat,
		std::map<uint32, std::string> const * searchableAttributeString,
		bool & applicableSearchCondition)
	{
#ifdef _DEBUG
		DEBUG_FATAL((searchCondition.comparison != AuctionQueryHeadersMessage::SCC_string_equal), ("searchConditionComparisonFnStringEqual() called for attributeNameCrc=(%lu), comparison=(%d) that is not AuctionQueryHeadersMessage::SCC_string_equal", searchCondition.attributeNameCrc, static_cast<int>(searchCondition.comparison)));
		DEBUG_REPORT_LOG(ConfigCommodityServer::getShowAllDebugInfo(), ("[Commodities Server QueryAuctionHeadersMessage] searchConditionComparisonFnStringEqual\n"));
#endif
		UNREF(searchableAttributeInt);
		UNREF(searchableAttributeFloat);

		// if the filter condition requires the attribute,
		// it will always be an applicable filter condition;
		// otherwise, it will only be applicable if the item
		// has the attribute
		applicableSearchCondition = searchCondition.requiredAttribute;

		if (searchableAttributeString)
		{
			std::map<uint32, std::string>::const_iterator iterAttr = searchableAttributeString->find(searchCondition.attributeNameCrc);
			if (iterAttr != searchableAttributeString->end())
			{
				applicableSearchCondition = true;
				return (0 == strcmp(iterAttr->second.c_str(), searchCondition.stringValue.c_str())); // lowercase is assumed (and required)
			}
		}

		return false;
	}

	// ----------------------------------------------------------------------

	bool searchConditionComparisonFnStringNotEqual(AuctionQueryHeadersMessage::SearchCondition const & searchCondition,
		std::map<uint32, int> const * searchableAttributeInt,
		std::map<uint32, double> const * searchableAttributeFloat,
		std::map<uint32, std::string> const * searchableAttributeString,
		bool & applicableSearchCondition)
	{
#ifdef _DEBUG
		DEBUG_FATAL((searchCondition.comparison != AuctionQueryHeadersMessage::SCC_string_not_equal), ("searchConditionComparisonFnStringNotEqual() called for attributeNameCrc=(%lu), comparison=(%d) that is not AuctionQueryHeadersMessage::SCC_string_not_equal", searchCondition.attributeNameCrc, static_cast<int>(searchCondition.comparison)));
		DEBUG_REPORT_LOG(ConfigCommodityServer::getShowAllDebugInfo(), ("[Commodities Server QueryAuctionHeadersMessage] searchConditionComparisonFnStringNotEqual\n"));
#endif
		UNREF(searchableAttributeInt);
		UNREF(searchableAttributeFloat);

		// if the filter condition requires the attribute,
		// it will always be an applicable filter condition;
		// otherwise, it will only be applicable if the item
		// has the attribute
		applicableSearchCondition = searchCondition.requiredAttribute;

		if (searchableAttributeString)
		{
			std::map<uint32, std::string>::const_iterator iterAttr = searchableAttributeString->find(searchCondition.attributeNameCrc);
			if (iterAttr != searchableAttributeString->end())
			{
				applicableSearchCondition = true;
				return (0 != strcmp(iterAttr->second.c_str(), searchCondition.stringValue.c_str())); // lowercase is assumed (and required)
			}
		}

		return false;
	}

	// ----------------------------------------------------------------------

	bool searchConditionComparisonFnStringContain(AuctionQueryHeadersMessage::SearchCondition const & searchCondition,
		std::map<uint32, int> const * searchableAttributeInt,
		std::map<uint32, double> const * searchableAttributeFloat,
		std::map<uint32, std::string> const * searchableAttributeString,
		bool & applicableSearchCondition)
	{
#ifdef _DEBUG
		DEBUG_FATAL((searchCondition.comparison != AuctionQueryHeadersMessage::SCC_string_contain), ("searchConditionComparisonFnStringContain() called for attributeNameCrc=(%lu), comparison=(%d) that is not AuctionQueryHeadersMessage::SCC_string_contain", searchCondition.attributeNameCrc, static_cast<int>(searchCondition.comparison)));
		DEBUG_REPORT_LOG(ConfigCommodityServer::getShowAllDebugInfo(), ("[Commodities Server QueryAuctionHeadersMessage] searchConditionComparisonFnStringContain\n"));
#endif
		UNREF(searchableAttributeInt);
		UNREF(searchableAttributeFloat);

		// if the filter condition requires the attribute,
		// it will always be an applicable filter condition;
		// otherwise, it will only be applicable if the item
		// has the attribute
		applicableSearchCondition = searchCondition.requiredAttribute;

		if (searchableAttributeString)
		{
			std::map<uint32, std::string>::const_iterator iterAttr = searchableAttributeString->find(searchCondition.attributeNameCrc);
			if (iterAttr != searchableAttributeString->end())
			{
				applicableSearchCondition = true;
				return (nullptr != strstr(iterAttr->second.c_str(), searchCondition.stringValue.c_str())); // lowercase is assumed (and required)
			}
		}

		return false;
	}

	// ----------------------------------------------------------------------

	bool searchConditionComparisonFnStringNotContain(AuctionQueryHeadersMessage::SearchCondition const & searchCondition,
		std::map<uint32, int> const * searchableAttributeInt,
		std::map<uint32, double> const * searchableAttributeFloat,
		std::map<uint32, std::string> const * searchableAttributeString,
		bool & applicableSearchCondition)
	{
#ifdef _DEBUG
		DEBUG_FATAL((searchCondition.comparison != AuctionQueryHeadersMessage::SCC_string_not_contain), ("searchConditionComparisonFnStringNotContain() called for attributeNameCrc=(%lu), comparison=(%d) that is not AuctionQueryHeadersMessage::SCC_string_not_contain", searchCondition.attributeNameCrc, static_cast<int>(searchCondition.comparison)));
		DEBUG_REPORT_LOG(ConfigCommodityServer::getShowAllDebugInfo(), ("[Commodities Server QueryAuctionHeadersMessage] searchConditionComparisonFnStringNotContain\n"));
#endif
		UNREF(searchableAttributeInt);
		UNREF(searchableAttributeFloat);

		// if the filter condition requires the attribute,
		// it will always be an applicable filter condition;
		// otherwise, it will only be applicable if the item
		// has the attribute
		applicableSearchCondition = searchCondition.requiredAttribute;

		if (searchableAttributeString)
		{
			std::map<uint32, std::string>::const_iterator iterAttr = searchableAttributeString->find(searchCondition.attributeNameCrc);
			if (iterAttr != searchableAttributeString->end())
			{
				applicableSearchCondition = true;
				return (nullptr == strstr(iterAttr->second.c_str(), searchCondition.stringValue.c_str())); // lowercase is assumed (and required)
			}
		}

		return false;
	}

	// ----------------------------------------------------------------------

	// <stop?, search return value>
	std::pair<bool, bool> s_searchConditionMatchAllAnyActionAnyMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_LAST)];
	std::pair<bool, bool> s_searchConditionMatchAllAnyActionAnyNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_LAST)];

	// search return value
	bool s_searchConditionMatchAllAnyActionAllMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_LAST)];
	bool s_searchConditionMatchAllAnyActionAllNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_LAST)];
};

using namespace AuctionNamespace;

// ======================================================================

Auction::Auction(
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
) :
m_creatorId(creatorId),
m_minBid(minBid),
m_auctionTimer(auctionTimer),
m_buyNowPrice(buyNowPrice),
m_userDescriptionLength(userDescriptionLength),
m_userDescription(userDescription),
m_oobLength(oobLength),
m_oobData(oobData),
m_attributes(),
m_searchableAttributeInt(nullptr),
m_searchableAttributeFloat(nullptr),
m_searchableAttributeString(nullptr),
m_highBid(nullptr),
m_item(new AuctionItem(itemId, itemType, itemTemplateId, itemTimer, itemNameLength, itemName, creatorId, itemSize)),
m_sold(false),
m_active(true),
m_bids(),
m_location(const_cast<AuctionLocation &>(location)),
m_flags(flags),
m_trackId(-1)
{
	assert(m_auctionTimer != 0);

	// it it's a instant sale resource container, add "credits per unit" attribute
	if (AuctionItem::IsCategoryResourceContainer(m_item->GetCategory()) && (m_buyNowPrice > 0))
	{
		for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
		{
			if (iter->first == "credits_per_unit")
			{
				// ignore any existing credits per unit attribute; we'll recalculate it and add the updated attribute
				continue;
			}

			m_attributes.push_back(*iter);

			if (iter->first == "resource_contents")
			{
				int count = ::atoi(Unicode::wideToNarrow(iter->second).c_str());
				if (count > 0)
				{
					char buffer[128];
					int const len = snprintf(buffer, sizeof(buffer)-1, "%.6f", (static_cast<double>(m_buyNowPrice) / static_cast<double>(count)));
					buffer[sizeof(buffer)-1] = '\0';

					// trim trailing 0s after the decimal point
					char * pChar = buffer + len - 1;
					while (pChar >= buffer)
					{
						if (*pChar == '.')
						{
							*pChar = '\0';
							break;
						}
						else if (*pChar != '0')
						{
							++pChar;
							*pChar = '\0';
							break;
						}

						--pChar;
					}

					m_attributes.push_back(std::make_pair("credits_per_unit", Unicode::narrowToWide(buffer)));
				}
			}
		}
	}
	else
	{
		m_attributes = attributes;
	}

	SetItemResourceContainerClassCrc();
	BuildSearchableAttributeList();
}

// ----------------------------------------------------------------------

Auction::Auction(
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
) :
m_creatorId(creatorId),
m_minBid(minBid),
m_auctionTimer(auctionTimer),
m_buyNowPrice(buyNowPrice),
m_userDescriptionLength(userDescriptionLength),
m_userDescription(userDescription),
m_oobLength(oobLength),
m_oobData(oobData),
m_attributes(),
m_searchableAttributeInt(nullptr),
m_searchableAttributeFloat(nullptr),
m_searchableAttributeString(nullptr),
m_highBid(nullptr),
m_item(new AuctionItem(itemId, itemType, itemTemplateId, itemTimer, itemNameLength, itemName, creatorId, itemSize)),
m_sold(isSold),
m_active(isActive),
m_bids(),
m_location(const_cast<AuctionLocation &>(location)),
m_flags(flags),
m_trackId(-1)
{
	assert(m_auctionTimer != 0);

	// it it's a instant sale resource container, add "credits per unit" attribute
	if (AuctionItem::IsCategoryResourceContainer(m_item->GetCategory()) && (m_buyNowPrice > 0))
	{
		for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
		{
			if (iter->first == "credits_per_unit")
			{
				// ignore any existing credits per unit attribute; we'll recalculate it and add the updated attribute
				continue;
			}

			m_attributes.push_back(*iter);

			if (iter->first == "resource_contents")
			{
				int count = ::atoi(Unicode::wideToNarrow(iter->second).c_str());
				if (count > 0)
				{
					char buffer[128];
					int const len = snprintf(buffer, sizeof(buffer)-1, "%.6f", (static_cast<double>(m_buyNowPrice) / static_cast<double>(count)));
					buffer[sizeof(buffer)-1] = '\0';

					// trim trailing 0s after the decimal point
					char * pChar = buffer + len - 1;
					while (pChar >= buffer)
					{
						if (*pChar == '.')
						{
							*pChar = '\0';
							break;
						}
						else if (*pChar != '0')
						{
							++pChar;
							*pChar = '\0';
							break;
						}

						--pChar;
					}

					m_attributes.push_back(std::make_pair("credits_per_unit", Unicode::narrowToWide(buffer)));
				}
			}
		}
	}
	else
	{
		m_attributes = attributes;
	}

	SetItemResourceContainerClassCrc();
	BuildSearchableAttributeList();
}

// ----------------------------------------------------------------------

Auction::~Auction()
{
	std::vector<AuctionBid *>::iterator i = m_bids.begin();
	for (; i != m_bids.end(); ++i)
	{
		AuctionBid *bid = (*i);
		delete bid;
	}
	delete m_item;

	delete m_searchableAttributeInt;
	delete m_searchableAttributeFloat;
	delete m_searchableAttributeString;
}

// ----------------------------------------------------------------------

void Auction::Initialization()
{
#ifdef _DEBUG
	for (int i = 0; i < static_cast<int>(AuctionQueryHeadersMessage::SCC_LAST); ++i)
	{
		s_searchConditionComparisonFn[i] = nullptr;
	}
#endif

	s_searchConditionComparisonFn[static_cast<int>(AuctionQueryHeadersMessage::SCC_int)] = searchConditionComparisonFnInt;
	s_searchConditionComparisonFn[static_cast<int>(AuctionQueryHeadersMessage::SCC_float)] = searchConditionComparisonFnFloat;
	s_searchConditionComparisonFn[static_cast<int>(AuctionQueryHeadersMessage::SCC_string_equal)] = searchConditionComparisonFnStringEqual;
	s_searchConditionComparisonFn[static_cast<int>(AuctionQueryHeadersMessage::SCC_string_not_equal)] = searchConditionComparisonFnStringNotEqual;
	s_searchConditionComparisonFn[static_cast<int>(AuctionQueryHeadersMessage::SCC_string_contain)] = searchConditionComparisonFnStringContain;
	s_searchConditionComparisonFn[static_cast<int>(AuctionQueryHeadersMessage::SCC_string_not_contain)] = searchConditionComparisonFnStringNotContain;

#ifdef _DEBUG
	for (int j = 0; j < static_cast<int>(AuctionQueryHeadersMessage::SCC_LAST); ++j)
	{
		DEBUG_FATAL((s_searchConditionComparisonFn[j] == nullptr), ("s_searchConditionComparisonFn array is nullptr at index (%d)", j));
	}
#endif

	// <stop?, search return value>
	static std::pair<bool, bool> const continueToCheckNextMatch(std::make_pair(false, false));
	static std::pair<bool, bool> const stopAndReturnTrue(std::make_pair(true, true));
	static std::pair<bool, bool> const stopAndReturnFalse(std::make_pair(true, false));

	// "will only match if all applicable attribute filter conditions match"
	// ASMAA_match_all -> true after all match
	//                 -> false after first mismatch
	s_searchConditionMatchAllAnyActionAnyMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_match_all)] = continueToCheckNextMatch; // after any match, continue to check next match
	s_searchConditionMatchAllAnyActionAnyNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_match_all)] = stopAndReturnFalse; // after any not match, stop and return false
	s_searchConditionMatchAllAnyActionAllMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_match_all)] = true; // after all match, return true
	s_searchConditionMatchAllAnyActionAllNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_match_all)] = false; // after all not match, return false

	// "will only match if any applicable attribute filter condition matches"
	// ASMAA_match_any -> false after all mismatch
	//                 -> true after first match
	s_searchConditionMatchAllAnyActionAnyMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_match_any)] = stopAndReturnTrue; // after any match, stop and return true
	s_searchConditionMatchAllAnyActionAnyNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_match_any)] = continueToCheckNextMatch; // after any not match, continue to check next match
	s_searchConditionMatchAllAnyActionAllMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_match_any)] = true; // after all match, return true
	s_searchConditionMatchAllAnyActionAllNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_match_any)] = false; // after all not match, return false

	// "will only match if no applicable attribute filter condition matches"
	// (i.e. match none)
	// ASMAA_not_match_all is basically "not ASMAA_match_any"
	// ASMAA_not_match_all -> true after all mismatch
	//                     -> false after first match
	s_searchConditionMatchAllAnyActionAnyMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_not_match_all)] = stopAndReturnFalse; // after any match, stop and return false
	s_searchConditionMatchAllAnyActionAnyNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_not_match_all)] = continueToCheckNextMatch; // after any not match, continue to check next match
	s_searchConditionMatchAllAnyActionAllMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_not_match_all)] = false; // after all match, return false
	s_searchConditionMatchAllAnyActionAllNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_not_match_all)] = true; // after all not match, return true

	// "will only match if any applicable attribute filter condition does not match"
	// ASMAA_not_match_any is basically "not ASMAA_match_all"
	// ASMAA_not_match_any -> false after all match
	//                     -> true after first mismatch
	s_searchConditionMatchAllAnyActionAnyMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_not_match_any)] = continueToCheckNextMatch; // after any match, continue to check next match
	s_searchConditionMatchAllAnyActionAnyNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_not_match_any)] = stopAndReturnTrue; // after any not match, stop and return true
	s_searchConditionMatchAllAnyActionAllMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_not_match_any)] = false; // after all match, return false
	s_searchConditionMatchAllAnyActionAllNotMatch[static_cast<int>(AuctionQueryHeadersMessage::ASMAA_not_match_any)] = true; // after all not match, return true
}

// ----------------------------------------------------------------------

void Auction::GetAttributes(std::string & output) const
{
	output.clear();

	output = Unicode::wideToNarrow(m_item->GetName());
	output += " (";
	output += m_item->GetItemId().getValueString();
	output += ") (GOT_";
	output += GameObjectTypes::getCanonicalName(m_item->GetCategory());
	output += ")\r\n\r\n";

	char buffer[2048];
	for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
	{
		snprintf(buffer, sizeof(buffer)-1, "(%s) (%s)", iter->first.c_str(), Unicode::wideToNarrow(iter->second).c_str());
		buffer[sizeof(buffer)-1] = '\0';

		output += buffer;
		output += "\r\n";
	}

	if (m_searchableAttributeInt)
	{
		output += "\r\nInteger indexed search attribute:\r\n";

		for (std::map<uint32, int>::const_iterator iter = m_searchableAttributeInt->begin(); iter != m_searchableAttributeInt->end(); ++iter)
		{
			std::string attributeName;
			for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iterAttribute = m_attributes.begin(); iterAttribute != m_attributes.end(); ++iterAttribute)
			{
				if (Crc::calculate(iterAttribute->first.c_str()) == iter->first)
				{
					attributeName = iterAttribute->first;
					break;
				}
			}

			snprintf(buffer, sizeof(buffer)-1, "(%s, %lu) (%d)", attributeName.c_str(), iter->first, iter->second);
			buffer[sizeof(buffer)-1] = '\0';

			output += buffer;
			output += "\r\n";
		}
	}

	if (m_searchableAttributeFloat)
	{
		output += "\r\nFloat indexed search attribute:\r\n";

		for (std::map<uint32, double>::const_iterator iter = m_searchableAttributeFloat->begin(); iter != m_searchableAttributeFloat->end(); ++iter)
		{
			std::string attributeName;
			for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iterAttribute = m_attributes.begin(); iterAttribute != m_attributes.end(); ++iterAttribute)
			{
				if (Crc::calculate(iterAttribute->first.c_str()) == iter->first)
				{
					attributeName = iterAttribute->first;
					break;
				}
			}

			snprintf(buffer, sizeof(buffer)-1, "(%s, %lu) (%.10g)", attributeName.c_str(), iter->first, iter->second);
			buffer[sizeof(buffer)-1] = '\0';

			output += buffer;
			output += "\r\n";
		}
	}

	if (m_searchableAttributeString)
	{
		output += "\r\nString indexed search attribute:\r\n";

		for (std::map<uint32, std::string>::const_iterator iter = m_searchableAttributeString->begin(); iter != m_searchableAttributeString->end(); ++iter)
		{
			std::string attributeName;
			for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iterAttribute = m_attributes.begin(); iterAttribute != m_attributes.end(); ++iterAttribute)
			{
				if (Crc::calculate(iterAttribute->first.c_str()) == iter->first)
				{
					attributeName = iterAttribute->first;
					break;
				}
			}

			snprintf(buffer, sizeof(buffer)-1, "(%s, %lu) (%s)", attributeName.c_str(), iter->first, iter->second.c_str());
			buffer[sizeof(buffer)-1] = '\0';

			output += buffer;
			output += "\r\n";
		}
	}
}

// ----------------------------------------------------------------------

int Auction::GetHighBidAmount() const
{
	if (m_highBid)
		return m_highBid->GetBid();
	else
		return 0;
}

// ----------------------------------------------------------------------

const AuctionBid *Auction::GetPreviousBid() const
{
	if (m_bids.size() <= 1)
	{
		return nullptr;
	}
	else
	{
		return m_bids[m_bids.size() - 2];		
	}
}

// ----------------------------------------------------------------------

/**
 * This isn't at all obvious, so here is a description.  This function returns
 * bid that will actually take place when a given bid is attempted. This bid
 * can be less than the existing bid, so the bid will fail, but we still need
 * to adjust the existing high bid to outbid the current one.
 */
int Auction::GetActualBid(AuctionBid *bid)
{
	assert(bid != nullptr);
	int bidNeeded = std::max(m_minBid, bid->GetBid());
	if (m_highBid != nullptr)
	{
		bidNeeded = m_highBid->GetBid();
		if (*bid > *m_highBid)
		{
			bidNeeded = std::max(bid->GetBid(), m_highBid->GetMaxProxyBid() + 1);
		}
		else if (*bid == *m_highBid)
		{
			bidNeeded = m_highBid->GetMaxProxyBid();
		}
		else if (*bid < *m_highBid)
		{
			bidNeeded = std::max(m_highBid->GetBid(), bid->GetMaxProxyBid() + 1);
		}
	}
	return bidNeeded;
}

// ----------------------------------------------------------------------

void Auction::AddLoadedBid(
	const NetworkId & bidderId,
	int bid,
	int maxProxyBid
)
{
	AuctionBid *auctionBid = new AuctionBid(bidderId, bid, maxProxyBid);
	std::vector<AuctionBid *>::iterator i = m_bids.begin();
	bool inserted = false;
	for (; i != m_bids.end(); ++i)
	{
		if ((*i)->GetMaxProxyBid() > auctionBid->GetMaxProxyBid())
		{
			m_bids.insert(i, auctionBid);
			inserted = true;
			break;	
		}
	}

	if (!inserted)
	{
		m_bids.push_back(auctionBid);
	}
	m_highBid = m_bids.back();
}

// ----------------------------------------------------------------------

AuctionResultCode Auction::AddBid(
	const NetworkId & bidderId,
	int bid,
	int maxProxyBid
)
{
	AuctionResultCode success = ARC_Success;
	int maxBid = std::max(bid, maxProxyBid);

	if (!m_active || m_sold)
	{
		return ARC_AuctionExpired;
		DEBUG_REPORT_LOG(true, ("[Commodities Server AddBid] : Auction Expired - Auction is sold and inactive.\n"));	
	}

	if (maxBid > MAX_BID && !m_location.IsVendorMarket())
	{
		return ARC_BidTooHigh;
		DEBUG_REPORT_LOG(true, ("[Commodities Server AddBid] : Bid too high.\n"));
	}

	/*!!!TODO, put me back in after testingif (bidderId == m_creatorId)
	{
		return ARC_OwnerBidOnOwnItem;
	}*/
	if (maxProxyBid < m_minBid)
	{
		return ARC_BidTooLow;
		DEBUG_REPORT_LOG(true, ("[Commodities Server AddBid] : Bid too low.\n"));
	}
	if (IsImmediate() && (GetFlags() & AUCTION_ALWAYS_PRESENT))
	{
		return ARC_SuccessPermanentAuction;
	}

	AuctionBid *auctionBid = new AuctionBid(bidderId, bid, maxProxyBid);
	int newBidForHighBidder = GetActualBid(auctionBid);
	if (m_highBid != nullptr)
	{
		if (*auctionBid <= *m_highBid)
		{
			delete auctionBid;
			success = ARC_BidOutbid;
			DEBUG_REPORT_LOG(true, ("[Commodities Server AddBid] : Bid outbid.\n"));
		}
	}

	if (success == ARC_Success)
	{
		m_highBid = auctionBid;
		m_bids.push_back(auctionBid);
	}
	m_highBid->SetBid(newBidForHighBidder);

	if (IsImmediate())
	{
		m_sold = true;
		DEBUG_REPORT_LOG(true, ("[Commodities Server AddBid] : Immediate Auction - marked as sold.\n"));

		// put auction into the completed auctions list so
		// that the auction can be processed to completion
		// during the next frame
		AuctionMarket::getInstance().AddAuctionToCompletedAuctionsList(*this);
	}
	return success;
}

// ----------------------------------------------------------------------

const AuctionBid *Auction::GetPlayerBid(const NetworkId & playerId) const
{
	// new bids are pushed on the back of the vector
	// so, we search using a reverse iterator
	std::vector<AuctionBid *>::const_reverse_iterator iter = m_bids.rbegin();
	while (iter != m_bids.rend())
	{
		if ((*iter) && ((*iter)->GetBidderId() == playerId))
		{
			return (*iter);
		}
		++iter;
	}
	return nullptr;
}

// ----------------------------------------------------------------------

bool Auction::Update(int gameTime)
{
	if (GetFlags() & AUCTION_ALWAYS_PRESENT)
	{
		return true;
	}
	if (m_active && m_sold)
	{
		//immediate sale
		DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Update] : Expiring immediate sale - Auction is active and sold.\n"));

		if ((m_trackId != -1) && (CommodityServer::getInstance().getGameServer(m_trackId) != nullptr))
		{
			Expire(true, false, m_trackId);
			m_trackId = -1;
		}
		else
		{
			Expire(true, false);
		}
	}
	else if (m_active && (gameTime >= m_auctionTimer))
	{
		DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Update] : Auction expiring based on timer.\n"));

		if ((m_trackId != -1) && (CommodityServer::getInstance().getGameServer(m_trackId) != nullptr))
		{
			Expire(m_highBid != nullptr, true, m_trackId);
			m_trackId = -1;
		}
		else
		{
			Expire(m_highBid != nullptr, true);
		}
	}

	return !m_item->IsExpired(gameTime);
}

// ----------------------------------------------------------------------

void Auction::Expire(bool sold, bool expiredBasedOnTimer, int track_id)
{
	AuctionMarket::getInstance().RemoveFromAuctionTimerPriorityQueue(m_auctionTimer, m_item->GetItemId());

	m_auctionTimer = 0;
	m_active = false;
	if (sold && m_highBid != nullptr)
	{
		m_sold = true;
	}
	else
	{
		m_sold = false;
	}

	bool immediate = IsImmediate();

	DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Expire] : Sold flag is %d.\n", m_sold));
	DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Expire] : Active flag is %d.\n", m_active));
	DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Expire] : Immediate flag is %d.\n", immediate));

	if (m_sold)
	{	
		DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Expire] : Old Owner is %s.\n", m_item->GetOwnerId().getValueString().c_str()));
		m_item->SetOwnerId(m_highBid->GetBidderId());
		DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Expire] : New Owner is %s.\n", m_item->GetOwnerId().getValueString().c_str()));

		DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Expire] : Sending OnAuctionExpired for sold auction .\n"));

		AuctionMarket::getInstance().OnAuctionExpired(
			GetCreatorId(), m_sold, m_flags,
			m_highBid->GetBidderId(), m_highBid->GetBid(), m_item->GetItemId(),
			m_highBid->GetMaxProxyBid(),
			m_location.GetLocationString(), immediate,
			m_item->GetNameLength(), m_item->GetName(),
			m_item->GetOwnerId(), track_id, true);
	}
	else
	{
		if (m_highBid)
		{
			DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Expire] : Sending OnAuctionExpired for non-sold auction and high bid obj .\n"));
			AuctionMarket::getInstance().OnAuctionExpired(
				GetCreatorId(), m_sold, m_flags,
				m_highBid->GetBidderId(), m_highBid->GetBid(), m_item->GetItemId(),
				m_highBid->GetMaxProxyBid(),
				m_location.GetLocationString(), immediate,
				m_item->GetNameLength(), m_item->GetName(),
				m_item->GetOwnerId(), track_id, true);
		}
		else
		{
			// for items on a vendor placed by the vendor owner, we
			// use the following rule to determine if a mail message
			// should be sent to the vendor owner when the item
			// auction expire and goes back into the vendor stockroom;
			// the goal here is to avoid sending too many mail message
			// to a vendor owner when a bunch of auctions expire around
			// the same time
			//
			// 1) if the auction did not expire because of the auction
			//    timer, don't send mail.  This means if the vendor owner
			//    withdraws the sale of an item, no mail is sent.
			// 2) if the auction expired because of the auction timer,
			//    send mail only for the first item, and don't send
			//    another mail until the next auction expires after
			//    the vendor is next accessed or the cluster is restarted.
			bool sendSellerMail = true;
			if (m_location.IsVendorMarket() && m_location.IsOwner(GetCreatorId()))
			{
				if (!expiredBasedOnTimer)
					sendSellerMail = false;
				else if (m_location.GetVendorFirstTimerExpiredAuctionDate() > 0)
					sendSellerMail = false;
				else
					m_location.SetVendorFirstTimerExpiredAuctionDate(time(0));
			}

			DEBUG_REPORT_LOG(true, ("[Commodities Server Auction::Expire] : Sending OnAuctionExpired for non-sold auction and nullptr high bid obj .\n"));
			AuctionMarket::getInstance().OnAuctionExpired(
				GetCreatorId(), m_sold, m_flags,
				NetworkId::cms_invalid, 0, m_item->GetItemId(), 0,
				m_location.GetLocationString(), immediate,
				m_item->GetNameLength(), m_item->GetName(),
				m_item->GetOwnerId(), track_id, sendSellerMail);
		}
		if (m_location.IsVendorMarket() &&
			m_location.IsOwner(GetCreatorId()))
		{
			//vendor owner canceled his own auction. Sorry this is such a hack
			m_location.CancelVendorSale(this);
		}
	}

	if (m_location.IsVendorMarket())
	{
		//If its a vendor, remove and then re-add the item to ensure it is in the right list
		m_location.RemoveAuction(this);
		m_location.AddAuction(this);
	}
}

// ----------------------------------------------------------------------

int Auction::GetBuyNowPriceWithSalesTax() const
{
	if( GetBuyNowPrice() > 0 )
	{
		int salesTax = GetLocation().GetSalesTax();
		return (int)(GetBuyNowPrice() + (GetBuyNowPrice() * (salesTax/10000.0f)));
	}
	else
	{
		return 0;
	}
}

// ----------------------------------------------------------------------

void Auction::AddAttributes(const std::string & attributeName, const Unicode::String & attributeValue)
{
	// it it's a instant sale resource container, add "credits per unit" attribute
	// once we receive the item count attribute for the resource container, and also
	// ignore any existing "credits per unit" attribute to avoid duplicate attribute
	if (AuctionItem::IsCategoryResourceContainer(m_item->GetCategory()))
	{
		if (m_buyNowPrice > 0)
		{
			if (attributeName == "credits_per_unit")
			{
				// ignore any existing credits per unit attribute; we'll recalculate it and add the updated attribute
				return;
			}

			m_attributes.push_back(std::make_pair(attributeName, attributeValue));

			if (attributeName == "resource_contents")
			{
				int count = ::atoi(Unicode::wideToNarrow(attributeValue).c_str());
				if (count > 0)
				{
					char buffer[128];
					int const len = snprintf(buffer, sizeof(buffer)-1, "%.6f", (static_cast<double>(m_buyNowPrice) / static_cast<double>(count)));
					buffer[sizeof(buffer)-1] = '\0';

					// trim trailing 0s after the decimal point
					char * pChar = buffer + len - 1;
					while (pChar >= buffer)
					{
						if (*pChar == '.')
						{
							*pChar = '\0';
							break;
						}
						else if (*pChar != '0')
						{
							++pChar;
							*pChar = '\0';
							break;
						}

						--pChar;
					}

					m_attributes.push_back(std::make_pair("credits_per_unit", Unicode::narrowToWide(buffer)));
				}

				return;
			}
		}
		else
		{
			m_attributes.push_back(std::make_pair(attributeName, attributeValue));
		}

		SetItemResourceContainerClassCrc(attributeName, attributeValue);
	}
	else
	{
		m_attributes.push_back(std::make_pair(attributeName, attributeValue));
	}
}

// ----------------------------------------------------------------------

void Auction::SetItemResourceContainerClassCrc()
{
	if (AuctionItem::IsCategoryResourceContainer(m_item->GetCategory()) && ((m_item->GetResourceContainerClassCrc() == 0) || (m_item->GetResourceNameCrc() == 0)))
	{
		for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iter = m_attributes.begin();  iter != m_attributes.end(); ++iter)
		{
			if ((iter->first == "resource_class") || (iter->first == "resource_name"))
			{
				SetItemResourceContainerClassCrc(iter->first, iter->second);

				if ((m_item->GetResourceContainerClassCrc() != 0) && (m_item->GetResourceNameCrc() != 0))
					break;
			}
		}
	}
}

// ----------------------------------------------------------------------

void Auction::SetItemResourceContainerClassCrc(const std::string & attributeName, const Unicode::String & attributeValue)
{
	if (AuctionItem::IsCategoryResourceContainer(m_item->GetCategory()) && ((m_item->GetResourceContainerClassCrc() == 0) || (m_item->GetResourceNameCrc() == 0)))
	{
		if ((attributeName == "resource_class") || (attributeName == "resource_name"))
		{
			if ((attributeName == "resource_class") && (m_item->GetResourceContainerClassCrc() == 0))
			{
				std::string className = Unicode::wideToNarrow(attributeValue);
				if (0 == className.find("@resource/resource_names:"))
				{
					className = className.substr(sizeof("@resource/resource_names:") - 1);
					if (!className.empty())
					{
						m_item->SetResourceContainerClassCrc(static_cast<int>(Crc::calculate(className.c_str())));
					}
				}
			}
			else if ((attributeName == "resource_name") && (m_item->GetResourceNameCrc() == 0))
			{
				std::string name = Unicode::wideToNarrow(attributeValue);
				if (!name.empty())
				{
					m_item->SetResourceName(name);
					m_item->SetResourceNameCrc(static_cast<int>(Crc::calculate(name.c_str())));
				}
			}

			if ((m_item->GetResourceContainerClassCrc() != 0) && (m_item->GetResourceNameCrc() != 0))
			{
				AuctionMarket::getInstance().AddResourceType(m_item->GetResourceContainerClassCrc(), m_item->GetResourceName());
			}
		}
	}
}

// ----------------------------------------------------------------------

void Auction::BuildSearchableAttributeList()
{
	std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const & sa = CommoditiesAdvancedSearchAttribute::getSearchAttributeForGameObjectType(m_item->GetCategory());
	if (sa.empty())
		return;

	std::map<std::string, std::string> const & saAlias = CommoditiesAdvancedSearchAttribute::getSearchAttributeNameAliasesForGameObjectType(m_item->GetCategory());

	// for factory crates, also include the attributes of the item inside the crate
	std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const * saItemInsideFactoryCrate = nullptr;
	std::map<std::string, std::string> const * saAliasItemInsideFactoryCrate = nullptr;
	int gotItemInsideFactoryCrate = 0;

	if (m_item->GetCategory() == SharedObjectTemplate::GOT_misc_factory_crate)
	{
		// determine the type of item inside the crate by looking at the value of the object_type attribute
		for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
		{
			if (iter->first == "object_type")
			{
				std::string const objectType(Unicode::wideToNarrow(iter->second));
				if (0 == objectType.find("@got_n:"))
				{
					gotItemInsideFactoryCrate = GameObjectTypes::getGameObjectType(objectType.substr(7));
					if (gotItemInsideFactoryCrate == SharedObjectTemplate::GOT_misc_factory_crate)
					{
						// factory crate inside factory crate???
						gotItemInsideFactoryCrate = 0;
					}

					if (gotItemInsideFactoryCrate > 0)
					{
						saItemInsideFactoryCrate = &(CommoditiesAdvancedSearchAttribute::getSearchAttributeForGameObjectType(gotItemInsideFactoryCrate));
						saAliasItemInsideFactoryCrate = &(CommoditiesAdvancedSearchAttribute::getSearchAttributeNameAliasesForGameObjectType(gotItemInsideFactoryCrate));

						if (saItemInsideFactoryCrate->empty())
							saItemInsideFactoryCrate = nullptr;

						if (saAliasItemInsideFactoryCrate->empty())
							saAliasItemInsideFactoryCrate = nullptr;
					}
				}

				break;
			}
		}
	}

	std::string attributeName;
	for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
	{
		// strip out the embedded trailing '\0' on those funky attribute
		// name that have the trailing '\0' embedded in the string
		//
		// also, remove any leading/trailing white space
		attributeName = Unicode::getTrim(iter->first.c_str());

		// handle attribute name alias
		if (!saAlias.empty())
		{
			std::map<std::string, std::string>::const_iterator const iterFindAlias = saAlias.find(attributeName);
			if (iterFindAlias != saAlias.end())
			{
				attributeName = iterFindAlias->second;
			}
		}

		std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *>::const_iterator iterFind = sa.find(attributeName);
		if ((iterFind == sa.end()) && (m_item->GetCategory() == SharedObjectTemplate::GOT_misc_factory_crate) && saItemInsideFactoryCrate)
		{
			// strip out the embedded trailing '\0' on those funky attribute
			// name that have the trailing '\0' embedded in the string
			//
			// also, remove any leading/trailing white space
			attributeName = Unicode::getTrim(iter->first.c_str());

			// handle attribute name alias
			if (saAliasItemInsideFactoryCrate && !saAliasItemInsideFactoryCrate->empty())
			{
				std::map<std::string, std::string>::const_iterator const iterFindAlias = saAliasItemInsideFactoryCrate->find(attributeName);
				if (iterFindAlias != saAliasItemInsideFactoryCrate->end())
				{
					attributeName = iterFindAlias->second;
				}
			}

			iterFind = saItemInsideFactoryCrate->find(attributeName);
			if (iterFind == saItemInsideFactoryCrate->end())
			{
				iterFind = sa.end();
			}
		}

		if (iterFind != sa.end())
		{
			if (iterFind->second->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_int)
			{
				if (!m_searchableAttributeInt)
					m_searchableAttributeInt = new std::map<uint32, int>;

				if (attributeName == "cat_wpn_other.wpn_range")
				{
					// special parsing for cat_wpn_other.wpn_range attribute which is
					// of the "range" form 22-49m and we want to use the second value
					std::string const narrowStr(Unicode::wideToNarrow(iter->second));

					char const * const pChar = strchr(narrowStr.c_str(), '-');
					if (pChar)
					{
						(*m_searchableAttributeInt)[iterFind->second->attributeNameCrc] = ::atoi(pChar + 1);
					}
					else
					{
						(*m_searchableAttributeInt)[iterFind->second->attributeNameCrc] = ::atoi(narrowStr.c_str());
					}
				}
				else if (attributeName == "storage_module_rating")
				{
					// Storage Module Rating - 10
					std::string const narrowStr(Unicode::wideToNarrow(iter->second));

					char const * const pChar = strchr(narrowStr.c_str(), '-');
					if (pChar)
					{
						(*m_searchableAttributeInt)[iterFind->second->attributeNameCrc] = ::atoi(pChar + 1);
					}
					else
					{
						(*m_searchableAttributeInt)[iterFind->second->attributeNameCrc] = ::atoi(narrowStr.c_str());
					}
				}
				else
				{
					(*m_searchableAttributeInt)[iterFind->second->attributeNameCrc] = ::atoi(Unicode::wideToNarrow(iter->second).c_str());
				}
			}
			else if (iterFind->second->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_float)
			{
				if (!m_searchableAttributeFloat)
					m_searchableAttributeFloat = new std::map<uint32, double>;

				(*m_searchableAttributeFloat)[iterFind->second->attributeNameCrc] = ::atof(Unicode::wideToNarrow(iter->second).c_str());
			}
			else if (iterFind->second->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_string)
			{
				if (!m_searchableAttributeString)
					m_searchableAttributeString = new std::map<uint32, std::string>;

				// strip out the embedded trailing '\0' on those funky attribute
				// values that have the trailing '\0' embedded in the string
				//
				// also, remove any leading/trailing white space
				std::string const attributeValue(Unicode::getTrim(Unicode::wideToNarrow(iter->second).c_str()));

				// for resource container's "Resource Class" attribute, convert
				// the string id to a display string so it can be searched on
				if (AuctionItem::IsCategoryResourceContainer(m_item->GetCategory()) && (attributeName == "resource_class"))
				{
					(*m_searchableAttributeString)[iterFind->second->attributeNameCrc] = Unicode::getTrim(Unicode::toLower(Unicode::wideToNarrow(StringId::decodeString(Unicode::narrowToWide(attributeValue)))));
				}
				// for fish type, convert the string id to a display string so it can be searched on
				else if ((m_item->GetCategory() == SharedObjectTemplate::GOT_misc) && (attributeName == "type"))
				{
					(*m_searchableAttributeString)[iterFind->second->attributeNameCrc] = Unicode::getTrim(Unicode::toLower(Unicode::wideToNarrow(StringId::decodeString(Unicode::narrowToWide(attributeValue)))));
				}
				else
				{
					(*m_searchableAttributeString)[iterFind->second->attributeNameCrc] = Unicode::toLower(attributeValue);
				}
			}
			else if (iterFind->second->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_enum)
			{
				if (!m_searchableAttributeString)
					m_searchableAttributeString = new std::map<uint32, std::string>;

				// strip out the embedded trailing '\0' on those funky attribute
				// values that have the trailing '\0' embedded in the string
				std::string attributeValue(Unicode::wideToNarrow(iter->second).c_str());

				// handle enum value alias
				std::map<std::string, std::string>::const_iterator const iterFindAlias = iterFind->second->enumValueAliasList.find(attributeValue);
				if (iterFindAlias != iterFind->second->enumValueAliasList.end())
				{
					attributeValue = iterFindAlias->second;
				}

				// remove any leading/trailing white space
				attributeValue = Unicode::getTrim(attributeValue);

				(*m_searchableAttributeString)[iterFind->second->attributeNameCrc] = Unicode::toLower(attributeValue);

				if (iterFind->second->defaultSearchValueList.count(attributeValue) <= 0)
				{
					std::string bufferStr;
					char buffer[16];
					for (size_t i = 0, size = attributeValue.size(); i < size; ++i)
					{
						snprintf(buffer, sizeof(buffer)-1, "%d ", attributeValue[i]);
						buffer[sizeof(buffer)-1] = '\0';

						bufferStr += buffer;
					}

					LOG("CustomerService", ("CommoditiesAttributeSearch:Item %s (%s) in category %d (%s) has value (%s) (%s) (%d) for attribute (%s) that is not in the attribute enum list",
						m_item->GetItemId().getValueString().c_str(),
						Unicode::wideToNarrow(m_item->GetName()).c_str(),
						m_item->GetCategory(),
						GameObjectTypes::getCanonicalName(m_item->GetCategory()).c_str(),
						attributeValue.c_str(),
						bufferStr.c_str(),
						attributeValue.size(),
						iterFind->second->attributeName.c_str()));
				}
			}
		}
	}

	// for ship component and ship flight computer (or factory crate containing them), need to add "certifications required" search attribute
	bool const itemIsShipComponentOrShipFlightComputer = ((GameObjectTypes::getMaskedType(m_item->GetCategory()) == SharedObjectTemplate::GOT_ship_component) || (m_item->GetCategory() == SharedObjectTemplate::GOT_deed_droid));
	bool const factoryCrateItemIsShipComponentOrShipFlightComputer = ((m_item->GetCategory() == SharedObjectTemplate::GOT_misc_factory_crate) && (gotItemInsideFactoryCrate > 0) && ((GameObjectTypes::getMaskedType(gotItemInsideFactoryCrate) == SharedObjectTemplate::GOT_ship_component) || (gotItemInsideFactoryCrate == SharedObjectTemplate::GOT_deed_droid)));
	if (itemIsShipComponentOrShipFlightComputer || factoryCrateItemIsShipComponentOrShipFlightComputer)
	{
		static std::map<int, std::string> requiredCerts;

		// load the required certs table
		static bool requiredCertsLoaded = false;
		if (!requiredCertsLoaded)
		{
			requiredCertsLoaded = true;

			DataTable * table = DataTableManager::getTable("datatables/commodity/ship_certification_attribute.iff", true);
			if (table)
			{
				int const columnItem = table->findColumnNumber("Item");
				int const columnCert = table->findColumnNumber("Certifications Required");

				if ((columnItem >= 0) && (columnCert >= 0) && (columnItem != columnCert))
				{
					std::string item, cert;
					int const numRows = table->getNumRows();
					for (int i = 0; i < numRows; ++i)
					{
						item = Unicode::getTrim(table->getStringValue(columnItem, i));
						cert = Unicode::getTrim(table->getStringValue(columnCert, i));

						if (!item.empty() && !cert.empty())
						{
							requiredCerts[static_cast<int>(CrcLowerString::calculateCrc(item.c_str()))] = cert;
						}
					}
				}

				DataTableManager::close("datatables/commodity/ship_certification_attribute.iff");
			}
		}

		std::map<int, std::string>::const_iterator const iterFind = requiredCerts.find(m_item->GetItemTemplateId());
		if (iterFind != requiredCerts.end())
		{
			if (!m_searchableAttributeString)
				m_searchableAttributeString = new std::map<uint32, std::string>;

			static uint32 const shipCertAttributeCrc = constcrc("ship_equipment_certification_search_attribute");
			static uint32 const astromechCertAttributeCrc = constcrc("astromech_certification_search_attribute");

			if (itemIsShipComponentOrShipFlightComputer)
			{
				(*m_searchableAttributeString)[((m_item->GetCategory() == SharedObjectTemplate::GOT_deed_droid) ? astromechCertAttributeCrc : shipCertAttributeCrc)] = iterFind->second;
			}
			else
			{
				(*m_searchableAttributeString)[((gotItemInsideFactoryCrate == SharedObjectTemplate::GOT_deed_droid) ? astromechCertAttributeCrc : shipCertAttributeCrc)] = iterFind->second;
			}
		}
	}
}

// ----------------------------------------------------------------------

bool Auction::IsAdvancedFilterMatch(std::list<AuctionQueryHeadersMessage::SearchCondition> const & advancedSearch, int matchAllAny) const
{
	if (advancedSearch.empty())
		return true;

	// the following is the logic what will be used to determine match/no match

	// "will only match if all applicable attribute filter conditions match"
	// ASMAA_match_all -> true after all match
	//                 -> false after first mismatch

	// "will only match if any applicable attribute filter condition does not match"
	// ASMAA_not_match_any is basically "not ASMAA_match_all"
	// ASMAA_not_match_any -> false after all match
	//                     -> true after first mismatch

	// "will only match if any applicable attribute filter condition matches"
	// ASMAA_match_any -> false after all mismatch
	//                 -> true after first match

	// "will only match if no applicable attribute filter condition matches"
	// (i.e. match none)
	// ASMAA_not_match_all is basically "not ASMAA_match_any"
	// ASMAA_not_match_all -> true after all mismatch
	//                     -> false after first match

	int filterCountMatch = 0;
	int filterCountNotMatch = 0;
	bool applicableFilter = 0;
	bool matchFilter;
	for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iter = advancedSearch.begin(); iter != advancedSearch.end(); ++iter)
	{
		matchFilter = (*(s_searchConditionComparisonFn[static_cast<int>(iter->comparison)]))(*iter, m_searchableAttributeInt, m_searchableAttributeFloat, m_searchableAttributeString, applicableFilter);

		if (!applicableFilter)
		{
			continue;
		}
		else if (matchFilter) // match filter
		{
			if (s_searchConditionMatchAllAnyActionAnyMatch[matchAllAny].first)
				return s_searchConditionMatchAllAnyActionAnyMatch[matchAllAny].second;

			++filterCountMatch;
		}
		else // does not match filter
		{
			if (s_searchConditionMatchAllAnyActionAnyNotMatch[matchAllAny].first)
				return s_searchConditionMatchAllAnyActionAnyNotMatch[matchAllAny].second;

			++filterCountNotMatch;
		}
	}

	// if we get this far, then either all applicable filters
	// match or all applicable filters do not match, or there
	// were no applicable filters
	//
	// it's impossible to have gotten this far and have some
	// applicable filters match and some not match, because
	// that situation should have caused an immediate return
	// as soon as it was encountered
	if (filterCountMatch > 0)
	{
		// all applicable filters match
		return s_searchConditionMatchAllAnyActionAllMatch[matchAllAny];
	}
	else if (filterCountNotMatch > 0)
	{
		// all applicable filters do not match
		return s_searchConditionMatchAllAnyActionAllNotMatch[matchAllAny];
	}

	// no filter was applicable, so treat as if no filter was specified
	return true;
}

// ======================================================================
