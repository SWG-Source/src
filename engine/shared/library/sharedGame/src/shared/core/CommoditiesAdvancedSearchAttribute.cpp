// ======================================================================
//
// CommoditiesAdvancedSearchAttribute.cpp
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CommoditiesAdvancedSearchAttribute.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"

#include <map>

// ======================================================================

namespace CommoditiesAdvancedSearchAttributeNamespace
{
	char const * const cs_advancedSearchAttributeDataTableName = "datatables/commodity/advanced_search_attribute.iff";

	std::map<int, std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> > s_commoditiesAdvancedSearchAttributes;
	std::map<int, std::map<std::string, std::string> > s_searchAttributeNameAliases;
	std::set<int> s_gameObjectTypeNoChildInherit;
}

using namespace CommoditiesAdvancedSearchAttributeNamespace;

// ======================================================================

void CommoditiesAdvancedSearchAttribute::install()
{
	DataTable * table = DataTableManager::getTable(cs_advancedSearchAttributeDataTableName, true);
	if (table)
	{
		int const columnGameObjectType = table->findColumnNumber("Game Object Type");
		int const columnSearchAttributeName = table->findColumnNumber("Search Attribute Name");
		int const columnSearchAttributeDataType = table->findColumnNumber("Search Attribute Data Type");

		FATAL((columnGameObjectType < 0), ("column \"Game Object Type\" not found in %s", cs_advancedSearchAttributeDataTableName));
		FATAL((columnSearchAttributeName < 0), ("column \"Search Attribute Name\" not found in %s", cs_advancedSearchAttributeDataTableName));
		FATAL((columnSearchAttributeDataType < 0), ("column \"Search Attribute Data Type\" not found in %s", cs_advancedSearchAttributeDataTableName));

		// the can be a variable number of "Default Search Value" columns, as long as the columns are
		// named "Default Search Value 1", "Default Search Value 2", "Default Search Value 3", and so on
		std::vector<int> columnDefaultSearchValue;
		char buffer[128];
		int columnNumber;
		for (int i = 1; i <= 1000000000; ++i)
		{
			snprintf(buffer, sizeof(buffer)-1, "Default Search Value %d", i);
			buffer[sizeof(buffer)-1] = '\0';

			columnNumber = table->findColumnNumber(buffer);
			if (columnNumber < 0)
				break;

			columnDefaultSearchValue.push_back(columnNumber);
		}

		int currentGameObjectType = 0;
		std::string currentGameObjectTypeString;

		int const numRows = table->getNumRows();
		std::string gameObjectType, searchAttributeName;
		uint32 searchAttributeNameCrc;
		int searchAttributeDataType;
		std::string defaultSearchValue;
		std::set<std::string> defaultSearchValueList;
		std::map<std::string, std::string> enumValueAliasList;
		std::set<std::pair<int, std::string> > excludedSearchAttribute;
		std::map<int, std::map<uint32, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> > commoditiesAdvancedSearchAttributesCrc;
		CommoditiesAdvancedSearchAttribute::SearchAttribute const * searchAttribute;
		Unicode::UnicodeStringVector tokens;
		std::string alias;
		std::map<std::string, std::string>::const_iterator iterFindAlias;
		for (int i = 0; i < numRows; ++i)
		{
			gameObjectType = Unicode::getTrim(table->getStringValue(columnGameObjectType, i));
			FATAL((gameObjectType != table->getStringValue(columnGameObjectType, i)), ("%s, row %d: gameObjectType (%s) has leading/trailing blanks(s)", cs_advancedSearchAttributeDataTableName, (i+3), gameObjectType.c_str()));

			searchAttributeName = Unicode::getTrim(table->getStringValue(columnSearchAttributeName, i));
			FATAL((searchAttributeName != table->getStringValue(columnSearchAttributeName, i)), ("%s, row %d: searchAttributeName (%s) has leading/trailing blanks(s)", cs_advancedSearchAttributeDataTableName, (i+3), searchAttributeName.c_str()));

			if (!gameObjectType.empty())
			{
				currentGameObjectTypeString = gameObjectType;
				currentGameObjectType = GameObjectTypes::getGameObjectType(gameObjectType);
				FATAL((currentGameObjectType <= 0), ("%s, row %d: invalid game object type (%s)", cs_advancedSearchAttributeDataTableName, (i+3), gameObjectType.c_str()));
			}

			FATAL((searchAttributeName.empty() && gameObjectType.empty()), ("%s, row %d: no search attribute specified", cs_advancedSearchAttributeDataTableName, (i+3)));
			FATAL((currentGameObjectType <= 0), ("%s, row %d: search attribute (%s) specified without corresponding game object type", cs_advancedSearchAttributeDataTableName, (i+3), searchAttributeName.c_str()));
			FATAL((searchAttributeName.empty() && !GameObjectTypes::isSubType(currentGameObjectType)), ("%s, row %d: no search attribute specified", cs_advancedSearchAttributeDataTableName, (i+3)));

			// this means that this (base) game object type's search attributes
			// should not be inherited by any of the derived game object type
			if (searchAttributeName == "no child inherit")
			{
				FATAL((GameObjectTypes::isSubType(currentGameObjectType)), ("%s, row %d: \"no child inherit\" qualifier for game object type (%d, %s) can only be used for a base game object type", cs_advancedSearchAttributeDataTableName, (i+3), currentGameObjectType, GameObjectTypes::getCanonicalName(currentGameObjectType).c_str()));

				s_gameObjectTypeNoChildInherit.insert(currentGameObjectType);

				searchAttributeName.clear();
			}
			// this means the derived game object type does not want to
			// include this search attribute from the parent game object type
			else if (searchAttributeName.find("exclude ") == 0)
			{
				FATAL((!GameObjectTypes::isSubType(currentGameObjectType)), ("%s, row %d: \"exclude\" qualifier for search attribute (%s) cannot be used with a base game object type", cs_advancedSearchAttributeDataTableName, (i+3), searchAttributeName.c_str()));

				searchAttributeName = searchAttributeName.substr(8);
				if (!searchAttributeName.empty())
					excludedSearchAttribute.insert(std::make_pair(currentGameObjectType, searchAttributeName));

				searchAttributeName.clear();
			}
			else
			{
				// handle search attribute name alias
				tokens.clear();
				if (Unicode::tokenize(Unicode::narrowToWide(searchAttributeName), tokens, nullptr, nullptr) && (tokens.size() > 1))
				{
					// the first value is the search attribute name to display
					searchAttributeName = Unicode::wideToNarrow(tokens[0]);

					// the other values will be aliased by the first value
					for (size_t j = 1, size = tokens.size(); j < size; ++j)
					{
						alias = Unicode::wideToNarrow(tokens[j]);

						FATAL((getSearchAttributeForGameObjectType(currentGameObjectType).count(alias) > 0), ("%s, row %d: aliased search attribute (%s) for game object type (%s) has already been defined as a search attribute", cs_advancedSearchAttributeDataTableName, (i+3), alias.c_str(), currentGameObjectTypeString.c_str()));

						iterFindAlias = getSearchAttributeNameAliasesForGameObjectType(currentGameObjectType).find(alias);
						FATAL((iterFindAlias != getSearchAttributeNameAliasesForGameObjectType(currentGameObjectType).end()), ("%s, row %d: aliased search attribute (%s) for game object type (%s) has already been defined as an alias for search attribute (%s)", cs_advancedSearchAttributeDataTableName, (i+3), alias.c_str(), currentGameObjectTypeString.c_str(), iterFindAlias->second.c_str()));

						s_searchAttributeNameAliases[currentGameObjectType].insert(std::make_pair(alias, searchAttributeName));
					}
				}
			}

			// if necessary, insert new entries for the game object type
			std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> & attributes = s_commoditiesAdvancedSearchAttributes[currentGameObjectType];
			std::map<uint32, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> & attributesCrc = commoditiesAdvancedSearchAttributesCrc[currentGameObjectType];
			if (!searchAttributeName.empty())
			{
				FATAL((attributes.count(searchAttributeName) > 0), ("%s, row %d: duplicate search attribute (%s) for game object type (%s)", cs_advancedSearchAttributeDataTableName, (i+3), searchAttributeName.c_str(), currentGameObjectTypeString.c_str()));

				iterFindAlias = getSearchAttributeNameAliasesForGameObjectType(currentGameObjectType).find(searchAttributeName);
				FATAL((iterFindAlias != getSearchAttributeNameAliasesForGameObjectType(currentGameObjectType).end()), ("%s, row %d: search attribute (%s) for game object type (%s) has already been defined as an alias for search attribute (%s)", cs_advancedSearchAttributeDataTableName, (i+3), searchAttributeName.c_str(), currentGameObjectTypeString.c_str(), iterFindAlias->second.c_str()));

				searchAttributeNameCrc = Crc::calculate(searchAttributeName.c_str());
				FATAL((attributesCrc.count(searchAttributeNameCrc) > 0), ("%s, row %d: search attribute (%s) for game object type (%s) has the same crc value as search attribute (%s)", cs_advancedSearchAttributeDataTableName, (i+3), searchAttributeName.c_str(), currentGameObjectTypeString.c_str(), attributesCrc[searchAttributeNameCrc]->attributeName.c_str()));

				searchAttributeDataType = table->getIntValue(columnSearchAttributeDataType, i);
				FATAL(((searchAttributeDataType < 0) || (searchAttributeDataType == static_cast<int>(SADT_unknown)) || (searchAttributeDataType >= static_cast<int>(SADT_LAST_ENUM))), ("%s, row %d: invalid search attribute data type (%d)", cs_advancedSearchAttributeDataTableName, (i+3), searchAttributeDataType));

				// if search attribute is string or enum, read list of default search value, if any
				defaultSearchValueList.clear();
				enumValueAliasList.clear();
				if ((static_cast<SearchAttributeDataType>(searchAttributeDataType) == SADT_string) || (static_cast<SearchAttributeDataType>(searchAttributeDataType) == SADT_enum))
				{
					for (std::vector<int>::const_iterator iterColumnDefaultSearchValue = columnDefaultSearchValue.begin(); iterColumnDefaultSearchValue != columnDefaultSearchValue.end(); ++iterColumnDefaultSearchValue)
					{
						defaultSearchValue = Unicode::getTrim(table->getStringValue(*iterColumnDefaultSearchValue, i));
						FATAL((defaultSearchValue != table->getStringValue(*iterColumnDefaultSearchValue, i)), ("%s, row %d: defaultSearchValue (%s) has leading/trailing blanks(s)", cs_advancedSearchAttributeDataTableName, (i+3), defaultSearchValue.c_str()));

						if (!defaultSearchValue.empty())
						{
							if (static_cast<SearchAttributeDataType>(searchAttributeDataType) == SADT_enum)
							{
								// for enum, parse out the aliases (if any) for the enum value
								tokens.clear();
								if (Unicode::tokenize(Unicode::narrowToWide(defaultSearchValue), tokens, nullptr, nullptr) && !tokens.empty())
								{
#ifdef _WIN32
#ifdef _DEBUG
									Unicode::String const enumDisplayString(StringId::decodeString(tokens[0]));
#endif
#endif
									// the first value is the enum value to display
									std::string const aliasMaster(Unicode::wideToNarrow(tokens[0]));
									defaultSearchValueList.insert(aliasMaster);

									// the other values will be aliased by the first value
									for (size_t j = 1, size = tokens.size(); j < size; ++j)
									{
										std::string narrowStr(Unicode::wideToNarrow(tokens[j]));
										if (narrowStr == "<<<a_blank_space>>>")
											narrowStr = " ";

										enumValueAliasList[narrowStr] = aliasMaster;

#ifdef _WIN32
#ifdef _DEBUG
										Unicode::String const enumAliasDisplayString(StringId::decodeString(Unicode::narrowToWide(narrowStr)));
										DEBUG_WARNING((enumDisplayString != enumAliasDisplayString), ("%s, row %d: enum alias (%s) (%s) has a different display string than the alias master (%s) (%s), so may not need to be aliased", cs_advancedSearchAttributeDataTableName, (i+3), narrowStr.c_str(), Unicode::wideToNarrow(enumAliasDisplayString).c_str(), aliasMaster.c_str(), Unicode::wideToNarrow(enumDisplayString).c_str()));
#endif
#endif
									}
								}
							}
							else
							{
								defaultSearchValueList.insert(defaultSearchValue);
							}
						}
					}
				}

				FATAL(((static_cast<SearchAttributeDataType>(searchAttributeDataType) == SADT_enum) && defaultSearchValueList.empty()), ("%s, row %d: no default value specified for enum search attribute (%s)", cs_advancedSearchAttributeDataTableName, (i+3), searchAttributeName.c_str()));

				// for factory crate object_type attribute, only allow an enum
				// alias to represent a single enum value, so that we can support
				// search by attribute on the item contained inside the crate
				if ((currentGameObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (searchAttributeName == "object_type") && !enumValueAliasList.empty())
				{
					std::set<std::string> enumAliasCount;
					for (std::map<std::string, std::string>::const_iterator iterAlias = enumValueAliasList.begin(); iterAlias != enumValueAliasList.end(); ++iterAlias)
					{
						FATAL((enumAliasCount.count(iterAlias->second) > 0), ("%s, row %d: for factory crate object_type attribute, an enum alias (%s) can only represent a single enum value", cs_advancedSearchAttributeDataTableName, (i+3), iterAlias->second.c_str()));

						enumAliasCount.insert(iterAlias->second);
					}
				}

				searchAttribute = new SearchAttribute(searchAttributeName, searchAttributeNameCrc, static_cast<SearchAttributeDataType>(searchAttributeDataType), defaultSearchValueList, enumValueAliasList);
				attributes[searchAttributeName] = searchAttribute;
				attributesCrc[searchAttributeNameCrc] = searchAttribute;
			}
		}

		DataTableManager::close(cs_advancedSearchAttributeDataTableName);

		// copy attributes from parent game object type into child game object type
		for (std::map<int, std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> >::iterator iterGameObjectType = s_commoditiesAdvancedSearchAttributes.begin(); iterGameObjectType != s_commoditiesAdvancedSearchAttributes.end(); ++iterGameObjectType)
		{
			if (!GameObjectTypes::isSubType(iterGameObjectType->first))
				continue;

			// parent game object type doesn't allow child game object type to inherit search attribute
			if (!allowChildGameObjectTypeToInheritSearchAttribute(GameObjectTypes::getMaskedType(iterGameObjectType->first)))
				continue;

			std::map<int, std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> >::const_iterator const iterParentGameObjectType = s_commoditiesAdvancedSearchAttributes.find(GameObjectTypes::getMaskedType(iterGameObjectType->first));
			if (iterParentGameObjectType != s_commoditiesAdvancedSearchAttributes.end())
			{
				std::map<uint32, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> & attributesCrc = commoditiesAdvancedSearchAttributesCrc[iterGameObjectType->first];

				for (std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *>::const_iterator iterParentAttribute = iterParentGameObjectType->second.begin(); iterParentAttribute != iterParentGameObjectType->second.end(); ++iterParentAttribute)
				{
					FATAL((iterGameObjectType->second.count(iterParentAttribute->second->attributeName) > 0), ("search attribute (%s) for game object type (%s) is already defined in parent game object type (%s)", iterParentAttribute->second->attributeName.c_str(), GameObjectTypes::getCanonicalName(iterGameObjectType->first).c_str(), GameObjectTypes::getCanonicalName(iterParentGameObjectType->first).c_str()));
					FATAL((attributesCrc.count(iterParentAttribute->second->attributeNameCrc) > 0), ("search attribute (%s) for parent game object type (%s) has the same crc value as search attribute (%s) in the game object type (%s)", iterParentAttribute->second->attributeName.c_str(), GameObjectTypes::getCanonicalName(iterParentGameObjectType->first).c_str(), attributesCrc[iterParentAttribute->second->attributeNameCrc]->attributeName.c_str(), GameObjectTypes::getCanonicalName(iterGameObjectType->first).c_str()));

					if (excludedSearchAttribute.count(std::make_pair(iterGameObjectType->first, iterParentAttribute->second->attributeName)) < 1)
					{
						iterGameObjectType->second[iterParentAttribute->second->attributeName] = iterParentAttribute->second;
						attributesCrc[iterParentAttribute->second->attributeNameCrc] = iterParentAttribute->second;
					}
				}
			}

			std::map<int, std::map<std::string, std::string> >::const_iterator const iterParentAttributeNameAliases = s_searchAttributeNameAliases.find(GameObjectTypes::getMaskedType(iterGameObjectType->first));
			if (iterParentAttributeNameAliases != s_searchAttributeNameAliases.end())
			{
				std::map<std::string, std::string> & alias = s_searchAttributeNameAliases[iterGameObjectType->first];
				for (std::map<std::string, std::string>::const_iterator iterAlias = iterParentAttributeNameAliases->second.begin(); iterAlias != iterParentAttributeNameAliases->second.end(); ++iterAlias)
				{
					alias.insert(*iterAlias);
				}
			}
		}
	}
	else
	{
		FATAL(true, ("commodities advanced search attribute datatable %s not found", cs_advancedSearchAttributeDataTableName));
	}

	ExitChain::add(remove, "CommoditiesAdvancedSearchAttribute::remove");
}

//----------------------------------------------------------------------

void CommoditiesAdvancedSearchAttribute::remove()
{
	std::set<CommoditiesAdvancedSearchAttribute::SearchAttribute const *> allSearchAttributes;

	for (std::map<int, std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> >::const_iterator iterGameObjectType = s_commoditiesAdvancedSearchAttributes.begin(); iterGameObjectType != s_commoditiesAdvancedSearchAttributes.end(); ++iterGameObjectType)
	{
		for (std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *>::const_iterator iterAttribute = iterGameObjectType->second.begin(); iterAttribute != iterGameObjectType->second.end(); ++iterAttribute)
		{
			allSearchAttributes.insert(iterAttribute->second);
		}
	}

	s_commoditiesAdvancedSearchAttributes.clear();

	for (std::set<CommoditiesAdvancedSearchAttribute::SearchAttribute const *>::const_iterator iterAttribute = allSearchAttributes.begin(); iterAttribute != allSearchAttributes.end(); ++iterAttribute)
	{
		delete *iterAttribute;
	}

	s_searchAttributeNameAliases.clear();

	s_gameObjectTypeNoChildInherit.clear();
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const & CommoditiesAdvancedSearchAttribute::getSearchAttributeForGameObjectType(int gameObjectType)
{
	static std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const empty;

	std::map<int, std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> >::const_iterator const iterFind = s_commoditiesAdvancedSearchAttributes.find(gameObjectType);
	if (iterFind != s_commoditiesAdvancedSearchAttributes.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

CommoditiesAdvancedSearchAttribute::SearchAttribute const * CommoditiesAdvancedSearchAttribute::getSearchAttribute(int gameObjectType, std::string const & attributeName)
{
	std::map<int, std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> >::const_iterator const iterFindGot = s_commoditiesAdvancedSearchAttributes.find(gameObjectType);
	if (iterFindGot != s_commoditiesAdvancedSearchAttributes.end())
	{
		std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *>::const_iterator const iterFindAttribute = iterFindGot->second.find(attributeName);
		if (iterFindAttribute != iterFindGot->second.end())
			return iterFindAttribute->second;
	}

	return nullptr;
}

//----------------------------------------------------------------------

std::map<std::string, std::string> const & CommoditiesAdvancedSearchAttribute::getSearchAttributeNameAliasesForGameObjectType(int gameObjectType)
{
	static std::map<std::string, std::string> const empty;

	std::map<int, std::map<std::string, std::string> >::const_iterator const iterFind = s_searchAttributeNameAliases.find(gameObjectType);
	if (iterFind != s_searchAttributeNameAliases.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------

bool CommoditiesAdvancedSearchAttribute::allowChildGameObjectTypeToInheritSearchAttribute(int gameObjectType)
{
	return (s_gameObjectTypeNoChildInherit.count(gameObjectType) <= 0);
}

// ======================================================================
