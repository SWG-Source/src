// ======================================================================
//
// ConfigSharedFile.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/ConfigSharedFile.h"

#include "sharedFoundation/ConfigFile.h"

#include <vector>

// ======================================================================

#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("SharedFile", #a, b))
#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedFile", #a, b))
//#define KEY_FLOAT(a,b)   (ms_ ## a = ConfigFile::getKeyFloat("SharedFile", #a, b))
//#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("SharedFile", #a, b))

// ======================================================================

namespace ConfigSharedFileNamespace
{
	typedef std::vector<char const *> StringPtrArray;
	bool           ms_enableAsynchronousLoader;
	int            ms_asynchronousLoaderPriority;
	int            ms_asynchronousLoaderCallbacksPerFrame;
	bool		   ms_validateIff;
	StringPtrArray ms_preloads; // ConfigFile owns the pointer
}

using namespace ConfigSharedFileNamespace;

// ======================================================================

void ConfigSharedFile::install()
{
	KEY_BOOL(enableAsynchronousLoader, true);
	KEY_INT(asynchronousLoaderPriority, 0);
	KEY_INT(asynchronousLoaderCallbacksPerFrame, 0);
	KEY_BOOL(validateIff, false);

	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("SharedFile", "preload", index++, 0);
		if (result)
			ms_preloads.push_back(result);
	}
	while (result);
}

// ----------------------------------------------------------------------

bool  ConfigSharedFile::getEnableAsynchronousLoader()
{
	return ms_enableAsynchronousLoader;
}

// ----------------------------------------------------------------------

int ConfigSharedFile::getAsynchronousLoaderPriority()
{
	return ms_asynchronousLoaderPriority;
}

// ----------------------------------------------------------------------

int ConfigSharedFile::getAsynchronousLoaderCallbacksPerFrame()
{
	return ms_asynchronousLoaderCallbacksPerFrame;
}

// ----------------------------------------------------------------------

bool ConfigSharedFile::getValidateIff()
{
	return(ms_validateIff);
}

// ----------------------------------------------------------------------

int ConfigSharedFile::getNumberOfTreeFilePreloads()
{
	return static_cast<int>(ms_preloads.size());
}

// ----------------------------------------------------------------------

char const * ConfigSharedFile::getTreeFilePreload(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfTreeFilePreloads());
	return ms_preloads[static_cast<size_t>(index)];
}

// ======================================================================

