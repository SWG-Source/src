// ======================================================================
//
// ApplicationVersion.h
// Copyright 2002-2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ApplicationVersion_H
#define INCLUDED_ApplicationVersion_H

// ======================================================================

class ApplicationVersion
{
public:
	static bool isBootlegBuild();
	static bool isPublishBuild();
	static char const * getPublicVersion();
	static char const * getInternalVersion();
};

// ======================================================================

#endif
