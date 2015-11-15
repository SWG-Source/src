//===================================================================
//
// ConfigSharedGame.cpp
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ConfigSharedGame.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Production.h"

//===================================================================

namespace ConfigSharedGameNamespace
{
	bool         ms_sample;
	const char * ms_defaultLocale;
	const char * ms_fontLocale;
	bool         ms_debugStringIds;
	const char * ms_schematicGroupDataTable;
	bool         ms_displayBadStringIds;
	int          ms_maxCreatureSkillModBonus;
	bool         ms_imageDesignerVerboseOutput;
	int          ms_imageDesignerCosmeticSessionTimeSeconds;
	int          ms_imageDesignerPhysicalSessionTimeSeconds;
	int          ms_imageDesignerStatMigrationSessionTimeSeconds;
	bool         ms_fatalOnBadFormData;
	bool         ms_spamAsteroidGenerationData;
	float        ms_movementThresholdForActionValidation;
}

using namespace ConfigSharedGameNamespace;

//===================================================================

bool ConfigSharedGame::getSample ()
{
	return ms_sample;
}

//-----------------------------------------------------------------

const char * ConfigSharedGame::getDefaultLocale()
{
	return ms_defaultLocale;
}

//-----------------------------------------------------------------

bool ConfigSharedGame::defaultLocaleIgnoresLAlt()
{
	if(strncmp("ja", ms_defaultLocale, 2) == 0)
		return true;
	return false;
}

//-----------------------------------------------------------------

const char * ConfigSharedGame::getFontLocale()
{
	return ms_fontLocale;
}

//----------------------------------------------------------------------

bool   ConfigSharedGame::getDebugStringIds ()
{
	return ms_debugStringIds;
}

//----------------------------------------------------------------------

const char * ConfigSharedGame::getSchematicGroupDataTable ()
{
	return ms_schematicGroupDataTable;
}

//----------------------------------------------------------------------

bool ConfigSharedGame::getDisplayBadStringIds     ()
{
	return ms_displayBadStringIds;
}

//----------------------------------------------------------------------

bool ConfigSharedGame::getFatalOnBadFormData ()
{
	return ms_fatalOnBadFormData;
}

//----------------------------------------------------------------------

int ConfigSharedGame::getMaxCreatureSkillModBonus()
{
	return ms_maxCreatureSkillModBonus;
}

//----------------------------------------------------------------------

bool ConfigSharedGame::getImageDesignerVerboseOutput()
{
	return ms_imageDesignerVerboseOutput;
}

//----------------------------------------------------------------------

int ConfigSharedGame::getImageDesignerCosmeticSessionTimeSeconds()
{
	return ms_imageDesignerCosmeticSessionTimeSeconds;
}

//----------------------------------------------------------------------

int ConfigSharedGame::getImageDesignerPhysicalSessionTimeSeconds()
{
	return ms_imageDesignerPhysicalSessionTimeSeconds;
}

//----------------------------------------------------------------------

int ConfigSharedGame::getImageDesignerStatMigrationSessionTimeSeconds()
{
	return ms_imageDesignerStatMigrationSessionTimeSeconds;
}

//----------------------------------------------------------------------

bool ConfigSharedGame::getSpamAsteroidGenerationData()
{
	return ms_spamAsteroidGenerationData;
}

//----------------------------------------------------------------------

float ConfigSharedGame::getMovementThresholdForActionValidation()
{
	return ms_movementThresholdForActionValidation;
}

//===================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedGame", #a, b))
#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("SharedGame", #a, b))
#define KEY_FLOAT(a,b)   (ms_ ## a = ConfigFile::getKeyFloat("SharedGame", #a, b))
#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("SharedGame", #a, b))

//===================================================================

void ConfigSharedGame::install()
{
	InstallTimer const installTimer("ConfigSharedGame::install");

	KEY_BOOL   (sample,                  false);
	KEY_STRING (defaultLocale,           "en");
	KEY_STRING (fontLocale,              "en");
	KEY_BOOL   (debugStringIds,          true);
	KEY_STRING (schematicGroupDataTable, "datatables/crafting/schematic_group.iff");
	KEY_BOOL   (fatalOnBadFormData,      false);
	KEY_BOOL   (spamAsteroidGenerationData, false);

	bool defaultDisplayBadStringIds = true;

	KEY_BOOL   (displayBadStringIds,     defaultDisplayBadStringIds);
	KEY_INT    (maxCreatureSkillModBonus, 25);
	KEY_BOOL   (imageDesignerVerboseOutput, false);
	KEY_INT    (imageDesignerCosmeticSessionTimeSeconds, 0);
	KEY_INT    (imageDesignerPhysicalSessionTimeSeconds, 0);
	KEY_INT    (imageDesignerStatMigrationSessionTimeSeconds, 240);
	KEY_FLOAT  (movementThresholdForActionValidation, 0.5f);
}

//===================================================================
