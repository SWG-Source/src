// ======================================================================
//
// ConfigStationPlayersCollector.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

//-----------------------------------------------------------------------

#include "FirstStationPlayersCollector.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigStationPlayersCollector.h"

//-----------------------------------------------------------------------

ConfigStationPlayersCollector::Data *	ConfigStationPlayersCollector::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("StationPlayersCollector", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("StationPlayersCollector", #a, b))
#define KEY_FLOAT(a,b)   (data->a = ConfigFile::getKeyFloat("StationPlayersCollector", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("StationPlayersCollector", #a, b))

//-----------------------------------------------------------------------

void ConfigStationPlayersCollector::install(void)
{
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);

	data = new ConfigStationPlayersCollector::Data;

	KEY_INT     (centralServerServiceBindPort, 50010);
	KEY_STRING  (centralServerServiceBindInterface, "localhost");
	KEY_STRING  (dsn,"swodb");
	KEY_STRING  (databaseUID,"character_data");
	KEY_STRING  (databasePWD, "changeme");
	KEY_STRING  (databaseSchema, "character_data");
	KEY_INT     (secondsBetweenDBReconnect, 45);
	KEY_STRING  (databaseProtocol, "OCI");
	KEY_BOOL    (enableQueryProfile,false);
	KEY_BOOL    (verboseQueryMode,false);
	KEY_INT     (databaseThreads, 3);
	KEY_BOOL    (logWorkerThreads, false);
	KEY_FLOAT   (defaultDBQueueUpdateTimeLimit, 0.25f);
	KEY_INT     (disconnectSleepTime, 30000);
	KEY_BOOL    (showAllDebugInfo, false);
}

//-----------------------------------------------------------------------

void ConfigStationPlayersCollector::remove(void)
{
	delete data;
	data = 0;
}

//-----------------------------------------------------------------------

unsigned short ConfigStationPlayersCollector::getCentralServerServiceBindPort()
{
	return data->centralServerServiceBindPort;
}

//-----------------------------------------------------------------------

const char * ConfigStationPlayersCollector::getCentralServerServiceBindInterface()
{
	return data->centralServerServiceBindInterface;
}

//-----------------------------------------------------------------------

const char * ConfigStationPlayersCollector::getDSN()
{
	return data->dsn;
}

//-----------------------------------------------------------------------

const char * ConfigStationPlayersCollector::getDatabaseUID()
{
	return data->databaseUID;
}

//-----------------------------------------------------------------------


const char * ConfigStationPlayersCollector::getDatabasePWD()
{
	return data->databasePWD;
}

//-----------------------------------------------------------------------

const char * ConfigStationPlayersCollector::getDatabaseSchema()
{
	return data->databaseSchema;
}

//-----------------------------------------------------------------------

unsigned short ConfigStationPlayersCollector::getSecondsBetweenDBReconnect()
{
	return data->secondsBetweenDBReconnect;
}

//-----------------------------------------------------------------------


const char * ConfigStationPlayersCollector::getDatabaseProtocol()
{
	return (data->databaseProtocol);
}

//-----------------------------------------------------------------------


const bool ConfigStationPlayersCollector::getEnableQueryProfile()
{
	return (data->enableQueryProfile);
}

// ----------------------------------------------------------------------

const bool ConfigStationPlayersCollector::getVerboseQueryMode()
{
	return (data->verboseQueryMode);
}

// ----------------------------------------------------------------------

int ConfigStationPlayersCollector::getDatabaseThreads()
{
	return (data->databaseThreads);
}

// ----------------------------------------------------------------------

const bool ConfigStationPlayersCollector::getLogWorkerThreads()
{
	return data->logWorkerThreads;
}

// ----------------------------------------------------------------------

float ConfigStationPlayersCollector::getDefaultDBQueueUpdateTimeLimit()
{
	return data->defaultDBQueueUpdateTimeLimit;
}

// ----------------------------------------------------------------------

const int ConfigStationPlayersCollector::getDisconnectSleepTime()
{
	return data->disconnectSleepTime;
}

// ----------------------------------------------------------------------

const bool ConfigStationPlayersCollector::getShowAllDebugInfo()
{
	return data->showAllDebugInfo;
}

// ----------------------------------------------------------------------
