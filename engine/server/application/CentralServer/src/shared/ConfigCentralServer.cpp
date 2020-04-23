// ConfigCentralServer.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigCentralServer.h"

//-----------------------------------------------------------------------

ConfigCentralServer::Data *	ConfigCentralServer::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("CentralServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("CentralServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("CentralServer", #a, b))

//-----------------------------------------------------------------------

namespace ConfigCentralServerNamespace
{
	typedef std::vector<char const *> StringPtrArray;
	StringPtrArray ms_startPlanets; // ConfigFile owns the pointer
	StringPtrArray ms_startConnectionServer;
}

using namespace ConfigCentralServerNamespace;

// ======================================================================

int ConfigCentralServer::getNumberOfStartPlanets()
{
	return static_cast<int>(ms_startPlanets.size());
}

// ----------------------------------------------------------------------

char const * ConfigCentralServer::getStartPlanet(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfStartPlanets());
	return ms_startPlanets[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

int ConfigCentralServer::getNumberOfStartingConnectionServers()
{
	return static_cast<int>(ms_startConnectionServer.size());
}

// ----------------------------------------------------------------------

char const * ConfigCentralServer::getStartingConnectionServer(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfStartingConnectionServers());
	return ms_startConnectionServer[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

void ConfigCentralServer::install(void)
{
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	ConfigServerUtility::install();

	data = new ConfigCentralServer::Data;

	KEY_INT    (chatServicePort, 61232);
	KEY_INT    (connectionServicePort, 0);
	KEY_INT    (customerServicePort,   0);
	KEY_INT    (gameServicePort, 44451);
	KEY_STRING (loginServerAddress, "127.0.0.1");
	KEY_INT    (loginServerPort, 44452);
	KEY_STRING (clusterName, "devcluster");
	KEY_STRING (startPlanet, "tatooine");
	KEY_INT    (taskManagerPort, 60001);
	KEY_INT    (planetServicePort, 44455);
	KEY_INT    (planetServerRestartDelayTimeSeconds, 60);
	KEY_INT    (connectionServerRestartDelayTimeSeconds, 60);
	KEY_INT    (chatServerRestartDelayTimeSeconds, 60);
	KEY_INT    (updatePlayerCountFrequency, 10);
	KEY_BOOL   (newbieTutorialEnabled, false);
	KEY_STRING (chatServiceBindInterface, "");
	KEY_STRING (connectionServiceBindInterface, "");
	KEY_STRING (customerServiceBindInterface, "");
	KEY_STRING (gameServiceBindInterface, "");
	KEY_STRING (loginServiceBindInterface, "");
	KEY_STRING (planetServiceBindInterface, "");
	KEY_INT    (firstPlanetWatcherPort, 60002);
	KEY_INT    (loginServicePort, 44452);
	KEY_BOOL   (developmentMode, true);
	KEY_BOOL   (shouldSleep, true);
	KEY_BOOL   (startPublic, true);
	KEY_INT    (consoleServicePort, 61000);
	KEY_STRING (consoleServiceBindInterface, "");
	KEY_INT    (characterCreationTimeout,5*60); // seconds
	KEY_STRING (chatServerHost, "local");
	KEY_STRING (dbServerHost, "local");
	KEY_INT    (serverPingTimeout,0);  // seconds
	KEY_INT    (recentLoadingStateSeconds,60*60);
	KEY_INT    (characterCreationRateLimitSeconds, 0);
	KEY_INT    (characterCtsCreationRateLimitSeconds, 0);
	KEY_STRING (transferServerAddress, "127.0.0.1");
	KEY_INT    (transferServerPort, 0); // TransferServer listens on 50005, default to "off"
	KEY_STRING (stationPlayersCollectorAddress, "127.0.0.1");
	KEY_INT    (stationPlayersCollectorPort, 0); // StationPlayersCollector listens on 50010, default to "off"
	KEY_BOOL   (allowZeroConnectionServerPort, true);
	KEY_INT    (clusterWideDataLockTimeout, 300); // seconds
	KEY_STRING (commodityServerServiceBindInterface, "");
	KEY_INT    (commodityServerServicePort, 44456);
	KEY_BOOL   (shutdown, false);
	KEY_BOOL   (validateBuildVersionNumber, false);
	KEY_INT    (gameServerConnectionPendingAllocatedSizeLimit, 0);
	KEY_INT    (systemTimeMismatchAlertIntervalSeconds, 1*60*60); // seconds
	KEY_INT    (ctsDenyLoginThresholdSeconds, 2*60*60); // seconds
	KEY_BOOL   (auctionEnabled, true);
	KEY_STRING (auctionServer, "localhost");
	KEY_INT    (auctionPort, 5901 );
	KEY_STRING (auctionIDPrefix, "SWG+");
	KEY_BOOL   (disconnectDuplicateConnectionsOnOtherGalaxies, false);
	KEY_BOOL   (requestDbSaveOnPlanetServerCrash, true);
	KEY_INT    (maxTimeToWaitForPlanetServerStartSeconds, 5*60); // seconds
	KEY_STRING (metricsDataURL, "");
	KEY_INT	   (webUpdateIntervalSeconds, 5);
	KEY_STRING (metricsSecretKey, "");

	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("CentralServer", "startPlanet", index++, 0);
		if (result != 0)
		{
			ms_startPlanets.push_back(result);
		}
	}
	while (result);

	index = 0;
	result = 0;
	do
	{
		result = ConfigFile::getKeyString("CentralServer", "startConnectionServer", index++, 0);
		if (result != 0)
		{
			ms_startConnectionServer.push_back(result);
		}
	}
	while (result);
}

//-----------------------------------------------------------------------

void ConfigCentralServer::remove(void)
{
	delete data;
	data = 0;
	ConfigServerUtility::remove();
}

//-----------------------------------------------------------------------
