// ======================================================================
//
// LfgDataTable.cpp
// Copyright 2008 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/LfgDataTable.h"

#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/LfgCharacterData.h"
#include "sharedGame/PvpData.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace LfgDataTableNamespace
{
	char const * const cs_lfgDataTableName = "datatables/lfg/lfg.iff";
	
	std::vector<LfgDataTable::LfgNode const *> s_topLevelNodes;
	std::map<std::string, LfgDataTable::LfgNode const *> s_allNodesByName;
	std::map<std::string, LfgDataTable::LfgNode const *> s_allLeafNodesByName;

	void getSetLfgNodeDebugString(LfgDataTable::LfgNode const & lfgNode, std::string & sOutput, std::string sIndention, std::set<LfgDataTable::LfgNode const *> const & allParentNodesWithSetLeafDescendants, std::map<LfgDataTable::LfgNode const *, std::pair<unsigned long, unsigned long> > const & allSetLeafNodes, BitArray const & ba, bool displayAnyOrAllValue)
	{
		if (lfgNode.isLeafNode())
		{
			std::map<LfgDataTable::LfgNode const *, std::pair<unsigned long, unsigned long> >::const_iterator iterFind = allSetLeafNodes.find(&lfgNode);
			if (iterFind != allSetLeafNodes.end())
			{
				sOutput += sIndention;
				sOutput += lfgNode.name;

				if (lfgNode.minValue > 0)
				{
					char buffer[128];
					snprintf(buffer, sizeof(buffer)-1, " (%lu, %lu)", iterFind->second.first, iterFind->second.second);
					buffer[sizeof(buffer)-1] = '\0';

					sOutput += std::string(buffer);
				}

				sOutput += "\n";
			}
		}
		else if (allParentNodesWithSetLeafDescendants.count(&lfgNode) > 0)
		{
			sOutput += sIndention;
			sOutput += lfgNode.name;

			if (displayAnyOrAllValue)
			{
				if ((lfgNode.defaultMatchCondition == LfgDataTable::DMCT_Any) || (lfgNode.defaultMatchCondition == LfgDataTable::DMCT_All))
				{
					if (LfgDataTable::testAnyOrAllBit(lfgNode, ba))
						sOutput += " (All)";
					else
						sOutput += " (Any)";
				}
			}

			sOutput += "\n";

			sIndention += "    ";
			for (std::vector<LfgDataTable::LfgNode const *>::const_iterator iterNode = lfgNode.children.begin(); iterNode != lfgNode.children.end(); ++iterNode)
			{
				getSetLfgNodeDebugString(**iterNode, sOutput, sIndention, allParentNodesWithSetLeafDescendants, allSetLeafNodes, ba, displayAnyOrAllValue);
			}
		}
	}

	bool matchFactionImperial(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return PvpData::isImperialFactionId(lfgCharacterData.faction);
	}

	bool matchFactionNeutral(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return PvpData::isNeutralFactionId(lfgCharacterData.faction);
	}

	bool matchFactionRebel(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return PvpData::isRebelFactionId(lfgCharacterData.faction);
	}

	bool matchGenderFemale(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.gender == SharedCreatureObjectTemplate::GE_female);
	}

	bool matchGenderMale(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.gender == SharedCreatureObjectTemplate::GE_male);
	}

	bool matchLevel(LfgCharacterData const & lfgCharacterData, void const * lowLevel, void const * highLevel, void const *, void const *, void const *)
	{
		return ((static_cast<unsigned long>(lfgCharacterData.level) >= reinterpret_cast<unsigned long>(lowLevel)) && (static_cast<unsigned long>(lfgCharacterData.level) <= reinterpret_cast<unsigned long>(highLevel)));
	}

	bool matchFriend(LfgCharacterData const & lfgCharacterData, void const * pFriendList, void const *, void const *, void const *, void const *)
	{
		// pFriendList points to the lowercase, sorted friend list
		if (!pFriendList)
			return false;

		std::vector<std::string> const & friendList = *(reinterpret_cast<std::vector<std::string> const *>(pFriendList));

		return std::binary_search(friendList.begin(), friendList.end(), lfgCharacterData.characterLowercaseFirstName);
	}

	bool matchCommonCtsSourceGalaxy(LfgCharacterData const & lfgCharacterData, void const * pCtsSourceGalaxy, void const * pMatchingCtsSourceGalaxy, void const *, void const *, void const *)
	{
		// pCtsSourceGalaxy points to the list of CTS source galaxies
		if (lfgCharacterData.ctsSourceGalaxy.empty() || !lfgCharacterData.searchableByCtsSourceGalaxy || !pCtsSourceGalaxy)
			return false;

		std::set<std::string> const & ctsSourceGalaxy = *(reinterpret_cast<std::set<std::string> const *>(pCtsSourceGalaxy));
		if (ctsSourceGalaxy.empty())
			return false;

		// pMatchingCtsSourceGalaxy, if specified, points to a std::vector<std::string> that will be populated with the matching CTS source galaxies
		static std::vector<std::string> sMatchingCtsSourceGalaxy;
		std::vector<std::string> * matchingCtsSourceGalaxy = &sMatchingCtsSourceGalaxy;
		if (pMatchingCtsSourceGalaxy)
			matchingCtsSourceGalaxy = reinterpret_cast<std::vector<std::string> *>(const_cast<void *>(pMatchingCtsSourceGalaxy));

		matchingCtsSourceGalaxy->clear();
		IGNORE_RETURN(std::set_intersection(lfgCharacterData.ctsSourceGalaxy.begin(), lfgCharacterData.ctsSourceGalaxy.end(), ctsSourceGalaxy.begin(), ctsSourceGalaxy.end(), std::back_inserter(*matchingCtsSourceGalaxy)));
		return (!matchingCtsSourceGalaxy->empty());
	}

	bool matchInSameGuild(LfgCharacterData const & lfgCharacterData, void const * pSearcherGuildAbbrev, void const *, void const *, void const *, void const *)
	{
		// pSearcherGuildAbbrev points to the guild that the searcher is in
		if (lfgCharacterData.guildAbbrev.empty() || !pSearcherGuildAbbrev)
			return false;

		std::string const & searcherGuildAbbrev = *(reinterpret_cast<std::string const *>(pSearcherGuildAbbrev));
		if (searcherGuildAbbrev.empty())
			return false;

		return (searcherGuildAbbrev == lfgCharacterData.guildAbbrev);
	}

	bool matchCitizenOfSameCity(LfgCharacterData const & lfgCharacterData, void const * pSearcherCitizenOfCity, void const *, void const *, void const *, void const *)
	{
		// pSearcherCitizenOfCity points to the city that the searcher is a citizen of
		if (lfgCharacterData.citizenOfCity.empty() || !pSearcherCitizenOfCity)
			return false;

		std::string const & searcherCitizenOfCity = *(reinterpret_cast<std::string const *>(pSearcherCitizenOfCity));
		if (searcherCitizenOfCity.empty())
			return false;

		return (searcherCitizenOfCity == lfgCharacterData.citizenOfCity);
	}

	bool matchPilotNeutral(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.pilot == LfgCharacterData::Pilot_Neutral);
	}

	bool matchPilotImperial(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.pilot == LfgCharacterData::Pilot_Imperial);
	}

	bool matchPilotRebel(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.pilot == LfgCharacterData::Pilot_Rebel);
	}

	bool matchProfBh(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Bh);
	}

	bool matchProfCommando(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Commando);
	}

	bool matchProfEntertainer(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Entertainer);
	}

	bool matchProfJedi(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Jedi);
	}

	bool matchProfMedic(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Medic);
	}

	bool matchProfOfficer(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Officer);
	}

	bool matchProfSmuggler(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Smuggler);
	}

	bool matchProfSpy(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Spy);
	}

	bool matchProfTraderDomestics(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Trader_Domestics);
	}

	bool matchProfTraderEngineering(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Trader_Engineering);
	}

	bool matchProfTraderMunitions(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Trader_Munitions);
	}

	bool matchProfTraderStructures(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.profession == LfgCharacterData::Prof_Trader_Structures);
	}

	bool matchSpeciesBothan(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_bothan);
	}

	bool matchSpeciesHuman(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_human);
	}

	bool matchSpeciesIthorian(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_ithorian);
	}

	bool matchSpeciesMonCalamari(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_monCalamari);
	}

	bool matchSpeciesRodian(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_rodian);
	}

	bool matchSpeciesSullustan(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_sullustan);
	}

	bool matchSpeciesTrandoshan(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_trandoshan);
	}

	bool matchSpeciesTwilek(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_twilek);
	}

	bool matchSpeciesWookiee(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_wookiee);
	}

	bool matchSpeciesZabrak(LfgCharacterData const & lfgCharacterData, void const *, void const *, void const *, void const *, void const *)
	{
		return (lfgCharacterData.species == SharedCreatureObjectTemplate::SP_zabrak);
	}
}

using namespace LfgDataTableNamespace;

// ======================================================================

void LfgDataTable::install()
{
	// set up the table containing the "match" function for every
	// "internal" attribute defined in the lfg datatable
	std::map<std::string, LfgDataTable::pf> mapInternalSearchAttributeMatchFunction;

	mapInternalSearchAttributeMatchFunction["imperial"] = matchFactionImperial;
	mapInternalSearchAttributeMatchFunction["neutral"] = matchFactionNeutral;
	mapInternalSearchAttributeMatchFunction["rebel"] = matchFactionRebel;
	mapInternalSearchAttributeMatchFunction["female"] = matchGenderFemale;
	mapInternalSearchAttributeMatchFunction["male"] = matchGenderMale;
	mapInternalSearchAttributeMatchFunction["level"] = matchLevel;
	mapInternalSearchAttributeMatchFunction["friend"] = matchFriend;
	mapInternalSearchAttributeMatchFunction["cts_source_galaxy"] = matchCommonCtsSourceGalaxy;
	mapInternalSearchAttributeMatchFunction["in_same_guild"] = matchInSameGuild;
	mapInternalSearchAttributeMatchFunction["citizen_of_same_city"] = matchCitizenOfSameCity;
	mapInternalSearchAttributeMatchFunction["pilot_neutral"] = matchPilotNeutral;
	mapInternalSearchAttributeMatchFunction["pilot_imperial"] = matchPilotImperial;
	mapInternalSearchAttributeMatchFunction["pilot_rebel"] = matchPilotRebel;
	mapInternalSearchAttributeMatchFunction["prof_bh"] = matchProfBh;
	mapInternalSearchAttributeMatchFunction["prof_commando"] = matchProfCommando;
	mapInternalSearchAttributeMatchFunction["prof_entertainer"] = matchProfEntertainer;
	mapInternalSearchAttributeMatchFunction["prof_jedi"] = matchProfJedi;
	mapInternalSearchAttributeMatchFunction["prof_medic"] = matchProfMedic;
	mapInternalSearchAttributeMatchFunction["prof_officer"] = matchProfOfficer;
	mapInternalSearchAttributeMatchFunction["prof_smuggler"] = matchProfSmuggler;
	mapInternalSearchAttributeMatchFunction["prof_spy"] = matchProfSpy;
	mapInternalSearchAttributeMatchFunction["prof_trader_domestics"] = matchProfTraderDomestics;
	mapInternalSearchAttributeMatchFunction["prof_trader_engineering"] = matchProfTraderEngineering;
	mapInternalSearchAttributeMatchFunction["prof_trader_munitions"] = matchProfTraderMunitions;
	mapInternalSearchAttributeMatchFunction["prof_trader_structures"] = matchProfTraderStructures;
	mapInternalSearchAttributeMatchFunction["bothan"] = matchSpeciesBothan;
	mapInternalSearchAttributeMatchFunction["human"] = matchSpeciesHuman;
	mapInternalSearchAttributeMatchFunction["ithorian"] = matchSpeciesIthorian;
	mapInternalSearchAttributeMatchFunction["mon_calamari"] = matchSpeciesMonCalamari;
	mapInternalSearchAttributeMatchFunction["rodian"] = matchSpeciesRodian;
	mapInternalSearchAttributeMatchFunction["sullustan"] = matchSpeciesSullustan;
	mapInternalSearchAttributeMatchFunction["trandoshan"] = matchSpeciesTrandoshan;
	mapInternalSearchAttributeMatchFunction["twilek"] = matchSpeciesTwilek;
	mapInternalSearchAttributeMatchFunction["wookiee"] = matchSpeciesWookiee;
	mapInternalSearchAttributeMatchFunction["zabrak"] = matchSpeciesZabrak;

	DataTable * table = DataTableManager::getTable(cs_lfgDataTableName, true);
	if (table)
	{
		int const columnTier1 = table->findColumnNumber("tier1");
		int const columnTier2 = table->findColumnNumber("tier2");
		int const columnTier3 = table->findColumnNumber("tier3");
		int const columnTier4 = table->findColumnNumber("tier4");
		int const columnInternalAttribute = table->findColumnNumber("internalAttribute");
		int const columnMinValueBeginSlotId = table->findColumnNumber("minValueBeginSlotId");
		int const columnMinValueEndSlotId = table->findColumnNumber("minValueEndSlotId");
		int const columnMaxValueBeginSlotId = table->findColumnNumber("maxValueBeginSlotId");
		int const columnMaxValueEndSlotId = table->findColumnNumber("maxValueEndSlotId");
		int const columnMinValue = table->findColumnNumber("minValue");
		int const columnMaxValue = table->findColumnNumber("maxValue");
		int const columnDefaultMatchCondition = table->findColumnNumber("defaultMatchCondition");

		FATAL((columnTier1 < 0), ("column \"tier1\" not found in %s", cs_lfgDataTableName));
		FATAL((columnTier2 < 0), ("column \"tier2\" not found in %s", cs_lfgDataTableName));
		FATAL((columnTier3 < 0), ("column \"tier3\" not found in %s", cs_lfgDataTableName));
		FATAL((columnTier4 < 0), ("column \"tier4\" not found in %s", cs_lfgDataTableName));
		FATAL((columnInternalAttribute < 0), ("column \"internalAttribute\" not found in %s", cs_lfgDataTableName));
		FATAL((columnMinValueBeginSlotId < 0), ("column \"minValueBeginSlotId\" not found in %s", cs_lfgDataTableName));
		FATAL((columnMinValueEndSlotId < 0), ("column \"minValueEndSlotId\" not found in %s", cs_lfgDataTableName));
		FATAL((columnMaxValueBeginSlotId < 0), ("column \"maxValueBeginSlotId\" not found in %s", cs_lfgDataTableName));
		FATAL((columnMaxValueEndSlotId < 0), ("column \"maxValueEndSlotId\" not found in %s", cs_lfgDataTableName));
		FATAL((columnMinValue < 0), ("column \"minValue\" not found in %s", cs_lfgDataTableName));
		FATAL((columnMaxValue < 0), ("column \"maxValue\" not found in %s", cs_lfgDataTableName));
		FATAL((columnDefaultMatchCondition < 0), ("column \"defaultMatchCondition\" not found in %s", cs_lfgDataTableName));

		int const numRows = table->getNumRows();

		// datatable values
		std::string tier1, tier2, tier3, tier4;
		bool internalAttribute;
		int minValueBeginSlotId, minValueEndSlotId, maxValueBeginSlotId, maxValueEndSlotId, minValue, maxValue;
		DefaultMatchConditionType defaultMatchCondition;
		
		// temps
		int tempCount;
		unsigned long maxValueForNumBits;
		std::map<std::string, int> names;
		std::map<int, LfgNode const *> allSlotsById;
		LfgNode * lfgNode = nullptr;
		LfgNode * currentTier1Node = nullptr;
		LfgNode * currentTier2Node = nullptr;
		LfgNode * currentTier3Node = nullptr;

		for (int i = 0; i < numRows; ++i)
		{
			tier1 = table->getStringValue(columnTier1, i);
			tier2 = table->getStringValue(columnTier2, i);
			tier3 = table->getStringValue(columnTier3, i);
			tier4 = table->getStringValue(columnTier4, i);

			if (tier1.empty() && tier2.empty() && tier3.empty() && tier4.empty())
				continue;

			// only one of tier1, tier2, tier3, or tier4 can be specified
			// and tier1, tier2, tier3, and tier4 must be unique
			tempCount = 0;
			if (!tier1.empty())
			{
				FATAL((names.count(tier1) >= 1), ("%s, row %d: tier1 name %s already used at row %d (either as a tier1, tier2, tier3, or tier4 name)", cs_lfgDataTableName, (i+3), tier1.c_str(), names[tier1]));
				names[tier1] = (i+3);
				++tempCount;
			}
			if (!tier2.empty())
			{
				FATAL((names.count(tier2) >= 1), ("%s, row %d: tier2 name %s already used at row %d (either as a tier1, tier2, tier3, or tier4 name)", cs_lfgDataTableName, (i+3), tier2.c_str(), names[tier2]));
				names[tier2] = (i+3);
				++tempCount;
			}
			if (!tier3.empty())
			{
				FATAL((names.count(tier3) >= 1), ("%s, row %d: tier3 name %s already used at row %d (either as a tier1, tier2, tier3, or tier4 name)", cs_lfgDataTableName, (i+3), tier3.c_str(), names[tier3]));
				names[tier3] = (i+3);
				++tempCount;
			}
			if (!tier4.empty())
			{
				FATAL((names.count(tier4) >= 1), ("%s, row %d: tier4 name %s already used at row %d (either as a tier1, tier2, tier3, or tier4 name)", cs_lfgDataTableName, (i+3), tier4.c_str(), names[tier4]));
				names[tier4] = (i+3);
				++tempCount;
			}

			FATAL((tempCount != 1), ("%s, row %d: only one of tier1, tier2, tier3, or tier4 can be specified", cs_lfgDataTableName, (i+3)));

			internalAttribute = (table->getIntValue(columnInternalAttribute, i) != 0);
			minValueBeginSlotId = table->getIntValue(columnMinValueBeginSlotId, i);
			minValueEndSlotId = table->getIntValue(columnMinValueEndSlotId, i);
			maxValueBeginSlotId = table->getIntValue(columnMaxValueBeginSlotId, i);
			maxValueEndSlotId = table->getIntValue(columnMaxValueEndSlotId, i);
			minValue = table->getIntValue(columnMinValue, i);
			maxValue = table->getIntValue(columnMaxValue, i);
			defaultMatchCondition = static_cast<DefaultMatchConditionType>(table->getIntValue(columnDefaultMatchCondition, i));

			// verify min/max value
			FATAL(((minValue != -1) && (minValue <= 0)), ("%s, row %d: minValue must be > 0", cs_lfgDataTableName, (i+3)));
			FATAL(((maxValue != -1) && (maxValue <= 0)), ("%s, row %d: maxValue must be > 0", cs_lfgDataTableName, (i+3)));
			FATAL(((minValue > 0) && (maxValue <= 0)), ("%s, row %d: maxValue must be specified if minValue is specified", cs_lfgDataTableName, (i+3)));
			FATAL(((maxValue > 0) && (minValue <= 0)), ("%s, row %d: minValue must be specified if maxValue is specified", cs_lfgDataTableName, (i+3)));
			FATAL(((maxValue > 0) && (maxValue <= minValue)), ("%s, row %d: maxValue must be > minValue", cs_lfgDataTableName, (i+3)));

			// verify slot ids
			FATAL(((minValueBeginSlotId != -1) && (minValueBeginSlotId < 0)), ("%s, row %d: minValueBeginSlotId must be >= 0", cs_lfgDataTableName, (i+3)));
			FATAL(((minValueEndSlotId != -1) && (minValueEndSlotId < 0)), ("%s, row %d: minValueEndSlotId must be >= 0", cs_lfgDataTableName, (i+3)));
			FATAL(((maxValueBeginSlotId != -1) && (maxValueBeginSlotId < 0)), ("%s, row %d: maxValueBeginSlotId must be >= 0", cs_lfgDataTableName, (i+3)));
			FATAL(((maxValueEndSlotId != -1) && (maxValueEndSlotId < 0)), ("%s, row %d: maxValueEndSlotId must be >= 0", cs_lfgDataTableName, (i+3)));

			FATAL(((minValueEndSlotId >= 0) && (minValueBeginSlotId < 0)), ("%s, row %d: minValueBeginSlotId must be specified if minValueEndSlotId is specified", cs_lfgDataTableName, (i+3)));
			FATAL(((minValueEndSlotId >= 0) && (minValueEndSlotId <= minValueBeginSlotId)), ("%s, row %d: minValueEndSlotId must be > minValueBeginSlotId", cs_lfgDataTableName, (i+3)));

			FATAL(((maxValueBeginSlotId >= 0) && (maxValueEndSlotId < 0)), ("%s, row %d: maxValueEndSlotId must be specified if maxValueBeginSlotId is specified", cs_lfgDataTableName, (i+3)));
			FATAL(((maxValueEndSlotId >= 0) && (maxValueBeginSlotId < 0)), ("%s, row %d: maxValueBeginSlotId must be specified if maxValueEndSlotId is specified", cs_lfgDataTableName, (i+3)));
			FATAL(((maxValueEndSlotId >= 0) && (maxValueEndSlotId <= maxValueBeginSlotId)), ("%s, row %d: maxValueEndSlotId must be > maxValueBeginSlotId", cs_lfgDataTableName, (i+3)));

			FATAL(((maxValueBeginSlotId >= 0) && (minValueEndSlotId < 0)), ("%s, row %d: minValueBeginSlotId/minValueEndSlotId must be specified if maxValueBeginSlotId/maxValueEndSlotId is specified", cs_lfgDataTableName, (i+3)));
			FATAL(((minValueEndSlotId >= 0) && (maxValueBeginSlotId < 0)), ("%s, row %d: maxValueBeginSlotId/maxValueEndSlotId must be specified if minValueBeginSlotId/minValueEndSlotId is specified", cs_lfgDataTableName, (i+3)));

			if (maxValueBeginSlotId >= 0)
			{
				FATAL(((minValueEndSlotId - minValueBeginSlotId) > 31), ("%s, row %d: minValueBeginSlotId/minValueEndSlotId cannot exceed 32 bits", cs_lfgDataTableName, (i+3)));
				FATAL(((maxValueEndSlotId - maxValueBeginSlotId) > 31), ("%s, row %d: maxValueBeginSlotId/maxValueEndSlotId cannot exceed 32 bits", cs_lfgDataTableName, (i+3)));
				FATAL(((maxValueEndSlotId - maxValueBeginSlotId) != (minValueEndSlotId - minValueBeginSlotId)), ("%s, row %d: maxValueBeginSlotId/maxValueEndSlotId must use the same number of slots as minValueBeginSlotId/minValueEndSlotId", cs_lfgDataTableName, (i+3)));
				FATAL((minValue <= 0), ("%s, row %d: minValue/maxValue must be specified if minValueBeginSlotId/minValueEndSlotId/maxValueBeginSlotId/maxValueEndSlotId is specified", cs_lfgDataTableName, (i+3)));
				FATAL((!internalAttribute), ("%s, row %d: internalAttribute must be true if minValueBeginSlotId/minValueEndSlotId/maxValueBeginSlotId/maxValueEndSlotId is specified", cs_lfgDataTableName, (i+3)));

				unsigned long const numBits = static_cast<unsigned long>(maxValueEndSlotId - maxValueBeginSlotId + 1);
				maxValueForNumBits = (0xffffffff >> (32 - numBits));
				FATAL((maxValueForNumBits < static_cast<unsigned long>(maxValue)), ("%s, row %d: minValueBeginSlotId/minValueEndSlotId uses %lu bits, which can only hold a max value of %lu, which is less than the specified max value of %d", cs_lfgDataTableName, (i+3), numBits, maxValueForNumBits, maxValue));
			}

			FATAL(((minValue > 0) && (maxValueBeginSlotId < 0)), ("%s, row %d: minValueBeginSlotId/minValueEndSlotId/maxValueBeginSlotId/maxValueEndSlotId must be specified if minValue/maxValue is specified", cs_lfgDataTableName, (i+3)));

			// create a new node
			lfgNode = nullptr;
			if (!tier1.empty())
			{
				lfgNode = new LfgNode(tier1, internalAttribute, minValueBeginSlotId, minValueEndSlotId, maxValueBeginSlotId, maxValueEndSlotId, minValue, maxValue, defaultMatchCondition, nullptr);
				s_topLevelNodes.push_back(lfgNode);

				currentTier1Node = lfgNode;
				currentTier2Node = nullptr;
				currentTier3Node = nullptr;
			}
			else if (!tier2.empty())
			{
				FATAL((!currentTier1Node), ("%s, row %d: tier2 %s specified without a parent tier1", cs_lfgDataTableName, (i+3), tier2.c_str()));

				lfgNode = new LfgNode(tier2, internalAttribute, minValueBeginSlotId, minValueEndSlotId, maxValueBeginSlotId, maxValueEndSlotId, minValue, maxValue, defaultMatchCondition, currentTier1Node);
				currentTier1Node->children.push_back(lfgNode);

				currentTier2Node = lfgNode;
				currentTier3Node = nullptr;
			}
			else if (!tier3.empty())
			{
				FATAL((!currentTier2Node), ("%s, row %d: tier3 %s specified without a parent tier2", cs_lfgDataTableName, (i+3), tier3.c_str()));

				lfgNode = new LfgNode(tier3, internalAttribute, minValueBeginSlotId, minValueEndSlotId, maxValueBeginSlotId, maxValueEndSlotId, minValue, maxValue, defaultMatchCondition, currentTier2Node);
				currentTier2Node->children.push_back(lfgNode);

				currentTier3Node = lfgNode;
			}
			else
			{
				FATAL((!currentTier3Node), ("%s, row %d: tier4 %s specified without a parent tier3", cs_lfgDataTableName, (i+3), tier4.c_str()));

				lfgNode = new LfgNode(tier4, internalAttribute, minValueBeginSlotId, minValueEndSlotId, maxValueBeginSlotId, maxValueEndSlotId, minValue, maxValue, defaultMatchCondition, currentTier3Node);
				currentTier3Node->children.push_back(lfgNode);
			}

			s_allNodesByName[lfgNode->name] = lfgNode;

			// verify slot ids are not reused
			if (minValueBeginSlotId >= 0)
			{
				if (minValueEndSlotId >= 0)
				{
					int j;
					for (j = minValueBeginSlotId; j <= minValueEndSlotId; ++j)
					{
						FATAL((allSlotsById.count(j) >= 1), ("%s, row %d: slot id %d already used by node %s", cs_lfgDataTableName, (i+3), j, allSlotsById[j]->name.c_str()));
						allSlotsById[j] = lfgNode;
					}

					for (j = maxValueBeginSlotId; j <= maxValueEndSlotId; ++j)
					{
						FATAL((allSlotsById.count(j) >= 1), ("%s, row %d: slot id %d already used by node %s", cs_lfgDataTableName, (i+3), j, allSlotsById[j]->name.c_str()));
						allSlotsById[j] = lfgNode;
					}
				}
				else
				{
					FATAL((allSlotsById.count(minValueBeginSlotId) >= 1), ("%s, row %d: slot id %d already used by node %s", cs_lfgDataTableName, (i+3), minValueBeginSlotId, allSlotsById[minValueBeginSlotId]->name.c_str()));
					allSlotsById[minValueBeginSlotId] = lfgNode;
				}
			}
		}

		// make sure that slot ids start at 0 and there are no "holes" in the slot ids
		int firstSlotId = -1;
		for (std::map<int, LfgNode const *>::const_iterator iterSlotId = allSlotsById.begin(); iterSlotId != allSlotsById.end(); ++iterSlotId)
		{
			if (iterSlotId == allSlotsById.begin())
			{
				FATAL((iterSlotId->first != 0), ("%s: slot id assignment must start at 0", cs_lfgDataTableName));
			}
			else
			{
				FATAL(((firstSlotId + 1) != iterSlotId->first), ("%s: slot id assignment must be contiguous (there is a \"hole\" between %d and %d)", cs_lfgDataTableName, firstSlotId, iterSlotId->first));
			}

			firstSlotId = iterSlotId->first;
		}

		// check each node to make sure they don't violate any "rules"
		for (std::map<std::string, LfgNode const *>::const_iterator iterNode = s_allNodesByName.begin(); iterNode != s_allNodesByName.end(); ++iterNode)
		{
			LfgNode const & node = *(iterNode->second);

			FATAL((node.name != iterNode->first), ("%s: node name mismatch (%s, %s)", node.name.c_str(), iterNode->first.c_str()));

			if (node.isLeafNode())
			{
				FATAL((node.defaultMatchCondition != DMCT_NA), ("%s: node %s is a leaf node and must have defaultMatchCondition value of NA", cs_lfgDataTableName, node.name.c_str()));
				FATAL((node.minValueBeginSlotId < 0), ("%s: node %s is a leaf node and must be assigned to a slot id for minValueBeginSlotId", cs_lfgDataTableName, node.name.c_str()));

				s_allLeafNodesByName[iterNode->first] = iterNode->second;

				// if the leaf node is an "internal" attribute, set the "matching"
				// function that can be used to determine if the character data
				// "matches" the "internal" attribute
				if (node.internalAttribute)
				{
					std::map<std::string, LfgDataTable::pf>::const_iterator iterMatchFunction = mapInternalSearchAttributeMatchFunction.find(node.name);
					FATAL((iterMatchFunction == mapInternalSearchAttributeMatchFunction.end()), ("%s: node %s is an \"internal\" attribute but doesn't have a corresponding \"match\" function defined", cs_lfgDataTableName, node.name.c_str()));

					const_cast<LfgNode &>(node).internalAttributeMatchFunction = iterMatchFunction->second;
				}

				// find out the leaf node's ancestor, if any, that has the Any/All option
				LfgNode const * parentNode = node.parent;
				bool const hasParent = (parentNode != nullptr);
				int countParentWithNonNaDefaultMatchCondition = 0;
				std::string stringParentWithNonNaDefaultMatchCondition;
				while (parentNode)
				{
					if (node.internalAttribute)
						const_cast<LfgNode *>(parentNode)->hasAnyInternalAttributeLeafNodeDescendants = true;
					else
						const_cast<LfgNode *>(parentNode)->hasAnyExternalAttributeLeafNodeDescendants = true;

					if (parentNode->defaultMatchCondition != DMCT_NA)
					{
						const_cast<LfgNode &>(node).anyAllGroupingParent = parentNode;

						++countParentWithNonNaDefaultMatchCondition;
						
						if (!stringParentWithNonNaDefaultMatchCondition.empty())
							stringParentWithNonNaDefaultMatchCondition += ", ";

						stringParentWithNonNaDefaultMatchCondition += parentNode->name;
					}

					parentNode = parentNode->parent;
				}

				if (hasParent)
				{
					FATAL((countParentWithNonNaDefaultMatchCondition > 1), ("%s: node %s is a leaf node and cannot have more than 1 ancestor nodes with a non-NA defaultMatchCondition (currently has %d ancestor nodes (%s) with a non-NA defaultMatchCondition)", cs_lfgDataTableName, node.name.c_str(), countParentWithNonNaDefaultMatchCondition, stringParentWithNonNaDefaultMatchCondition.c_str()));
				}
			}
			else
			{
				FATAL((node.internalAttribute), ("%s: node %s is a non-leaf node and cannot have a true value for internalAttribute", cs_lfgDataTableName, node.name.c_str()));
				FATAL((node.minValueEndSlotId >= 0), ("%s: node %s is a non-leaf node and cannot have a value for minValueEndSlotId", cs_lfgDataTableName, node.name.c_str()));
				FATAL((node.maxValueBeginSlotId >= 0), ("%s: node %s is a non-leaf node and cannot have a value for maxValueBeginSlotId", cs_lfgDataTableName, node.name.c_str()));
				FATAL((node.maxValueEndSlotId >= 0), ("%s: node %s is a non-leaf node and cannot have a value for maxValueEndSlotId", cs_lfgDataTableName, node.name.c_str()));
				FATAL((node.minValue > 0), ("%s: node %s is a non-leaf node and cannot have a value for minValue", cs_lfgDataTableName, node.name.c_str()));
				FATAL((node.maxValue > 0), ("%s: node %s is a non-leaf node and cannot have a value for maxValue", cs_lfgDataTableName, node.name.c_str()));

				if ((node.defaultMatchCondition == DMCT_Any) || (node.defaultMatchCondition == DMCT_All))
				{
					FATAL((node.minValueBeginSlotId < 0), ("%s: node %s has a defaultMatchCondition of Any/All so must be assigned to a slot id for minValueBeginSlotId", cs_lfgDataTableName, node.name.c_str()));
				}
				else
				{
					FATAL((node.minValueBeginSlotId >= 0), ("%s: node %s doesn't have a defaultMatchCondition of Any/All so cannot have a value for minValueBeginSlotId", cs_lfgDataTableName, node.name.c_str()));
				}
			}
		}
	}
	else
	{
		FATAL(true, ("lfg datatable %s not found", cs_lfgDataTableName));
	}

	ExitChain::add(remove, "LfgDataTable::remove");
}

//----------------------------------------------------------------------

void LfgDataTable::remove()
{
	for (std::map<std::string, LfgNode const *>::const_iterator iterNode = s_allNodesByName.begin(); iterNode != s_allNodesByName.end(); ++iterNode)
		delete iterNode->second;

	s_topLevelNodes.clear();
	s_allNodesByName.clear();
	s_allLeafNodesByName.clear();
}

//----------------------------------------------------------------------

std::vector<LfgDataTable::LfgNode const *> const & LfgDataTable::getTopLevelLfgNodes()
{
	return s_topLevelNodes;
}

//----------------------------------------------------------------------

std::map<std::string, LfgDataTable::LfgNode const *> const & LfgDataTable::getAllLfgLeafNodes()
{
	return s_allLeafNodesByName;
}

//----------------------------------------------------------------------

LfgDataTable::LfgNode const * LfgDataTable::getLfgNodeByName(std::string const & lfgNodeName)
{
	std::map<std::string, LfgNode const *>::const_iterator iterNode = s_allNodesByName.find(lfgNodeName);
	if (iterNode == s_allNodesByName.end())
		return nullptr;

	return iterNode->second;
}

//----------------------------------------------------------------------

LfgDataTable::LfgNode const * LfgDataTable::getLfgLeafNodeByName(std::string const & lfgNodeName)
{
	std::map<std::string, LfgNode const *>::const_iterator iterNode = s_allLeafNodesByName.find(lfgNodeName);
	if (iterNode == s_allLeafNodesByName.end())
		return nullptr;

	return iterNode->second;
}

//----------------------------------------------------------------------

void LfgDataTable::setBit(std::string const & lfgNodeName, BitArray & ba)
{
	LfgNode const * lfgNode = getLfgNodeByName(lfgNodeName);
	if (lfgNode)
		setBit(*lfgNode, ba);
}

//----------------------------------------------------------------------

void LfgDataTable::setBit(LfgNode const & lfgNode, BitArray & ba)
{
	if (lfgNode.isLeafNode() && (lfgNode.minValueBeginSlotId >= 0) && (lfgNode.minValueEndSlotId < 0))
		ba.setBit(lfgNode.minValueBeginSlotId);
}

//----------------------------------------------------------------------

void LfgDataTable::clearBit(std::string const & lfgNodeName, BitArray & ba)
{
	LfgNode const * lfgNode = getLfgNodeByName(lfgNodeName);
	if (lfgNode)
		clearBit(*lfgNode, ba);
}

//----------------------------------------------------------------------

void LfgDataTable::clearBit(LfgNode const & lfgNode, BitArray & ba)
{
	if (lfgNode.isLeafNode() && (lfgNode.minValueBeginSlotId >= 0) && (lfgNode.minValueEndSlotId < 0))
		ba.clearBit(lfgNode.minValueBeginSlotId);
}

//----------------------------------------------------------------------

bool LfgDataTable::testBit(std::string const & lfgNodeName, BitArray const & ba)
{
	LfgNode const * lfgNode = getLfgNodeByName(lfgNodeName);
	if (lfgNode)
		return testBit(*lfgNode, ba);

	return false;
}

//----------------------------------------------------------------------

bool LfgDataTable::testBit(LfgNode const & lfgNode, BitArray const & ba)
{
	if (lfgNode.isLeafNode() && (lfgNode.minValueBeginSlotId >= 0) && (lfgNode.minValueEndSlotId < 0))
		return ba.testBit(lfgNode.minValueBeginSlotId);

	return false;
}

//----------------------------------------------------------------------

void LfgDataTable::setAnyOrAllBit(std::string const & lfgNodeName, bool any, BitArray & ba)
{
	// any -> clear bit
	// all -> set bit
	LfgNode const * lfgNode = getLfgNodeByName(lfgNodeName);
	if (lfgNode)
		setAnyOrAllBit(*lfgNode, any, ba);
}

//----------------------------------------------------------------------

void LfgDataTable::setAnyOrAllBit(LfgNode const & lfgNode, bool any, BitArray & ba)
{
	// any -> clear bit
	// all -> set bit
	if (!lfgNode.isLeafNode() && ((lfgNode.defaultMatchCondition == LfgDataTable::DMCT_Any) || (lfgNode.defaultMatchCondition == LfgDataTable::DMCT_All)) && (lfgNode.minValueBeginSlotId >= 0) && (lfgNode.minValueEndSlotId < 0))
	{
		if (any)
			ba.clearBit(lfgNode.minValueBeginSlotId);
		else
			ba.setBit(lfgNode.minValueBeginSlotId);
	}
}

//----------------------------------------------------------------------

bool LfgDataTable::testAnyOrAllBit(std::string const & lfgNodeName, BitArray const & ba)
{
	// all -> bit is set (returns true)
	// any -> bit is not set (returns false)
	LfgNode const * lfgNode = getLfgNodeByName(lfgNodeName);
	if (lfgNode)
		return testAnyOrAllBit(*lfgNode, ba);

	return false;
}

//----------------------------------------------------------------------

bool LfgDataTable::testAnyOrAllBit(LfgNode const & lfgNode, BitArray const & ba)
{
	// all -> bit is set (returns true)
	// any -> bit is not set (returns false)
	if (!lfgNode.isLeafNode() && ((lfgNode.defaultMatchCondition == LfgDataTable::DMCT_Any) || (lfgNode.defaultMatchCondition == LfgDataTable::DMCT_All)) && (lfgNode.minValueBeginSlotId >= 0) && (lfgNode.minValueEndSlotId < 0))
		return ba.testBit(lfgNode.minValueBeginSlotId);

	return false;
}

//----------------------------------------------------------------------

void LfgDataTable::setLowHighValue(std::string const & lfgNodeName, unsigned long lowValue, unsigned long highValue, BitArray & ba)
{
	LfgNode const * lfgNode = getLfgNodeByName(lfgNodeName);
	if (lfgNode)
		setLowHighValue(*lfgNode, lowValue, highValue, ba);
}

//----------------------------------------------------------------------

void LfgDataTable::setLowHighValue(LfgNode const & lfgNode, unsigned long lowValue, unsigned long highValue, BitArray & ba)
{
	if ((lfgNode.isLeafNode()) &&
	    (lfgNode.minValueBeginSlotId >= 0) &&
	    (lfgNode.minValueEndSlotId >= 0) &&
	    (lfgNode.minValueBeginSlotId < lfgNode.minValueEndSlotId) &&
	    (lfgNode.maxValueBeginSlotId >= 0) &&
	    (lfgNode.maxValueEndSlotId >= 0) &&
	    (lfgNode.maxValueBeginSlotId < lfgNode.maxValueEndSlotId) &&
	    (lfgNode.minValue > 0) &&
	    (lfgNode.maxValue > 0) &&
	    (lfgNode.minValue < lfgNode.maxValue) &&
	    (lowValue <= highValue)
	   )
	{
		// lowValue and highValue of 0 means "clear"
		if ((lowValue == 0) && (highValue == 0))
		{
			ba.setValue(lfgNode.minValueBeginSlotId, lfgNode.minValueEndSlotId, 0lu);
			ba.setValue(lfgNode.maxValueBeginSlotId, lfgNode.maxValueEndSlotId, 0lu);
		}
		else if ((lowValue >= static_cast<unsigned long>(lfgNode.minValue)) &&
		         (lowValue <= static_cast<unsigned long>(lfgNode.maxValue)) &&
		         (highValue >= static_cast<unsigned long>(lfgNode.minValue)) &&
		         (highValue <= static_cast<unsigned long>(lfgNode.maxValue))
		        )
		{
			ba.setValue(lfgNode.minValueBeginSlotId, lfgNode.minValueEndSlotId, lowValue);
			ba.setValue(lfgNode.maxValueBeginSlotId, lfgNode.maxValueEndSlotId, highValue);
		}
	}
}

//----------------------------------------------------------------------

void LfgDataTable::getLowHighValue(std::string const & lfgNodeName, unsigned long & lowValue, unsigned long & highValue, BitArray const & ba)
{
	lowValue = 0;
	highValue = 0;

	LfgNode const * lfgNode = getLfgNodeByName(lfgNodeName);
	if (lfgNode)
		getLowHighValue(*lfgNode, lowValue, highValue, ba);
}

//----------------------------------------------------------------------

void LfgDataTable::getLowHighValue(LfgNode const & lfgNode, unsigned long & lowValue, unsigned long & highValue, BitArray const & ba)
{
	lowValue = 0;
	highValue = 0;

	if ((lfgNode.isLeafNode()) &&
	    (lfgNode.minValueBeginSlotId >= 0) &&
	    (lfgNode.minValueEndSlotId >= 0) &&
	    (lfgNode.minValueBeginSlotId < lfgNode.minValueEndSlotId) &&
	    (lfgNode.maxValueBeginSlotId >= 0) &&
	    (lfgNode.maxValueEndSlotId >= 0) &&
	    (lfgNode.maxValueBeginSlotId < lfgNode.maxValueEndSlotId) &&
	    (lfgNode.minValue > 0) &&
	    (lfgNode.maxValue > 0) &&
	    (lfgNode.minValue < lfgNode.maxValue)
	   )
	{
		lowValue = ba.getValue(lfgNode.minValueBeginSlotId, lfgNode.minValueEndSlotId);
		highValue = ba.getValue(lfgNode.maxValueBeginSlotId, lfgNode.maxValueEndSlotId);
	}
}

//----------------------------------------------------------------------

std::string LfgDataTable::getSetLfgNodeDebugString(BitArray const & ba, bool displayAnyOrAllValue/*= true*/)
{
	std::string output;
	std::map<LfgDataTable::LfgNode const *, std::pair<unsigned long, unsigned long> > allSetLeafNodes;

	unsigned long lowValue, highValue;
	for (std::map<std::string, LfgDataTable::LfgNode const *>::const_iterator iterLeafNode = s_allLeafNodesByName.begin(); iterLeafNode != s_allLeafNodesByName.end(); ++iterLeafNode)
	{
		if (iterLeafNode->second->minValue > 0)
		{
			getLowHighValue(*iterLeafNode->second, lowValue, highValue, ba);
			if ((lowValue > 0) && (highValue > 0))
			{
				allSetLeafNodes[iterLeafNode->second] = std::make_pair(lowValue, highValue);
			}
		}
		else
		{
			if (testBit(*iterLeafNode->second, ba))
			{
				allSetLeafNodes[iterLeafNode->second] = std::make_pair(0, 0);
			}
		}
	}

	std::set<LfgDataTable::LfgNode const *> allParentNodesWithSetLeafDescendants;
	for (std::map<LfgDataTable::LfgNode const *, std::pair<unsigned long, unsigned long> >::const_iterator iterSetLeafNode = allSetLeafNodes.begin(); iterSetLeafNode != allSetLeafNodes.end(); ++iterSetLeafNode)
	{
		LfgDataTable::LfgNode const * parent = iterSetLeafNode->first->parent;
		while (parent)
		{
			IGNORE_RETURN(allParentNodesWithSetLeafDescendants.insert(parent));
			parent = parent->parent;
		}
	}

	for (std::vector<LfgDataTable::LfgNode const *>::const_iterator iterTopLevelNode = s_topLevelNodes.begin(); iterTopLevelNode != s_topLevelNodes.end(); ++iterTopLevelNode)
	{
		std::string sTopLevelNode;
		LfgDataTableNamespace::getSetLfgNodeDebugString(**iterTopLevelNode, sTopLevelNode, std::string(), allParentNodesWithSetLeafDescendants, allSetLeafNodes, ba, displayAnyOrAllValue);
		output += sTopLevelNode;
	}

	return output;
}
