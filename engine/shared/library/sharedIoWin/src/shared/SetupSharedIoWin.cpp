// ======================================================================
//
// SetupSharedIoWin.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedIoWin/FirstSharedIoWin.h"
#include "sharedIoWin/SetupSharedIoWin.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedIoWin/ConfigSharedIoWin.h"
#include "sharedIoWin/IoWinManager.h"

// ======================================================================

void SetupSharedIoWin::install()
{
	InstallTimer const installTimer("SetupSharedIoWin::install");

	ConfigSharedIoWin::install();
	IoWinManager::install();
}

// ======================================================================
