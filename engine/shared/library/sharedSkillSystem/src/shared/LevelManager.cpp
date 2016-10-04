//======================================================================
//
// LevelManager.cpp
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#include "sharedSkillSystem/FirstSharedSkillSystem.h"
#include "sharedSkillSystem/LevelManager.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"

#include "sharedSkillSystem/SkillObject.h"

#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>

//======================================================================

namespace LevelManagerNamespace
{
	bool m_installed = false;

	// level data
	struct LevelRecord
	{
		int16 level;
		int xp_required;
		int health;
	};

	std::map<int, LevelRecord> ms_levelRecords;
	typedef std::map<int, LevelRecord>::iterator LevelRecordsIterator;

	
	// xp multipliers
	// map xp_type_nameCrc to multiplier
	std::map<uint32, int> ms_xpTypeRecords;
	typedef std::map<uint32, int>::iterator XpRecordsIterator;


	// skill data
	struct SkillRecord
	{
		uint32 xp_type_name_Crc;
		int    xp_cost;
	};

	std::map<uint32, SkillRecord> ms_skillRecords;
	typedef std::map<uint32, SkillRecord>::iterator SkillRecordsIterator;


	int ms_numLevels = 0;
	int ms_maxLevel = 0;


	LevelRecord const * getLevelRecord(int xp);


	inline int getXpValueForType(uint32 expTypeCrc, int xpBaseValue)
	{
		int value = 0;

		// look up the xp type and only add the xp if the type is found
		XpRecordsIterator xpItr = ms_xpTypeRecords.find(expTypeCrc);

		if (xpItr != ms_xpTypeRecords.end())
		{
			const int multiplier = (*xpItr).second;
			value = xpBaseValue * multiplier;
		}

		return value;
	}

};

using namespace LevelManagerNamespace;

//======================================================================

void LevelManager::install()
{
	if(m_installed)
		return;
	m_installed = true;

	DataTable *playerLevelDatatable = DataTableManager::getTable("datatables/player/player_level.iff", true);

	if (playerLevelDatatable)
	{
		ms_numLevels = playerLevelDatatable->getNumRows();

		const int levelColumn          = playerLevelDatatable->findColumnNumber("level");
		const int xp_requiredColumn    = playerLevelDatatable->findColumnNumber("xp_required");
		const int health_grantedColumn = playerLevelDatatable->findColumnNumber("health_granted");
		const int xp_typeColumn        = playerLevelDatatable->findColumnNumber("xp_type");
		const int xp_multiplierColumn  = playerLevelDatatable->findColumnNumber("xp_multiplier");


		// @NOTE: there are basically two independent tables in this data file
		//   there is level information and xp type information
		//   these are independent and have diffent lengths, so I'm putting the info
		//   into two different data structures
		int i;
		for(i = 0; i < ms_numLevels; ++i)
		{
			LevelRecord levelRecord;

			levelRecord.level       = (int16) playerLevelDatatable->getIntValue(levelColumn, i);
			levelRecord.xp_required = playerLevelDatatable->getIntValue(xp_requiredColumn, i);
			levelRecord.health      = playerLevelDatatable->getIntValue(health_grantedColumn, i);

			ms_levelRecords.insert(std::make_pair(levelRecord.level, levelRecord));

			std::string xpTypeName  = playerLevelDatatable->getStringValue(xp_typeColumn, i);
			if (!xpTypeName.empty())
			{
				const uint32 xpTypeNameCrc = Crc::calculate(xpTypeName.c_str());
				const int xpMultiplier = playerLevelDatatable->getIntValue(xp_multiplierColumn, i);
				ms_xpTypeRecords.insert(std::make_pair(xpTypeNameCrc, xpMultiplier));
			}
			if(levelRecord.level > ms_maxLevel)
				ms_maxLevel = levelRecord.level;
		}
		DataTableManager::close("datatables/player/player_level.iff");
	}

	DataTable *skillDatatable = DataTableManager::getTable("datatables/skill/skills.iff", true);

	if (skillDatatable)
	{
		const int numSkillRows = skillDatatable->getNumRows();
		
		const int skill_nameColumn = skillDatatable->findColumnNumber("NAME");
		const int type_nameColumn  = skillDatatable->findColumnNumber("XP_TYPE");
		const int xp_costColumn    = skillDatatable->findColumnNumber("XP_COST");

		int i;
		for(i = 0; i < numSkillRows; i++)
		{
			SkillRecord skillRecord;
			
			std::string skillName = skillDatatable->getStringValue(skill_nameColumn, i);
			uint32 skillNameCrc = Crc::calculate(skillName.c_str());

			std::string xpTypeName = skillDatatable->getStringValue(type_nameColumn, i);
			skillRecord.xp_type_name_Crc = Crc::calculate(xpTypeName.c_str());

			skillRecord.xp_cost = skillDatatable->getIntValue(xp_costColumn, i);

			ms_skillRecords.insert(std::make_pair(skillNameCrc, skillRecord));
		}
		DataTableManager::close("datatables/skill/skills.iff");
	}

	ExitChain::add(LevelManager::remove, "LevelManager::remove", 0, false);
}

//======================================================================

void LevelManager::remove()
{
	
}

void LevelManager::setLevelDataFromXp(LevelData &levelData, int xp)
{
	LevelRecord const * levelRecord = getLevelRecord(xp);

	if (levelRecord)
	{
		levelData.currentLevel   = levelRecord->level;
		levelData.currentLevelXp = levelRecord->xp_required;
		levelData.currentHealth  = levelRecord->health;
	}
	else
	{
		// assume level 0
		levelData.currentLevel = 0;
		levelData.currentLevelXp = 0;
		levelData.currentHealth = 0;
	}
}

//======================================================================
// This method forces the specified level and sets the creatures
//   levelxp and health based on the level (not the skills)
//
void LevelManager::setLevelDataFromLevel(LevelData &levelData, int newLevel)
{
	LevelRecordsIterator it = ms_levelRecords.find(newLevel);
	if(it == ms_levelRecords.end())
	{
		bool issueWarning = true;

		if (!ms_levelRecords.empty())
		{
			// find the max level and set levelData to the max if possible
			// assuming that we're asking for a level that is higher than
			// possible
			std::map<int, LevelRecord>::reverse_iterator last = ms_levelRecords.rbegin();

			int const highestLevel = last->first;

			if (newLevel > highestLevel)
			{
				LevelRecord const &levelRecord = last->second;
				levelData.currentLevel   = levelRecord.level;
				levelData.currentLevelXp = levelRecord.xp_required;
				levelData.currentHealth  = levelRecord.health;
				issueWarning = false;
			}
		}

		WARNING(issueWarning, ("setLevelDataFromLevel: Invalid level specified[%d]\n", newLevel));
	}
	else
	{
		LevelRecord const &levelRecord = it->second;
		levelData.currentLevel   = levelRecord.level;
		levelData.currentLevelXp = levelRecord.xp_required;
		levelData.currentHealth  = levelRecord.health;
	}
}

//======================================================================
// This method calculates the level data based upon the level XP 
//
void LevelManager::calculateLevelData(int currentLevelXp, LevelData &levelData)
{
	levelData.currentLevelXp = currentLevelXp;

	// get the health for this record
	LevelRecord const * levelRecord = getLevelRecord(levelData.currentLevelXp);
	if (levelRecord)
	{
		levelData.currentHealth = levelRecord->health;
		levelData.currentLevel = levelRecord->level;
	}
	else
	{
		WARNING(true, ("calculateLevelData: Couldn't find level for xp value [%d]\n", levelData.currentLevelXp));
	}
}

//======================================================================
// Updates the xp and, if necessary, the level and health based on the added skill
//
void LevelManager::addSkillToLevelData(LevelData &levelData, std::string const &skillName)
{
	updateLevelDataWithSkill(levelData, skillName, false);
}

//======================================================================
// Updates the xp and, if necessary, the level and health based on the added skill
//
void LevelManager::removeSkillFromLevelData(LevelData &levelData, std::string const &skillName)
{
	updateLevelDataWithSkill(levelData, skillName, true);
}


//======================================================================
// Updates the xp and, if necessary, the level and health based on the added skill
//
void LevelManager::addXpToLevelData(LevelData &levelData, std::string const &experienceType, int xp)
{
	CrcLowerString crcExpType(experienceType.c_str());

	levelData.currentLevelXp += getXpValueForType(crcExpType.getCrc(), xp);
	
	LevelRecord const * levelRecord = getLevelRecord(levelData.currentLevelXp);

	if (levelRecord)
	{
		levelData.currentLevel   = levelRecord->level;
		levelData.currentHealth  = levelRecord->health;
	}
	else
	{
		WARNING(true, ("updateLevelDataWithSkill: Couldn't find level for xp value [%d]\n", levelData.currentLevelXp));
	}
}

//======================================================================
// Returns the total xp for the specified skill
//
int LevelManager::getSkillXpValue(CrcLowerString const &skillName)
{
	int value = 0;

	SkillRecordsIterator itr = ms_skillRecords.find(skillName.getCrc());

	if (itr != ms_skillRecords.end())
	{
		int xpBaseValue = (*itr).second.xp_cost;
		uint32 xpTypeNameCrc = (*itr).second.xp_type_name_Crc;

		value = getXpValueForType(xpTypeNameCrc, xpBaseValue);
	}

	return value;
}

//======================================================================
// Updates the xp and therefore level and health based on the specified
//   skill (and whether it was revoked)
//
void LevelManager::updateLevelDataWithSkill(LevelData &levelData, std::string const &skillName, bool revoked)
{
	CrcLowerString crcSkillName(skillName.c_str());
	const int skillValue = getSkillXpValue(crcSkillName);

	levelData.currentLevelXp += revoked ? -skillValue : skillValue;
	
	LevelRecord const * levelRecord = getLevelRecord(levelData.currentLevelXp);

	if (levelRecord)
	{
		levelData.currentLevel   = levelRecord->level;
		levelData.currentHealth  = levelRecord->health;
	}
	else
	{
		WARNING(true, ("updateLevelDataWithSkill: Couldn't find level for xp value [%d]\n", levelData.currentLevelXp));
	}
}

//======================================================================
// Find the level corresponding to the xp value and
//   returns nullptr if not found
LevelRecord const * LevelManagerNamespace::getLevelRecord(int xp)
{
	LevelRecordsIterator itr = ms_levelRecords.begin();
	LevelRecordsIterator nextItr = itr;
	while (itr != ms_levelRecords.end())
	{
		LevelRecord const & currentRecord = (*itr).second;
		++nextItr;
		if (nextItr != ms_levelRecords.end())
		{
			const LevelRecord & nextRecord = (*nextItr).second;
			{
				if (xp < nextRecord.xp_required)
				{
					return &currentRecord;
				}
			}
		}
		else
		{
			// no more levels, so return this (the last) one
			return &currentRecord;
		}

		++itr;
	}

	WARNING(true, ("getLevelRecord: Couldn't find level for xp value[%d]\n", xp));

	return nullptr;
}

//======================================================================

LevelManager::LevelData::LevelData()
: currentLevel(0)
, currentLevelXp(0)
, currentHealth(0)
{
}

//======================================================================

LevelManager::LevelData::LevelData(int16 level, int levelXp)
: currentLevel(level)
, currentLevelXp(levelXp)
, currentHealth(0)
{
}

//----------------------------------------------------------------------

int LevelManager::getRequiredXpToReachLevel(int16 level)
{
	LevelData levelData;

	setLevelDataFromLevel(levelData, level);

	LevelRecord const * const levelRecord = getLevelRecord(levelData.currentLevelXp);

	int const xp = (levelRecord != 0) ? levelRecord->xp_required : 0;
	return xp;
}

//----------------------------------------------------------------------

bool LevelManager::canSkillIncreaseLevel(std::string const &skillName)
{
	CrcLowerString crcExpType(skillName.c_str());

	int const xp = getXpValueForType(crcExpType.getCrc(), 1);

	return (xp >= 1);
}

//----------------------------------------------------------------------

int LevelManager::getMaxLevel()
{
	return ms_maxLevel;
}

//======================================================================
