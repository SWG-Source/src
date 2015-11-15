// =====================================================================
//
// ConfigSharedObject.h
// asommers
//
// copyright 2002, sony online entertainment
//
// =====================================================================

#ifndef INCLUDED_ConfigSharedObject_H
#define INCLUDED_ConfigSharedObject_H

// =====================================================================

class ConfigSharedObject
{
public:

	static void  install ();

	static float getPortalScale ();
	static bool getAllowDisabledPortals();
	static float getAppearanceTemplateListKeepTime ();
	static float getAppearanceTemplateListKeepEpsilon ();
	static bool  getValidateCellContentsAttached ();
	static bool  getValidateWorld ();

	static int   getAlterSchedulerNoAlterScheduleDelay ();

	static bool  getLogCustomizationDataIssues ();

	static bool  getDebugAlterChecking();

	static bool  getContainerLoopChecking();
	static int   getContainerMaxDepth(); 

	static bool  getAllowDisallowObjectDelete();
private:

	ConfigSharedObject ();
	ConfigSharedObject (const ConfigSharedObject&);
	ConfigSharedObject& operator= (const ConfigSharedObject&);
};

// =====================================================================

#endif
