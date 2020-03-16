// ConfigTaskManager.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigTaskManager.h"

#include <vector>

//-----------------------------------------------------------------------

ConfigTaskManager::Data *	ConfigTaskManager::data = 0;

#define KEY_INT(a,b)       (data->a = ConfigFile::getKeyInt("TaskManager", #a, b))
#define KEY_BOOL(a,b)      (data->a = ConfigFile::getKeyBool("TaskManager", #a, b))
#define KEY_FLOAT(a,b)     (data->a = ConfigFile::getKeyFloat("TaskManager", #a, b))
#define KEY_STRING(a,b)    (data->a = ConfigFile::getKeyString("TaskManager", #a, b))

//-----------------------------------------------------------------------

namespace ConfigTaskManagerNamespace
{
	typedef std::vector<char const *> StringPtrArray;
	StringPtrArray ms_environmentVariable; // ConfigFile owns the pointer
}

using namespace ConfigTaskManagerNamespace;

// ======================================================================

int ConfigTaskManager::getNumberOfEnvironmentVariables()
{
	return static_cast<int>(ms_environmentVariable.size());
}

// ----------------------------------------------------------------------

char const * ConfigTaskManager::getEnvironmentVariable(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfEnvironmentVariables());
	return ms_environmentVariable[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

void ConfigTaskManager::install(void)
{
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	data = new ConfigTaskManager::Data;

	KEY_BOOL    (autoStart, true);
	KEY_STRING  (clusterName, "devcluster");
	KEY_BOOL    (verifyClusterName, false);
	KEY_INT     (gameServerTimeout, 600);
	KEY_STRING  (gameServiceBindInterface, "");
	KEY_INT     (gameServicePort, 60001);
	KEY_INT     (consoleConnectionPort, 60000);
	KEY_STRING  (consoleServiceBindInterface, "");
	KEY_STRING	(loginServerAddress, "localhost");
	KEY_INT	    (loginServerTaskServicePort, 44459);
	KEY_FLOAT   (maximumLoad, 3.0f);
	KEY_FLOAT   (loadConnectionServer, 0.5f);
	KEY_FLOAT   (loadPlanetServer, 0.128f);
	KEY_FLOAT   (loadGameServer, 1.0f);
	KEY_BOOL    (publishMode, false);
	KEY_STRING  (rcFileName, "taskmanager.rc");
	KEY_BOOL    (restartCentral, false);
	KEY_INT     (restartDelayCentralServer, 60); // seconds
	KEY_INT     (restartDelayLogServer, 30);
	KEY_INT     (restartDelayMetricsServer, 60);
	KEY_INT     (restartDelayCommoditiesServer, 60);
	KEY_INT     (restartDelayTransferServer, 30);
	KEY_STRING  (taskManagerServiceBindInterface, "");
	KEY_INT     (taskManagerServicePort, 50001);
	KEY_INT     (maximumClockDriftToleranceSeconds, 10); // seconds
	KEY_INT     (systemTimeCheckIntervalSeconds, 60); // seconds
	KEY_INT     (clockDriftFatalIntervalSeconds, 1*60*60); // seconds
	KEY_BOOL    (allowPreferredServerOnMasterNode, false);

	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("TaskManager", "environmentVariable", index++, 0);
		if (result != 0)
		{
			ms_environmentVariable.push_back(result);
		}
	}
	while (result);
}

//-----------------------------------------------------------------------

void ConfigTaskManager::remove(void)
{
	delete data;
	data = 0;
}

//-----------------------------------------------------------------------

