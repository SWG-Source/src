// ======================================================================
//
// GroupPickupPoint.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_GroupPickupPoint_H
#define INCLUDED_GroupPickupPoint_H

class Object;

// ======================================================================

class GroupPickupPoint // static class
{
public:

	static bool isGroupPickupAllowedAtLocation(std::string const & planetName, Object const & object);
	static bool isGroupPickupAllowedAtLocation(std::string const & planetName, int x, int z);
	static bool getGroupPickupTravelCost(const std::string& sourcePlanet, const std::string& targetPlanet, int& cost);

private: // disabled

	GroupPickupPoint();
	GroupPickupPoint(GroupPickupPoint const &);
	GroupPickupPoint &operator =(GroupPickupPoint const &);
};

// ======================================================================

#endif // INCLUDED_GroupPickupPoint_H
