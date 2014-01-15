// ======================================================================
//
// LineOfSightCache.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LineOfSightCache_H
#define INCLUDED_LineOfSightCache_H

// ======================================================================

class Location;
class Object;

// ======================================================================

class LineOfSightCache
{
public:
	static void update();
	static bool checkLOS(Object const &a, Object const &b);
	static bool checkLOS(Object const &a, Location const &b);
};

// ======================================================================

#endif // INCLUDED_LineOfSightCache_H

