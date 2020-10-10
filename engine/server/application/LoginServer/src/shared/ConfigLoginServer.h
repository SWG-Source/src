// ConfigLoginServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_ConfigLoginServer_H
#define	_ConfigLoginServer_H

//-----------------------------------------------------------------------

class ConfigLoginServer
{
  public:
	struct Data
	{
		int             centralServicePort;
 		int             clientServicePort;
		int             maxClients;
		int             taskServicePort;
		int             pingServicePort;
		int		        httpServicePort;
		bool            validateClientVersion;
		bool            validateStationKey;
    bool            easyExternalAccess;
		bool            doSessionLogin;
		bool            doConsumption;
		const char *    sessionServers;
		int             sessionType;

		const char *    DSN;
 		const char *    databaseUID;
		const char *    schemaOwner;
		const char *    databasePWD;
		const char *    databaseProtocol;
		bool            enableQueryProfile;
		bool            verboseQueryMode;
		bool            logWorkerThreads;

		const char *    privateIpMask;
		int             maxPlayersPerCluster;
		int             maxCharactersPerCluster;
		int             maxCharactersPerAccount;
		int             clientOverflowLimit;
		bool            developmentMode;
		int             databaseThreads;
		bool            compressClientNetworkTraffic;
		int             metricsListenerPort;
		float           defaultDBQueueUpdateTimeLimit;
		int             disconnectSleepTime;
		int             clusterGroup;
		int             maxSimultaneousPurgeAccounts;
		int             purgeSleepTime;
		bool            enableStructurePurge;
		bool            enableCharacterPurge;
		int             updatePurgeAccountListTime;
		const char *    purgeAccountSourceTable;
		const char *    adminAccountDataTable;
		bool            allowSkipTutorialToAll;

		bool            internalBypassOnlineLimit;

		int             populationExtremelyHeavyThresholdPercent;
		int             populationVeryHeavyThresholdPercent;
		int             populationHeavyThresholdPercent;
		int             populationMediumThresholdPercent;
		int             populationLightThresholdPercent;
		int             csToolPort;

		bool		    requireSecureLoginForCsTool;

		bool            useOldSuidGenerator;
		bool            useExternalAuth;
		bool            useJsonWebApi;
		const char *	externalAuthURL;
    const char *  externalAuthSecretKey;
	};

	static const uint16 getCentralServicePort();
	static const uint16 getClientServicePort();
	static const int    getClientOverflowLimit();
	static const uint16 getTaskServicePort();
	static const uint16 getPingServicePort();
	static const uint16 getHttpServicePort();
	static const bool   getValidateClientVersion();
	static const bool   getValidateStationKey();
  static const bool   getEasyExternalAccess();
	static const bool   getDoSessionLogin();
	static const bool   getDoConsumption();
	static const char * getSessionServers();
	static const int    getSessionType();

	static const int    getMaxClients ();
	static const char * getPrivateIpMask();
	static const char * getDSN();
	static const char * getDatabaseUID();
	static const char * getSchemaOwner();
	static const char * getDatabasePWD();
	static const char * getDatabaseProtocol();
	static const bool   getEnableQueryProfile();
	static const bool   getVerboseQueryMode();
	static const bool   getLogWorkerThreads();
	static int          getMaxPlayersPerCluster();
	static int          getMaxCharactersPerCluster();
	static int          getMaxCharactersPerAccount();
	//static int          getMaxCharactersPerAccountPerCluster();
	static const bool   getDevelopmentMode();
	static int          getDatabaseThreads();
	static bool         getCompressClientNetworkTraffic();
	static uint16       getMetricsListenerPort();
	static float        getDefaultDBQueueUpdateTimeLimit();
	static void         install ();
	static void         remove ();

	static int          getNumberOfSessionServers();
	static char const * getSessionServer(int index);
	static const int    getDisconnectSleepTime (void);
	static const int    getClusterGroup();
	static int          getMaxSimultaneousPurgeAccounts();
	static int          getPurgeSleepTime();
	static bool         getEnableStructurePurge();
	static bool         getEnableCharacterPurge();
	static int          getUpdatePurgeAccountListTime();
	static const char * getPurgeAccountSourceTable();
	static int          getPurgePhaseAdvanceDays(int purgePhase);
	static const char * getAdminAccountDataTable();
	static bool         getAllowSkipTutorialToAll();

	static bool         getInternalBypassOnlineLimit();
	static const int    getCSToolPort();

	static bool	        getRequireSecureLoginForCsTool();

	static int          getPopulationExtremelyHeavyThresholdPercent();
	static int          getPopulationVeryHeavyThresholdPercent();
	static int          getPopulationHeavyThresholdPercent();
	static int          getPopulationMediumThresholdPercent();
	static int          getPopulationLightThresholdPercent();

	static bool	        getUseExternalAuth();
	static bool	        getUseJsonWebApi();
	static const char * getExternalAuthUrl();
  static const char * getExternalAuthSecretKey();
	static bool	        getUseOldSuidGenerator();

	// has character creation for this cluster been disabled through config option
	static bool         isCharacterCreationDisabled(std::string const & cluster);
	static              std::set<std::string> const & getCharacterCreationDisabledClusterList();

private:
	static Data *	data;
};

//-----------------------------------------------------------------------

inline const uint16 ConfigLoginServer::getCentralServicePort()
{
	return static_cast<const uint16>(data->centralServicePort);
}

//-----------------------------------------------------------------------

inline const uint16 ConfigLoginServer::getHttpServicePort()
{
	return static_cast<const uint16>(data->httpServicePort);
}

//-----------------------------------------------------------------------

inline const uint16 ConfigLoginServer::getClientServicePort()
{
	return static_cast<const uint16>(data->clientServicePort);
}

//-----------------------------------------------------------------------

inline const int ConfigLoginServer::getClientOverflowLimit()
{
	return data->clientOverflowLimit;
}

//-----------------------------------------------------------------------


inline const int ConfigLoginServer::getMaxClients()
{
	return data->maxClients;
}

//-----------------------------------------------------------------------

inline const char * ConfigLoginServer::getPrivateIpMask()
{
	return data->privateIpMask;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigLoginServer::getTaskServicePort()
{
	return static_cast<const uint16>(data->taskServicePort);
}

//-----------------------------------------------------------------------

inline const uint16 ConfigLoginServer::getPingServicePort()
{
	return static_cast<const uint16>(data->pingServicePort);
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getValidateClientVersion()
{
	return (data->validateClientVersion);
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getValidateStationKey()
{
	return (data->validateStationKey);
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getEasyExternalAccess()
{
	return (data->easyExternalAccess);
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getDoSessionLogin()
{
	return (data->doSessionLogin);
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getDoConsumption()
{
	return (data->doConsumption);
}

// ----------------------------------------------------------------------

inline const char * ConfigLoginServer::getSessionServers()
{
	return (data->sessionServers);
}

//------------------------------------------------------------------------------------------

inline const int ConfigLoginServer::getSessionType()
{
	return data->sessionType;
}

// ----------------------------------------------------------------------

inline const char * ConfigLoginServer::getDSN()
{
	return (data->DSN);
}

// ----------------------------------------------------------------------

inline const char *	ConfigLoginServer::getDatabaseUID()
{
	return (data->databaseUID);
}

// ----------------------------------------------------------------------

inline const char * ConfigLoginServer::getDatabasePWD()
{
	return (data->databasePWD);
}

// ----------------------------------------------------------------------

inline const char * ConfigLoginServer::getDatabaseProtocol()
{
	return (data->databaseProtocol);
}

//-----------------------------------------------------------------------

inline int ConfigLoginServer::getMaxPlayersPerCluster()
{
	return (data->maxPlayersPerCluster);
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getMaxCharactersPerCluster()
{
	return (data->maxCharactersPerCluster);
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getMaxCharactersPerAccount()
{
	return (data->maxCharactersPerAccount);
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getEnableQueryProfile()
{
	return (data->enableQueryProfile);
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getVerboseQueryMode()
{
	return (data->verboseQueryMode);
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getLogWorkerThreads()
{
	return data->logWorkerThreads;
}

// ----------------------------------------------------------------------

inline const bool ConfigLoginServer::getDevelopmentMode()
{
	return (data->developmentMode);
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getDatabaseThreads()
{
	return (data->databaseThreads);
}

// ----------------------------------------------------------------------

inline bool ConfigLoginServer::getCompressClientNetworkTraffic()
{
	return data->compressClientNetworkTraffic;
}

// ----------------------------------------------------------------------

inline uint16 ConfigLoginServer::getMetricsListenerPort()
{
	return static_cast<uint16>(data->metricsListenerPort);
}

// ----------------------------------------------------------------------

inline const char *ConfigLoginServer::getSchemaOwner()
{
	return data->schemaOwner;
}

// ----------------------------------------------------------------------

inline float ConfigLoginServer::getDefaultDBQueueUpdateTimeLimit()
{
	return data->defaultDBQueueUpdateTimeLimit;
}

// ----------------------------------------------------------------------

inline const int ConfigLoginServer::getDisconnectSleepTime(void)
{
	return data->disconnectSleepTime;
}

// ----------------------------------------------------------------------

inline const int ConfigLoginServer::getClusterGroup()
{
	return data->clusterGroup;
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getMaxSimultaneousPurgeAccounts()
{
	return data->maxSimultaneousPurgeAccounts;
}

//-----------------------------------------------------------------------

inline int ConfigLoginServer::getPurgeSleepTime()
{
	return data->purgeSleepTime;
}

// ----------------------------------------------------------------------

inline bool ConfigLoginServer::getEnableStructurePurge()
{
	return data->enableStructurePurge;
}

// ----------------------------------------------------------------------

inline bool ConfigLoginServer::getEnableCharacterPurge()
{
	return data->enableCharacterPurge;
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getUpdatePurgeAccountListTime()
{
	return data->updatePurgeAccountListTime;
}

// ----------------------------------------------------------------------

inline char const * ConfigLoginServer::getPurgeAccountSourceTable()
{
	return data->purgeAccountSourceTable;
}

// ----------------------------------------------------------------------

inline const char * ConfigLoginServer::getAdminAccountDataTable()
{
	return data->adminAccountDataTable;
}

// ----------------------------------------------------------------------

inline bool ConfigLoginServer::getAllowSkipTutorialToAll()
{
	return data->allowSkipTutorialToAll;
}

// ----------------------------------------------------------------------

inline bool ConfigLoginServer::getInternalBypassOnlineLimit()
{
	return data->internalBypassOnlineLimit;
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getPopulationExtremelyHeavyThresholdPercent()
{
	return data->populationExtremelyHeavyThresholdPercent;
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getPopulationVeryHeavyThresholdPercent()
{
	return data->populationVeryHeavyThresholdPercent;
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getPopulationHeavyThresholdPercent()
{
	return data->populationHeavyThresholdPercent;
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getPopulationMediumThresholdPercent()
{
	return data->populationMediumThresholdPercent;
}

// ----------------------------------------------------------------------

inline int ConfigLoginServer::getPopulationLightThresholdPercent()
{
	return data->populationLightThresholdPercent;
}

inline bool ConfigLoginServer::getRequireSecureLoginForCsTool()
{
	return data->requireSecureLoginForCsTool;
}

inline const int ConfigLoginServer::getCSToolPort()
{
	return data->csToolPort;
}

inline bool ConfigLoginServer::getUseJsonWebApi()
{
	return data->useJsonWebApi;
}

inline bool ConfigLoginServer::getUseExternalAuth()
{
	return data->useExternalAuth;
}

inline const char * ConfigLoginServer::getExternalAuthUrl()
{
	return data->externalAuthURL;
}

inline const char * ConfigLoginServer::getExternalAuthSecretKey()
{
	return data->externalAuthSecretKey;
}

inline bool ConfigLoginServer::getUseOldSuidGenerator()
{
    return data->useOldSuidGenerator;
}
// ======================================================================

#endif	// _ConfigLoginServer_H
