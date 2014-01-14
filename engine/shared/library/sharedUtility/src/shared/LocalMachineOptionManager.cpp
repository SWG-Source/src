// ======================================================================
//
// LocalMachineOptionManager.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/OptionManager.h"

// ======================================================================
// LocalMachineOptionManagerNamespace
// ======================================================================

namespace LocalMachineOptionManagerNamespace
{
	char const * const cms_fileName = "local_machine_options.iff";

	bool               ms_installed;
	OptionManager *    ms_optionManager;
}

using namespace LocalMachineOptionManagerNamespace;

// ======================================================================
// STATIC PUBLIC LocalMachineOptionManager
// ======================================================================

void LocalMachineOptionManager::install ()
{
	DEBUG_FATAL (ms_installed, ("LocalMachineOptionManager::install: already installed"));
	ms_installed = true;

	ms_optionManager = new OptionManager;
	ms_optionManager->load (cms_fileName);

	ExitChain::add (remove, "LocalMachineOptionManager::remove");
}

// ----------------------------------------------------------------------

void LocalMachineOptionManager::remove ()
{
	DEBUG_FATAL (!ms_installed, ("LocalMachineOptionManager::remove: not installed"));
	ms_installed = false;

	delete ms_optionManager;
	ms_optionManager = 0;
}

// ----------------------------------------------------------------------

void LocalMachineOptionManager::save ()
{
	DEBUG_FATAL (!ms_installed, ("LocalMachineOptionManager::save: not installed"));
	ms_optionManager->save (cms_fileName);
}

// ----------------------------------------------------------------------

void LocalMachineOptionManager::registerOption (bool & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("LocalMachineOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

void LocalMachineOptionManager::registerOption (float & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("LocalMachineOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

void LocalMachineOptionManager::registerOption (int & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("LocalMachineOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

void LocalMachineOptionManager::registerOption (std::string & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("LocalMachineOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

void LocalMachineOptionManager::registerOption (Unicode::String & variable, char const * const section, char const * const name, const int version)
{
	DEBUG_FATAL (!ms_installed, ("LocalMachineOptionManager::registerOption: not installed"));
	ms_optionManager->registerOption (variable, section, name, version);
}

// ----------------------------------------------------------------------

float LocalMachineOptionManager::findFloat(char const * const section, char const * const name, float const defaultValue)
{
	DEBUG_FATAL (!ms_installed, ("LocalMachineOptionManager::findFloat: not installed"));
	return ms_optionManager->findFloat (section, name, defaultValue);
}

// ======================================================================
