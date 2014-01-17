// ConfigMetricsServer.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstMetricsServer.h"
#include "sharedFoundation/ConfigFile.h"
#include "ConfigMetricsServer.h"

//-----------------------------------------------------------------------

ConfigMetricsServer::Data *	ConfigMetricsServer::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("MetricsServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("MetricsServer", #a, b)) 
#define KEY_REAL(a,b)   (data->a = ConfigFile::getKeyReal("MetricsServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("MetricsServer", #a, b))

//-----------------------------------------------------------------------

void ConfigMetricsServer::install(void)
{
	data = new ConfigMetricsServer::Data;

	KEY_STRING(authenticationFileName, "../../exe/shared/metricsAuthentication.cfg");
	KEY_STRING(clusterName, "DevCluster");
	KEY_INT(metricsListenerPort, 2200);
	KEY_INT(metricsServicePort, 44480);
	KEY_BOOL(runTestStats, false);
	KEY_INT(taskManagerPort, 60001);
	KEY_STRING(metricsServiceBindInterface, "");
}

//-----------------------------------------------------------------------

void ConfigMetricsServer::remove(void)
{
	delete data;
	data = 0;
}

//-----------------------------------------------------------------------

