// ======================================================================
//
// ScriptListEntry.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/ScriptListEntry.h"
#include "serverScript/GameScriptObject.h"

// ======================================================================

std::string const &ScriptListEntry::getScriptName() const
{
static const std::string emptyString;

	if (m_data != nullptr)
		return m_data->first;
	WARNING_STRICT_FATAL(true, ("ScriptListEntry::getScriptName called with m_data = nullptr"));
	return emptyString;
}

// ----------------------------------------------------------------------

ScriptData &ScriptListEntry::getScriptData() const
{
static ScriptData emptyData;

	if (m_data != nullptr)
		return m_data->second;
	WARNING_STRICT_FATAL(true, ("ScriptListEntry::getScriptData called with m_data = nullptr"));
	return emptyData;
}

// ======================================================================

