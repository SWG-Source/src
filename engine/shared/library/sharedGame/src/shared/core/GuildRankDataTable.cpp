// ======================================================================
//
// GuildRankDataTable.cpp
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/GuildRankDataTable.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"

#include <map>

// ======================================================================

namespace GuildRankDataTableNamespace
{
	char const * const cs_guildRankDataTableName = "datatables/guild/guild_rank.iff";

	std::vector<std::string> s_allGuildRankNames;
	std::vector<GuildRankDataTable::GuildRank const *> s_allGuildRanks;
	std::vector<GuildRankDataTable::GuildRank const *> s_allTitleableGuildRanks;
	std::map<std::string, GuildRankDataTable::GuildRank const *> s_allGuildRanksByName;
	std::map<std::string, GuildRankDataTable::GuildRank const *> s_allGuildRanksByDisplayName;
	std::map<std::string, GuildRankDataTable::GuildRank const *> s_allGuildRanksByTitle;
}

using namespace GuildRankDataTableNamespace;

// ======================================================================

void GuildRankDataTable::install()
{
	DataTable * table = DataTableManager::getTable(cs_guildRankDataTableName, true);
	if (table)
	{
		int const columnRankName = table->findColumnNumber("rankName");
		int const columnRankSlotId = table->findColumnNumber("rankSlotId");
		int const columnTitle = table->findColumnNumber("title");

		// the can be a variable number of "alternate title" columns, as long as the columns
		// are named alternateTitle1, alternateTitle2, alternateTitle3, alternateTitle4,
		// alternateTitle5, and so on
		std::vector<int> columnAlternateTitle;
		char buffer[128];
		int columnNumber;
		for (int i = 1; i <= 1000000000; ++i)
		{
			snprintf(buffer, sizeof(buffer)-1, "alternateTitle%d", i);
			buffer[sizeof(buffer)-1] = '\0';

			columnNumber = table->findColumnNumber(buffer);
			if (columnNumber < 0)
				break;

			columnAlternateTitle.push_back(columnNumber);
		}

		FATAL((columnRankName < 0), ("column \"rankName\" not found in %s", cs_guildRankDataTableName));
		FATAL((columnRankSlotId < 0), ("column \"rankSlotId\" not found in %s", cs_guildRankDataTableName));
		FATAL((columnTitle < 0), ("column \"title\" not found in %s", cs_guildRankDataTableName));

		GuildRankDataTable::GuildRank const * currentRank = nullptr;

		int const numRows = table->getNumRows();
		std::string rankName, alternateTitle;
		Unicode::String displayRankName;
		int slotId;
		bool title;
		std::vector<std::string> titles;
		std::map<std::string, int> names;
		std::map<int, GuildRankDataTable::GuildRank const *> allRanksById;
		for (int i = 0; i < numRows; ++i)
		{
			rankName = table->getStringValue(columnRankName, i);

			if (rankName.empty())
				continue;

			FATAL((names.count(rankName) >= 1), ("%s, row %d: rank name %s already used at row %d (either as a rank name or rank alternate title)", cs_guildRankDataTableName, (i+3), rankName.c_str(), names[rankName]));
			names[rankName] = (i+3);

			FATAL(!StringId("guild_rank_title", rankName).localize(displayRankName, true), ("%s, row %d: unable to load display string for guild rank %s from guild_rank_title.stf", cs_guildRankDataTableName, (i+3), rankName.c_str()));
			FATAL((s_allGuildRanksByDisplayName.count(Unicode::toLower(Unicode::wideToNarrow(displayRankName))) > 0), ("%s, row %d: guild rank %s has display string %s from guild_rank_title.stf that is already used by another guild rank", cs_guildRankDataTableName, (i+3), rankName.c_str(), Unicode::wideToNarrow(displayRankName).c_str()));

			slotId = table->getIntValue(columnRankSlotId, i);
			title = (table->getIntValue(columnTitle, i) != 0);

			// read all alternate titles
			titles.clear();
			if (title)
				titles.push_back(rankName);

			for (std::vector<int>::const_iterator iterColumnAlternateTitle = columnAlternateTitle.begin(); iterColumnAlternateTitle != columnAlternateTitle.end(); ++iterColumnAlternateTitle)
			{
				alternateTitle = table->getStringValue(*iterColumnAlternateTitle, i);

				if (!alternateTitle.empty())
				{
					FATAL((names.count(alternateTitle) >= 1), ("%s, row %d: alternate title %s already used at row %d (either as a rank name or rank alternate title)", cs_guildRankDataTableName, (i+3), alternateTitle.c_str(), names[alternateTitle]));
					names[alternateTitle] = (i+3);

					titles.push_back(alternateTitle);
				}
			}

			FATAL((!titles.empty() && !title), ("%s: rank %s cannot have any alternate titles unless it is defined as \"titleable\")", cs_guildRankDataTableName, rankName.c_str()));

			// check for valid slot id
			FATAL((slotId < 0), ("%s, row %d: rank slot id %d must be >= 0", cs_guildRankDataTableName, (i+3), slotId));

			// create guild rank info
			currentRank = new GuildRank(rankName, Unicode::wideToNarrow(displayRankName), slotId, titles);

			// check for duplicate slot id
			FATAL((allRanksById.count(slotId) >= 1), ("%s, row %d: rank slot id %d already used by rank %s", cs_guildRankDataTableName, (i+3), slotId, allRanksById[slotId]->name.c_str()));
			allRanksById[slotId] = currentRank;

			s_allGuildRankNames.push_back(currentRank->name);
			s_allGuildRanks.push_back(currentRank);

			if (!currentRank->titles.empty())
				s_allTitleableGuildRanks.push_back(currentRank);

			s_allGuildRanksByName[currentRank->name] = currentRank;
			s_allGuildRanksByDisplayName[Unicode::toLower(currentRank->displayName)] = currentRank;

			for (std::vector<std::string>::const_iterator iterTitle = currentRank->titles.begin(); iterTitle != currentRank->titles.end(); ++iterTitle)
				s_allGuildRanksByTitle[*iterTitle] = currentRank;
		}

		DataTableManager::close(cs_guildRankDataTableName);

		// do additional verification
		slotId = -1;
		for (std::map<int, GuildRankDataTable::GuildRank const *>::const_iterator iterSlotId = allRanksById.begin(); iterSlotId != allRanksById.end(); ++iterSlotId)
		{
			FATAL((iterSlotId->first != iterSlotId->second->slotId), ("%s: slot id mismatch for slot %s (%d, %d)", cs_guildRankDataTableName, iterSlotId->second->name.c_str(), iterSlotId->first, iterSlotId->second->slotId));

			// make sure that slot ids start at 0 and there are no "holes" in the slot ids
			if (iterSlotId == allRanksById.begin())
			{
				FATAL((iterSlotId->first != 0), ("%s: slot id must start at 0", cs_guildRankDataTableName));
			}
			else
			{
				FATAL(((slotId + 1) != iterSlotId->first), ("%s: slot id must be contiguous (there is a \"hole\" between %d and %d)", cs_guildRankDataTableName, slotId, iterSlotId->first));
			}

			slotId = iterSlotId->first;
		}
	}
	else
	{
		FATAL(true, ("guild rank datatable %s not found", cs_guildRankDataTableName));
	}

	ExitChain::add(remove, "GuildRankDataTable::remove");
}

//----------------------------------------------------------------------

void GuildRankDataTable::remove()
{
	for (std::vector<GuildRankDataTable::GuildRank const *>::const_iterator iter = s_allGuildRanks.begin(); iter != s_allGuildRanks.end(); ++iter)
		delete *iter;

	s_allGuildRankNames.clear();
	s_allGuildRanks.clear();
	s_allTitleableGuildRanks.clear();
	s_allGuildRanksByName.clear();
	s_allGuildRanksByDisplayName.clear();
	s_allGuildRanksByTitle.clear();
}

//----------------------------------------------------------------------

std::vector<std::string> const & GuildRankDataTable::getAllRankNames()
{
	return s_allGuildRankNames;
}

//----------------------------------------------------------------------

std::vector<GuildRankDataTable::GuildRank const *> const & GuildRankDataTable::getAllRanks()
{
	return s_allGuildRanks;
}

//----------------------------------------------------------------------

std::vector<GuildRankDataTable::GuildRank const *> const & GuildRankDataTable::getAllTitleableRanks()
{
	return s_allTitleableGuildRanks;
}

//----------------------------------------------------------------------

GuildRankDataTable::GuildRank const * GuildRankDataTable::getRank(std::string const & rankName)
{
	std::map<std::string, GuildRankDataTable::GuildRank const *>::const_iterator iterFind = s_allGuildRanksByName.find(rankName);
	if (iterFind != s_allGuildRanksByName.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------

GuildRankDataTable::GuildRank const * GuildRankDataTable::getRankForDisplayRankName(std::string const & displayRankName)
{
	std::map<std::string, GuildRankDataTable::GuildRank const *>::const_iterator iterFind = s_allGuildRanksByDisplayName.find(Unicode::toLower(displayRankName));
	if (iterFind != s_allGuildRanksByDisplayName.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------

GuildRankDataTable::GuildRank const * GuildRankDataTable::isARankTitle(std::string const & rankTitle)
{
	std::map<std::string, GuildRankDataTable::GuildRank const *>::const_iterator iterFind = s_allGuildRanksByTitle.find(rankTitle);
	if (iterFind != s_allGuildRanksByTitle.end())
		return iterFind->second;

	return nullptr;
}

// ======================================================================
