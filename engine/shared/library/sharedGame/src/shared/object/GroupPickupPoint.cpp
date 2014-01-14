// ======================================================================
//
// GroupPickupPoint.cpp
// Copyright 2008 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/GroupPickupPoint.h"

#include "sharedGame/TravelManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include <map>

// ======================================================================

bool GroupPickupPoint::isGroupPickupAllowedAtLocation(std::string const & planetName, Object const & object)
{
	Vector const objectWorldLocation = object.findPosition_w();
	return isGroupPickupAllowedAtLocation(planetName, static_cast<int>(objectWorldLocation.x), static_cast<int>(objectWorldLocation.z));
}

// ----------------------------------------------------------------------

bool GroupPickupPoint::isGroupPickupAllowedAtLocation(std::string const & planetName, int x, int z)
{
	static std::map<std::string, std::pair<std::pair<int, int>, std::pair<int, int> > > s_groupPickupAllowedLocation;
	if (s_groupPickupAllowedLocation.empty())
	{
		s_groupPickupAllowedLocation["tatooine"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["naboo"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["corellia"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["rori"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["talus"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["yavin4"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["endor"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["lok"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["dantooine"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["dathomir"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["kashyyyk_main"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["kashyyyk_hunting"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["kashyyyk_dead_forest"] = std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
		s_groupPickupAllowedLocation["mustafar"] = std::make_pair(std::make_pair(-6880, -1024), std::make_pair(1120, 6976)); // the "public" area of mustafar, from areas_mustafar.tab
	}

	std::map<std::string, std::pair<std::pair<int, int>, std::pair<int, int> > >::const_iterator iterFind = s_groupPickupAllowedLocation.find(planetName);
	if (iterFind != s_groupPickupAllowedLocation.end())
	{
		if ((iterFind->second.first.first == 0) && (iterFind->second.first.second == 0) && (iterFind->second.second.first == 0) && (iterFind->second.second.second == 0))
			return true;

		if ((x >= iterFind->second.first.first) && (x <= iterFind->second.second.first) && (z >= iterFind->second.first.second) && (z <= iterFind->second.second.second))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool GroupPickupPoint::getGroupPickupTravelCost(const std::string& sourcePlanet, const std::string& targetPlanet, int& cost)
{
	// none of the other kashyyyk zones have starport, so use kashyyyk_main for them
	std::string source = sourcePlanet;
	std::string target = targetPlanet;

	if (source.find("kashyyyk") == 0)
		source = "kashyyyk_main";

	if (target.find("kashyyyk") == 0)
		target = "kashyyyk_main";

	if (source == target)
	{
		cost = 200;
		return true;
	}

	return TravelManager::getPlanetAnyHopLeastCost(source, target, cost);
}

// ======================================================================
