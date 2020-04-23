// ConfigLoginServer.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstLoginServer.h"
#include "serverUtility/ConfigServerUtility.h"
#include "Session/LoginAPI/Client.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigLoginServer.h"

//-----------------------------------------------------------------------

ConfigLoginServer::Data *	ConfigLoginServer::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("LoginServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("LoginServer", #a, b))
// #define KEY_REAL(a,b)   (data->a = ConfigFile::getKeyReal("LoginServer", #a, b))
#define KEY_FLOAT(a,b)  (data->a = ConfigFile::getKeyFloat("LoginServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("LoginServer", #a, b))

//-----------------------------------------------------------------------

namespace ConfigLoginServerNamespace
{
	typedef std::vector<char const *> StringPtrArray;
	StringPtrArray ms_privateIpMasks; // ConfigFile owns the pointer
	StringPtrArray ms_sessionServer; // ConfigFile owns the pointer

	int const ms_numPurgePhases=5;
	int ms_purgePhaseAdvanceDays[ms_numPurgePhases]={0,30,60,90,120};

	// disable character creation for these clusters through config option
	std::set<std::string> ms_clusterCharacterCreationDisable;
}

using namespace ConfigLoginServerNamespace;

// ======================================================================

int ConfigLoginServer::getNumberOfSessionServers()
{
	return static_cast<int>(ms_sessionServer.size());
}

// ----------------------------------------------------------------------

char const * ConfigLoginServer::getSessionServer(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfSessionServers());
	return ms_sessionServer[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

void ConfigLoginServer::install(void)
{
	ConfigServerUtility::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	data = new ConfigLoginServer::Data;

	KEY_INT	(centralServicePort, 44452);
	KEY_INT	(clientServicePort, 44453);
	KEY_INT (clientOverflowLimit, 1024 * 8);
	KEY_INT	(maxClients, 250);
	KEY_INT (taskServicePort, 44459);
	KEY_INT (pingServicePort, 44460);
	KEY_INT (httpServicePort, 44490);
	KEY_STRING (DSN, "loginserver");
	KEY_STRING (privateIpMask, "127.0.0.1");
	KEY_STRING (databaseUID, "loginserver");
	KEY_STRING (schemaOwner, "");
	KEY_STRING (databasePWD, "loginserver");
	KEY_STRING (databaseProtocol, "OCI");
	KEY_BOOL (enableQueryProfile,false);
	KEY_BOOL (verboseQueryMode,false);
	KEY_BOOL(logWorkerThreads, false);
	KEY_INT (maxPlayersPerCluster, 2500);
	KEY_INT (maxCharactersPerCluster, 10000);
	KEY_INT (maxCharactersPerAccount, 20);
	KEY_BOOL (validateClientVersion, true);
	KEY_BOOL (validateStationKey, false);
	KEY_BOOL (easyExternalAccess, false);
	KEY_BOOL (doSessionLogin, false);
	KEY_BOOL (doConsumption, false);
	KEY_STRING (sessionServers, "localhost:3004");
	KEY_INT (sessionType, SESSION_TYPE_STARWARS);
	KEY_BOOL (developmentMode, true);
	KEY_INT (databaseThreads, 1);
	KEY_BOOL (compressClientNetworkTraffic, true);
	KEY_INT (metricsListenerPort, 0); // defaults to 2201 but we don't use it
	KEY_FLOAT (defaultDBQueueUpdateTimeLimit, 0.25f);
	KEY_INT (disconnectSleepTime, 30000);
	KEY_INT (clusterGroup,1);
	KEY_INT(maxSimultaneousPurgeAccounts,1000);
	KEY_INT(purgeSleepTime,600);
	KEY_BOOL(enableStructurePurge,true);
	KEY_BOOL(enableCharacterPurge,true);
	KEY_INT (updatePurgeAccountListTime, 0);
	KEY_STRING (purgeAccountSourceTable,"account_extract");
	KEY_STRING (adminAccountDataTable, "datatables/admin/us_admin.iff");
	KEY_BOOL(allowSkipTutorialToAll,true);
	KEY_BOOL(internalBypassOnlineLimit,true);
	KEY_INT (populationExtremelyHeavyThresholdPercent, 50);
	KEY_INT (populationVeryHeavyThresholdPercent, 40);
	KEY_INT (populationHeavyThresholdPercent, 32);
	KEY_INT (populationMediumThresholdPercent, 16);
	KEY_INT (populationLightThresholdPercent, 8);
	KEY_INT (csToolPort, 0); // use 10666 if you want to turn the tool on
	KEY_BOOL(requireSecureLoginForCsTool, true);
	KEY_BOOL(useJsonWebApi, false);
	KEY_BOOL(useExternalAuth, false);
	KEY_STRING(externalAuthURL, "");
	KEY_STRING(externalAuthSecretKey, "");
	KEY_BOOL(useOldSuidGenerator, false);

	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("LoginServer", "privateIpMask", index++, 0);
		if (result != 0)
		{
			ms_privateIpMasks.push_back(result);
		}
	}
	while (result);

	index = 0;
	result = 0;
	do
	{
		result = ConfigFile::getKeyString("LoginServer", "sessionServer", index++, 0);
		if (result != 0)
		{
			ms_sessionServer.push_back(result);
		}
	}
	while (result);

	index = 0;
	result = 0;
	do
	{
		result = ConfigFile::getKeyString("LoginServer", "disableCharacterCreation", index++, 0);
		if (result != 0)
		{
			ms_clusterCharacterCreationDisable.insert(result);
		}
	}
	while (result);

	if (!ms_clusterCharacterCreationDisable.empty())
	{
		for (std::set<std::string>::const_iterator iter = ms_clusterCharacterCreationDisable.begin(); iter != ms_clusterCharacterCreationDisable.end(); ++iter)
		{
			DEBUG_REPORT_LOG(true, ("Character creation for cluster (%s) has been disabled through config option.\n", iter->c_str()));
		}
	}

	{
		char keyName[500];
		for (int index=0; index < ms_numPurgePhases; index++)
		{
			snprintf(keyName,sizeof(keyName),"purgePhaseAdvanceDays%i",index);
			ms_purgePhaseAdvanceDays[index] = ConfigFile::getKeyInt("LoginServer", keyName, 0, ms_purgePhaseAdvanceDays[index]);
		}
	}
}


//-----------------------------------------------------------------------

void ConfigLoginServer::remove(void)
{
	delete data;
	data = 0;
	ConfigServerUtility::remove();
}

// ----------------------------------------------------------------------

int ConfigLoginServer::getPurgePhaseAdvanceDays(int purgePhase)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, purgePhase, ms_numPurgePhases);
	return ms_purgePhaseAdvanceDays[purgePhase];
}

// ----------------------------------------------------------------------

bool ConfigLoginServer::isCharacterCreationDisabled(std::string const & cluster)
{
	return (ms_clusterCharacterCreationDisable.count(cluster) >= 1);
}

// ----------------------------------------------------------------------

std::set<std::string> const & ConfigLoginServer::getCharacterCreationDisabledClusterList()
{
	return ms_clusterCharacterCreationDisable;
}

// ======================================================================
