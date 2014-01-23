// ======================================================================
//
// CombatDataTable.cpp
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CombatDataTable.h"

#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTable.h"

#include <string>
#include <algorithm>
#include <cctype>
#include <unordered_map>

// ======================================================================

namespace CombatDataTableNamespace
{
	char const * const cs_combatDataTableName = "datatables/combat/combat_data.iff";
	std::string const cs_columnActionNameCrc("actionNameCrc");
	int s_actionNameCrcColumn;

	std::unordered_map<uint32, int> s_commandsWithMinInvisLevelRequired;
}

using namespace CombatDataTableNamespace;

// ======================================================================

void CombatDataTable::install()
{
	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return;
	}
	s_actionNameCrcColumn = actionData->findColumnNumber(cs_columnActionNameCrc);

	// cache off commands with a minInvisLevelRequired specified for later use/reuse
	unsigned int const numRows = static_cast<unsigned int>(actionData->getNumRows());
	int const minInvisLevelRequiredColumn = actionData->findColumnNumber("minInvisLevelRequired");
	if ((numRows > 0) && (s_actionNameCrcColumn != -1) && (minInvisLevelRequiredColumn != -1))
	{
		uint32 actionNameCrc;
		int minInvisLevelRequired;
		for (unsigned int i = 0; i < numRows; ++i)
		{
			actionNameCrc = actionData->getIntValue(s_actionNameCrcColumn, i);
			minInvisLevelRequired = actionData->getIntValue(minInvisLevelRequiredColumn, i);

			if (minInvisLevelRequired != -1)
				s_commandsWithMinInvisLevelRequired.insert(std::make_pair(actionNameCrc, minInvisLevelRequired));
		}
	}
}

// ----------------------------------------------------------------------

bool CombatDataTable::isActionClientAnim(uint32 commandHash)
{
	if(commandHash == 0)
		return false;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return false;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s, %d].", commandHash, cs_combatDataTableName, s_actionNameCrcColumn));
		return false;
	}

	//get/return action animation field "doClientAnim" from action data
	return (0 != actionData->getIntValue("doClientAnim", row));
}

// ----------------------------------------------------------------------

std::string CombatDataTable::getActionAnimationData(uint32 commandHash, char const * const weaponType)
{
	if(commandHash == 0)
		return "";

	std::string actionAnimationData;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return "";
	}


	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return "";
	}

	//get action animation name from action data (need to parse)
	std::string fieldName = "anim_";

	std::string lowerWeaponType(weaponType);
	int (*pf)(int)=::tolower; 
	std::transform(lowerWeaponType.begin(), lowerWeaponType.end(), lowerWeaponType.begin(), pf); 

	fieldName += lowerWeaponType;

	if (actionData->doesColumnExist(fieldName))
	{
		actionAnimationData = actionData->getStringValue(fieldName, row);
	}
	if (0 >= actionAnimationData.length())
	{
		actionAnimationData = actionData->getStringValue("animDefault", row);
	}

	return actionAnimationData;
}

// ----------------------------------------------------------------------

bool CombatDataTable::actionForcesCharacterIntoCombat(uint32 commandHash)
{
	if(commandHash == 0)
		return false;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (actionData == 0)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return false;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return false;
	}

	//get/return action animation field "forcesCharacterIntoCombat" from action data
	return (actionData->getIntValue("forcesCharacterIntoCombat", row) != 0);
}

// ----------------------------------------------------------------------

int32 CombatDataTable::getAttackType(uint32 commandHash)
{
	if(commandHash == 0)
		return AT_none;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (actionData == 0)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return AT_none;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return AT_none;
	}

	return (actionData->getIntValue("attackType", row));
}

// ----------------------------------------------------------------------

int32 CombatDataTable::getHitType(uint32 commandHash)
{
	if(commandHash == 0)
		return HT_attack;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (actionData == 0)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return HT_attack;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return HT_attack;
	}

	return (actionData->getIntValue("hitType", row));
}

// ----------------------------------------------------------------------

int32 CombatDataTable::getValidTarget(uint32 commandHash)
{
	if(commandHash == 0)
		return VTT_none;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return VTT_none;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return VTT_none;
	}

	return (actionData->getIntValue("validTarget", row));
}

// ----------------------------------------------------------------------

float CombatDataTable::getMinRange(uint32 commandHash)
{
	if(commandHash == 0)
		return 0.0f;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return 0.0f;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return 0.0f;
	}

	return (actionData->getFloatValue("minRange", row));
}

// ----------------------------------------------------------------------

float CombatDataTable::getMaxRange(uint32 commandHash)
{
	if(commandHash == 0)
		return 0.0f;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return 0.0f;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return 0.0f;
	}

	return (actionData->getFloatValue("maxRange", row));
}

// ----------------------------------------------------------------------

float CombatDataTable::getActionCost(uint32 commandHash)
{
	if(commandHash == 0)
		return 0.0f;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return 0.0f;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return 0.0f;
	}

	return (actionData->getFloatValue("actionCost", row));
}

// ----------------------------------------------------------------------

float CombatDataTable::getMindCost(uint32 commandHash)
{
	if(commandHash == 0)
		return 0.0f;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return 0.0f;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return 0.0f;
	}

	return (actionData->getFloatValue("mindCost", row));
}

// ----------------------------------------------------------------------

bool CombatDataTable::getCancelsAutoAttack(uint32 commandHash)
{
	if(commandHash == 0)
		return false;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return false;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return false;
	}

	return (actionData->getIntValue("cancelsAutoAttack", row) == 1);
}

// ----------------------------------------------------------------------

float CombatDataTable::getPercentAddFromWeapon(uint32 commandHash)
{
	if(commandHash == 0)
		return 0.0f;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return 0.0f;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return 0.0f;
	}

	return actionData->getFloatValue("percentAddFromWeapon", row);
}

// ----------------------------------------------------------------------

std::string CombatDataTable::getSpecialLine(uint32 commandHash)
{
	if(commandHash == 0)
		return "";

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return "";
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return "";
	}

	return actionData->getStringValue("specialLine", row);
}

// ----------------------------------------------------------------------

int CombatDataTable::getMinInvisLevelRequired(uint32 commandHash)
{
	if(commandHash == 0)
		return -1;

	std::unordered_map<uint32, int>::const_iterator it = s_commandsWithMinInvisLevelRequired.find(commandHash);
	if (it != s_commandsWithMinInvisLevelRequired.end())
		return it->second;

	return -1;
}

// ----------------------------------------------------------------------

int CombatDataTable::getTrailBits(uint32 commandHash)
{
	if(commandHash == 0)
		return 0;

	DataTable const * const actionData = DataTableManager::getTable(cs_combatDataTableName, true);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("Could not find table [%s].", cs_combatDataTableName));
		return 0;
	}

	int const row = actionData->searchColumnInt(s_actionNameCrcColumn, commandHash);
	if (row < 0)
	{
		DEBUG_WARNING(true, ("Could not find string hash [%u] in table [%s].", commandHash, cs_combatDataTableName));
		return 0;
	}

	bool rightHand = (actionData->getIntValue("intRightHand", row) == 1);
	bool leftHand = (actionData->getIntValue("intLeftHand", row) == 1);
	bool rightFoot = (actionData->getIntValue("intRightFoot", row) == 1);
	bool leftFoot = (actionData->getIntValue("intLeftFoot", row) == 1);
	bool weapon = (actionData->getIntValue("intWeapon", row) == 1);

	int bits = 0;

	if(leftFoot)
		bits = bits | 1 << 0;
	if(rightFoot)
		bits = bits | 1 << 1;
	if(leftHand)
		bits = bits | 1 << 2;
	if(rightHand)
		bits = bits | 1 << 3;
	if(weapon)
		bits = bits | 1 << 4;

	return bits;
}
// ======================================================================
