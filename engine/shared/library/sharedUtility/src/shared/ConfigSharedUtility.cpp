// ======================================================================
//
// ConfigSharedUtility.cpp
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/ConfigSharedUtility.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedUtility",   #a, (b)))
#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("SharedUtility",    #a, (b)))
#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("SharedUtility", #a, (b)))

// ======================================================================

namespace ConfigSharedUtilityNamespace
{
	bool ms_disableFileCaching;
	char const * ms_useCacheFile;
	int ms_chunkSize;
	bool ms_logOptionManager;
}

using namespace ConfigSharedUtilityNamespace;

// ======================================================================

void ConfigSharedUtility::install()
{
	KEY_BOOL(disableFileCaching, false);
	KEY_STRING(useCacheFile, "");
	KEY_INT(chunkSize, 32);
	KEY_BOOL(logOptionManager, false);
}

// ----------------------------------------------------------------------

bool ConfigSharedUtility::getDisableFileCaching()
{
	return ms_disableFileCaching;
}

// ----------------------------------------------------------------------

char const * ConfigSharedUtility::getUseCacheFile()
{
	return ms_useCacheFile;
}

// ----------------------------------------------------------------------

int ConfigSharedUtility::getChunkSize()
{
	return ms_chunkSize;
}

// ----------------------------------------------------------------------

bool ConfigSharedUtility::getLogOptionManager()
{
	return ms_logOptionManager;
}

// ======================================================================
