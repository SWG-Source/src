// ======================================================================
//
// LocationManager.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_LocationManager_H
#define INCLUDED_LocationManager_H

// ======================================================================

class NetworkId;

// ======================================================================

class LocationManager
{
public:

	static void install ();
	
	static void setPlanetName (char const * planetName);

	static void updateObject (NetworkId const & networkId, int x, int z, int radius);
	static void removeObject (NetworkId const & networkId);

	static bool requestLocation (float searchX, float searchZ, float searchRadius, float locationRadius, bool checkWater, bool checkSlope, float & resultX, float & resultZ);
};

// ======================================================================

#endif
