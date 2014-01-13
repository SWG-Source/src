// ======================================================================
//
// ApplicationVersion.cpp
// Copyright 2002-2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ApplicationVersion.h"

// ======================================================================

namespace ApplicationVersionNamespace
{
	// WARNING do not change this string without making sure that the hardcoded offsets below are correct
	char ms_version[] = "ApplicationVersionMarker 000 123456789012345678901234567890";
	int const brandOffset   = 25;
	int const bootlegOffset = 26;
	int const publishOffset = 27;
	int const versionOffset = 29;

	bool isBranded();
}
using namespace ApplicationVersionNamespace;

// ======================================================================

bool ApplicationVersionNamespace::isBranded()
{
	return ms_version[brandOffset] == '1';
}

// ----------------------------------------------------------------------

bool ApplicationVersion::isBootlegBuild()
{
	return ms_version[bootlegOffset] == '1';
}

// ----------------------------------------------------------------------

bool ApplicationVersion::isPublishBuild()
{
	return ms_version[publishOffset] == '1';
}

// ----------------------------------------------------------------------

const char * ApplicationVersion::getPublicVersion()
{
	if (!isBranded())
		return "0";

	char const * internal = ms_version + versionOffset;
	while (*internal != '\0' && *internal != '.')
		++internal;

	if (*internal == '\0' || internal[1] == '\0')
		return "0";

	return internal + 1;
}

// ----------------------------------------------------------------------

char const * ApplicationVersion::getInternalVersion()
{
	if (!isBranded())
		return "unknown.0";

	return ms_version + versionOffset;
}

// ======================================================================
