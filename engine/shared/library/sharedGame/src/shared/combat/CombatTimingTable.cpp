// ======================================================================
//
// CombatTimingTable.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CombatTimingTable.h"

#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTable.h"

// ======================================================================

namespace CombatTimingTableNamespace
{
	// ----------------------------------------------------------------------

	char const * const cs_combatTimingTableName = "datatables/combat/combat_timing.iff";

	// ----------------------------------------------------------------------
}

// ======================================================================

using namespace CombatTimingTableNamespace;

int CombatTimingTable::getMaximumShotsTillWeaponReload(std::string const & weaponType)
{
	int result = 0;

	DataTable * combatTimingTable = DataTableManager::getTable(cs_combatTimingTableName, true);
	if (combatTimingTable)
	{
		int rowNum = combatTimingTable->searchColumnString(0, weaponType);
		result = combatTimingTable->getIntValue("MaximumShotsTillWeaponReload", rowNum);
	}

	return result;
}

bool CombatTimingTable::isContinuous(std::string const & weaponType)
{
	bool result = false;

	DataTable * combatTimingTable = DataTableManager::getTable(cs_combatTimingTableName, true);
	if (combatTimingTable)
	{
		int rowNum = combatTimingTable->searchColumnString(0, weaponType);
		result = combatTimingTable->getIntValue("Continuous", rowNum) ? true : false;
	}

	return result;
}

float CombatTimingTable::getWeaponReloadTimeSeconds(std::string const & weaponType)
{
	float result = 0;

	DataTable * combatTimingTable = DataTableManager::getTable(cs_combatTimingTableName, true);
	if (combatTimingTable)
	{
		int rowNum = combatTimingTable->searchColumnString(0, weaponType);
		result = combatTimingTable->getFloatValue("WeaponReloadTimeSeconds", rowNum);
	}

	return result;
}

const char *CombatTimingTable::getWeaponReloadClientEffect(std::string const & weaponType)
{
	const char * result = 0;

	DataTable * combatTimingTable = DataTableManager::getTable(cs_combatTimingTableName, true);
	if (combatTimingTable)
	{
		int rowNum = combatTimingTable->searchColumnString(0, weaponType);
		result = combatTimingTable->getStringValue("WeaponReloadClientEffect", rowNum);
	}

	return result;
}

const char *CombatTimingTable::getWeaponReloadClientAnimation(std::string const & weaponType)
{
	const char * result = 0;

	DataTable * combatTimingTable = DataTableManager::getTable(cs_combatTimingTableName, true);
	if (combatTimingTable)
	{
		int rowNum = combatTimingTable->searchColumnString(0, weaponType);
		result = combatTimingTable->getStringValue("WeaponReloadClientAnimation", rowNum);
	}

	return result;
}

// ======================================================================
