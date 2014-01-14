// =====================================================================
//
// ConfigSharedObject.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// =====================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ConfigSharedObject.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConfigFile.h"

// =====================================================================

#define KEY_BOOL(s, a,b)    (ms_ ## a = ConfigFile::getKeyBool(s, #a, b))
#define KEY_INT(s, a,b)     (ms_ ## a = ConfigFile::getKeyInt(s, #a, b))
#define KEY_FLOAT(s, a,b)   (ms_ ## a = ConfigFile::getKeyFloat(s, #a, b))
//#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("SharedObject", #a, b))

// =====================================================================
// ConfigSharedObjectNamespace
// =====================================================================

namespace ConfigSharedObjectNamespace
{
	float ms_portalScale;
	bool ms_allowDisabledPortals;
	float ms_appearanceTemplateListKeepTime;
	float ms_appearanceTemplateListKeepEpsilon;
	bool  ms_validateCellContentsAttached;
	bool  ms_validateWorld;

	int   ms_alterSchedulerNoAlterScheduleDelay;

	bool  ms_logCustomizationDataIssues;

	bool ms_debugAlterChecking;

	bool ms_containerLoopChecking;
	int  ms_containerMaxDepth;

	bool ms_allowDisallowObjectDelete;
}

using namespace ConfigSharedObjectNamespace;

// =====================================================================
// STATIC PUBLIC ConfigSharedObjectNamespace
// =====================================================================

void ConfigSharedObject::install()
{
	KEY_FLOAT ("SharedObject",                portalScale,                        1.1f);
	KEY_BOOL("SharedObject", allowDisabledPortals, false);
	KEY_FLOAT ("SharedObject",                appearanceTemplateListKeepTime,     15.f);
	KEY_FLOAT ("SharedObject",                appearanceTemplateListKeepEpsilon,  5.f);
	KEY_BOOL  ("SharedObject",                validateCellContentsAttached,       true);
	KEY_BOOL  ("SharedObject",                validateWorld,                      false);
	KEY_BOOL  ("SharedObject",                logCustomizationDataIssues,         false);

	KEY_INT   ("SharedObject/AlterScheduler", alterSchedulerNoAlterScheduleDelay, 0);

	KEY_BOOL  ("SharedObject",                debugAlterChecking,                 false);

	KEY_BOOL  ("SharedObject",                containerLoopChecking,              true);
	KEY_INT   ("SharedObject",                containerMaxDepth,                  9);

	KEY_BOOL  ("SharedObject",                allowDisallowObjectDelete,          true);
}

// =====================================================================
// STATIC PRIVATE ConfigSharedObjectNamespace
// =====================================================================

float ConfigSharedObject::getPortalScale ()
{
	return ms_portalScale;
}

// ---------------------------------------------------------------------

bool ConfigSharedObject::getAllowDisabledPortals()
{
	return ms_allowDisabledPortals;
}

// ---------------------------------------------------------------------

float ConfigSharedObject::getAppearanceTemplateListKeepTime ()
{
	return ms_appearanceTemplateListKeepTime;
}

// ---------------------------------------------------------------------

float ConfigSharedObject::getAppearanceTemplateListKeepEpsilon ()
{
	return ms_appearanceTemplateListKeepEpsilon;
}

// ---------------------------------------------------------------------

bool ConfigSharedObject::getValidateCellContentsAttached ()
{
	return ms_validateCellContentsAttached;
}

// ---------------------------------------------------------------------

bool ConfigSharedObject::getValidateWorld ()
{
	return ms_validateWorld;
}

// ----------------------------------------------------------------------
/**
 * If set to non-zero, the alter scheduler will schedule objects requesting
 * no alter at this return value's number of seconds into the future.
 *
 * @return the number of seconds in the future in which no-alter-desired objects
 *         will be altered into the future.
 */

int ConfigSharedObject::getAlterSchedulerNoAlterScheduleDelay ()
{
	return ms_alterSchedulerNoAlterScheduleDelay;
}

// ----------------------------------------------------------------------

bool ConfigSharedObject::getLogCustomizationDataIssues ()
{
	return ms_logCustomizationDataIssues;
}

// ----------------------------------------------------------------------

bool ConfigSharedObject::getDebugAlterChecking()
{
	return ms_debugAlterChecking;
}

// ----------------------------------------------------------------------

bool ConfigSharedObject::getContainerLoopChecking()
{
	return ms_containerLoopChecking;
}

// ----------------------------------------------------------------------

int ConfigSharedObject::getContainerMaxDepth()
{
	return ms_containerMaxDepth;
}

// ----------------------------------------------------------------------

bool ConfigSharedObject::getAllowDisallowObjectDelete()
{
	return ms_allowDisallowObjectDelete;
}

// =====================================================================

