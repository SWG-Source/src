//======================================================================
//
// StartingLocationManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_StartingLocationManager_H
#define INCLUDED_StartingLocationManager_H

//======================================================================

class StartingLocationData;

//----------------------------------------------------------------------

class StartingLocationManager
{
public:

	typedef stdvector<StartingLocationData>::fwd StartingLocationVector;

	static const StartingLocationData *       findLocationByName   (const std::string & name);
	static void                               getLocationsByPlanet (const std::string & planet, StartingLocationVector & slv);
	static const StartingLocationVector &     getLocations         ();
};

//======================================================================

#endif
