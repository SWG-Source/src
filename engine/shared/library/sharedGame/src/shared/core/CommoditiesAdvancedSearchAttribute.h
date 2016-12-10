// ======================================================================
//
// CommoditiesAdvancedSearchAttribute.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_CommoditiesAdvancedSearchAttribute_H
#define INCLUDED_CommoditiesAdvancedSearchAttribute_H

#include <map>
#include <set>
#include <string>

// ======================================================================

class CommoditiesAdvancedSearchAttribute // static class
{
public:
	static void install();
	static void remove();

	enum SearchAttributeDataType
	{
		SADT_unknown = 0,
		SADT_int,
		SADT_float,
		SADT_string,
		SADT_enum,
		SADT_LAST_ENUM
	};

	class SearchAttribute
	{
	public:
		SearchAttribute(std::string const & pAttributeName, uint32 pAttributeNameCrc, SearchAttributeDataType const pAttributeDataType, std::set<std::string> const & pDefaultSearchValueList, std::map<std::string, std::string> const & pEnumValueAliasList) :
		  attributeName(pAttributeName), attributeNameCrc(pAttributeNameCrc), attributeDataType(pAttributeDataType), defaultSearchValueList(pDefaultSearchValueList), enumValueAliasList(pEnumValueAliasList) {};

		std::string const attributeName;
		uint32 const attributeNameCrc;
		SearchAttributeDataType const attributeDataType;
		std::set<std::string> const defaultSearchValueList;
		std::map<std::string, std::string> const enumValueAliasList;

	private:
		SearchAttribute();
		SearchAttribute(SearchAttribute const &);
		SearchAttribute &operator =(SearchAttribute const &);
	};

	static std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const & getSearchAttributeForGameObjectType(int gameObjectType);
	static CommoditiesAdvancedSearchAttribute::SearchAttribute const * getSearchAttribute(int gameObjectType, std::string const & attributeName);

	static std::map<std::string, std::string> const & getSearchAttributeNameAliasesForGameObjectType(int gameObjectType);

	static bool allowChildGameObjectTypeToInheritSearchAttribute(int gameObjectType);

private: // disabled

	CommoditiesAdvancedSearchAttribute();
	CommoditiesAdvancedSearchAttribute(CommoditiesAdvancedSearchAttribute const &);
	CommoditiesAdvancedSearchAttribute &operator =(CommoditiesAdvancedSearchAttribute const &);
};

// ======================================================================

#endif // INCLUDED_CommoditiesAdvancedSearchAttribute_H
