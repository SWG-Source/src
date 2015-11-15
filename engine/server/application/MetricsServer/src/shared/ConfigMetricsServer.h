// ConfigMetricsServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_ConfigMetricsServer_H
#define	_ConfigMetricsServer_H

//-----------------------------------------------------------------------

class ConfigMetricsServer
{
public:
	struct Data
	{
		const char * authenticationFileName;
		const char * clusterName;
		uint16       metricsListenerPort;
		uint16       metricsServicePort;
		bool         runTestStats;
		uint16       taskManagerPort;
		const char * metricsServiceBindInterface;
	};
	
	static void			         install                     ();
	static void			         remove                      ();


	static const char* getAuthenticationFileName();
	static const char* getClusterName();
	static uint16      getMetricsListenerPort();
	static uint16      getMetricsServicePort();
	static bool        getRunTestStats();
	static uint16      getTaskManagerPort();
	static const char* getMetricsServiceBindInterface();
		
private:
	static Data *	data;
};

//-----------------------------------------------------------------------

inline const char* ConfigMetricsServer::getAuthenticationFileName()
{
	return data->authenticationFileName;
}
//-----------------------------------------------------------------------

inline const char* ConfigMetricsServer::getClusterName()
{
	return data->clusterName;
}
//-----------------------------------------------------------------------

inline uint16 ConfigMetricsServer::getMetricsListenerPort()
{
	return data->metricsListenerPort;
}
//-----------------------------------------------------------------------

inline uint16 ConfigMetricsServer::getMetricsServicePort()
{
	return data->metricsServicePort;
}
//-----------------------------------------------------------------------

inline bool ConfigMetricsServer::getRunTestStats()
{
	return data->runTestStats;
}
//-----------------------------------------------------------------------

inline uint16 ConfigMetricsServer::getTaskManagerPort()
{
	return data->taskManagerPort;
}

//-----------------------------------------------------------------------

inline const char * ConfigMetricsServer::getMetricsServiceBindInterface()
{
	return data->metricsServiceBindInterface;
}

//-----------------------------------------------------------------------


#endif	// _ConfigMetricsServer_H


