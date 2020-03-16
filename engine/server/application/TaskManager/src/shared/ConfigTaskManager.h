// ConfigTaskManager.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_ConfigTaskManager_H
#define	_ConfigTaskManager_H

//-----------------------------------------------------------------------

class ConfigTaskManager
{
  public:
    struct Data
    {
		bool          autoStart;
		const char *  clusterName;
		bool          verifyClusterName;
		int           consoleConnectionPort;
		const char *  consoleServiceBindInterface;
		unsigned long gameServerTimeout;
		const char *  gameServiceBindInterface;
		int           gameServicePort;
		const char *  loginServerAddress;
		int           loginServerTaskServicePort;
		float         maximumLoad;
		float         loadConnectionServer;
		float         loadPlanetServer;
		float         loadGameServer;
		bool          publishMode;
		const char *  rcFileName;
		bool          restartCentral; 
		int           restartDelayCentralServer;
		int           restartDelayLogServer;
		int           restartDelayMetricsServer;
		int           restartDelayCommoditiesServer;
		int           restartDelayTransferServer;
		int           taskManagerServicePort;
		const char *  taskManagerServiceBindInterface;
		int           maximumClockDriftToleranceSeconds;
		int           systemTimeCheckIntervalSeconds;
		int           clockDriftFatalIntervalSeconds;
		bool          allowPreferredServerOnMasterNode; 
    };

	static const bool    getAutoStart                   ();
	static const char *  getClusterName                 ();
	static const bool    getVerifyClusterName           ();
	static const uint16  getConsoleConnectionPort       ();
	static const char *  getConsoleServiceBindInterface  ();
	static const char *  getLoginServerAddress          ();
	static const uint16  getLoginServerTaskServicePort  ();
	static const bool    getPublishMode();
	static const char *  getRcFileName                  ();
	static bool          getRestartCentral();
	static unsigned int  getRestartDelayCentralServer   ();
	static unsigned int  getRestartDelayLogServer       ();
	static unsigned int  getRestartDelayMetricsServer   ();
	static unsigned int  getRestartDelayCommoditiesServer ();
	static unsigned int  getRestartDelayTransferServer  ();
	static unsigned long getGameServerTimeout           ();
	static const char *  getGameServiceBindInterface    ();
	static uint16        getGameServicePort();
	static float         getMaximumLoad                 ();
	static float         getLoadConnectionServer        ();
	static float         getLoadPlanetServer            ();
	static float         getLoadGameServer              ();
	static uint16        getTaskManagerServicePort();
	static const char *  getTaskManagerServiceBindInterface();
	static int           getMaximumClockDriftToleranceSeconds();
	static int           getSystemTimeCheckIntervalSeconds();
	static int           getClockDriftFatalIntervalSeconds();
	static const bool    getAllowPreferredServerOnMasterNode();
	static void          install                        ();
	static void          remove                         ();

	static int getNumberOfEnvironmentVariables();
	static char const * getEnvironmentVariable(int index);

private:
    static Data *	data;
};

//-----------------------------------------------------------------------

inline const bool ConfigTaskManager::getAutoStart()
{
	return data->autoStart;
}

//-----------------------------------------------------------------------

inline const char * ConfigTaskManager::getClusterName()
{
	return data->clusterName;
}

//-----------------------------------------------------------------------

inline const bool ConfigTaskManager::getVerifyClusterName()
{
	return data->verifyClusterName;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigTaskManager::getConsoleConnectionPort()
{
	return static_cast<uint16>(data->consoleConnectionPort);
}

//-----------------------------------------------------------------------

inline uint16 ConfigTaskManager::getGameServicePort()
{
	return static_cast<uint16>(data->gameServicePort);
}

//-----------------------------------------------------------------------

inline uint16 ConfigTaskManager::getTaskManagerServicePort()
{
	return static_cast<uint16>(data->taskManagerServicePort);
}

//-----------------------------------------------------------------------

inline const char * ConfigTaskManager::getLoginServerAddress()
{
	return data->loginServerAddress;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigTaskManager::getLoginServerTaskServicePort()
{
	return static_cast<uint16>(data->loginServerTaskServicePort);
}

//-----------------------------------------------------------------------

inline const char * ConfigTaskManager::getRcFileName()
{
	return data->rcFileName;
}

//-----------------------------------------------------------------------

inline const char * ConfigTaskManager::getConsoleServiceBindInterface()
{
	return data->consoleServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const char * ConfigTaskManager::getGameServiceBindInterface()
{
	return data->gameServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const char * ConfigTaskManager::getTaskManagerServiceBindInterface()
{
	return data->taskManagerServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const bool ConfigTaskManager::getPublishMode()
{
	return data->publishMode;
}

//-----------------------------------------------------------------------

inline float ConfigTaskManager::getMaximumLoad()
{
	return data->maximumLoad;
}

//-----------------------------------------------------------------------

inline float ConfigTaskManager::getLoadConnectionServer()
{
	return data->loadConnectionServer;
}

//-----------------------------------------------------------------------

inline float ConfigTaskManager::getLoadPlanetServer()
{
	return data->loadPlanetServer;
}

//-----------------------------------------------------------------------

inline float ConfigTaskManager::getLoadGameServer()
{
	return data->loadGameServer;
}

// ----------------------------------------------------------------------

inline unsigned long ConfigTaskManager::getGameServerTimeout()
{
	return data->gameServerTimeout;
}

// ----------------------------------------------------------------------

inline bool ConfigTaskManager::getRestartCentral()
{
	return data->restartCentral;
}

// ----------------------------------------------------------------------

inline unsigned int ConfigTaskManager::getRestartDelayCentralServer()
{
	return static_cast<unsigned int>(data->restartDelayCentralServer);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigTaskManager::getRestartDelayLogServer()
{
	return static_cast<unsigned int>(data->restartDelayLogServer);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigTaskManager::getRestartDelayMetricsServer()
{
	return static_cast<unsigned int>(data->restartDelayMetricsServer);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigTaskManager::getRestartDelayCommoditiesServer()
{
	return static_cast<unsigned int>(data->restartDelayCommoditiesServer);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigTaskManager::getRestartDelayTransferServer()
{
	return static_cast<unsigned int>(data->restartDelayTransferServer);
}

// ----------------------------------------------------------------------

inline int ConfigTaskManager::getMaximumClockDriftToleranceSeconds()
{
	return data->maximumClockDriftToleranceSeconds;
}

// ----------------------------------------------------------------------

inline int ConfigTaskManager::getSystemTimeCheckIntervalSeconds()
{
	return data->systemTimeCheckIntervalSeconds;
}

// ----------------------------------------------------------------------

inline int ConfigTaskManager::getClockDriftFatalIntervalSeconds()
{
	return data->clockDriftFatalIntervalSeconds;
}

// ----------------------------------------------------------------------

inline const bool ConfigTaskManager::getAllowPreferredServerOnMasterNode()
{
	return data->allowPreferredServerOnMasterNode;
}

// ----------------------------------------------------------------------

#endif	// _ConfigTaskManager_H
