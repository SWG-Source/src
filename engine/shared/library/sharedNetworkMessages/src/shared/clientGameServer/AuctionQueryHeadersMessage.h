// ======================================================================
//
// AuctionQueryHeadersMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionQueryHeadersMessage_H
#define	_AuctionQueryHeadersMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Tag.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class AuctionQueryHeadersMessage : public GameNetworkMessage
{
public: // types
	/*  auction query types */
	enum AuctionSearchType
	{
		AST_ByCategory,
		AST_ByLocation,
		AST_ByAll,
		AST_ByPlayerSales,
		AST_ByPlayerBids,
		AST_ByPlayerStockroom,
		AST_ByVendorOffers,
		AST_ByVendorSelling,
		AST_ByVendorStockroom,
		AST_ByPlayerOffersToVendor
	};

	enum AuctionLocationSearch
	{
		ALS_Galaxy,
		ALS_Planet,
		ALS_Region,
		ALS_Market
	};

	enum SearchConditionComparison
	{
		// DO NOT CHANGE THE ORDERING OR THE NUMBERING
		// ADD NEW ENUM VALUE BEFORE SCC_LAST
		SCC_int = 0,
		SCC_float,
		SCC_string_equal,
		SCC_string_not_equal,
		SCC_string_contain,
		SCC_string_not_contain,
		SCC_LAST
	};

	// order of enum value MUST match the order defined in the combo box for dsComboMatchAllAny defined in ui_auction.inc
	enum AdvancedSearchMatchAllAny
	{
		// DO NOT CHANGE THE ORDERING OR THE NUMBERING
		// ADD NEW ENUM VALUE BEFORE ASMAA_LAST
		ASMAA_match_all = 0,
		ASMAA_match_any,
		ASMAA_not_match_all,
		ASMAA_not_match_any,
		ASMAA_LAST
	};

	class SearchCondition
	{
	public:
		SearchCondition() :
		  attributeNameCrc(), requiredAttribute(false), comparison(SCC_int), intMin(0), intMax(0), floatMin(0.0f), floatMax(0.0f), stringValue() {};

		SearchCondition(SearchCondition const & rhs) :
		  attributeNameCrc(rhs.attributeNameCrc), requiredAttribute(rhs.requiredAttribute), comparison(rhs.comparison), intMin(rhs.intMin), intMax(rhs.intMax), floatMin(rhs.floatMin), floatMax(rhs.floatMax), stringValue(rhs.stringValue) {};

		explicit SearchCondition(uint32 const pAttributeNameCrc, bool const pRequiredAttribute, int const pIntMin, int const pIntMax) :
		  attributeNameCrc(pAttributeNameCrc), requiredAttribute(pRequiredAttribute), comparison(SCC_int), intMin(pIntMin), intMax(pIntMax), floatMin(0.0f), floatMax(0.0f), stringValue() {};

		explicit SearchCondition(uint32 const pAttributeNameCrc, bool const pRequiredAttribute, double const pFloatMin, double const pFloatMax) :
		  attributeNameCrc(pAttributeNameCrc), requiredAttribute(pRequiredAttribute), comparison(SCC_float), intMin(0), intMax(0), floatMin(pFloatMin), floatMax(pFloatMax), stringValue() {};

		explicit SearchCondition(uint32 const pAttributeNameCrc, bool const pRequiredAttribute, SearchConditionComparison const pComparison, std::string const & pStringValue) :
		  attributeNameCrc(pAttributeNameCrc), requiredAttribute(pRequiredAttribute), comparison(pComparison), intMin(0), intMax(0), floatMin(0.0f), floatMax(0.0f), stringValue(pStringValue) {};

		SearchCondition &operator =(SearchCondition const & rhs)
		{
			if (this != (&rhs))
			{
				attributeNameCrc = rhs.attributeNameCrc;
				requiredAttribute = rhs.requiredAttribute;
				comparison = rhs.comparison;
				intMin = rhs.intMin;
				intMax = rhs.intMax;
				floatMin = rhs.floatMin;
				floatMax = rhs.floatMax;
				stringValue = rhs.stringValue;
			}

			return *this;
		}

		bool operator== (SearchCondition const & rhs) const
		{
			if (this == &rhs)
			{
				return true;
			}

			if ((attributeNameCrc != rhs.attributeNameCrc) ||
				(requiredAttribute != rhs.requiredAttribute) ||
				(comparison != rhs.comparison))
			{
				return false;
			}

			if (comparison == SCC_int)
			{
				return ((intMin == rhs.intMin) && (intMax == rhs.intMax));
			}
			else if (comparison == SCC_float)
			{
				return ((floatMin == rhs.floatMin) && (floatMax == rhs.floatMax));
			}

			return (stringValue == rhs.stringValue);
		}

		bool operator!= (SearchCondition const & rhs) const
		{
			return !operator==(rhs);
		}

		uint32 attributeNameCrc;
		bool requiredAttribute;
		SearchConditionComparison comparison;

		int intMin;
		int intMax;

		double floatMin;
		double floatMax;

		std::string stringValue;
	};

public:

	AuctionQueryHeadersMessage(
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
		bool myVendorsOnly = false,
		uint16 queryOffset = 0);
	explicit AuctionQueryHeadersMessage(Archive::ReadIterator &source);

	~AuctionQueryHeadersMessage();

public: // methods

	int                              getLocationSearchType() const;
	int                              getRequestId() const;
	int                              getSearchType() const;
	int                              getItemType() const;
	bool                             getItemTypeExactMatch() const;
	int                              getItemTemplateId() const;
	const Unicode::String &          getTextFilterAll() const;
	const Unicode::String &          getTextFilterAny() const;
	int                              getPriceFilterMin() const;
	int                              getPriceFilterMax() const;
	bool                             getPriceFilterIncludesFee() const;
	const std::list<SearchCondition> & getAdvancedSearch() const;
	AdvancedSearchMatchAllAny        getAdvancedSearchMatchAllAny() const;
	const NetworkId &                getContainer() const;
	bool                             getMyVendorsOnly() const;
	uint16                           getQueryOffset() const;

private: 
	Archive::AutoVariable<int>              m_locationSearchType;   //string to search by
	Archive::AutoVariable<int>              m_requestId;      //sequence number
	Archive::AutoVariable<int>              m_searchType;     //item category
	Archive::AutoVariable<int>              m_itemType;       //item category
	Archive::AutoVariable<bool>             m_itemTypeExactMatch;
	Archive::AutoVariable<int>              m_itemTemplateId; //if non-zero, specifies the template id of a particular item
	Archive::AutoVariable<Unicode::String>  m_textFilterAll;  //text filter for name
	Archive::AutoVariable<Unicode::String>  m_textFilterAny;
	Archive::AutoVariable<int>              m_priceFilterMin;
	Archive::AutoVariable<int>              m_priceFilterMax;
	Archive::AutoVariable<bool>             m_priceFilterIncludesFee;
	Archive::AutoList<SearchCondition>      m_advancedSearch;
	Archive::AutoVariable<int8>             m_advancedSearchMatchAllAny;
	Archive::AutoVariable<NetworkId>        m_container;      
	Archive::AutoVariable<bool>             m_myVendorsOnly;
	Archive::AutoVariable<uint16>           m_queryOffset;
};

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getLocationSearchType() const
{
	return m_locationSearchType.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getRequestId() const
{
	return m_requestId.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getSearchType() const
{
	return m_searchType.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getItemType() const
{
	return m_itemType.get();
}

// ----------------------------------------------------------------------

inline bool AuctionQueryHeadersMessage::getItemTypeExactMatch() const
{
	return m_itemTypeExactMatch.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getItemTemplateId() const
{
	return m_itemTemplateId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & AuctionQueryHeadersMessage::getTextFilterAll() const
{
	return m_textFilterAll.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & AuctionQueryHeadersMessage::getTextFilterAny() const
{
	return m_textFilterAny.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getPriceFilterMin() const
{
	return m_priceFilterMin.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getPriceFilterMax() const
{
	return m_priceFilterMax.get();
}

// ----------------------------------------------------------------------

inline bool AuctionQueryHeadersMessage::getPriceFilterIncludesFee() const
{
	return m_priceFilterIncludesFee.get();
}

// ----------------------------------------------------------------------

inline const std::list<AuctionQueryHeadersMessage::SearchCondition> & AuctionQueryHeadersMessage::getAdvancedSearch() const
{
	return m_advancedSearch.get();
}

// ----------------------------------------------------------------------

inline AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny AuctionQueryHeadersMessage::getAdvancedSearchMatchAllAny() const
{
	return static_cast<AdvancedSearchMatchAllAny>(m_advancedSearchMatchAllAny.get());
}

// ----------------------------------------------------------------------

inline const NetworkId & AuctionQueryHeadersMessage::getContainer() const
{
	return m_container.get();
}

// ----------------------------------------------------------------------

inline bool AuctionQueryHeadersMessage::getMyVendorsOnly() const
{
	return m_myVendorsOnly.get();
}

// ----------------------------------------------------------------------

inline uint16 AuctionQueryHeadersMessage::getQueryOffset() const
{
	return m_queryOffset.get();
}

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, AuctionQueryHeadersMessage::SearchCondition & target);
	void put(ByteStream & target, const AuctionQueryHeadersMessage::SearchCondition & source);
}

// ======================================================================

#endif // _AuctionQueryHeadersMessage_H
