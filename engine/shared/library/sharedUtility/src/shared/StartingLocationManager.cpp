//======================================================================
//
// StartingLocationManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/StartingLocationManager.h"

#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/StartingLocationData.h"
#include <map>
#include <vector>

//======================================================================

namespace StartingLocationManagerNamespace
{
	typedef std::map<std::string, StartingLocationData> StartingLocationMap;
	StartingLocationMap s_startingLocationMap;

	typedef StartingLocationManager::StartingLocationVector StartingLocationVector;
	StartingLocationVector s_startingLocationVector;

	bool s_installed = false;

	enum DtCols
	{
		DC_location,
		DC_planet,
		DC_x,
		DC_y,
		DC_z,
		DC_cell,
		DC_image,
		DC_description
	};

	void install ()
	{
		if (s_installed)
			return;

		static const char * const filename = "datatables/creation/starting_locations.iff";
		
		Iff iff;
		
		if (!iff.open (filename, true))
			WARNING (true, ("Data file %s not available.", filename));
		else
		{			
			DataTable dt;
			dt.load (iff);
			iff.close ();

			const int numRows = dt.getNumRows ();
			s_startingLocationVector.reserve (static_cast<size_t>(numRows));

			for (int i = 0; i < numRows; ++i)
			{
				StartingLocationData data;
				data.name          = dt.getStringValue (DC_location,     i);
				data.planet        = dt.getStringValue (DC_planet,       i);
				data.x             = dt.getFloatValue  (DC_x,            i);
				data.y             = dt.getFloatValue  (DC_y,            i);
				data.z             = dt.getFloatValue  (DC_z,            i);
				data.cellId        = dt.getStringValue (DC_cell,         i);
				data.image         = dt.getStringValue (DC_image,        i);
				data.descriptionId = dt.getStringValue (DC_description,  i);

				s_startingLocationMap.insert (std::make_pair (data.name, data));
				s_startingLocationVector.push_back (data);
			}
		}

		s_installed = true;
	}
}

using namespace StartingLocationManagerNamespace;

//----------------------------------------------------------------------

const StartingLocationData * StartingLocationManager::findLocationByName   (const std::string & name)
{
	if (!s_installed)
		install ();

	const StartingLocationMap::const_iterator it = s_startingLocationMap.find (name);
	if (it != s_startingLocationMap.end ())
		return &(*it).second;

	return 0;
}

//----------------------------------------------------------------------

void StartingLocationManager::getLocationsByPlanet (const std::string & planet, StartingLocationVector & slv)
{
	if (!s_installed)
		install ();

	const bool isPlanetEmpty = planet.empty ();
	for (StartingLocationMap::const_iterator it = s_startingLocationMap.begin (); it != s_startingLocationMap.end (); ++it)
	{
		const StartingLocationData & data = (*it).second;
		if (isPlanetEmpty || data.planet == planet)
			slv.push_back (data);
	}
}

//----------------------------------------------------------------------

const StartingLocationManager::StartingLocationVector &  StartingLocationManager::getLocations         ()
{
	if (!s_installed)
		install ();

	return s_startingLocationVector;
}

//======================================================================
