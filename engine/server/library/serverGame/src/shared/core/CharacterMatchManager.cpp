// ============================================================================
//
// CharacterMatchManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CharacterMatchManager.h"

#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/MatchMakingCharacterPreferenceId.h"
#include "sharedGame/MatchMakingCharacterResult.h"
#include "sharedMath/Vector2d.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include <unordered_map>
#include <limits>
#include <map>
#include <vector>

// ============================================================================

unsigned long CharacterMatchManager::ms_numberOfCharacterMatchRequests = 0;
unsigned long CharacterMatchManager::ms_numberOfCharacterMatchResults = 0;
unsigned long CharacterMatchManager::ms_timeSpentOnCharacterMatchRequestsMs = 0;

// ============================================================================
//
// CharacterMatchManagerNamespace
//
// ============================================================================

namespace CharacterMatchManagerNamespace
{
	// used when invoking LfgDataTable::LfgNode::internalAttributeMatchFunction
	struct LfgInternalAttributeMatchFunctionParams
	{
		LfgInternalAttributeMatchFunctionParams() : param1(nullptr), param2(nullptr), param3(nullptr), param4(nullptr), param5(nullptr) {}

		void const * param1;
		void const * param2;
		void const * param3;
		void const * param4;
		void const * param5;
	};

	bool isMatch(std::map<LfgDataTable::LfgNode const *, std::vector<std::pair<LfgDataTable::LfgNode const *, LfgInternalAttributeMatchFunctionParams> > > const & specifiedSearchAttributes, std::string const & searchSubstring, LfgCharacterData const & lfgCharacterData, BitArray & matchingAttributes);
}

using namespace CharacterMatchManagerNamespace;

// ======================================================================

bool CharacterMatchManagerNamespace::isMatch(std::map<LfgDataTable::LfgNode const *, std::vector<std::pair<LfgDataTable::LfgNode const *, LfgInternalAttributeMatchFunctionParams> > > const & specifiedSearchAttributes, std::string const & searchSubstring, LfgCharacterData const & lfgCharacterData, BitArray & matchingAttributes)
{
	matchingAttributes.clear();

	bool matchSingleSearchAttribute;
	int countMatchingSearchAttributeForGroup;
	bool matchAllSearchAttributeForGroup;
	for (std::map<LfgDataTable::LfgNode const *, std::vector<std::pair<LfgDataTable::LfgNode const *, LfgInternalAttributeMatchFunctionParams> > >::const_iterator iterAnyAllParentNode = specifiedSearchAttributes.begin(); iterAnyAllParentNode != specifiedSearchAttributes.end(); ++iterAnyAllParentNode)
	{
		matchAllSearchAttributeForGroup = true;
		if (iterAnyAllParentNode->first)
		{
			if ((iterAnyAllParentNode->first->actualMatchCondition == LfgDataTable::DMCT_Any) || (iterAnyAllParentNode->first->actualMatchCondition == LfgDataTable::DMCT_AnyOnly))
				matchAllSearchAttributeForGroup = false;
		}

		countMatchingSearchAttributeForGroup = 0;
		for (std::vector<std::pair<LfgDataTable::LfgNode const *, LfgInternalAttributeMatchFunctionParams> >::const_iterator iterSearchAttribute = iterAnyAllParentNode->second.begin(); iterSearchAttribute != iterAnyAllParentNode->second.end(); ++iterSearchAttribute)
		{
			if (iterSearchAttribute->first->internalAttribute)
				matchSingleSearchAttribute = (*(iterSearchAttribute->first->internalAttributeMatchFunction))(lfgCharacterData, iterSearchAttribute->second.param1, iterSearchAttribute->second.param2, iterSearchAttribute->second.param3, iterSearchAttribute->second.param4, iterSearchAttribute->second.param5);
			else
				matchSingleSearchAttribute = LfgDataTable::testBit(*(iterSearchAttribute->first), lfgCharacterData.characterInterests);

			if (matchSingleSearchAttribute)
			{
				++countMatchingSearchAttributeForGroup;

				if (iterSearchAttribute->first->minValue > 0)
				{
					LfgDataTable::setLowHighValue(*(iterSearchAttribute->first), reinterpret_cast<unsigned long>(iterSearchAttribute->second.param1), reinterpret_cast<unsigned long>(iterSearchAttribute->second.param2), matchingAttributes);
				}
				else
				{
					LfgDataTable::setBit(*(iterSearchAttribute->first), matchingAttributes);
				}

				// if "match any", then we are done for this group of search attributes
				// because we have found a match
				if (!matchAllSearchAttributeForGroup)
					break;
			}
			else
			{
				// if "match all", then we have failed for this group of search attributes,
				// and have also failed for the entire match, since the groups of search
				// attributes must all match
				if (matchAllSearchAttributeForGroup)
				{
					matchingAttributes.clear();
					return false;
				}
			}
		}

		// if "match all" and we exited the loop, then every attribute in the group
		// matched (or else it would have already returned false on the first mismatch);
		// if "match any" and we exited the loop without matching any attribute,
		// then we have failed for this group of search attributes, and have also failed
		// for the entire match, since the groups of search attributes must all match
		if ((!matchAllSearchAttributeForGroup) && (countMatchingSearchAttributeForGroup <= 0))
		{
			matchingAttributes.clear();
			return false;
		}
	}

	// if substring is specified, it must match as well
	if (!searchSubstring.empty())
	{
		// Compare a sub-string vs the player name and guild

		// Player name (lowercase)
		std::string playerName(Unicode::wideToNarrow(lfgCharacterData.characterName));
		std::transform(playerName.begin(), playerName.end(), playerName.begin(), tolower);

		// Guild name (lowercase)
		std::string guildName(lfgCharacterData.guildName);
		std::transform(guildName.begin(), guildName.end(), guildName.begin(), tolower);

		if ((strstr(playerName.c_str(), searchSubstring.c_str()) == nullptr) &&
			(strstr(guildName.c_str(), searchSubstring.c_str()) == nullptr))
		{
			// fail because couldn't match substring
			matchingAttributes.clear();
			return false;
		}
	}

	return true;
}

// ============================================================================
//
// CharacterMatchManager
//
// ============================================================================

void CharacterMatchManager::requestMatch(NetworkId const &networkId, MatchMakingCharacterPreferenceId const &matchMakingCharacterPreferenceId)
{
	PROFILER_AUTO_BLOCK_DEFINE("CharacterMatchManager::requestMatch");
	const unsigned long startTimeMs = Clock::timeMs();
	++ms_numberOfCharacterMatchRequests;

	ServerObject * const requestServerObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(networkId));
	Client * const requestClient = (requestServerObject ? requestServerObject->getClient() : nullptr);
	CreatureObject * const requestCreatureObject = (requestServerObject ? requestServerObject->asCreatureObject() : nullptr);
	PlayerObject * const requestPlayerObject = (requestCreatureObject ? PlayerCreatureController::getPlayerObject(requestCreatureObject) : nullptr);

	if (requestCreatureObject && requestPlayerObject && requestClient)
	{
		BitArray const & searchAttribute = matchMakingCharacterPreferenceId.getSearchAttribute();
		std::string const searchSubstring = Unicode::toLower(matchMakingCharacterPreferenceId.getSubString());

		bool const debugOutput = requestCreatureObject->getObjVars().hasItem("characterMatchDebugOutput");

		if (debugOutput)
		{
			ConsoleMgr::broadcastString("*****BEGIN REQUESTED SEARCH ATTRIBUTE*****", requestClient);

			std::string searchAttributeDebugString = LfgDataTable::getSetLfgNodeDebugString(searchAttribute);
			if (searchAttributeDebugString.empty())
				ConsoleMgr::broadcastString("(no search attribute specified)", requestClient);
			else
				ConsoleMgr::broadcastString(searchAttributeDebugString, requestClient);

			if (!searchSubstring.empty())
			{
				std::string s = "substring is (";
				s += searchSubstring;
				s += ")";

				ConsoleMgr::broadcastString(s, requestClient);
			}

			ConsoleMgr::broadcastString("*****END REQUESTED SEARCH ATTRIBUTE*****", requestClient);
		}

		// if "friend" is one of the search criteria, this will contain the requester's friends list
		PlayerObject::StringVector const * requestPlayerObjectSortedLowercaseFriendList = nullptr;

		// if "cts_source_galaxy" is one of the search criteria, this will contain the requester's CTS source galaxy list
		std::set<std::string> const * requestPlayerObjectCtsSourceGalaxy = nullptr;

		// if "in_same_guild" is one of the search criteria, this will contain the requester's guild abbrev
		bool inSameGuildSearch = false;
		std::string requestPlayerObjectGuildAbbrev;

		// if "citizen_of_same_city" is one of the search criteria, this will contain the requester's citizenship city
		bool citizenOfSameCitySearch = false;
		std::string requestPlayerObjectCitizenOfCity;

		// if "cts_source_galaxy" is one of the search criteria, this will contain the list of matching CTS source galaxy
		std::vector<std::string> * matchingCtsSourceGalaxy = nullptr;

		// allows search of characters marked anonymous
		bool bypassAnonymous = false;

		// allows location of characters to be displayed even if it is marked to not display
		bool bypassHideLocation = false;

		// no search results limit
		bool bypassSearchResultsLimit = false;

		// list of specified search attributes
		//
		// the "key" is the LfgDataTable::LfgNode::anyAllGroupingParent
		//
		// the "value" is the list of leaf LfgDataTable::LfgNode that
		// specifie the attribute the player wants to search on, and
		// if it's an "internal" attribute, it may include a 
		// LfgInternalAttributeMatchFunctionParams structure that will
		// contain additional information to pass to the "match" function
		// for that particular "internal" attribute
		std::map<LfgDataTable::LfgNode const *, std::vector<std::pair<LfgDataTable::LfgNode const *, LfgInternalAttributeMatchFunctionParams> > > specifiedSearchAttributes;

		// build the list of specified search attributes
		unsigned long lowValue, highValue;
		std::map<std::string, LfgDataTable::LfgNode const *> const & allLeafNodes = LfgDataTable::getAllLfgLeafNodes();
		for (std::map<std::string, LfgDataTable::LfgNode const *>::const_iterator iterLeafNode = allLeafNodes.begin(); iterLeafNode != allLeafNodes.end(); ++iterLeafNode)
		{
			if (iterLeafNode->second->minValue > 0)
			{
				LfgDataTable::getLowHighValue(*iterLeafNode->second, lowValue, highValue, searchAttribute);
				if ((lowValue > 0) && (highValue > 0))
				{
					LfgInternalAttributeMatchFunctionParams params;
					params.param1 = reinterpret_cast<void const *>(lowValue);
					params.param2 = reinterpret_cast<void const *>(highValue);

					specifiedSearchAttributes[iterLeafNode->second->anyAllGroupingParent].push_back(std::make_pair(iterLeafNode->second, params));
				}
			}
			else
			{
				if (LfgDataTable::testBit(*iterLeafNode->second, searchAttribute))
				{
					// special handling for "friend" search attribute
					if (iterLeafNode->second->name == "friend")
					{
						if (requestPlayerObjectSortedLowercaseFriendList == nullptr)
						{
							requestPlayerObjectSortedLowercaseFriendList = &(requestPlayerObject->getSortedLowercaseFriendList());
						}

						LfgInternalAttributeMatchFunctionParams params;
						params.param1 = requestPlayerObjectSortedLowercaseFriendList;

						specifiedSearchAttributes[iterLeafNode->second->anyAllGroupingParent].push_back(std::make_pair(iterLeafNode->second, params));
					}
					// special handling for "cts_source_galaxy" search attribute
					else if (iterLeafNode->second->name == "cts_source_galaxy")
					{
						if (requestPlayerObjectCtsSourceGalaxy == nullptr)
						{
							std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
							std::map<NetworkId, LfgCharacterData>::const_iterator iterFindLfg = connectedCharacterLfgData.find(networkId);
							if (iterFindLfg != connectedCharacterLfgData.end())
								requestPlayerObjectCtsSourceGalaxy = &(iterFindLfg->second.ctsSourceGalaxy);
						}

						if (matchingCtsSourceGalaxy == nullptr)
							matchingCtsSourceGalaxy = new std::vector<std::string>;

						LfgInternalAttributeMatchFunctionParams params;
						params.param1 = requestPlayerObjectCtsSourceGalaxy;
						params.param2 = matchingCtsSourceGalaxy;

						specifiedSearchAttributes[iterLeafNode->second->anyAllGroupingParent].push_back(std::make_pair(iterLeafNode->second, params));
					}
					// special handling for "in_same_guild" search attribute
					else if (iterLeafNode->second->name == "in_same_guild")
					{
						inSameGuildSearch = true;

						if (requestPlayerObjectGuildAbbrev.empty())
						{
							int const guildId = requestCreatureObject->getGuildId();
							if (guildId != 0)
								requestPlayerObjectGuildAbbrev = GuildInterface::getGuildAbbrev(guildId);
						}

						// a guild member searching for other guild members
						// will return all matches and ignore the anonymous
						// and hide location settings; that is, guild member
						// can always search for other guild members and will
						// always see the location of other guild members and
						// will return all guild members
						if (!requestPlayerObjectGuildAbbrev.empty())
						{
							bypassAnonymous = true;
							bypassHideLocation = true;
							bypassSearchResultsLimit = true;
						}

						// don't need to include this search condition in the search
						// tree, because we'll do it here locally for optimization
					}
					// special handling for "citizen_of_same_city" search attribute
					else if (iterLeafNode->second->name == "citizen_of_same_city")
					{
						citizenOfSameCitySearch = true;

						if (requestPlayerObjectCitizenOfCity.empty())
						{
							std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(requestCreatureObject->getNetworkId());
							if (!cityIds.empty())
								requestPlayerObjectCitizenOfCity = CityInterface::getCityInfo(cityIds.front()).getCityName();
						}

						// a citizen searching for other citizens in the same
						// city will return all matches and ignore the anonymous
						// and hide location settings; that is, citizen can
						// always search for other citizens in the same city and
						// will always see the location of other citizens and
						// will return all citizens in the same city
						if (!requestPlayerObjectCitizenOfCity.empty())
						{
							bypassAnonymous = true;
							bypassHideLocation = true;
							bypassSearchResultsLimit = true;
						}

						// don't need to include this search condition in the search
						// tree, because we'll do it here locally for optimization
					}
					else
					{
						specifiedSearchAttributes[iterLeafNode->second->anyAllGroupingParent].push_back(std::make_pair(iterLeafNode->second, LfgInternalAttributeMatchFunctionParams()));
					}
				}
			}
		}

		// go through the Any/All grouping parent nodes, and set each to
		// the Any/All value as specified by the user, if applicable
		for (std::map<LfgDataTable::LfgNode const *, std::vector<std::pair<LfgDataTable::LfgNode const *, LfgInternalAttributeMatchFunctionParams> > >::const_iterator iterAnyAllParentNode = specifiedSearchAttributes.begin(); iterAnyAllParentNode != specifiedSearchAttributes.end(); ++iterAnyAllParentNode)
		{
			if (iterAnyAllParentNode->first)
			{
				LfgDataTable::LfgNode & lfgAnyAllParentNode = *(const_cast<LfgDataTable::LfgNode *>(iterAnyAllParentNode->first));
				if ((lfgAnyAllParentNode.defaultMatchCondition == LfgDataTable::DMCT_Any) || (lfgAnyAllParentNode.defaultMatchCondition == LfgDataTable::DMCT_All))
				{
					if (LfgDataTable::testAnyOrAllBit(lfgAnyAllParentNode, searchAttribute))
						lfgAnyAllParentNode.actualMatchCondition = LfgDataTable::DMCT_All;
					else
						lfgAnyAllParentNode.actualMatchCondition = LfgDataTable::DMCT_Any;
				}
				else
				{
					lfgAnyAllParentNode.actualMatchCondition = lfgAnyAllParentNode.defaultMatchCondition;
				}
			}
		}

		if (debugOutput)
		{
			ConsoleMgr::broadcastString("\n*****BEGIN PARSED SEARCH ATTRIBUTE*****", requestClient);

			for (std::map<LfgDataTable::LfgNode const *, std::vector<std::pair<LfgDataTable::LfgNode const *, LfgInternalAttributeMatchFunctionParams> > >::const_iterator iterAnyAllParentNode = specifiedSearchAttributes.begin(); iterAnyAllParentNode != specifiedSearchAttributes.end(); ++iterAnyAllParentNode)
			{
				if (iterAnyAllParentNode->first)
				{
					std::string allAnyType;
					switch (iterAnyAllParentNode->first->actualMatchCondition)
					{
						case LfgDataTable::DMCT_NA:	
							allAnyType = "(NA)";
							break;
						case LfgDataTable::DMCT_Any:
							allAnyType = "(Any)";
							break;
						case LfgDataTable::DMCT_AnyOnly:
							allAnyType = "(AnyOnly)";
							break;
						case LfgDataTable::DMCT_All:
							allAnyType = "(All)";
							break;
						case LfgDataTable::DMCT_AllOnly:
							allAnyType = "(AllOnly)";
							break;
						default:
							allAnyType = "(Unknown)";
							break;
					}

					ConsoleMgr::broadcastString(FormattedString<512>().sprintf("%s %s", iterAnyAllParentNode->first->name.c_str(), allAnyType.c_str()), requestClient);
				}
				else
				{
					ConsoleMgr::broadcastString("(nullptr) (All)", requestClient);
				}

				for (std::vector<std::pair<LfgDataTable::LfgNode const *, LfgInternalAttributeMatchFunctionParams> >::const_iterator iterSearchAttribute = iterAnyAllParentNode->second.begin(); iterSearchAttribute != iterAnyAllParentNode->second.end(); ++iterSearchAttribute)
				{
					if (iterSearchAttribute->first->minValue > 0)
						ConsoleMgr::broadcastString(FormattedString<512>().sprintf("    %s (%lu, %lu)", iterSearchAttribute->first->name.c_str(), reinterpret_cast<unsigned long>(iterSearchAttribute->second.param1), reinterpret_cast<unsigned long>(iterSearchAttribute->second.param2)), requestClient);
					else
						ConsoleMgr::broadcastString(FormattedString<512>().sprintf("    %s", iterSearchAttribute->first->name.c_str()), requestClient);
				}
			}

			ConsoleMgr::broadcastString("*****END PARSED SEARCH ATTRIBUTE*****\n\n", requestClient);
		}

		// now go through the character list to find matching characters
		std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
		std::map<NetworkId, Unicode::String> const & connectedCharacterBiographyData = ServerUniverse::getConnectedCharacterBiographyData();

		// to give each character an equal chance of getting matched, we are going to
		// randomly pick characters to look for a match; we will create a random list
		// of searchable characters and do the search from that list
		static size_t sizeVectorSearchableCharacterLfgData = 2000; // to avoid reallocating a new vector each time, allocate a large enough vector, and if necessary, increase it later
		static std::vector<const LfgCharacterData *> vectorSearchableCharacterLfgData(sizeVectorSearchableCharacterLfgData);

		if (sizeVectorSearchableCharacterLfgData < connectedCharacterLfgData.size())
		{
			sizeVectorSearchableCharacterLfgData = connectedCharacterLfgData.size();
			vectorSearchableCharacterLfgData.resize(sizeVectorSearchableCharacterLfgData);
		}

		size_t numberSearchableCharacters = 0;

		// if in_same_guild search specified and searcher is not in a guild, will not match any results, so just return;
		// if citizen_of_same_city search specified and searcher is not a citizen of a city, will not match any results, so just return
		if ((inSameGuildSearch && requestPlayerObjectGuildAbbrev.empty()) ||
			(citizenOfSameCitySearch && requestPlayerObjectCitizenOfCity.empty()))
		{
		}
		else
		{
			size_t randomIdx;

			for (std::map<NetworkId, LfgCharacterData>::const_iterator iter = connectedCharacterLfgData.begin(); iter != connectedCharacterLfgData.end(); ++iter)
			{
				// ignore unsearchable characters
				if (iter->second.anonymous && !bypassAnonymous)
					continue;

				// ignore self
				if (iter->second.characterId == networkId)
					continue;

				// if in_same_guild search attribute specified, only include characters from matching guild
				if (inSameGuildSearch && (requestPlayerObjectGuildAbbrev != iter->second.guildAbbrev))
					continue;

				// if citizen_of_same_city search attribute specified, only include characters from matching city
				if (citizenOfSameCitySearch && (requestPlayerObjectCitizenOfCity != iter->second.citizenOfCity))
					continue;

				if (numberSearchableCharacters == 0)
				{
					vectorSearchableCharacterLfgData[0] = &(iter->second);
				}
				else if (inSameGuildSearch || citizenOfSameCitySearch)
				{
					// if it's a in_same_guild or citizen_of_same_city search,
					// we want to return a stable search results so don't randomize
					vectorSearchableCharacterLfgData[numberSearchableCharacters] = &(iter->second);
				}
				else
				{
					// insert the searchable character at a random place in the list, and move the
					// character currently at that random place in the list to the end of the list
					randomIdx = ::rand() % (numberSearchableCharacters + 1);
					vectorSearchableCharacterLfgData[numberSearchableCharacters] = vectorSearchableCharacterLfgData[randomIdx];
					vectorSearchableCharacterLfgData[randomIdx] = &(iter->second);
				}

				++numberSearchableCharacters;
			}
		}

		// for a matching character, this contains the attributes that
		// resulted in the match; it is not necessarily *ALL* the
		// matching attributes, because we're doing short-circuit
		// evaluation here
		BitArray matchingAttributes;

		// final matching results are stored here
		MatchMakingCharacterResult mmcr;

		// search for matching characters

		// start search at a random place in the random list unless it's
		// a in_same_guild or citizen_of_same_city search for which
		// we want to return a stable search results so don't randomize
		size_t currentIndex = ((inSameGuildSearch || citizenOfSameCitySearch || (numberSearchableCharacters == 0)) ? 0 : (::rand() % numberSearchableCharacters));

		size_t numberSearchableCharactersChecked = 0;
		while (numberSearchableCharactersChecked < numberSearchableCharacters)
		{
			matchingAttributes.clear();
			if (isMatch(specifiedSearchAttributes, searchSubstring, *(vectorSearchableCharacterLfgData[currentIndex]), matchingAttributes))
			{
				// already found maximum number of matches allowed
				if ((mmcr.m_matchingCharacterData.size() >= ConfigServerGame::getCharacterMatchMaxMatchCount()) && !bypassSearchResultsLimit)
				{
					mmcr.m_hasMoreMatches = true;
					break;
				}

				// Create the result
				LfgCharacterData const & matchingLfgCharacterData = *(vectorSearchableCharacterLfgData[currentIndex]);
				mmcr.m_matchingCharacterData.push_back(matchingLfgCharacterData);
				LfgCharacterSearchResultData & lfgCharacterData = mmcr.m_matchingCharacterData[mmcr.m_matchingCharacterData.size() - 1];
				lfgCharacterData.characterInterests = matchingAttributes;

				// If matching character is grouped, include group member list in returned result
				if (lfgCharacterData.groupId.isValid() && (mmcr.m_matchingCharacterGroup.count(lfgCharacterData.groupId) == 0))
				{
					ServerObject const * const soGroupObject = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(lfgCharacterData.groupId));
					GroupObject const * const groupObject = (soGroupObject ? soGroupObject->asGroupObject() : nullptr);
					if (groupObject)
					{
						std::vector<MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo> & groupInfo = mmcr.m_matchingCharacterGroup[lfgCharacterData.groupId];
						MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo groupMemberInfo;

						GroupObject::GroupMemberVector const & groupMembers = groupObject->getGroupMembers();
						unsigned int groupMemberIndex = 0;
						for (GroupObject::GroupMemberVector::const_iterator iterGroupMember = groupMembers.begin(); iterGroupMember != groupMembers.end(); ++iterGroupMember)
						{
							groupMemberInfo.groupMemberId = iterGroupMember->first;

							// only return group member name if the group member is searchable
							if (groupObject->isMemberPC(iterGroupMember->first))
							{
								std::map<NetworkId, LfgCharacterData>::const_iterator iterGroupMemberData = connectedCharacterLfgData.find(iterGroupMember->first);
								if ((iterGroupMemberData == connectedCharacterLfgData.end()) || iterGroupMemberData->second.anonymous)
									groupMemberInfo.groupMemberName = "(anonymous)";
								else
									groupMemberInfo.groupMemberName = iterGroupMember->second;
							}
							else
							{
								groupMemberInfo.groupMemberName = iterGroupMember->second;
							}

							groupMemberInfo.groupMemberLevel = groupObject->getMemberLevelByMemberIndex(groupMemberIndex);
							groupMemberInfo.groupMemberProfession = groupObject->getMemberProfessionByMemberIndex(groupMemberIndex);

							++groupMemberIndex;

							groupInfo.push_back(groupMemberInfo);
						}
					}
					else
					{
						lfgCharacterData.groupId = NetworkId::cms_invalid;
					}
				}

				std::map<NetworkId, Unicode::String>::const_iterator iterBiography = connectedCharacterBiographyData.find(lfgCharacterData.characterId);
				if ((iterBiography != connectedCharacterBiographyData.end()) && !iterBiography->second.empty())
					mmcr.m_matchingCharacterBiography[lfgCharacterData.characterId] = iterBiography->second;

				if (debugOutput)
				{
					ConsoleMgr::broadcastString(matchingLfgCharacterData.getDebugString(), requestClient);

					if (mmcr.m_matchingCharacterBiography.count(lfgCharacterData.characterId) > 0)
					{
						ConsoleMgr::broadcastString("Biography:", requestClient);
						ConsoleMgr::broadcastString(mmcr.m_matchingCharacterBiography[lfgCharacterData.characterId], requestClient);
						ConsoleMgr::broadcastString("\n", requestClient);
					}
					else
					{
						ConsoleMgr::broadcastString("(no biography)\n\n", requestClient);
					}
				}

				// See if common CTS source galaxy should be returned
				lfgCharacterData.ctsSourceGalaxy.clear();
				if (matchingCtsSourceGalaxy && !matchingCtsSourceGalaxy->empty())
				{
					for (std::vector<std::string>::const_iterator iter = matchingCtsSourceGalaxy->begin(); iter != matchingCtsSourceGalaxy->end(); ++iter)
						IGNORE_RETURN(lfgCharacterData.ctsSourceGalaxy.insert(*iter));
				}

				// See if matching character's location should be returned
				if (!matchingLfgCharacterData.displayLocationInSearchResults && !bypassHideLocation)
				{
					lfgCharacterData.locationPlanet.clear();
					lfgCharacterData.locationRegion.clear();
					lfgCharacterData.locationPlayerCity.clear();
				}
			}

			++numberSearchableCharactersChecked;

			++currentIndex;
			if (currentIndex >= numberSearchableCharacters) // wraparound
				currentIndex = 0;
		}

		delete matchingCtsSourceGalaxy;

		// To prevent skewing statistics when more than the limits is returned
		// (for guild member and citizen search), use the limits if the number
		// of results exceed the limits
		ms_numberOfCharacterMatchResults += std::min(mmcr.m_matchingCharacterData.size(), ConfigServerGame::getCharacterMatchMaxMatchCount());

		// Send the results to the creature object, even if there is no matches
		requestCreatureObject->onCharacterMatchRetrieved(mmcr);
	}

	const unsigned long endTimeMs = Clock::timeMs();
	if (endTimeMs >= startTimeMs)
	{
		ms_timeSpentOnCharacterMatchRequestsMs += (endTimeMs - startTimeMs);
	}
	else // time wrapped
	{
		static const unsigned long max = std::numeric_limits<unsigned long>::max();
		ms_timeSpentOnCharacterMatchRequestsMs += (max - startTimeMs + endTimeMs);
	}
}

// ============================================================================
