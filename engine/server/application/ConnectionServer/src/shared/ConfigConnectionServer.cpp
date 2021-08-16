// ConfigConnectionServer.cpp
// copyright 2001 Verant Interactive


//-----------------------------------------------------------------------

#include "FirstConnectionServer.h"
#include "ConfigConnectionServer.h"
#include "SessionApiClient.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedNetwork/SetupSharedNetwork.h"

//-----------------------------------------------------------------------

ConfigConnectionServer::Data *	ConfigConnectionServer::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("ConnectionServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("ConnectionServer", #a, b))
#define KEY_FLOAT(a,b)   (data->a = ConfigFile::getKeyFloat("ConnectionServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("ConnectionServer", #a, b))

//-----------------------------------------------------------------------

namespace ConfigConnectionServerNamespace
{
	typedef std::vector<char const *> StringPtrArray;
	StringPtrArray ms_sessionServer; // ConfigFile owns the pointer
}

using namespace ConfigConnectionServerNamespace;

// ======================================================================

int ConfigConnectionServer::getNumberOfSessionServers()
{
	return static_cast<int>(ms_sessionServer.size());
}

// ----------------------------------------------------------------------

char const * ConfigConnectionServer::getSessionServer(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfSessionServers());
	return ms_sessionServer[static_cast<size_t>(index)];
}

void ConfigConnectionServer::install(void)
{
	ConfigServerUtility::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	data = new ConfigConnectionServer::Data;

	KEY_STRING  (sessionURL, "");
	KEY_STRING  (centralServerAddress, "localhost");
	KEY_INT     (centralServerPort, 0);
	KEY_STRING  (clientServiceBindInterface, "");
	KEY_INT     (clientServicePortPrivate, 44464);
	KEY_INT     (clientServicePortPublic, 44463);
	KEY_INT     (clientOverflowLimit, 1024 * 1024); // 1MB overflow
	KEY_INT     (gameServicePort, 0);
	KEY_INT     (pingPort, 0);
	KEY_STRING  (clusterName, "devcluster");
	KEY_INT     (maxClients, 200);
	KEY_BOOL    (spamLimitEnabled, false);
	KEY_INT     (spamLimitResetTimeMs, 30*1000);
	KEY_INT     (spamLimitResetScaleFactor, 8);
	KEY_INT     (spamLimitBytesPerSec, 32000);
	KEY_INT     (spamLimitPacketsPerSec, 50);
	KEY_BOOL    (startPublicServer, true);
	KEY_BOOL    (disableWorldSnapshot, true);
	KEY_STRING  (gameServiceBindInterface, "");
	KEY_STRING  (chatServiceBindInterface, "");
	KEY_STRING  (customerServiceBindInterface, "");
	KEY_BOOL    (compressClientNetworkTraffic, true);
	KEY_INT     (crashRecoveryTimeout, 15*1000); // timeout for players to recover from a server crash
	KEY_BOOL    (shouldSleep, true);
	KEY_INT     (clientMaxOutstandingPackets, 1000);
	KEY_INT     (clientMaxRawPacketSize, 500);
	KEY_INT     (clientMaxConnections, 200);
	KEY_INT     (clientFragmentSize, 500);
	KEY_INT     (clientMaxDataHoldTime, 20);
	KEY_INT     (clientHashTableSize, 200);
	KEY_INT     (lagReportThreshold, 10000);
	KEY_INT     (defaultGameFeatures, 0xFFFFFFFF);
	KEY_INT     (defaultSubscriptionFeatures, 0xFFFFFFFF);

	KEY_BOOL     (validateStationKey, false);
	KEY_STRING   (sessionServers, "");
	KEY_INT      (sessionType, SESSION_TYPE_STARWARS);
	KEY_BOOL     (disableSessionLogout, false);
	KEY_BOOL     (sessionRecordPlayTime, true);
	KEY_BOOL     (disconnectOnInactive, false);
	KEY_BOOL     (disconnectFreeTrialOnInactive, false);

	KEY_STRING  (adminAccountDataTable, "datatables/admin/us_admin.iff");

	KEY_INT     (requiredSubscriptionBits, 0);
	KEY_INT     (requiredGameBits, 0);
	KEY_BOOL    (setJtlRetailIfBetaIsSet, false);

	KEY_BOOL    (validateClientVersion, true);
	KEY_BOOL    (setEpisode3RetailIfBetaIsSet, false);
	KEY_BOOL    (setTrialsOfObiwanRetailIfBetaIsSet, false);
	KEY_INT     (disabledFeatureBits, 0);

	KEY_FLOAT   (timeBetweenSessionUpdates, 60.0f * 5.0f);

	KEY_INT     (connectionServerNumber, 0);
	KEY_INT     (fakeBuddyPoints, 0);

	KEY_STRING  (altPublicBindAddress, "");
	KEY_BOOL    (useOldSuidGenerator, false);

	KEY_BOOL (useSecureLoginForGodAccess, false);

	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("ConnectionServer", "sessionServer", index++, 0);
		if (result != 0)
		{
			ms_sessionServer.push_back(result);
		}
	}
	while (result);
}

//-----------------------------------------------------------------------

void ConfigConnectionServer::remove(void)
{
	delete data;
	data = 0;
	ConfigServerUtility::remove();
}

//-----------------------------------------------------------------------

int ConfigConnectionServer::getDisabledFeatureBits()
{
	return data->disabledFeatureBits;
}

// ----------------------------------------------------------------------

bool ConfigConnectionServer::getDisconnectOnInactive()
{
	return data->disconnectOnInactive;
}

// ----------------------------------------------------------------------

bool ConfigConnectionServer::getDisconnectFreeTrialOnInactive()
{
	return data->disconnectFreeTrialOnInactive;
}

// ----------------------------------------------------------------------

int ConfigConnectionServer::getFakeBuddyPoints()
{
	return data->fakeBuddyPoints;
}

// ----------------------------------------------------------------------

const char * ConfigConnectionServer::getPublicBindAddress()
{
	return data->altPublicBindAddress;
}

// ======================================================================
