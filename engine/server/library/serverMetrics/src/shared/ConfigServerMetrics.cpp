// ConfigServerMetrics.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "serverMetrics/FirstServerMetrics.h"
#include "sharedFoundation/ConfigFile.h"
#include "ConfigServerMetrics.h"

//-----------------------------------------------------------------------

ConfigServerMetrics::Data *	ConfigServerMetrics::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("ServerMetrics", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("ServerMetrics", #a, b)) 
#define KEY_REAL(a,b)   (data->a = ConfigFile::getKeyReal("ServerMetrics", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("ServerMetrics", #a, b))

//-----------------------------------------------------------------------

void ConfigServerMetrics::install(void)
{
	data = new ConfigServerMetrics::Data;

	KEY_STRING(metricsServerAddress, "localhost");

	data->secondsBetweenUpdates = static_cast<uint16>(ConfigFile::getKeyInt("ServerMetrics", "secondsBetweenUpdates", 5));
	data->metricsServerPort     = static_cast<uint16>(ConfigFile::getKeyInt("ServerMetrics", "metricsServerPort",     44480));

	KEY_STRING(primaryName, "unknownProcess");
	KEY_STRING(secondaryName, "");
	KEY_INT(frameTimeAveragingSize, 11);
	
}

//-----------------------------------------------------------------------

void ConfigServerMetrics::remove(void)
{
	delete data;
	data = 0;
}

//-----------------------------------------------------------------------

