// ======================================================================
//
// TravelManager.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/TravelManager.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"

#include <map>
#include <vector>
#include <set>

// ======================================================================

namespace TravelManagerNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<std::string> PlanetNameList;
	typedef std::map<std::string, int> PlanetNameToPlanetIndexMap;
	typedef std::map<std::pair<int, int>, int> SingleHopRouteList; // <<planet1, planet2>, cost> (planet1 ***IS ALWAYS <=*** planet2)
	typedef std::map<std::pair<int, int>, std::pair<int, std::vector<int> > > AnyHopLeastCostRouteList; // <<planet1, planet2>, <cost, <list of planets along the route>>> (planet1 ***IS ALWAYS <=*** planet2)

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove ();
	void load (const char* fileName);
	bool getPlanetIndex (const std::string& planetName, int& planetIndex);
	bool getPlanetSingleHopCost(int planetIndex1, int planetIndex2, int& planetCost);
	bool getPlanetAnyHopLeastCost(int planetIndex1, int planetIndex2, int& planetCost);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool           ms_installed;
	bool           ms_debugReport;
	PlanetNameList ms_planetNameList;
	PlanetNameToPlanetIndexMap ms_planetNameToPlanetIndexList;
	SingleHopRouteList ms_singleHopRouteList;
	AnyHopLeastCostRouteList ms_anyHopLeastCostRouteList;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace TravelManagerNamespace;

// ======================================================================
// STATIC PUBLIC TravelManager
// ======================================================================

void TravelManager::install ()
{
	InstallTimer const installTimer("TravelManager::install ");

	DEBUG_FATAL (ms_installed, ("TravelManager::install: already installed"));
	ms_installed = true;

	load ("datatables/travel/travel.iff");

	DebugFlags::registerFlag (ms_debugReport, "SharedGame", "reportTravelManager");
	ExitChain::add (TravelManagerNamespace::remove, "TravelManagerNamespace::remove");
}

// ----------------------------------------------------------------------

int TravelManager::getNumberOfPlanets ()
{
	return static_cast<int> (ms_planetNameToPlanetIndexList.size ());
}

// ----------------------------------------------------------------------

bool TravelManager::getPlanetName (int planetIndex, std::string& planetName)
{
	if (planetIndex < 0 || planetIndex >= getNumberOfPlanets ())
	{
		DEBUG_WARNING (true, ("TravelManager::getPlanetName: planetIndex out of range %i >= %i\n", planetIndex, getNumberOfPlanets ()));
		return false;
	}

	planetName = ms_planetNameList [static_cast<uint> (planetIndex)];
	return true;
}

// ----------------------------------------------------------------------

bool TravelManager::getPlanetSingleHopCost (const std::string& planetName1, const std::string& planetName2, int& planetCost)
{
	int planetIndex1(0);
	if (!getPlanetIndex (planetName1, planetIndex1))
	{
		DEBUG_WARNING (true, ("TravelManager::getPlanetSingleHopCost: planet %s does not exist\n", planetName1.c_str ()));
		return false;
	}

	int planetIndex2(0);
	if (!getPlanetIndex (planetName2, planetIndex2))
	{
		DEBUG_WARNING (true, ("TravelManager::getPlanetSingleHopCost: planet %s does not exist\n", planetName2.c_str ()));
		return false;
	}

	return TravelManagerNamespace::getPlanetSingleHopCost(planetIndex1, planetIndex2, planetCost);
}

// ----------------------------------------------------------------------

bool TravelManager::getPlanetAnyHopLeastCost (const std::string& planetName1, const std::string& planetName2, int& planetCost)
{
	int planetIndex1(0);
	if (!getPlanetIndex (planetName1, planetIndex1))
	{
		DEBUG_WARNING (true, ("TravelManager::getPlanetAnyHopLeastCost: planet %s does not exist\n", planetName1.c_str ()));
		return false;
	}

	int planetIndex2(0);
	if (!getPlanetIndex (planetName2, planetIndex2))
	{
		DEBUG_WARNING (true, ("TravelManager::getPlanetAnyHopLeastCost: planet %s does not exist\n", planetName2.c_str ()));
		return false;
	}

	return TravelManagerNamespace::getPlanetAnyHopLeastCost(planetIndex1, planetIndex2, planetCost);
}

// ----------------------------------------------------------------------

bool TravelManager::getAdjacentPlanets (const std::string& planetName, std::set<std::string>& planetList)
{
	int planetIndex(0);
	if (!getPlanetIndex (planetName, planetIndex))
	{
		DEBUG_WARNING (true, ("TravelManager::getAdjacentPlanets: planet %s does not exist\n", planetName.c_str ()));
		return false;
	}

	planetList.clear ();
	planetList.insert (planetName);

	SingleHopRouteList::const_iterator iter = ms_singleHopRouteList.begin ();
	for (; iter != ms_singleHopRouteList.end (); ++iter)
	{
		if (iter->first.first == planetIndex)
			planetList.insert (ms_planetNameList [static_cast<uint> (iter->first.second)]);
		else if (iter->first.second == planetIndex)
			planetList.insert (ms_planetNameList [static_cast<uint> (iter->first.first)]);
	}

	return true;
}

// ======================================================================
// STATIC PRIVATE TravelManager
// ======================================================================

void TravelManagerNamespace::remove ()
{
	DEBUG_FATAL (!ms_installed, ("TravelManager::install: not installed"));
	ms_installed = false;

	DebugFlags::unregisterFlag (ms_debugReport);
}

// ----------------------------------------------------------------------

void TravelManagerNamespace::load (const char* fileName)
{
	ms_planetNameList.clear ();
	ms_planetNameToPlanetIndexList.clear ();
	ms_singleHopRouteList.clear ();
	ms_anyHopLeastCostRouteList.clear ();

	Iff iff;
	if (iff.open (fileName, true))
	{
		DataTable dataTable;
		dataTable.load (iff);

		//-- setup planet names
		const int numberOfColumns = dataTable.getNumColumns ();
		int column(0);
		for (column = 1; column < numberOfColumns; ++column)
		{
			std::string const & planetName = dataTable.getColumnName (column);
			FATAL ((ms_planetNameToPlanetIndexList.count(planetName) > 0), ("TravelManagerNamespace::load: duplicate planet %s in column %i", planetName.c_str (), column));

			ms_planetNameToPlanetIndexList[planetName] = ms_planetNameList.size();
			ms_planetNameList.push_back (planetName);
		}

		//-- setup "single hop" routes
		const int numberOfRows = dataTable.getNumRows ();
		int row(0);
		for (row = 0; row < numberOfRows; ++row)
		{
			const std::string planetName1 = dataTable.getStringValue (0, row);
			int planetIndex1(0);
			if (!getPlanetIndex (planetName1, planetIndex1))
				DEBUG_FATAL (true, ("TravelManagerNamespace::load: planet %s could not be found for row %i", planetName1.c_str (), planetIndex1));
			
			const int numberOfColumns = dataTable.getNumColumns ();
			int column(0);
			for (column = 1; column < numberOfColumns; ++column)
			{
				const int cost = dataTable.getIntValue (column, row);
				if (cost != 0)
				{
					const std::string planetName2 = dataTable.getColumnName (column);
					int planetIndex2(0);
					if (!getPlanetIndex (planetName2, planetIndex2))
						DEBUG_FATAL (true, ("TravelManagerNamespace::load: planet %s could not be found for row %i", planetName2.c_str (), planetIndex2));

					int dummy(0);
					if (!TravelManager::getPlanetSingleHopCost (planetName1, planetName2, dummy))
					{
						if (planetIndex2 < planetIndex1)
							std::swap (planetIndex1, planetIndex2);

						ms_singleHopRouteList[std::make_pair(planetIndex1, planetIndex2)] = cost;

						DEBUG_REPORT_LOG (ms_debugReport, ("Added single hop travel route %s <--> %s costing %i\n", planetName1.c_str (), planetName2.c_str (), cost));
					}
				}
			}
		}

		// setup "least cost" routes, some of which may involve multiple hops
		int routeCost;
		int const numberOfPlanets = ms_planetNameToPlanetIndexList.size();
		int indexPlanet1, indexPlanet2;
		for (indexPlanet1 = 0; indexPlanet1 < numberOfPlanets; ++indexPlanet1)
		{
			for (indexPlanet2 = indexPlanet1; indexPlanet2 < numberOfPlanets; ++indexPlanet2)
			{
				std::multimap<int, std::vector<std::pair<int, int> > > aStarSearchTree;

				// initialize A* search tree with all single hops from indexPlanet1
				for (int i = 0; i < numberOfPlanets; ++i)
				{
					if (getPlanetSingleHopCost(i, indexPlanet1, routeCost))
					{
						std::vector<std::pair<int, int> > route;
						route.push_back(std::make_pair(indexPlanet1, i));

						aStarSearchTree.insert(std::make_pair(routeCost, route));
					}
				}

				// now process the A* search tree
				bool found = false;
				int infiniteLoopSentinel = 0;
				while (true)
				{
					if (aStarSearchTree.empty())
						break;

					if (++infiniteLoopSentinel == 10000)
						break;

					// get the current best route
					std::multimap<int, std::vector<std::pair<int, int> > >::iterator begin = aStarSearchTree.begin();

					// we are done if the current best route ends at the destination
					if (begin->second[begin->second.size() - 1].second == indexPlanet2)
					{
						std::vector<int> routeList;
						for (std::vector<std::pair<int, int> >::const_iterator iter = begin->second.begin(); iter != begin->second.end(); ++iter)
						{
							if (iter != begin->second.begin())
								routeList.push_back(iter->first);
						}

						ms_anyHopLeastCostRouteList[std::make_pair(indexPlanet1, indexPlanet2)] = std::make_pair(begin->first, routeList);

						std::string route = ms_planetNameList[indexPlanet1];
						for (std::vector<int>::const_iterator iter2 = routeList.begin(); iter2 != routeList.end(); ++iter2)
						{
							route += " <--> ";
							route += ms_planetNameList[*iter2];
						}

						route += " <--> ";
						route += ms_planetNameList[indexPlanet2];

						DEBUG_REPORT_LOG (ms_debugReport, ("Added least cost travel route %s costing %i\n", route.c_str (), begin->first));

						if (getPlanetSingleHopCost(indexPlanet1, indexPlanet2, routeCost) && (routeCost > begin->first))
						{
							DEBUG_REPORT_LOG (ms_debugReport, ("Single hop travel route %s <--> %s (%i) cost ***MORE*** than least cost travel route %s (%i)\n", ms_planetNameList[indexPlanet1].c_str(), ms_planetNameList[indexPlanet2].c_str(), routeCost, route.c_str (), begin->first));
						}

						found = true;
						break;
					}

					// otherwise add on new routes that can be reached from the current best route
					int const currentBestRouteCost = begin->first;
					std::vector<std::pair<int, int> > const currentBestRoute = begin->second;

					aStarSearchTree.erase(begin);

					for (int i = 0; i < numberOfPlanets; ++i)
					{
						if ((i != currentBestRoute[currentBestRoute.size() - 1].second) && (getPlanetSingleHopCost(i, currentBestRoute[currentBestRoute.size() - 1].second, routeCost)))
						{
							std::vector<std::pair<int, int> > route = currentBestRoute;
							route.push_back(std::make_pair(currentBestRoute[currentBestRoute.size() - 1].second, i));

							aStarSearchTree.insert(std::make_pair((currentBestRouteCost + routeCost), route));
						}
					}
				}

				if (!found)
					FATAL (true, ("TravelManagerNamespace::load: couldn't find least cost travel route from %s to %s", ms_planetNameList[indexPlanet1].c_str(), ms_planetNameList[indexPlanet2].c_str()));
			}
		}

		// sanity check to make sure all planets have routes to each other
		for (indexPlanet1 = 0; indexPlanet1 < numberOfPlanets; ++indexPlanet1)
		{
			for (indexPlanet2 = 0; indexPlanet2 < numberOfPlanets; ++indexPlanet2)
			{
				FATAL (!TravelManager::getPlanetAnyHopLeastCost(ms_planetNameList[indexPlanet1], ms_planetNameList[indexPlanet2], routeCost), ("TravelManagerNamespace::load: couldn't find least cost travel route from %s to %s", ms_planetNameList[indexPlanet1].c_str(), ms_planetNameList[indexPlanet2].c_str()));
				FATAL (!getPlanetAnyHopLeastCost(indexPlanet1, indexPlanet2, routeCost), ("TravelManagerNamespace::load: couldn't find least cost travel route from %s to %s", ms_planetNameList[indexPlanet1].c_str(), ms_planetNameList[indexPlanet2].c_str()));
			}
		}
	}
}

// ----------------------------------------------------------------------

bool TravelManagerNamespace::getPlanetIndex (const std::string& planetName, int& planetIndex)
{
	PlanetNameToPlanetIndexMap::const_iterator iterFind = ms_planetNameToPlanetIndexList.find(planetName);
	if (iterFind == ms_planetNameToPlanetIndexList.end())
		return false;

	planetIndex = iterFind->second;
	return true;
}

// ----------------------------------------------------------------------

bool TravelManagerNamespace::getPlanetSingleHopCost(int planetIndex1, int planetIndex2, int& planetCost)
{
	if (planetIndex2 < planetIndex1)
		std::swap (planetIndex1, planetIndex2);

	SingleHopRouteList::const_iterator iterFind = ms_singleHopRouteList.find(std::make_pair(planetIndex1, planetIndex2));
	if (iterFind != ms_singleHopRouteList.end())
	{
		planetCost = iterFind->second;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool TravelManagerNamespace::getPlanetAnyHopLeastCost(int planetIndex1, int planetIndex2, int& planetCost)
{
	if (planetIndex2 < planetIndex1)
		std::swap (planetIndex1, planetIndex2);

	AnyHopLeastCostRouteList::const_iterator iterFind = ms_anyHopLeastCostRouteList.find(std::make_pair(planetIndex1, planetIndex2));
	if (iterFind != ms_anyHopLeastCostRouteList.end())
	{
		planetCost = iterFind->second.first;
		return true;
	}

	return false;
}

// ======================================================================
