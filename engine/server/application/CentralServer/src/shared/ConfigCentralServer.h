// ConfigCentralServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_ConfigCentralServer_H
#define	_ConfigCentralServer_H

//-----------------------------------------------------------------------

class ConfigCentralServer
{
public:
	struct Data
	{
		int           chatServicePort;
		int           connectionServicePort;
		int           customerServicePort;
		const char *  clusterName;
		int	   gameServicePort;
		const char *  loginServerAddress;
		int           loginServerPort;
		const char *  startPlanet;
		int           taskManagerPort;
		int           planetServicePort;
		int           planetServerRestartDelayTimeSeconds;
		int           connectionServerRestartDelayTimeSeconds;
		int           chatServerRestartDelayTimeSeconds;
		int           updatePlayerCountFrequency;
		bool          newbieTutorialEnabled;
		const char *  chatServiceBindInterface;
		const char *  connectionServiceBindInterface;
		const char *  customerServiceBindInterface;
		const char *  gameServiceBindInterface;
		const char *  loginServiceBindInterface;
		const char *  planetServiceBindInterface;
		int           firstPlanetWatcherPort;
		int           loginServicePort;
		bool          developmentMode;
		bool          shouldSleep;
		bool          startPublic;
		const char *  consoleServiceBindInterface;
		int           consoleServicePort;
		int           characterCreationTimeout;
		const char *  chatServerHost;
		const char *  dbServerHost;
		int           serverPingTimeout;
		int           recentLoadingStateSeconds;
		int           characterCreationRateLimitSeconds;
		int           characterCtsCreationRateLimitSeconds;
		const char *  transferServerAddress;
		int           transferServerPort;
		const char *  stationPlayersCollectorAddress;
		int           stationPlayersCollectorPort;
		bool          allowZeroConnectionServerPort;
		int           clusterWideDataLockTimeout;
		const char *  commodityServerServiceBindInterface;
		int           commodityServerServicePort;
		bool          shutdown;
		bool          validateBuildVersionNumber;
		int           gameServerConnectionPendingAllocatedSizeLimit;
		int           systemTimeMismatchAlertIntervalSeconds;
		int           ctsDenyLoginThresholdSeconds;

		bool          auctionEnabled;
		const char *  auctionServer;
		int           auctionPort;
		const char *  auctionIDPrefix;

		bool          disconnectDuplicateConnectionsOnOtherGalaxies;

		bool          requestDbSaveOnPlanetServerCrash;

		int           maxTimeToWaitForPlanetServerStartSeconds;

		const char *  metricsDataURL;
		int	      webUpdateIntervalSeconds;
		const char *  metricsSecretKey;
	};

	static const unsigned short  getChatServicePort     ();
	static const unsigned short  getConnectionServicePort();
	static const char *          getConsoleServiceBindInterface();
	static const unsigned short  getConsoleServicePort();
	static const unsigned short  getCustomerServicePort     ();
	static const char *          getClusterName         ();
	static const unsigned short  getGameServicePort     ();
	static const char *          getLoginServerAddress  ();
	static const unsigned short  getLoginServerPort     ();
	static const char *          getStartPlanet         ();
	static const unsigned short  getTaskManagerPort     ();
	static const unsigned short  getPlanetServicePort   ();
	static const unsigned int    getPlanetServerRestartDelayTimeSeconds();
	static const unsigned int    getConnectionServerRestartDelayTimeSeconds();
	static const unsigned int    getChatServerRestartDelayTimeSeconds();
	static int                   getUpdatePlayerCountFrequency ();
	static bool                  getNewbieTutorialEnabled();
	static const char *          getChatServiceBindInterface  ();
	static const char *          getConnectionServiceBindInterface  ();
	static const char *          getCustomerServiceBindInterface    ();
	static const char *          getGameServiceBindInterface        ();
	static const char *          getLoginServiceBindInterface      ();
	static const char *          getPlanetServiceBindInterface      ();
	static const unsigned short  getLoginServicePort    ();
	static void                  install                ();
	static void                  remove                 ();
	static uint16                getFirstPlanetWatcherPort();
	static bool                  getDevelopmentMode     ();
	static bool                  getLogLoopTimes        ();
	static bool                  getShouldSleep         ();
	static bool                  getStartPublic         ();
	static unsigned int          getCharacterCreationTimeout();
	static const char *          getChatServerHost();
	static const char *          getDbServerHost();
	static const int             getServerPingTimeout();
	static const int             getRecentLoadingStateSeconds();
	static const int             getCharacterCreationRateLimitSeconds();
	static const int             getCharacterCtsCreationRateLimitSeconds();
	static const char *          getTransferServerAddress();
	static unsigned short        getTransferServerPort();
	static const char *          getStationPlayersCollectorAddress();
	static unsigned short        getStationPlayersCollectorPort();
	static bool                  getAllowZeroConnectionServerPort();
	static int                   getClusterWideDataLockTimeout();
	static const char *          getCommodityServerServiceBindInterface();
	static const unsigned short  getCommodityServerServicePort();

	static int getNumberOfStartPlanets();
	static char const * getStartPlanet(int index);
	static int getNumberOfStartingConnectionServers();
	static char const * getStartingConnectionServer(int index);
	static bool                  getShutdown();
	static bool                  getValidateBuildVersionNumber();
	static unsigned int          getGameServerConnectionPendingAllocatedSizeLimit();
	static int                   getSystemTimeMismatchAlertIntervalSeconds();
	static int                   getCtsDenyLoginThresholdSeconds();

	static bool          getAuctionEnabled();
	static const char *  getAuctionServer();
	static int           getAuctionPort();
	static const char *  getAuctionIDPrefix();

	static bool          getDisconnectDuplicateConnectionsOnOtherGalaxies();

	static bool          getRequestDbSaveOnPlanetServerCrash();

	static int           getMaxTimeToWaitForPlanetServerStartSeconds();

	static const char *  getMetricsDataURL();
	static int	     getWebUpdateIntervalSeconds();
	static const char *  getMetricsSecretKey();
private:
	static Data *	data;
};

//-----------------------------------------------------------------------

inline const unsigned short ConfigCentralServer::getChatServicePort()
{
	return static_cast<unsigned short>(data->chatServicePort);
}

//-----------------------------------------------------------------------

inline bool ConfigCentralServer::getShouldSleep()
{
	return data->shouldSleep;
}

//-----------------------------------------------------------------------

inline const unsigned short ConfigCentralServer::getConnectionServicePort()
{
	return static_cast<unsigned short>(data->connectionServicePort);
}

//-----------------------------------------------------------------------

inline const unsigned short ConfigCentralServer::getCustomerServicePort()
{
	return static_cast<unsigned short>(data->customerServicePort);
}

//-----------------------------------------------------------------------

inline const char * ConfigCentralServer::getClusterName()
{
	return data->clusterName;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigCentralServer::getGameServicePort()
{
	return static_cast<uint16>(data->gameServicePort);
}

//-----------------------------------------------------------------------

inline const char * ConfigCentralServer::getLoginServerAddress()
{
	return data->loginServerAddress;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigCentralServer::getLoginServerPort()
{
	return static_cast<const uint16>(data->loginServerPort);
}

//-----------------------------------------------------------------------

inline const char * ConfigCentralServer::getStartPlanet()
{
	return data->startPlanet;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigCentralServer::getTaskManagerPort()
{
	return static_cast<uint16>(data->taskManagerPort);
}

// ----------------------------------------------------------------------

inline const uint16 ConfigCentralServer::getPlanetServicePort()
{
	return static_cast<uint16>(data->planetServicePort);
}

// ----------------------------------------------------------------------

inline const unsigned int ConfigCentralServer::getPlanetServerRestartDelayTimeSeconds()
{
	return static_cast<unsigned int>(data->planetServerRestartDelayTimeSeconds);
}

// ----------------------------------------------------------------------

inline const unsigned int ConfigCentralServer::getConnectionServerRestartDelayTimeSeconds()
{
	return static_cast<unsigned int>(data->connectionServerRestartDelayTimeSeconds);
}

// ----------------------------------------------------------------------

inline const unsigned int ConfigCentralServer::getChatServerRestartDelayTimeSeconds()
{
	return static_cast<unsigned int>(data->chatServerRestartDelayTimeSeconds);
}

//-----------------------------------------------------------------------

inline int ConfigCentralServer::getUpdatePlayerCountFrequency()
{
	return data->updatePlayerCountFrequency;
}

// ----------------------------------------------------------------------

inline bool ConfigCentralServer::getNewbieTutorialEnabled()
{
	return data->newbieTutorialEnabled;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getChatServiceBindInterface()
{
	return data->chatServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getConnectionServiceBindInterface()
{
	return data->connectionServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getCustomerServiceBindInterface()
{
	return data->customerServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getGameServiceBindInterface()
{
	return data->gameServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getLoginServiceBindInterface()
{
	return data->loginServiceBindInterface;
}

//-----------------------------------------------------------------------


inline const char * ConfigCentralServer::getPlanetServiceBindInterface()
{
	return data->planetServiceBindInterface;
}

// ----------------------------------------------------------------------

inline uint16 ConfigCentralServer::getFirstPlanetWatcherPort()
{
	return static_cast<uint16>(data->firstPlanetWatcherPort);
}

// ----------------------------------------------------------------------

inline const unsigned short ConfigCentralServer::getLoginServicePort()
{
	return static_cast<unsigned short>(data->loginServicePort);
}

// ----------------------------------------------------------------------

inline bool ConfigCentralServer::getStartPublic()
{
	return data->startPublic;
}

// ----------------------------------------------------------------------

inline bool ConfigCentralServer::getDevelopmentMode()
{
	return data->developmentMode;
}

//-----------------------------------------------------------------------

inline const char * ConfigCentralServer::getConsoleServiceBindInterface()
{
	return data->consoleServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const unsigned short ConfigCentralServer::getConsoleServicePort()
{
	return static_cast<unsigned short>(data->consoleServicePort);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigCentralServer::getCharacterCreationTimeout()
{
	return static_cast<unsigned int>(data->characterCreationTimeout);
}

//------------------------------------------------------------------------------------------

inline const char * ConfigCentralServer::getChatServerHost()
{
	return data->chatServerHost;
}

//------------------------------------------------------------------------------------------

inline const char * ConfigCentralServer::getDbServerHost()
{
	return data->dbServerHost;
}

//------------------------------------------------------------------------------------------

inline const int ConfigCentralServer::getServerPingTimeout()
{
	return data->serverPingTimeout;
}

// ----------------------------------------------------------------------

inline const int ConfigCentralServer::getRecentLoadingStateSeconds()
{
	return data->recentLoadingStateSeconds;
}

// ----------------------------------------------------------------------

inline const int ConfigCentralServer::getCharacterCreationRateLimitSeconds()
{
	return data->characterCreationRateLimitSeconds;
}

// ----------------------------------------------------------------------

inline const int ConfigCentralServer::getCharacterCtsCreationRateLimitSeconds()
{
	return data->characterCtsCreationRateLimitSeconds;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getTransferServerAddress()
{
	return data->transferServerAddress;
}

// ----------------------------------------------------------------------

inline unsigned short ConfigCentralServer::getTransferServerPort()
{
	return static_cast<unsigned short>(data->transferServerPort);
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getStationPlayersCollectorAddress()
{
	return data->stationPlayersCollectorAddress;
}

// ----------------------------------------------------------------------

inline unsigned short ConfigCentralServer::getStationPlayersCollectorPort()
{
	return static_cast<unsigned short>(data->stationPlayersCollectorPort);
}

// ----------------------------------------------------------------------

inline bool ConfigCentralServer::getAllowZeroConnectionServerPort()
{
	return data->allowZeroConnectionServerPort;
}

// ----------------------------------------------------------------------

inline int ConfigCentralServer::getClusterWideDataLockTimeout()
{
	return data->clusterWideDataLockTimeout;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getCommodityServerServiceBindInterface()
{
	return data->commodityServerServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const unsigned short ConfigCentralServer::getCommodityServerServicePort()
{
	return static_cast<unsigned short>(data->commodityServerServicePort);
}

// ----------------------------------------------------------------------

inline bool ConfigCentralServer::getShutdown()
{
	return data->shutdown;
}

// ----------------------------------------------------------------------

inline bool ConfigCentralServer::getValidateBuildVersionNumber()
{
	return data->validateBuildVersionNumber;
}

// ----------------------------------------------------------------------

inline unsigned int ConfigCentralServer::getGameServerConnectionPendingAllocatedSizeLimit()
{
	return static_cast<unsigned int>(data->gameServerConnectionPendingAllocatedSizeLimit);
}

// ----------------------------------------------------------------------

inline int ConfigCentralServer::getSystemTimeMismatchAlertIntervalSeconds()
{
	return data->systemTimeMismatchAlertIntervalSeconds;
}

// ----------------------------------------------------------------------

inline int ConfigCentralServer::getCtsDenyLoginThresholdSeconds()
{
	return data->ctsDenyLoginThresholdSeconds;
}

// ----------------------------------------------------------------------

inline	bool          ConfigCentralServer::getAuctionEnabled() { return data->auctionEnabled; }
inline	const char *  ConfigCentralServer::getAuctionServer() { return data->auctionServer; }
inline	int           ConfigCentralServer::getAuctionPort() { return data->auctionPort; }
inline	const char *  ConfigCentralServer::getAuctionIDPrefix() { return data->auctionIDPrefix; }

// ----------------------------------------------------------------------

inline bool ConfigCentralServer::getDisconnectDuplicateConnectionsOnOtherGalaxies()
{
	return data->disconnectDuplicateConnectionsOnOtherGalaxies;
}

// ----------------------------------------------------------------------

inline bool ConfigCentralServer::getRequestDbSaveOnPlanetServerCrash()
{
	return data->requestDbSaveOnPlanetServerCrash;
}

// ----------------------------------------------------------------------

inline int ConfigCentralServer::getMaxTimeToWaitForPlanetServerStartSeconds()
{
	return data->maxTimeToWaitForPlanetServerStartSeconds;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getMetricsDataURL()
{
	return data->metricsDataURL;
}

// ----------------------------------------------------------------------

inline int ConfigCentralServer::getWebUpdateIntervalSeconds()
{
	return data->webUpdateIntervalSeconds;
}

// ----------------------------------------------------------------------

inline const char * ConfigCentralServer::getMetricsSecretKey()
{
	return data->metricsSecretKey;
}

// ======================================================================

#endif	// _ConfigCentralServer_H
