// ======================================================================
//
// ConfigLogServer.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstLogServer.h"
#include "ConfigLogServer.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"

// ======================================================================

ConfigLogServer::Data *ConfigLogServer::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("LogServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("LogServer", #a, b))

// ======================================================================

void ConfigLogServer::install()
{
	ConfigServerUtility::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	data = new ConfigLogServer::Data;

	KEY_INT (logServicePort,  44467);
	KEY_STRING (logServiceBindInterface, "");
	KEY_STRING (loggingServerApiAddress, "localhost");
	KEY_STRING (loggingServerApiLoginName, "MISCONFIGURED_LOG_SERVER_LOGIN_NAME");
	KEY_STRING (loggingServerApiPassword, "pacman");
	KEY_STRING (loggingServerApiDefaultDirectory, "swg");
	KEY_STRING (clusterName, "MISCONFIGURED_LOG_SERVER");
	KEY_INT    (logServiceMaxConnections, 150);
	KEY_INT    (loggingServerApiQueueSize, 4096);
}

// ----------------------------------------------------------------------

void ConfigLogServer::remove()
{
	delete data;
	data = 0;
	ConfigServerUtility::remove();
}

// ======================================================================

