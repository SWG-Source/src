// ======================================================================
//
// SetupScript.cpp
//
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/SetupScript.h"

#include "serverScript/ConfigServerScript.h"

// ----------------------------------------------------------------------
/**
 * Install the engine.
 * 
 * The settings in the Data structure will determine which subsystems
 * get initialized.
 */

void SetupScript::install()
{
	// setup the engine configuration
	ConfigServerScript::install();
}

// ----------------------------------------------------------------------
/**
 * Uninstall the engine.
 * 
 * This routine will properly uninstall the engine componenets that were
 * installed by SetupScript::install().
 */

void SetupScript::remove(void)
{
}

// ----------------------------------------------------------------------

void SetupScript::setupDefaultGameData(Data &data)
{
	Zero(data);

	data.version                                  = DATA_VERSION;
	data.useRemoteDebugJava = false;
}

// ----------------------------------------------------------------------

