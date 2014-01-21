// ConfigLoginPing.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "FirstLoginPing.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigLoginPing.h"

//-----------------------------------------------------------------------

ConfigLoginPing::Data *	ConfigLoginPing::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("LoginPing", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("LoginPing", #a, b))
#define KEY_REAL(a,b)   (data->a = ConfigFile::getKeyReal("LoginPing", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("LoginPing", #a, b))

//-----------------------------------------------------------------------

void ConfigLoginPing::install(void)
{
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	data = new ConfigLoginPing::Data;
	KEY_BOOL(passthroughMode, false);
	KEY_STRING(loginServerAddress, "localhost");
	KEY_INT(loginServerPingServicePort, 44460);
	KEY_STRING(rcFileName, "loginPing.cfg");
}

//-----------------------------------------------------------------------

void ConfigLoginPing::remove(void)
{
	delete data;
	data = 0;
}

//-----------------------------------------------------------------------
