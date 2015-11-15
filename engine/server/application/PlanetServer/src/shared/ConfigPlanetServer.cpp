// ======================================================================
//
// ConfigPlanetServer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "ConfigPlanetServer.h"
#include "QuadtreeNode.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"

//-----------------------------------------------------------------------

ConfigPlanetServer::Data *	ConfigPlanetServer::data = 0;

#define KEY_INT(a,b)     (data->a = ConfigFile::getKeyInt("PlanetServer", #a, b))
#define KEY_BOOL(a,b)    (data->a = ConfigFile::getKeyBool("PlanetServer", #a, b)) 
//#define KEY_FLOAT(a,b) (data->a = ConfigFile::getKeyFloat("PlanetServer", #a, b)) // commented out for Lint.  Uncomment if you need this macro again
#define KEY_STRING(a,b)  (data->a = ConfigFile::getKeyString("PlanetServer", #a, b))

// ======================================================================

void ConfigPlanetServer::install(void)
{

	ConfigServerUtility::install();
	ExitChain::add(&remove, "ConfigPlanetServer::remove");
	
	data = new ConfigPlanetServer::Data;

	KEY_STRING (centralServerAddress, "localhost");
	KEY_INT    (centralServerPort, 44455);
	KEY_STRING (sceneID, "default");
	KEY_INT    (gameServicePort, 0);
	KEY_INT    (taskManagerPort, 60001);
	KEY_INT    (watcherServicePort, 60002);
	KEY_INT    (maxWatcherConnections, 1);
	KEY_INT    (watcherOverflowLimit, 1024 * 1024 * 8); // 8MB overflow for CS PlanetWatcher tool
	KEY_BOOL   (logObjectLoading, false);
	KEY_INT    (maxWatcherUpdatesPerMessage,500); // Max object updates to send to the watcher in a single message
	KEY_STRING (gameServiceBindInterface, "");
	KEY_STRING (watcherServiceBindInterface, "");
	KEY_BOOL   (loadWholePlanet, false);
	KEY_BOOL   (loadWholePlanetMultiserver, false);
	KEY_BOOL   (logPreloading, false);
	KEY_INT    (numTutorialServers, 1);
	KEY_INT    (maxInterestRadius, Node::getNodeSize());
	KEY_INT    (populationCountTime, 60); // seconds
	KEY_BOOL   (logChunkLoading, false);
	KEY_INT    (preloadBailoutTime, 0); // seconds
	KEY_STRING (preloadDataTableName, "datatables/planet_server/preload_list.iff");
	KEY_INT    (authTransferSanityCheckTimeMs, 15000);
	KEY_INT    (gameServerRestartDelayTimeSeconds, 60);
	KEY_BOOL   (enableContentsChecking, false);
	KEY_INT    (maxGameServers, 0);
	KEY_BOOL   (enableStartupCreateProxies, true);
	KEY_BOOL   (requestDbSaveOnGameServerCrash, true);
	KEY_BOOL   (gameServerProfiling, false);
	KEY_BOOL   (gameServerDebugging, false);
	KEY_INT    (gameServerDebuggingPortBase, 0);
	KEY_INT    (maxTimeToWaitForGameServerStartSeconds, 5*60); // seconds
}

//-----------------------------------------------------------------------

bool ConfigPlanetServer::getGameServerProfiling()
{
	return data->gameServerProfiling;
}

//-----------------------------------------------------------------------

bool ConfigPlanetServer::getGameServerDebugging()
{
	return data->gameServerDebugging;
}

//-----------------------------------------------------------------------

void ConfigPlanetServer::remove(void)
{
	delete data;
	data = 0;
	ConfigServerUtility::remove();
}

//-----------------------------------------------------------------------

int ConfigPlanetServer::getGameServerDebuggingPortBase()
{
	return data->gameServerDebuggingPortBase;
}

//-----------------------------------------------------------------------
