// ======================================================================
//
// TravelManager.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_TravelManager_H
#define INCLUDED_TravelManager_H

// ======================================================================

class TravelManager
{
public:

	static void        install ();

	static int         getNumberOfPlanets ();
	static bool        getPlanetName (int planetIndex, std::string& planetName);
	static bool        getPlanetSingleHopCost (const std::string& planetName1, const std::string& planetName2, int& planetCost);
	static bool        getPlanetAnyHopLeastCost (const std::string& planetName1, const std::string& planetName2, int& planetCost);
	static bool        getAdjacentPlanets (const std::string& planetName, std::set<std::string>& planetSet);
};

// ======================================================================

#endif
