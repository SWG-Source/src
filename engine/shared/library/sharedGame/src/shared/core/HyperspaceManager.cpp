//======================================================================
//
// HyperspaceManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/HyperspaceManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>

//======================================================================

namespace HyperspaceManagerNamespace
{
	namespace SystemLocationsColumnNames
	{
		char const * const scene = "SCENE";
		char const * const x = "X";
		char const * const y = "Y";
		char const * const z = "Z";
	}

	namespace HyperspaceLocationsColumnNames
	{
		char const * const hyperspacePointName = "HYPERSPACE_POINT_NAME";
		char const * const scene = "SCENE";
		char const * const x = "X";
		char const * const y = "Y";
		char const * const z = "Z";
		char const * const requiredCommand = "REQUIRED_COMMAND";
	}

	char const * const cms_systemLocationsTableName = "datatables/space/hyperspace/system_locations.iff";
	char const * const cms_hyperspaceLocationsTableName = "datatables/space/hyperspace/hyperspace_locations.iff";
	char const * const cms_hyperspaceHomeLocationHyperspacePointName = "space_home_location";
	char const * const cms_invalidHomeLocationHyperspaceScene = "space_npe_falcon";
	
	const int cms_minSystemLoc = 0;
	const int cms_maxSystemLoc = 100;

	std::map<std::string, Vector> ms_sceneToRelativeSystemLocations;
	std::map<std::string, HyperspaceManager::HyperspaceLocation> ms_hyperspaceLocationNameToHyperspaceLocations;
	std::map<std::string, std::vector<HyperspaceManager::HyperspaceLocation> > ms_sceneToHyperspaceLocations;
}

using namespace HyperspaceManagerNamespace;

//----------------------------------------------------------------------

void HyperspaceManager::install()
{
	InstallTimer const installTimer("HyperspaceManager::install");

	ms_hyperspaceLocationNameToHyperspaceLocations.clear();
	ms_sceneToRelativeSystemLocations.clear();
	ms_sceneToHyperspaceLocations.clear();
	{
		if(!TreeFile::exists(cms_systemLocationsTableName))
		{
			DEBUG_WARNING(true, ("Datatable [%s] not found to build the system locations", cms_systemLocationsTableName));
			return;
		}

		DataTable const * const systemLocationsTable = DataTableManager::getTable(cms_systemLocationsTableName, true);
		std::string scene;
		Vector loc;
		if(systemLocationsTable)
		{
			for(int i = 0; i < systemLocationsTable->getNumRows(); ++i)
			{
				scene = systemLocationsTable->getStringValue(SystemLocationsColumnNames::scene, i);
				loc.x = static_cast<float>(systemLocationsTable->getIntValue(SystemLocationsColumnNames::x, i));
				loc.y = static_cast<float>(systemLocationsTable->getIntValue(SystemLocationsColumnNames::y, i));
				loc.z = static_cast<float>(systemLocationsTable->getIntValue(SystemLocationsColumnNames::z, i));

				for(std::map<std::string, Vector>::const_iterator it = ms_sceneToRelativeSystemLocations.begin(); it != ms_sceneToRelativeSystemLocations.end(); ++it)
				{
					DEBUG_FATAL(it->second == loc, ("Data error: System location [%s] has same location as other system [%s] , in [%s], row[%d]. All systems must have a unique location", scene.c_str(), it->first.c_str(), cms_systemLocationsTableName, i));
				}

				DEBUG_FATAL(loc.x < cms_minSystemLoc, ("Data error: System location x < %d in [%s], row[%d], all system locations must be between %d and %d", cms_minSystemLoc, cms_systemLocationsTableName, i, cms_minSystemLoc, cms_maxSystemLoc));
				DEBUG_FATAL(loc.x > cms_maxSystemLoc, ("Data error: System location x > %d in [%s], row[%d], all system locations must be between %d and %d", cms_maxSystemLoc, cms_systemLocationsTableName, i, cms_minSystemLoc, cms_maxSystemLoc));
				DEBUG_FATAL(loc.y < cms_minSystemLoc, ("Data error: System location y < %d in [%s], row[%d], all system locations must be between %d and %d", cms_minSystemLoc, cms_systemLocationsTableName, i, cms_minSystemLoc, cms_maxSystemLoc));
				DEBUG_FATAL(loc.y > cms_maxSystemLoc, ("Data error: System location y > %d in [%s], row[%d], all system locations must be between %d and %d", cms_maxSystemLoc, cms_systemLocationsTableName, i, cms_minSystemLoc, cms_maxSystemLoc));
				DEBUG_FATAL(loc.z < cms_minSystemLoc, ("Data error: System location z < %d in [%s], row[%d], all system locations must be between %d and %d", cms_minSystemLoc, cms_systemLocationsTableName, i, cms_minSystemLoc, cms_maxSystemLoc));
				DEBUG_FATAL(loc.z > cms_maxSystemLoc, ("Data error: System location z > %d in [%s], row[%d], all system locations must be between %d and %d", cms_maxSystemLoc, cms_systemLocationsTableName, i, cms_minSystemLoc, cms_maxSystemLoc));

				ms_sceneToRelativeSystemLocations[scene] = loc;
			}
		}
	}

	{
		if(!TreeFile::exists(cms_hyperspaceLocationsTableName))
		{
			DEBUG_WARNING(true, ("Datatable [%s] not found to build the hyperspace locations", cms_hyperspaceLocationsTableName));
			return;
		}
		DataTable const * const hyperspaceLocationsTable = DataTableManager::getTable(cms_hyperspaceLocationsTableName, true);
		if(hyperspaceLocationsTable)
		{
			HyperspaceLocation hyperspaceLoc;
			for(int i = 0; i < hyperspaceLocationsTable->getNumRows(); ++i)
			{
				hyperspaceLoc.name = hyperspaceLocationsTable->getStringValue(HyperspaceLocationsColumnNames::hyperspacePointName, i);
				hyperspaceLoc.sceneName = hyperspaceLocationsTable->getStringValue(HyperspaceLocationsColumnNames::scene, i);
				hyperspaceLoc.location.x = static_cast<float>(hyperspaceLocationsTable->getIntValue(HyperspaceLocationsColumnNames::x, i));
				hyperspaceLoc.location.y = static_cast<float>(hyperspaceLocationsTable->getIntValue(HyperspaceLocationsColumnNames::y, i));
				hyperspaceLoc.location.z = static_cast<float>(hyperspaceLocationsTable->getIntValue(HyperspaceLocationsColumnNames::z, i));
				hyperspaceLoc.requiredCommand = hyperspaceLocationsTable->getStringValue(HyperspaceLocationsColumnNames::requiredCommand, i);
				ms_hyperspaceLocationNameToHyperspaceLocations[hyperspaceLoc.name] = hyperspaceLoc;
				ms_sceneToHyperspaceLocations[hyperspaceLoc.sceneName].push_back(hyperspaceLoc);
			}
		}
		
		// add in the home location hyperspace point name into the map so it's recognized as a valid point
		HyperspaceLocation hyperspaceLoc;

		hyperspaceLoc.name = cms_hyperspaceHomeLocationHyperspacePointName;
		hyperspaceLoc.sceneName.clear();
		hyperspaceLoc.location.x = 0;
		hyperspaceLoc.location.y = 0;
		hyperspaceLoc.location.z = 0;
		hyperspaceLoc.requiredCommand.clear();
		
		ms_hyperspaceLocationNameToHyperspaceLocations[hyperspaceLoc.name] = hyperspaceLoc;
	}
	ExitChain::add(HyperspaceManager::remove, "HyperspaceManager::remove");
}

//----------------------------------------------------------------------

void HyperspaceManager::remove()
{
	ms_sceneToRelativeSystemLocations.clear();
	ms_hyperspaceLocationNameToHyperspaceLocations.clear();
	ms_sceneToHyperspaceLocations.clear();
}

//----------------------------------------------------------------------

Vector HyperspaceManager::getDirectionToHyperspacePoint_w(std::string const & fromSceneName, Vector const & fromLocation_w, std::string const & hyperspacePoint)
{
	if(!isValidHyperspacePoint(hyperspacePoint))
		return Vector::unitZ;

	HyperspaceLocation toLocation;
	getHyperspacePoint(hyperspacePoint, toLocation);
	//if warping to point in the same scene, aim at new point in this scene
	Vector directionVec(Vector::unitZ);
	if(fromSceneName == toLocation.sceneName)
	{
		Vector const & endLoc = toLocation.location;
		Vector const & startLoc = fromLocation_w;
		directionVec = endLoc - startLoc;
	}
	//else point at the new system
	else
	{
		std::map<std::string, Vector>::const_iterator i = ms_sceneToRelativeSystemLocations.find(fromSceneName);
		if(i == ms_sceneToRelativeSystemLocations.end())
			return Vector::unitZ;

		std::map<std::string, Vector>::const_iterator j = ms_sceneToRelativeSystemLocations.find(toLocation.sceneName);
		if(j == ms_sceneToRelativeSystemLocations.end())
			return Vector::unitZ;

		Vector const & endLoc = j->second;
		Vector const & startLoc = i->second;
		directionVec = endLoc - startLoc;
	}

	if (directionVec.normalize())
		return directionVec;
	else
		return Vector::unitZ;
}

//----------------------------------------------------------------------

Vector HyperspaceManager::getDirectionToHyperspacePoint_w(std::string const & fromSceneName, Vector const & fromLocation_w, std::string const & toSceneName, Vector const & toLocation_w)
{
	//if warping to point in the same scene, aim at new point in this scene
	Vector directionVec(Vector::unitZ);
	if(fromSceneName == toSceneName)
	{
		directionVec = toLocation_w - fromLocation_w;
	}
	//else point at the new system
	else
	{
		std::map<std::string, Vector>::const_iterator i = ms_sceneToRelativeSystemLocations.find(fromSceneName);
		if(i == ms_sceneToRelativeSystemLocations.end())
			return Vector::unitZ;

		std::map<std::string, Vector>::const_iterator j = ms_sceneToRelativeSystemLocations.find(toSceneName);
		if(j == ms_sceneToRelativeSystemLocations.end())
			return Vector::unitZ;

		Vector const & endLoc = j->second;
		Vector const & startLoc = i->second;
		directionVec = endLoc - startLoc;
	}

	if (directionVec.normalize())
		return directionVec;
	else
		return Vector::unitZ;
}

//----------------------------------------------------------------------

bool HyperspaceManager::isValidHyperspacePoint(std::string const & hyperspacePointName)
{
	std::map<std::string, HyperspaceLocation>::const_iterator i = ms_hyperspaceLocationNameToHyperspaceLocations.find(hyperspacePointName);
	return (i != ms_hyperspaceLocationNameToHyperspaceLocations.end());
}

//----------------------------------------------------------------------

bool HyperspaceManager::getHyperspacePoint(std::string const & hyperspacePointName, HyperspaceManager::HyperspaceLocation & /*OUT*/ location)
{
	std::map<std::string, HyperspaceLocation>::const_iterator i = ms_hyperspaceLocationNameToHyperspaceLocations.find(hyperspacePointName);
	if(i != ms_hyperspaceLocationNameToHyperspaceLocations.end())
	{
		location = i->second;
		return true;
	}
	else
	{
		location.name.clear();
		location.sceneName.clear();
		location.location.makeZero();
		return false;
	}
}

//----------------------------------------------------------------------

bool HyperspaceManager::getHyperspacePoints(std::string const & sceneName, std::vector<HyperspaceLocation> & /*OUT*/ locations)
{
	locations.clear();
	std::map<std::string, std::vector<HyperspaceLocation> >::const_iterator i = ms_sceneToHyperspaceLocations.find(sceneName);
	if(i != ms_sceneToHyperspaceLocations.end())
	{
		locations = i->second;
		return true;
	}
	else
		return false;
}

//----------------------------------------------------------------------

std::string HyperspaceManager::getHomeLocationHyperspacePointName()
{
	return cms_hyperspaceHomeLocationHyperspacePointName;
}

//----------------------------------------------------------------------

bool HyperspaceManager::isValidSceneForHomeLocationHyperspace(std::string const & sceneName)
{
	// we do a find instead of a compare because we want to check for <sceneName>_2, _3, etc...
	if (sceneName.find(cms_invalidHomeLocationHyperspaceScene, 0) != std::string::npos)
		return false;

	return true;
}

//======================================================================
