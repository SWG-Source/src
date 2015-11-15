//======================================================================
//
// LevelManager.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_LevelManager_H
#define INCLUDED_LevelManager_H

//======================================================================

class CrcLowerString;

//----------------------------------------------------------------------

class LevelManager
{
public:

	struct LevelData
	{
		LevelData();
		LevelData(int16 level, int levelXp);
		int16 currentLevel;
		int currentLevelXp;
		int currentHealth;
	};

	static void install();
	static void remove();

	static void setLevelDataFromXp(LevelData &levelData, int xp);
	static void setLevelDataFromLevel(LevelData &levelData, int newLevel);

	static void addXpToLevelData(LevelData &levelData, std::string const &experienceType, int xp);

	static void calculateLevelData(int currentLevelXp, LevelData &levelData);

	static void addSkillToLevelData(LevelData &levelData, std::string const &skillName);
	static void removeSkillFromLevelData(LevelData &levelData, std::string const &skillName);

	static bool canSkillIncreaseLevel(std::string const & skillName);
	static int getRequiredXpToReachLevel(int16 level);

	static int getMaxLevel();

	static int  getSkillXpValue(CrcLowerString const &skillName);
	
protected:

	static void updateLevelDataWithSkill(LevelData &levelData, std::string const &skillName, bool revoked);
};


//======================================================================

#endif
