//======================================================================
//
// GroundZoneManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_GroundZoneManager_H
#define INCLUDED_GroundZoneManager_H

#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"

class BuildoutArea;

//======================================================================

class GroundZoneManager
{
public:
	static void        install ();
	static void        remove ();
	
	static BuildoutArea const * getZoneName(std::string const & sceneName, Vector const & location_w, std::string & zoneName);
	static Vector2d getZoneSize(std::string const & zoneName);
	static Vector transformWorldLocationToZoneLocation(std::string const & sceneName, Vector const & location_w);
	static Vector adjustForNonSquareZones(std::string  const & encodedZoneName, Vector const & zoneLocation);
	static float getXToZRatioOfZone(std::string const & zoneName);
	static float getXToZRatioOfZoneFromScene(std::string const & sceneName, Vector const & location_w);
	static Vector getRelativePositionFromPlayer(const char *sceneName, Vector const & playerPosition_w, Vector const & worldPosition_w);
	static Vector getRelativePosition(const char *sceneName, Vector const & worldPosition_w);
};

//======================================================================

#endif
