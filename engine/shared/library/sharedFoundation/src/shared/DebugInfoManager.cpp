// ======================================================================
//
// DebugInfoManager.cpp
// Copyright 2005 Sony Online Entertainment Inc
// All Rights Reserved
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/DebugInfoManager.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include "UnicodeUtils.h"

#include <map>

// ======================================================================

namespace DebugInfoManagerNamespace
{
	char ms_buffer[256];

	Unicode::String const cms_emptyQuotes(Unicode::narrowToWide("\"\""));
}
using namespace DebugInfoManagerNamespace;

// ======================================================================

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, int const propertyValue)
{
	_itoa(propertyValue, ms_buffer, 10);
	addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(ms_buffer));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, float const propertyValue)
{
	sprintf(ms_buffer, "%.2f", propertyValue);
	addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(ms_buffer));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, unsigned int const propertyValue)
{
	sprintf(ms_buffer, "%u", propertyValue);
	addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(ms_buffer));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, unsigned long const propertyValue)
{
	sprintf(ms_buffer, "%lu", propertyValue);
	addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(ms_buffer));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, bool const propertyValue)
{
	addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(propertyValue ? "true" : "false"));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, std::string const & propertyValue)
{
	addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(propertyValue));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, char const * const propertyValue)
{
	if(propertyValue)
		addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(propertyValue));
	else
		addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(""));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, Vector const & propertyValue)
{
	sprintf(ms_buffer, "(%.2f, %.2f, %.2f)", propertyValue.x, propertyValue.y, propertyValue.z);
	addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(ms_buffer));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, Transform const & propertyValue)
{
	Transform::matrix_t const & matrix = propertyValue.getMatrix();
	sprintf(ms_buffer, "\n\t\t[[%.2f, %.2f, %.2f, %.2f]\n\t\t[%.2f, %.2f, %.2f, %.2f]\n\t\t[%.2f, %.2f, %.2f, %.2f]]", matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3], matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3], matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]);
	addProperty(propertyMap, section, propertyName, Unicode::narrowToWide(ms_buffer));
}

// ----------------------------------------------------------------------

void DebugInfoManager::addProperty(DebugInfoManager::PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, Unicode::String const & propertyValue)
{
	//okay to create entry if the section doesn't exist;
	PropertySection sectionMap = propertyMap[section];

	if(sectionMap.find(propertyName) == sectionMap.end())
	{
		if(!propertyValue.empty())
			sectionMap[propertyName] = propertyValue;
		else
			sectionMap[propertyName] = cms_emptyQuotes;
	}
	else
	{
		if(!propertyValue.empty())
			sectionMap[propertyName + "_a"] = propertyValue;
		else
			sectionMap[propertyName + "_a"] = cms_emptyQuotes;
	}

	propertyMap[section] = sectionMap;
}

// ======================================================================
