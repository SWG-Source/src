// ======================================================================
//
// AiCreatureData.cpp
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiCreatureData.h"

#include "serverGame/AiCreatureCombatProfile.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ServerWorld.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedLog/Log.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"

#include <map>

// ======================================================================
//
// AiCreatureDataNamespace
//
// ======================================================================

namespace AiCreatureDataNamespace
{
	bool s_installed = false;
	char const * const s_creaureDataTable = "datatables/mob/creatures.iff";
	char const * const s_weaponDataTable = "datatables/ai/ai_weapons.iff";

	typedef std::map<PersistentCrcString, AiCreatureData *> CreatureDataMap;
	typedef std::vector<PersistentCrcString> WeaponList;
	typedef std::map<PersistentCrcString, WeaponList> WeaponDataMap;

	CreatureDataMap s_creatureDataMap;
	WeaponDataMap s_weaponDataMap;
	AiCreatureData const * s_defaultCreatureData = nullptr;
	int s_creatureErrorCount = 0;
	int s_weaponErrorCount = 0;

	void remove();
	void loadCreatureData(DataTable const & dataTable);
	void loadWeaponData(DataTable const & dataTable);
	void loadWeaponColumn(WeaponList & weaponList, std::string const & weaponTemplate);
	void clearCreatureData();
	void killAi();

	void verifyMovementSpeed(CrcString const & creatureName, float & movementSpeed);
	void verifyAggressive(CrcString const & creatureName, float & aggressive);
	void verifyPrimaryWeapon(CrcString const & creatureName, CrcString & primaryWeapon);
	void verifySecondaryWeapon(CrcString const & creatureName, CrcString & secondaryWeapon);
	void verifyPrimarySpecials(CrcString const & creatureName, CrcString & primarySpecials);
	void verifySecondarySpecials(CrcString const & creatureName, CrcString & secondarySpecials);
}

using namespace AiCreatureDataNamespace;

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::remove()
{
	clearCreatureData();

	s_weaponDataMap.clear();

	delete s_defaultCreatureData;
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::clearCreatureData()
{
	CreatureDataMap::iterator iterCreatureDataMap = s_creatureDataMap.begin();

	for (; iterCreatureDataMap != s_creatureDataMap.end(); ++iterCreatureDataMap)
	{
		delete iterCreatureDataMap->second;
	}

	s_creatureDataMap.clear();
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::verifyMovementSpeed(CrcString const & creatureName, float & movementSpeed)
{
	float const minMovementSpeed = 0.0f;
	float const maxMovementSpeed = 3.0f;

	if (   (movementSpeed < minMovementSpeed)
	    || (movementSpeed > maxMovementSpeed))
	{
		WARNING(true, ("AiCreatureData::verifyMovementSpeed() Creature(%s) has invalid movementSpeed(%.f) validRange[%.2f...%.2f]", creatureName.getString(), movementSpeed, minMovementSpeed, maxMovementSpeed));
		++s_creatureErrorCount;
		movementSpeed = clamp(minMovementSpeed, movementSpeed, maxMovementSpeed);
	}
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::verifyAggressive(CrcString const & creatureName, float & aggressive)
{
	float const minAggressive = 0.0f;
	float const maxAggressive = ConfigServerGame::getAiMaxAggroRadius();

	if (   (aggressive < minAggressive)
	    || (aggressive > maxAggressive))
	{
		WARNING(true, ("AiCreatureData::verifyAggressive() Creature(%s) has invalid aggressive(%.f) validRange[%.2f...%.2f]", creatureName.getString(), aggressive, minAggressive, maxAggressive));
		++s_creatureErrorCount;
		aggressive = clamp(minAggressive, aggressive, maxAggressive);
	}
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::verifyPrimaryWeapon(CrcString const & creatureName, CrcString & primaryWeapon)
{
	if (   (primaryWeapon != TemporaryCrcString("unarmed", false))
	    && (primaryWeapon != TemporaryCrcString("none", false)))
	{
		if (   (s_weaponDataMap.find(PersistentCrcString(primaryWeapon)) == s_weaponDataMap.end())
		    && (ObjectTemplateList::lookUp(primaryWeapon.getString()).getCrc() == 0))
		{
			++s_creatureErrorCount;
			WARNING(true, ("AiCreatureData::verifyPrimaryWeapon() Creature(%s) has an invalid primary_weapon(%s)", creatureName.getString(), primaryWeapon.getString()));
		}
	}
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::verifySecondaryWeapon(CrcString const & creatureName, CrcString & secondaryWeapon)
{
	if (   (secondaryWeapon != TemporaryCrcString("unarmed", false))
	    && (secondaryWeapon != TemporaryCrcString("none", false)))
	{
		if (   (s_weaponDataMap.find(PersistentCrcString(secondaryWeapon)) == s_weaponDataMap.end())
		    && (ObjectTemplateList::lookUp(secondaryWeapon.getString()).getCrc() == 0))
		{
			++s_creatureErrorCount;
			WARNING(true, ("AiCreatureData::verifySecondaryWeapon() Creature(%s) has an invalid secondary_weapon(%s)", creatureName.getString(), secondaryWeapon.getString()));
		}
	}
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::verifyPrimarySpecials(CrcString const & creatureName, CrcString & primarySpecials)
{
	if (   !primarySpecials.isEmpty()
	    && (AiCreatureCombatProfile::getCombatProfile(primarySpecials) == nullptr))
	{
		++s_creatureErrorCount;
		WARNING(true, ("AiCreatureData::verifyPrimarySpecials() Creature(%s) has invalid primary_weapon_specials(%s)", creatureName.getString(), primarySpecials.getString()));
	}
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::verifySecondarySpecials(CrcString const & creatureName, CrcString & secondarySpecials)
{
	if (   !secondarySpecials.isEmpty()
	    && (AiCreatureCombatProfile::getCombatProfile(secondarySpecials) == nullptr))
	{
		++s_creatureErrorCount;
		WARNING(true, ("AiCreatureData::verifySecondarySpecials() Creature(%s) has invalid secondary_weapon_specials(%s)", creatureName.getString(), secondarySpecials.getString()));
	}
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::loadCreatureData(DataTable const & dataTable)
{
	// if we are loading the data for the first time, check for duplicates
	bool const checkForDuplicate = s_creatureDataMap.empty();

	int aggressiveCount = 0;
	int assistCount = 0;

	int const rowCount = dataTable.getNumRows();

	for (int row = 0; row < rowCount; ++row)
	{
		PersistentCrcString const name(dataTable.getStringValue("creatureName", row), false);

		CreatureDataMap::iterator iterCreatureData = s_creatureDataMap.find(name);

		if (iterCreatureData == s_creatureDataMap.end())
		{
			AiCreatureData * const creatureData = new AiCreatureData;

			std::pair<CreatureDataMap::iterator, bool> result = s_creatureDataMap.insert(std::make_pair(name, creatureData));

			iterCreatureData = result.first;
		}
		else if (checkForDuplicate)
		{
			LOG("debug_ai", ("AiCreatureData::loadCreatureData() duplicate data at row=%d, crc=%lu, name=\"%s\"", row, name.getCrc(), name.getString()));
		}

		if (iterCreatureData != s_creatureDataMap.end())
		{
			AiCreatureData & creatureData = *NON_NULL(iterCreatureData->second);

			creatureData.m_name = &(iterCreatureData->first);
			creatureData.m_movementSpeedPercent = dataTable.getFloatValue("movement_speed", row);
			creatureData.m_primaryWeapon.set(Unicode::getTrim(dataTable.getStringValue("primary_weapon", row)).c_str(), false);
			creatureData.m_secondaryWeapon.set(Unicode::getTrim(dataTable.getStringValue("secondary_weapon", row)).c_str(), false);
			creatureData.m_aggressive = dataTable.getFloatValue("aggressive", row);
			creatureData.m_stalker = (dataTable.getIntValue("stalker", row) > 0);
			creatureData.m_assist = dataTable.getFloatValue("assist", row);
			creatureData.m_deathBlow = static_cast<AiCreatureData::DeathBlow>(dataTable.getIntValue("death_blow", row));
			creatureData.m_primarySpecials.set(Unicode::getTrim(dataTable.getStringValue("primary_weapon_specials", row)).c_str(), false);
			creatureData.m_secondarySpecials.set(Unicode::getTrim(dataTable.getStringValue("secondary_weapon_specials", row)).c_str(), false);
			creatureData.m_difficulty = static_cast<CreatureObject::Difficulty>(dataTable.getIntValue("difficultyClass", row));

			verifyMovementSpeed(*creatureData.m_name, creatureData.m_movementSpeedPercent);
			verifyAggressive(*creatureData.m_name, creatureData.m_aggressive);
			verifyPrimaryWeapon(*creatureData.m_name, creatureData.m_primaryWeapon);
			verifyPrimarySpecials(*creatureData.m_name, creatureData.m_primarySpecials);
			verifySecondaryWeapon(*creatureData.m_name, creatureData.m_secondaryWeapon);
			verifySecondarySpecials(*creatureData.m_name, creatureData.m_secondarySpecials);

			// Keep track of some statistics

			if (creatureData.m_aggressive > 0.0f)
			{
				++aggressiveCount;
			}

			if (creatureData.m_assist > 0.0f)
			{
				++assistCount;
			}
		}
		else
		{
			// @TODO: Handle this error case
		}
	}

	float const aggressivePercent = !s_creatureDataMap.empty() ? static_cast<float>(aggressiveCount) / static_cast<float>(s_creatureDataMap.size()) * 100.0f : 0.0f;
	float const assistPercent = !s_creatureDataMap.empty() ? static_cast<float>(assistCount) / static_cast<float>(s_creatureDataMap.size()) * 100.0f : 0.0f;

	LOG("debug_ai", ("AiCreatureData::loadCreatureData() Loading...%s - creatures(%u) errors(%i) aggressive(%.1f%%) assist(%.1f%%)", dataTable.getName().c_str(), s_creatureDataMap.size(), s_creatureErrorCount, aggressivePercent, assistPercent));
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::loadWeaponData(DataTable const & dataTable)
{
	// if we are loading the data for the first time, check for duplicates
	bool const checkForDuplicate = s_weaponDataMap.empty();

	int const rowCount = dataTable.getNumRows();

	for (int row = 0; row < rowCount; ++row)
	{
		PersistentCrcString const name(dataTable.getStringValue("weapon_id", row), false);

		WeaponDataMap::iterator iterWeaponData = s_weaponDataMap.find(name);

		if (iterWeaponData == s_weaponDataMap.end())
		{
			std::pair<WeaponDataMap::iterator, bool> result = s_weaponDataMap.insert(std::make_pair(name, WeaponList()));

			iterWeaponData = result.first;
		}
		else if (checkForDuplicate)
		{
			LOG("debug_ai", ("AiCreatureData::loadWeaponData() duplicate data at row=%d, crc=%lu, name=\"%s\"", row, name.getCrc(), name.getString()));
		}

		if (iterWeaponData != s_weaponDataMap.end())
		{
			WeaponList & weaponList = iterWeaponData->second;

			weaponList.clear();

			// @TODO: Put all the possible weapon template names in a master list or change to ConstCharCrcString

			loadWeaponColumn(weaponList, dataTable.getStringValue("weapon1", row));
			loadWeaponColumn(weaponList, dataTable.getStringValue("weapon2", row));
			loadWeaponColumn(weaponList, dataTable.getStringValue("weapon3", row));
			loadWeaponColumn(weaponList, dataTable.getStringValue("weapon4", row));
		}
		else
		{
			// @TODO: Handle this error case
		}
	}

	LOG("debug_ai", ("AiCreatureData::loadWeaponData() Loading...%s - weaponProfiles(%u) errors(%i)", dataTable.getName().c_str(), s_weaponDataMap.size(), s_weaponErrorCount));
}

// ----------------------------------------------------------------------
void AiCreatureDataNamespace::loadWeaponColumn(WeaponList & weaponList, std::string const & weaponTemplate)
{
	if (!weaponTemplate.empty())
	{
		if (   (weaponTemplate != "unarmed")
		    && (ObjectTemplateList::lookUp(weaponTemplate.c_str()).getCrc() == 0))
		{
			++s_weaponErrorCount;
			WARNING(true , ("AiCreatureDataNamespace::loadWeaponColumn() Unable to find weapon(%s)", weaponTemplate.c_str()));
		}

		weaponList.push_back(PersistentCrcString(weaponTemplate.c_str(), true));
	}
}

// ======================================================================
//
// AiCreatureData
//
// ======================================================================

// ----------------------------------------------------------------------
AiCreatureData::AiCreatureData()
 : m_name(nullptr)
 , m_movementSpeedPercent(1.0f)
 , m_primaryWeapon()
 , m_secondaryWeapon()
 , m_aggressive(0.0f)
 , m_stalker(false)
 , m_assist(0.0f)
 , m_deathBlow(DB_no)
 , m_primarySpecials()
 , m_secondarySpecials()
 , m_difficulty(CreatureObject::D_normal)
{
}

// ----------------------------------------------------------------------
void AiCreatureData::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));
	ExitChain::add(AiCreatureDataNamespace::remove, "AiCreatureDataNamespace::remove");

	s_defaultCreatureData = new AiCreatureData;

	// Load data from ai_weapons.tab
	{
		s_weaponErrorCount = 0;
		bool const openIfNotFound = true;
		DataTable * const dataTable = DataTableManager::getTable(s_weaponDataTable, openIfNotFound);

		if (dataTable != nullptr)
		{
			DataTableManager::addReloadCallback(s_weaponDataTable, &loadWeaponData);
			loadWeaponData(*dataTable);
			DataTableManager::close(s_weaponDataTable);
		}
		else
		{
			FATAL(true, ("AiCreatureData::install() Could not open file %s", s_weaponDataTable));
		}
	}

	// Load data from creatures.tab
	{
		s_creatureErrorCount = 0;
		bool const openIfNotFound = true;
		DataTable * const dataTable = DataTableManager::getTable(s_creaureDataTable, openIfNotFound);

		if (dataTable != nullptr)
		{
			DataTableManager::addReloadCallback(s_creaureDataTable, &loadCreatureData);
			loadCreatureData(*dataTable);
			DataTableManager::close(s_creaureDataTable);
		}
		else
		{
			FATAL(true, ("AiCreatureData::install() Could not open file %s", s_creaureDataTable));
		}
	}

	s_installed = true;
}

// ----------------------------------------------------------------------
AiCreatureData const & AiCreatureData::getCreatureData(CrcString const & creatureName)
{
	CreatureDataMap::const_iterator iterCreatureDataMap = s_creatureDataMap.find(PersistentCrcString(creatureName));

	if (iterCreatureDataMap != s_creatureDataMap.end())
	{
		return *iterCreatureDataMap->second;
	}
	else
	{
		WARNING(true, ("AiCreatureData::getCreatureData() Unable to find data for creatureName(%s)", creatureName.getString()));
	}

	return *s_defaultCreatureData;
}

// ----------------------------------------------------------------------
AiCreatureData const & AiCreatureData::getDefaultCreatureData()
{
	return *s_defaultCreatureData;
}

// ----------------------------------------------------------------------
std::string AiCreatureData::getWeaponTemplateName(CrcString const & weaponName)
{
	// @TODO: Make this return a PersistentCrcString

	std::string result;
	
	if (TreeFile::exists(weaponName.getString()))
	{
		result = weaponName.getString();
	}
	else
	{
		WeaponDataMap::const_iterator iterWeaponDataMap = s_weaponDataMap.find(PersistentCrcString(weaponName));

		if (   (iterWeaponDataMap != s_weaponDataMap.end())
			&& !iterWeaponDataMap->second.empty())
		{
			WeaponList const & weaponList = iterWeaponDataMap->second;
			unsigned int const index = rand() % weaponList.size();
			
			result = weaponList[index].getString();
		}
		else
		{
			result = "unarmed";

			WARNING(true, ("AiCreatureData::getWeaponTemplateName() Unable to find data for weaponName(%s), defaulting to ""unarmed""", weaponName.getString()));
		}
	}

	return result;
}

// ======================================================================
