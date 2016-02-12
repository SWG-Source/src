// ======================================================================
//
// CitizenRankDataTable.cpp
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CitizenRankDataTable.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>

// ======================================================================

namespace CitizenRankDataTableNamespace
{
	char const * const cs_citizenRankDataTableName = "datatables/city/citizen_rank.iff";

	std::vector<std::string> s_allCitizenRankNames;
	std::vector<CitizenRankDataTable::CitizenRank const *> s_allCitizenRanks;
	std::vector<CitizenRankDataTable::CitizenRank const *> s_allTitleableCitizenRanks;
	std::map<std::string, CitizenRankDataTable::CitizenRank const *> s_allCitizenRanksByName;
	std::map<std::string, CitizenRankDataTable::CitizenRank const *> s_allCitizenRanksByTitle;
}

using namespace CitizenRankDataTableNamespace;

// ======================================================================

void CitizenRankDataTable::install()
{
	DataTable * table = DataTableManager::getTable(cs_citizenRankDataTableName, true);
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

		FATAL((columnRankName < 0), ("column \"rankName\" not found in %s", cs_citizenRankDataTableName));
		FATAL((columnRankSlotId < 0), ("column \"rankSlotId\" not found in %s", cs_citizenRankDataTableName));
		FATAL((columnTitle < 0), ("column \"title\" not found in %s", cs_citizenRankDataTableName));

		CitizenRankDataTable::CitizenRank const * currentRank = nullptr;

		int const numRows = table->getNumRows();
		std::string rankName, alternateTitle;
		int slotId;
		bool title;
		std::vector<std::string> titles;
		std::map<std::string, int> names;
		std::map<int, CitizenRankDataTable::CitizenRank const *> allRanksById;
		for (int i = 0; i < numRows; ++i)
		{
			rankName = table->getStringValue(columnRankName, i);

			if (rankName.empty())
				continue;

			FATAL((names.count(rankName) >= 1), ("%s, row %d: rank name %s already used at row %d (either as a rank name or rank alternate title)", cs_citizenRankDataTableName, (i+3), rankName.c_str(), names[rankName]));
			names[rankName] = (i+3);

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
					FATAL((names.count(alternateTitle) >= 1), ("%s, row %d: alternate title %s already used at row %d (either as a rank name or rank alternate title)", cs_citizenRankDataTableName, (i+3), alternateTitle.c_str(), names[alternateTitle]));
					names[alternateTitle] = (i+3);

					titles.push_back(alternateTitle);
				}
			}

			FATAL((!titles.empty() && !title), ("%s: rank %s cannot have any alternate titles unless it is defined as \"titleable\")", cs_citizenRankDataTableName, rankName.c_str()));

			// check for valid slot id
			FATAL((slotId < 0), ("%s, row %d: rank slot id %d must be >= 0", cs_citizenRankDataTableName, (i+3), slotId));

			// create citizen rank info
			currentRank = new CitizenRank(rankName, slotId, titles);

			// check for duplicate slot id
			FATAL((allRanksById.count(slotId) >= 1), ("%s, row %d: rank slot id %d already used by rank %s", cs_citizenRankDataTableName, (i+3), slotId, allRanksById[slotId]->name.c_str()));
			allRanksById[slotId] = currentRank;

			s_allCitizenRankNames.push_back(currentRank->name);
			s_allCitizenRanks.push_back(currentRank);

			if (!currentRank->titles.empty())
				s_allTitleableCitizenRanks.push_back(currentRank);

			s_allCitizenRanksByName[currentRank->name] = currentRank;

			for (std::vector<std::string>::const_iterator iterTitle = currentRank->titles.begin(); iterTitle != currentRank->titles.end(); ++iterTitle)
				s_allCitizenRanksByTitle[*iterTitle] = currentRank;
		}

		DataTableManager::close(cs_citizenRankDataTableName);

		// do additional verification
		slotId = -1;
		for (std::map<int, CitizenRankDataTable::CitizenRank const *>::const_iterator iterSlotId = allRanksById.begin(); iterSlotId != allRanksById.end(); ++iterSlotId)
		{
			FATAL((iterSlotId->first != iterSlotId->second->slotId), ("%s: slot id mismatch for slot %s (%d, %d)", cs_citizenRankDataTableName, iterSlotId->second->name.c_str(), iterSlotId->first, iterSlotId->second->slotId));

			// make sure that slot ids start at 0 and there are no "holes" in the slot ids
			if (iterSlotId == allRanksById.begin())
			{
				FATAL((iterSlotId->first != 0), ("%s: slot id must start at 0", cs_citizenRankDataTableName));
			}
			else
			{
				FATAL(((slotId + 1) != iterSlotId->first), ("%s: slot id must be contiguous (there is a \"hole\" between %d and %d)", cs_citizenRankDataTableName, slotId, iterSlotId->first));
			}

			slotId = iterSlotId->first;
		}
	}
	else
	{
		FATAL(true, ("citizen rank datatable %s not found", cs_citizenRankDataTableName));
	}

	ExitChain::add(remove, "CitizenRankDataTable::remove");
}

//----------------------------------------------------------------------

void CitizenRankDataTable::remove()
{
	for (std::vector<CitizenRankDataTable::CitizenRank const *>::const_iterator iter = s_allCitizenRanks.begin(); iter != s_allCitizenRanks.end(); ++iter)
		delete *iter;

	s_allCitizenRankNames.clear();
	s_allCitizenRanks.clear();
	s_allTitleableCitizenRanks.clear();
	s_allCitizenRanksByName.clear();
	s_allCitizenRanksByTitle.clear();
}

//----------------------------------------------------------------------

std::vector<std::string> const & CitizenRankDataTable::getAllRankNames()
{
	return s_allCitizenRankNames;
}

//----------------------------------------------------------------------

std::vector<CitizenRankDataTable::CitizenRank const *> const & CitizenRankDataTable::getAllRanks()
{
	return s_allCitizenRanks;
}

//----------------------------------------------------------------------

std::vector<CitizenRankDataTable::CitizenRank const *> const & CitizenRankDataTable::getAllTitleableRanks()
{
	return s_allTitleableCitizenRanks;
}

//----------------------------------------------------------------------

CitizenRankDataTable::CitizenRank const * CitizenRankDataTable::getRank(std::string const & rankName)
{
	std::map<std::string, CitizenRankDataTable::CitizenRank const *>::const_iterator iterFind = s_allCitizenRanksByName.find(rankName);
	if (iterFind != s_allCitizenRanksByName.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------

CitizenRankDataTable::CitizenRank const * CitizenRankDataTable::isARankTitle(std::string const & rankTitle)
{
	std::map<std::string, CitizenRankDataTable::CitizenRank const *>::const_iterator iterFind = s_allCitizenRanksByTitle.find(rankTitle);
	if (iterFind != s_allCitizenRanksByTitle.end())
		return iterFind->second;

	return nullptr;
}

// ======================================================================
