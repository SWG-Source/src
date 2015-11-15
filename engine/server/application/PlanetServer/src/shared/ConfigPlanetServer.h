// ======================================================================
//
// ConfigPlanetServer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConfigPlanetServer_H
#define INCLUDED_ConfigPlanetServer_H

// ======================================================================

class ConfigPlanetServer
{
public:
	struct Data
	{
		const char * centralServerAddress;
		int          centralServerPort;
		const char * sceneID;
		int          gameServicePort;
		int          watcherServicePort;
		int          taskManagerPort;
		int          maxWatcherConnections;
		int          watcherOverflowLimit;
		bool         logObjectLoading;
		int          maxWatcherUpdatesPerMessage;
		const char * gameServiceBindInterface;
		const char * watcherServiceBindInterface;
		bool         loadWholePlanet;
		bool         loadWholePlanetMultiserver;
		bool         logPreloading;
		int          numTutorialServers;
		int          maxInterestRadius;
		int          populationCountTime;
		bool         logChunkLoading;
		int          preloadBailoutTime;
		const char * preloadDataTableName;
		int          authTransferSanityCheckTimeMs;
		int          gameServerRestartDelayTimeSeconds;
		bool         enableContentsChecking;
		int          maxGameServers;
		bool         enableStartupCreateProxies;
		bool         requestDbSaveOnGameServerCrash;
		bool         gameServerDebugging;
		bool         gameServerProfiling;
		int          gameServerDebuggingPortBase;
		int          maxTimeToWaitForGameServerStartSeconds;
	};

	static void			install					(); 
  	static void			remove					(); 

	static const char * getCentralServerAddress ();
	static const uint16 getCentralServerPort    ();
	static const char * getSceneID              ();
	static const uint16 getGameServicePort      ();
	static const uint16 getWatcherServicePort   ();
	static const uint16 getTaskManagerPort      ();
	static const int    getMaxWatcherConnections();
	static const int    getWatcherOverflowLimit ();
	static const bool   getLogObjectLoading     ();
	static const int    getMaxWatcherUpdatesPerMessage();
	static const char * getGameServiceBindInterface();
	static const char * getWatcherServiceBindInterface();
	static const bool   getLoadWholePlanet();
	static const bool   getLoadWholePlanetMultiserver();
	static const bool   getLogPreloading();
	static const int    getNumTutorialServers();
	static const int    getMaxInterestRadius();
	static const int    getPopulationCountTime();
	static const bool   getLogChunkLoading();
	static const int    getPreloadBailoutTime();
	static const char * getPreloadDataTableName();
	static const int    getAuthTransferSanityCheckTimeMs();
	static const unsigned int getGameServerRestartDelayTimeSeconds();
	static bool         getEnableContentsChecking();
	static int          getMaxGameServers();
	static bool         getEnableStartupCreateProxies();
	static bool         getRequestDbSaveOnGameServerCrash();
	static bool         getGameServerDebugging();
	static bool         getGameServerProfiling();
	static int          getGameServerDebuggingPortBase();
	static int          getMaxTimeToWaitForGameServerStartSeconds();
  private:
	static Data *	data;
};

// ======================================================================

inline const char * ConfigPlanetServer::getCentralServerAddress()
{
	return data->centralServerAddress;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigPlanetServer::getCentralServerPort()
{
	return static_cast<const uint16>(data->centralServerPort);
}

//-----------------------------------------------------------------------

inline const char * ConfigPlanetServer::getSceneID()
{
	return data->sceneID;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigPlanetServer::getGameServicePort()
{
	return static_cast<uint16>(data->gameServicePort);
}

//-----------------------------------------------------------------------

inline const uint16 ConfigPlanetServer::getTaskManagerPort()
{
	return static_cast<uint16>(data->taskManagerPort);
}

// ----------------------------------------------------------------------

inline const uint16 ConfigPlanetServer::getWatcherServicePort()
{
	return static_cast<uint16>(data->watcherServicePort);
}

// ----------------------------------------------------------------------

inline const int ConfigPlanetServer::getMaxWatcherConnections()
{
	return data->maxWatcherConnections;
}

//-----------------------------------------------------------------------

inline const int ConfigPlanetServer::getWatcherOverflowLimit()
{
	return data->watcherOverflowLimit;
}

// ----------------------------------------------------------------------

inline const bool ConfigPlanetServer::getLogObjectLoading()
{
	return data->logObjectLoading;
}

// ----------------------------------------------------------------------

inline const int ConfigPlanetServer::getMaxWatcherUpdatesPerMessage()
{
	return data->maxWatcherUpdatesPerMessage;
}

// ----------------------------------------------------------------------

inline const char * ConfigPlanetServer::getGameServiceBindInterface()
{
	return data->gameServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const char * ConfigPlanetServer::getWatcherServiceBindInterface()
{
	return data->watcherServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const bool ConfigPlanetServer::getLoadWholePlanet()
{
	return data->loadWholePlanet;
}

// ----------------------------------------------------------------------

inline const bool ConfigPlanetServer::getLoadWholePlanetMultiserver()
{
	return data->loadWholePlanetMultiserver;
}

// ----------------------------------------------------------------------

inline const bool ConfigPlanetServer::getLogPreloading()
{
	return data->logPreloading;
}

// ----------------------------------------------------------------------

inline const int ConfigPlanetServer::getNumTutorialServers()
{
	return data->numTutorialServers;
}

// ----------------------------------------------------------------------

inline const int ConfigPlanetServer::getMaxInterestRadius()
{
	return data->maxInterestRadius;
}

// ----------------------------------------------------------------------

inline const int ConfigPlanetServer::getPopulationCountTime()
{
	return data->populationCountTime;
}

// ----------------------------------------------------------------------

inline const bool ConfigPlanetServer::getLogChunkLoading()
{
	return data->logChunkLoading;
}

// ----------------------------------------------------------------------

inline const int ConfigPlanetServer::getPreloadBailoutTime()
{
	return data->preloadBailoutTime;
}

// ----------------------------------------------------------------------

inline const char * ConfigPlanetServer::getPreloadDataTableName()
{
	return data->preloadDataTableName;
}

// ----------------------------------------------------------------------

inline const int ConfigPlanetServer::getAuthTransferSanityCheckTimeMs()
{
	return data->authTransferSanityCheckTimeMs;
}

// ----------------------------------------------------------------------

inline const unsigned int ConfigPlanetServer::getGameServerRestartDelayTimeSeconds()
{
	return static_cast<unsigned int>(data->gameServerRestartDelayTimeSeconds);
}

// ----------------------------------------------------------------------

inline bool ConfigPlanetServer::getEnableContentsChecking()
{
	return data->enableContentsChecking;
}

// ----------------------------------------------------------------------

inline int ConfigPlanetServer::getMaxGameServers()
{
	return data->maxGameServers;
}

// ----------------------------------------------------------------------

inline bool ConfigPlanetServer::getEnableStartupCreateProxies()
{
	return data->enableStartupCreateProxies;
}

// ----------------------------------------------------------------------

inline bool ConfigPlanetServer::getRequestDbSaveOnGameServerCrash()
{
	return data->requestDbSaveOnGameServerCrash;
}

// ----------------------------------------------------------------------

inline int ConfigPlanetServer::getMaxTimeToWaitForGameServerStartSeconds()
{
	return data->maxTimeToWaitForGameServerStartSeconds;
}

// ======================================================================

#endif
