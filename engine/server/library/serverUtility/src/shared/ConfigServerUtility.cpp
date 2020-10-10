// ConfigServerUtility.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/ConfigFile.h"

//-----------------------------------------------------------------------

namespace ConfigServerUtilityNamespace
{
	int spawnCookie;
	int chatLogMinutes;
	int serverMaxChatLogLines;
	int playerMaxChatLogLines;
	bool chatLogManagerLoggingEnabled;
	bool externalAdminLevelsEnabled;
	const char * externalAdminLevelsURL;
	const char * externalAdminLevelsSecretKey;
}

using namespace ConfigServerUtilityNamespace;

#define KEY_INT(a,b)    (a = ConfigFile::getKeyInt("ServerUtility", #a, b))
#define KEY_BOOL(a,b)   (a = ConfigFile::getKeyBool("ServerUtility", #a, b))
#define KEY_FLOAT(a,b)  (a = ConfigFile::getKeyFloat("ServerUtility", #a, b))
#define KEY_STRING(a,b) (a = ConfigFile::getKeyString("ServerUtility", #a, b))

//-----------------------------------------------------------------------

ConfigServerUtility::ConfigServerUtility()
{
}

//-----------------------------------------------------------------------

ConfigServerUtility::~ConfigServerUtility()
{
}

//-----------------------------------------------------------------------

int ConfigServerUtility::getSpawnCookie()
{
	return spawnCookie;
}

//-----------------------------------------------------------------------

int ConfigServerUtility::getChatLogMinutes()
{
	return chatLogMinutes;
}

//-----------------------------------------------------------------------

int ConfigServerUtility::getServerMaxChatLogLines()
{
	return serverMaxChatLogLines;
}

//-----------------------------------------------------------------------

int ConfigServerUtility::getPlayerMaxChatLogLines()
{
	return playerMaxChatLogLines;
}

//-----------------------------------------------------------------------

bool ConfigServerUtility::isChatLogManagerLoggingEnabled()
{
	return chatLogManagerLoggingEnabled;
}

//-----------------------------------------------------------------------

bool ConfigServerUtility::isExternalAdminLevelsEnabled()
{
	return externalAdminLevelsEnabled;
}

//-----------------------------------------------------------------------

const char * ConfigServerUtility::getExternalAdminLevelsURL()
{
	return externalAdminLevelsURL;
}

//-----------------------------------------------------------------------

const char * ConfigServerUtility::getExternalAdminLevelsSecretKey()
{
	return externalAdminLevelsSecretKey;
}

//-----------------------------------------------------------------------

void ConfigServerUtility::install()
{
	KEY_INT(spawnCookie, 0);
	KEY_INT(chatLogMinutes, 10);
	KEY_INT(serverMaxChatLogLines, 5000);
	KEY_INT(playerMaxChatLogLines, 200);
	KEY_BOOL(chatLogManagerLoggingEnabled, false);
	KEY_BOOL(externalAdminLevelsEnabled, false);
	KEY_STRING(externalAdminLevelsURL, "http://localhost/");
	KEY_STRING(externalAdminLevelsSecretKey, "");
}

//-----------------------------------------------------------------------

void ConfigServerUtility::remove()
{
}

//-----------------------------------------------------------------------
