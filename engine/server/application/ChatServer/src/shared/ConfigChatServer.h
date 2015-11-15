// ConfigChatServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_ConfigChatServer_H
#define	_ConfigChatServer_H

//-----------------------------------------------------------------------

class ConfigChatServer
{
public:
	struct Data
	{
		const char *    backupGatewayServerIP;
		int             backupGatewayServerPort;
		const char *    clusterName;
		const char *    centralServerAddress;
		int             centralServerPort;
		const char *    gameCode;
		const char *    gatewayServerIP;
		int             gatewayServerPort;
		time_t          roomInactivityTimeout;
		time_t          roomUnpopulatedTimeout;
		const char *    gameServiceBindInterface;
		const char *    planetServiceBindInterface;
		int             loginFlowControlRate;
		int             maxRoomQueriesPerFrame;
		bool            loggingEnabled;
		const char *    registrarHost;
		int             registrarPort;
		int             intervalToSendHeadersToClientSeconds;
		int             maxHeadersToSendToClientPerInterval;
		int             chatStatisticsReportIntervalSeconds;
		int             chatSpamLimiterNumCharacters;
		bool            chatSpamLimiterEnabledForFreeTrial;
		int             chatSpamNotifyPlayerWhenLimitedIntervalSeconds;
		bool            voiceChatLoggingEnabled;
		int             voiceChatRoomListRefresh;
	};

	static const char *          getBackupGatewayServerIP    ();
	static const unsigned short  getBackupGatewayServerPort  ();
	static const char *          getCentralServerAddress     ();
	static const unsigned short  getCentralServerPort        ();
	static const char *	         getClusterName			     ();
	static const char *          getGameCode                 ();
	static const char *          getGatewayServerIP          ();
	static const unsigned short  getGatewayServerPort        ();
	static const time_t          getRoomInactivityTimeout    ();
	static const time_t          getRoomUnpopulatedTimeout   ();
	static const char *          getGameServiceBindInterface ();
	static const char *          getPlanetServiceBindInterface ();
	static int                   getLoginFlowControlRate     ();
	static int                   getMaxRoomQueriesPerFrame   ();
	static void			         install                     ();
	static void			         remove                      ();
	static bool isLoggingEnabled();
	static const char *          getRegistrarHost            ();
	static int                   getRegistrarPort            ();
	static int getNumberOfCreateRooms();
	static char const * getCreateRoom(int index);
	static int                   getIntervalToSendHeadersToClientSeconds();
	static int                   getMaxHeadersToSendToClientPerInterval();
	static int                   getChatStatisticsReportIntervalSeconds();
	static int                   getChatSpamLimiterNumCharacters();
	static bool                  getChatSpamLimiterEnabledForFreeTrial();
	static int                   getChatSpamNotifyPlayerWhenLimitedIntervalSeconds();
	static bool                  getVoiceChatLoggingEnabled();
	static int                   getVChatRoomListRefresh();
	static int                   getNumberOfVoiceChatGateways();
	static char const *          getVoiceChatGateway(int index);


private:
	static Data *	data;
};

//-----------------------------------------------------------------------

inline const char * ConfigChatServer::getBackupGatewayServerIP()
{
	return data->backupGatewayServerIP;
}

//-----------------------------------------------------------------------

inline const unsigned short ConfigChatServer::getBackupGatewayServerPort()
{
	return static_cast<unsigned short>(data->backupGatewayServerPort);
}

//-----------------------------------------------------------------------

inline const char * ConfigChatServer::getCentralServerAddress() 
{
	return data->centralServerAddress;
}

//-----------------------------------------------------------------------

inline const unsigned short ConfigChatServer::getCentralServerPort()
{
	return static_cast<unsigned short>(data->centralServerPort);
}

//-----------------------------------------------------------------------

inline const char * ConfigChatServer::getClusterName()
{
	return data->clusterName;
}

//-----------------------------------------------------------------------

inline const char * ConfigChatServer::getGameCode() 
{
	return data->gameCode;
}

//-----------------------------------------------------------------------

inline const char * ConfigChatServer::getGatewayServerIP()
{
	return data->gatewayServerIP;
}

//-----------------------------------------------------------------------

inline const unsigned short ConfigChatServer::getGatewayServerPort()
{
	return static_cast<unsigned short>(data->gatewayServerPort);
}
	
//-----------------------------------------------------------------------

inline const time_t ConfigChatServer::getRoomInactivityTimeout()
{
	return static_cast<time_t>(data->roomInactivityTimeout);
}
	
//-----------------------------------------------------------------------

inline const time_t ConfigChatServer::getRoomUnpopulatedTimeout()
{
	return static_cast<time_t>(data->roomUnpopulatedTimeout);
}
	
//-----------------------------------------------------------------------

inline const char * ConfigChatServer::getGameServiceBindInterface()
{
	return data->gameServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const char * ConfigChatServer::getPlanetServiceBindInterface()
{
	return data->planetServiceBindInterface;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getLoginFlowControlRate()
{
	return data->loginFlowControlRate;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getMaxRoomQueriesPerFrame()
{
	return data->maxRoomQueriesPerFrame;
}

//-----------------------------------------------------------------------

inline const char * ConfigChatServer::getRegistrarHost() 
{
	return data->registrarHost;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getRegistrarPort()
{
	return data->registrarPort;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getIntervalToSendHeadersToClientSeconds()
{
	return data->intervalToSendHeadersToClientSeconds;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getMaxHeadersToSendToClientPerInterval()
{
	return data->maxHeadersToSendToClientPerInterval;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getChatStatisticsReportIntervalSeconds()
{
	return data->chatStatisticsReportIntervalSeconds;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getChatSpamLimiterNumCharacters()
{
	return data->chatSpamLimiterNumCharacters;
}

//-----------------------------------------------------------------------

inline bool ConfigChatServer::getChatSpamLimiterEnabledForFreeTrial()
{
	return data->chatSpamLimiterEnabledForFreeTrial;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getChatSpamNotifyPlayerWhenLimitedIntervalSeconds()
{
	return data->chatSpamNotifyPlayerWhenLimitedIntervalSeconds;
}

//-----------------------------------------------------------------------

inline bool ConfigChatServer::getVoiceChatLoggingEnabled()
{
	return data->voiceChatLoggingEnabled;
}

//-----------------------------------------------------------------------

inline int ConfigChatServer::getVChatRoomListRefresh()
{
	return data->voiceChatRoomListRefresh;
}

#endif	// _ConfigChatServer_H
