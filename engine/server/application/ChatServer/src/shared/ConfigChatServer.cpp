// ConfigChatServer.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstChatServer.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigChatServer.h"

//-----------------------------------------------------------------------

ConfigChatServer::Data *	ConfigChatServer::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("ChatServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("ChatServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("ChatServer", #a, b)) 

//Commented out so lint doesn't whine that we're not using them
//#define KEY_REAL(a,b)   (data->a = ConfigFile::getKeyReal("ChatServer", #a, b))

// ======================================================================

namespace ConfigChatServerNamespace
{
	typedef std::vector<char const *> StringPtrArray;
	StringPtrArray ms_createRoom; // ConfigFile owns the pointer
	StringPtrArray ms_voiceGateways;
}

using namespace ConfigChatServerNamespace;

// ======================================================================

int ConfigChatServer::getNumberOfCreateRooms()
{
	return static_cast<int>(ms_createRoom.size());
}

// ----------------------------------------------------------------------

char const * ConfigChatServer::getCreateRoom(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCreateRooms());
	return ms_createRoom[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

int ConfigChatServer::getNumberOfVoiceChatGateways()
{
	return static_cast<int>(ms_voiceGateways.size());
}

// ----------------------------------------------------------------------

char const * ConfigChatServer::getVoiceChatGateway(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfVoiceChatGateways());
	return ms_voiceGateways[static_cast<size_t>(index)];
}

void ConfigChatServer::install(void)
{
	ConfigServerUtility::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	data = new ConfigChatServer::Data;

	KEY_STRING (backupGatewayServerIP, "localhost");
	KEY_INT    (backupGatewayServerPort, 15150);
	KEY_STRING (clusterName, "devcluster");
	KEY_STRING (centralServerAddress, "localhost");
	KEY_INT    (centralServerPort, 61232);
	KEY_STRING (gameCode, "SWG");
	KEY_STRING (gatewayServerIP, "localhost");
	KEY_INT    (gatewayServerPort, 5001);
	KEY_INT    (roomInactivityTimeout, 60 * 60 * 24 * 3);
	KEY_INT    (roomUnpopulatedTimeout, 60 * 5);
	KEY_STRING (gameServiceBindInterface, "");
	KEY_STRING (planetServiceBindInterface, "");
	KEY_INT    (loginFlowControlRate, 50);
	KEY_INT    (maxRoomQueriesPerFrame, 5);
	KEY_BOOL   (loggingEnabled, false);
	KEY_STRING (registrarHost, "localhost");
	KEY_INT    (registrarPort, 5000);
	KEY_INT    (intervalToSendHeadersToClientSeconds, 1);
	KEY_INT    (maxHeadersToSendToClientPerInterval, 100);
	KEY_INT    (chatStatisticsReportIntervalSeconds, 60);
	KEY_INT    (chatSpamLimiterNumCharacters, 400);
	KEY_BOOL   (chatSpamLimiterEnabledForFreeTrial, true);
	KEY_INT    (chatSpamNotifyPlayerWhenLimitedIntervalSeconds, 30); // <= 0 to disable
	KEY_BOOL   (voiceChatLoggingEnabled, false);
	KEY_INT    (voiceChatRoomListRefresh, 600000);

	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("ChatServer", "createRoom", index++, 0);
		if (result != 0)
		{
			ms_createRoom.push_back(result);
		}
	}
	while (result);

	index = 0;
	result = 0;
	do 
	{
		result = ConfigFile::getKeyString("ChatServer", "voiceChatGateway", index++, 0);
		if(result != 0)
		{
			ms_voiceGateways.push_back(result);
		}
	} while(result);

	if(ms_voiceGateways.empty())
	{
		static char const * const defaultGateway = "localhost:9102";
		ms_voiceGateways.push_back(defaultGateway);
	}
}

//-----------------------------------------------------------------------

void ConfigChatServer::remove(void)
{
	delete data;
	data = 0;
	ConfigServerUtility::remove();
}

//-----------------------------------------------------------------------

bool ConfigChatServer::isLoggingEnabled()
{
	return data->loggingEnabled;
}

//-----------------------------------------------------------------------
