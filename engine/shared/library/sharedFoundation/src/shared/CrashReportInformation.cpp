// ======================================================================
//
// CrashReportInformation.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/CrashReportInformation.h"

#include "sharedFoundation/ExitChain.h"

#include <vector>
#include <algorithm>

// ======================================================================

namespace CrashReportInformationNamespace
{
	typedef std::vector<char *> TextVector;
	typedef std::vector<char const *> ConstTextVector;
	TextVector      ms_staticText;
	ConstTextVector ms_dynamicText;
}
using namespace CrashReportInformationNamespace;

// ======================================================================

void CrashReportInformation::install()
{
	ExitChain::add(&remove, "CrashReportInformation::remove");
}

// ----------------------------------------------------------------------

void CrashReportInformation::remove()
{
	while (!ms_staticText.empty())
	{
		delete [] ms_staticText.back();
		ms_staticText.pop_back();
	}

	ms_dynamicText.clear();
}

// ----------------------------------------------------------------------

void CrashReportInformation::addStaticText(char const * format, ...)
{
	va_list va;

	va_start(va, format);

		char buffer[256];
		vsnprintf(buffer, sizeof(buffer), format, va);
		ms_staticText.push_back(DuplicateString(buffer));

	va_end(va);
}

// ----------------------------------------------------------------------

void CrashReportInformation::addDynamicText(char const * text)
{
	ms_dynamicText.push_back(text);
}

// ----------------------------------------------------------------------

void CrashReportInformation::removeDynamicText(char const * text)
{
	ConstTextVector::iterator i = std::find(ms_dynamicText.begin(), ms_dynamicText.end(), text);
	if (i != ms_dynamicText.end())
		ms_dynamicText.erase(i);
}

// ----------------------------------------------------------------------

char const * CrashReportInformation::getEntry(int index)
{
	if (index < static_cast<int>(ms_staticText.size()))
		return ms_staticText[index];

	index -= static_cast<int>(ms_staticText.size());
	if (index < static_cast<int>(ms_dynamicText.size()))
		return ms_dynamicText[index];

	return nullptr;
}

// ======================================================================
