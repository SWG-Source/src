// ======================================================================
//
// ConfigSharedPathfinding.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConfigSharedPathfinding_H
#define INCLUDED_ConfigSharedPathfinding_H

// ======================================================================

class ConfigSharedPathfinding
{
public:

	static void        install();

	static bool        getAutoCreateBuildingPathNodes();
	static bool        getUseCityRegions();
	static bool        getJitterCityWaypoints();
	static bool        getEnableDirtyBoxes();
	static bool        getEnablePathScrubber();

private:

	static bool        ms_autoCreateBuildingPathNodes;
	static bool        ms_useCityRegions;
	static bool        ms_jitterCityWaypoints;
	static bool        ms_enableDirtyBoxes;
	static bool        ms_enablePathScrubber;
};

//--------------------------------------------------------------------

inline bool ConfigSharedPathfinding::getAutoCreateBuildingPathNodes()
{
	return ms_autoCreateBuildingPathNodes;
}

inline bool ConfigSharedPathfinding::getUseCityRegions()
{
	return ms_useCityRegions;
}

inline bool ConfigSharedPathfinding::getJitterCityWaypoints()
{
	return ms_jitterCityWaypoints;
}

inline bool ConfigSharedPathfinding::getEnableDirtyBoxes()
{
	return ms_enableDirtyBoxes;
}

inline bool ConfigSharedPathfinding::getEnablePathScrubber()
{
	return ms_enablePathScrubber;
}

// ======================================================================

#endif
