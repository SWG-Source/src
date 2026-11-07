// ======================================================================
//
// CrashReportInformation.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CrashReportInformation_H
#define INCLUDED_CrashReportInformation_H


#include "sharedFoundation/FormatAttribute.h"
// ======================================================================

class CrashReportInformation
{
public:

	static void install();
	static void remove();

	static DLLEXPORT void addStaticText(char const * format, ...) SWG_FORMAT_PRINTF(1, 2);

	static void           addDynamicText(char const * text);
	static void           removeDynamicText(char const * text);

	static char const *   getEntry(int index);

private:
	CrashReportInformation();
	CrashReportInformation(CrashReportInformation const &);
	CrashReportInformation & operator=(CrashReportInformation const &);
};

// ======================================================================

#endif
