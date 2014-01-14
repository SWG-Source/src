//===================================================================
//
// ConfigSharedTerrain.h
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ConfigSharedTerrain_H
#define INCLUDED_ConfigSharedTerrain_H

//===================================================================

class ConfigSharedTerrain
{
public:

	static void install ();

	static int  getMaximumNumberOfChunksAllowed ();
	static bool getDisableGetHeight ();
	static bool getDebugReportInstall ();
	static bool getDebugReportLogPrint ();
	static bool getDisableFloraCaching ();

	static float getMaximumValidHeightInMeters ();

private:

	ConfigSharedTerrain ();
	ConfigSharedTerrain (const ConfigSharedTerrain&);
	ConfigSharedTerrain& operator= (const ConfigSharedTerrain&);
};

//===================================================================

#endif
