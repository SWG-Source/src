// ======================================================================
//
// PvpFactions.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpFactions.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <cstdio>
#include <map>
#include <set>

// ======================================================================

namespace PvpFactionsNamespace
{
	bool s_factionTablesLoaded = false;
	std::set<Pvp::FactionId> s_nonaggressiveFactions;
	std::set<Pvp::FactionId> s_unattackableFactions;
	std::set<Pvp::FactionId> s_bountyTargetFactions;
	std::map<Pvp::FactionId, std::vector<Pvp::FactionId> > s_factionOpponents;
	ConstCharCrcString s_battlefieldFaction("battlefield");
	ConstCharCrcString s_duelFaction("duel");
	ConstCharCrcString s_bountyDuelFaction("bountyduel");
	ConstCharCrcString s_guildWarCoolDownPeriodFaction("guildwarcooldownperiod");
	ConstCharCrcString s_bubbleCombatFaction("bubblecombat");

	// ----------------------------------------------------------------------

	void loadFactionTables()
	{
		// nonaggressive factions
		{
			char const *fileName = ConfigServerGame::getPvpNonaggressiveFactionsFilename();
			DataTable *t = DataTableManager::getTable(fileName, true);
			FATAL(!t, ("Could not find pvp nonaggressive factions table %s", fileName));
			for (int row = 0; row < t->getNumRows(); ++row)
				IGNORE_RETURN(s_nonaggressiveFactions.insert(static_cast<Pvp::FactionId>(t->getIntValue("faction", row))));
			DataTableManager::close(fileName);
		}
		// unattackable factions
		{
			char const *fileName = ConfigServerGame::getPvpUnattackableFactionsFilename();
			DataTable *t = DataTableManager::getTable(fileName, true);
			FATAL(!t, ("Could not find pvp unattackable factions table %s", fileName));
			for (int row = 0; row < t->getNumRows(); ++row)
				IGNORE_RETURN(s_unattackableFactions.insert(static_cast<Pvp::FactionId>(t->getIntValue("faction", row))));
			DataTableManager::close(fileName);
		}
		// bounty target factions
		{
			char const *fileName = ConfigServerGame::getPvpBountyTargetFactionsFilename();
			DataTable *t = DataTableManager::getTable(fileName, true);
			FATAL(!t, ("Could not find pvp bounty target factions table %s", fileName));
			for (int row = 0; row < t->getNumRows(); ++row)
				IGNORE_RETURN(s_bountyTargetFactions.insert(static_cast<Pvp::FactionId>(t->getIntValue("faction", row))));
			DataTableManager::close(fileName);
		}
		// faction opponents
		{
			char const *fileName = ConfigServerGame::getPvpFactionOpponentsFilename();
			DataTable *t = DataTableManager::getTable(fileName, true);
			FATAL(!t, ("Could not find pvp faction opponents table %s", fileName));
			for (int row = 0; row < t->getNumRows(); ++row)
			{
				Pvp::FactionId factionId = static_cast<Pvp::FactionId>(t->getIntValue("faction", row));
				int opponentNum = 1;
				bool found;
				do
				{
					char colName[64];
					sprintf(colName, "opponent%d", opponentNum);
					found = t->findColumnNumber(colName) >= 0;
					if (found)
					{
						s_factionOpponents[factionId].push_back(static_cast<Pvp::FactionId>(t->getIntValue(colName, row)));
						++opponentNum;
					}
				} while (found);
			}
			DataTableManager::close(fileName);
		}
		s_factionTablesLoaded = true;
	}
}
using namespace PvpFactionsNamespace;

// ======================================================================

bool PvpFactions::isNonaggressiveFaction(Pvp::FactionId factionId)
{
	if (!s_factionTablesLoaded)
		loadFactionTables();
	return s_nonaggressiveFactions.find(factionId) != s_nonaggressiveFactions.end();
}

// ----------------------------------------------------------------------

bool PvpFactions::isUnattackableFaction(Pvp::FactionId factionId)
{
	if (!s_factionTablesLoaded)
		loadFactionTables();
	return s_unattackableFactions.find(factionId) != s_unattackableFactions.end();
}

// ----------------------------------------------------------------------

bool PvpFactions::isBountyTargetFaction(Pvp::FactionId factionId)
{
	if (!s_factionTablesLoaded)
		loadFactionTables();
	return s_bountyTargetFactions.find(factionId) != s_bountyTargetFactions.end();
}

// ----------------------------------------------------------------------

bool PvpFactions::isBubbleFaction(Pvp::FactionId factionId)
{
	return factionId == s_bubbleCombatFaction.getCrc();
}

// ----------------------------------------------------------------------

std::vector<Pvp::FactionId> const &PvpFactions::getOpposingFactions(Pvp::FactionId factionId)
{
	static std::vector<Pvp::FactionId> nullFactionVector;

	if (!s_factionTablesLoaded)
		loadFactionTables();

	std::map<Pvp::FactionId, std::vector<Pvp::FactionId> >::const_iterator i = s_factionOpponents.find(factionId);
	if (i != s_factionOpponents.end())
		return (*i).second;
	return nullFactionVector;
}

// ----------------------------------------------------------------------

Pvp::FactionId PvpFactions::getBattlefieldFactionId()
{
	return s_battlefieldFaction.getCrc();
}

// ----------------------------------------------------------------------

Pvp::FactionId PvpFactions::getDuelFactionId()
{
	return s_duelFaction.getCrc();
}

// ----------------------------------------------------------------------

Pvp::FactionId PvpFactions::getBountyDuelFactionId()
{
	return s_bountyDuelFaction.getCrc();
}

// ----------------------------------------------------------------------

Pvp::FactionId PvpFactions::getGuildWarCoolDownPeriodFactionId()
{
	return s_guildWarCoolDownPeriodFaction.getCrc();
}

// ----------------------------------------------------------------------

Pvp::FactionId PvpFactions::getBubbleCombatFactionId()
{
	return s_bubbleCombatFaction.getCrc();
}

// ======================================================================
