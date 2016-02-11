// ======================================================================
//
// CurrentUserOptionManager.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/CurrentUserOptionManager.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/OptionManager.h"
#include <string>

// ======================================================================
// CurrentUserOptionManagerNamespace
// ======================================================================

namespace CurrentUserOptionManagerNamespace
{
	bool            ms_installed;
	OptionManager * ms_optionManager;
	std::string     ms_userName;

	bool s_verbose = false;
}

using namespace CurrentUserOptionManagerNamespace;

// ======================================================================
// STATIC PUBLIC CurrentUserOptionManager
// ======================================================================

void CurrentUserOptionManager::install ()
{
	DEBUG_FATAL (ms_installed, ("CurrentUserOptionManager::install: already installed"));
	ms_installed = true;

	ms_optionManager = new OptionManager;

	ExitChain::add (remove, "CurrentUserOptionManager::remove");

	DebugFlags::registerFlag(s_verbose, "sharedUtility/CurrentUserOptionManager", "verbose");
}

// ----------------------------------------------------------------------

void CurrentUserOptionManager::remove ()
{
	DEBUG_FATAL (!ms_installed, ("CurrentUserOptionManager::remove: not installed"));
	ms_installed = false;

	delete ms_optionManager;
	ms_optionManager = 0;
}

// ----------------------------------------------------------------------

void CurrentUserOptionManager::load (char const * const userName)
{
	if (userName && *userName)
	{
		ms_userName = userName;

		DEBUG_REPORT_LOG (s_verbose, ("CurrentUserOptionManager::load: user name set to %s, loading %s\n", userName, ms_userName.c_str ()));

		ms_optionManager->load (ms_userName.c_str ());
	}
	else
		DEBUG_WARNING (true, ("CurrentUserOptionManager::load: userName is nullptr"));
}

// ----------------------------------------------------------------------

void CurrentUserOptionManager::save ()
{
	DEBUG_FATAL (!ms_installed, ("CurrentUserOptionManager::save: not installed"));

	if (!ms_userName.empty ())	
		ms_optionManager->save (ms_userName.c_str ());
	else
		DEBUG_REPORT_LOG (s_verbose, ("CurrentUserOptionManager::save: userName is nullptr\n"));
}

// ----------------------------------------------------------------------

void CurrentUserOptionManager::registerOption (bool & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("CurrentUserOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

void CurrentUserOptionManager::registerOption (float & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("CurrentUserOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

void CurrentUserOptionManager::registerOption (int & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("CurrentUserOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

void CurrentUserOptionManager::registerOption (std::string & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("CurrentUserOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

void CurrentUserOptionManager::registerOption (Unicode::String & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("CurrentUserOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ======================================================================
