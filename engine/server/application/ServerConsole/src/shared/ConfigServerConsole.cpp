// ConfigServerConsole.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstServerConsole.h"
#include "sharedFoundation/ConfigFile.h"
#include "ConfigServerConsole.h"


//-----------------------------------------------------------------------

namespace ConfigServerConsoleNamespace
{
	const char *          serverAddress;
	int                   serverPort;
}

using namespace ConfigServerConsoleNamespace;

#define KEY_INT(a,b)    (a = ConfigFile::getKeyInt("ServerConsole", #a, b))
#define KEY_BOOL(a,b)   (a = ConfigFile::getKeyBool("ServerConsole", #a, b))
// #define KEY_REAL(a,b)   (data->a = ConfigFile::getKeyReal("LoginServer", #a, b))
#define KEY_STRING(a,b) (a = ConfigFile::getKeyString("ServerConsole", #a, b))

//-----------------------------------------------------------------------

ConfigServerConsole::ConfigServerConsole()
{
}

//-----------------------------------------------------------------------

ConfigServerConsole::~ConfigServerConsole()
{
}

//-----------------------------------------------------------------------

const char * const ConfigServerConsole::getServerAddress()
{
	return serverAddress;
}

//-----------------------------------------------------------------------

unsigned short ConfigServerConsole::getServerPort()
{
	return static_cast<unsigned short>(serverPort);
}

//-----------------------------------------------------------------------

void ConfigServerConsole::install()
{
	KEY_STRING(serverAddress, "127.0.0.1");
	KEY_INT(serverPort, 61000);
}

//-----------------------------------------------------------------------

void ConfigServerConsole::remove()
{
}

//-----------------------------------------------------------------------

