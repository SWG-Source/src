//======================================================================
//
// HyperspaceManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_HyperspaceManager_H
#define INCLUDED_HyperspaceManager_H

//======================================================================

#include "sharedMath/Vector.h"

//----------------------------------------------------------------------

class HyperspaceManager
{
public:

	struct HyperspaceLocation
	{
		std::string name;
		std::string sceneName;
		Vector location;
		std::string requiredCommand;
	};

	static void install();
	static void remove();

	static Vector getDirectionToHyperspacePoint_w(std::string const & fromSceneName, Vector const & fromLocation_w, std::string const & hyperspacePoint);
	static Vector getDirectionToHyperspacePoint_w(std::string const & fromSceneName, Vector const & fromLocation_w, std::string const & toSceneName, Vector const & toLocation_w);
	static bool isValidHyperspacePoint(std::string const & hyperspacePointName);
	static bool getHyperspacePoint(std::string const & hyperspacePointName, HyperspaceLocation & /*OUT*/ location);
	static bool getHyperspacePoints(std::string const & sceneName, std::vector<HyperspaceLocation> & /*OUT*/ locations);

	static std::string getHomeLocationHyperspacePointName();
	static bool isValidSceneForHomeLocationHyperspace(std::string const & sceneName);
};

//======================================================================

#endif
