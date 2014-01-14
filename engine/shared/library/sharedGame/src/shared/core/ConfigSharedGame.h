//===================================================================
//
// ConfigSharedGame.h
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ConfigSharedGame_H
#define INCLUDED_ConfigSharedGame_H

//===================================================================

class ConfigSharedGame
{
public:

	static void install ();

	static bool         getSample                  ();
	static const char * getDefaultLocale           ();
	static bool         defaultLocaleIgnoresLAlt   ();
	static const char * getFontLocale              ();
	static bool         getDebugStringIds          ();
	static const char * getSchematicGroupDataTable ();
	static bool         getDisplayBadStringIds     ();
	static int          getMaxCreatureSkillModBonus();
	static bool         getImageDesignerVerboseOutput();
	static int          getImageDesignerCosmeticSessionTimeSeconds();
	static int          getImageDesignerPhysicalSessionTimeSeconds();
	static int          getImageDesignerStatMigrationSessionTimeSeconds();
	static bool         getFatalOnBadFormData      ();
	static bool         getSpamAsteroidGenerationData ();
	static float        getMovementThresholdForActionValidation       ();

private:

	ConfigSharedGame ();
	ConfigSharedGame (const ConfigSharedGame&);
	ConfigSharedGame& operator= (const ConfigSharedGame&);
};

//===================================================================

#endif
