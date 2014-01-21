// ======================================================================
//
// ConfigServerBase.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

//-----------------------------------------------------------------------

#include "serverBase/FirstServerBase.h"
#include "sharedFoundation/ConfigFile.h"
#include "ConfigServerBase.h"

//-----------------------------------------------------------------------

ConfigServerBase::Data *	ConfigServerBase::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("GameServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("GameServer", #a, b))
#define KEY_FLOAT(a,b)   (data->a = ConfigFile::getKeyFloat("GameServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("GameServer", #a, b))

//-----------------------------------------------------------------------

void ConfigServerBase::install(void)
{
	data = new ConfigServerBase::Data;

	KEY_STRING  (serverName, "BaseServer");
	KEY_INT     (sleepTimePerFrameMs, 10);
	KEY_INT     (defaultFrameRateLimit, 10);
	KEY_BOOL    (showAllDebugInfo, true);
}

//-----------------------------------------------------------------------

const char * ConfigServerBase::getServerName()
{
	return data->serverName;
}

//-----------------------------------------------------------------------

int ConfigServerBase::getSleepTimePerFrameMs()
{
	return data->sleepTimePerFrameMs;
}

//-----------------------------------------------------------------------

int ConfigServerBase::getDefaultFrameRateLimit()
{
	return data->defaultFrameRateLimit;
}

//-----------------------------------------------------------------------
void ConfigServerBase::remove(void)
{
	delete data;
	data = 0;
}

//-----------------------------------------------------------------------

const bool ConfigServerBase::getShowAllDebugInfo()
{
	return (data->showAllDebugInfo);
}

//-----------------------------------------------------------------------