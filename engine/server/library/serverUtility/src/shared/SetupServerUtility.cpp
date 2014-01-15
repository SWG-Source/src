// ======================================================================
//
// SetupServerUtility.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/SetupServerUtility.h"

#include "serverUtility/ChatLogManager.h"
#include "serverUtility/ConfigServerUtility.h"
#include "serverUtility/ServerConnection.h"

// ======================================================================

void SetupServerUtility::install()
{
	ConfigServerUtility::install();
	ChatLogManager::install();
	ServerConnection::install();
}


// ======================================================================
