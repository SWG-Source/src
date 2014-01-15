// ======================================================================
//
// ShipAiEnemySearchManager.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipAiEnemySearchManager_H
#define INCLUDED_ShipAiEnemySearchManager_H

// ======================================================================

class ShipObject;

// ======================================================================

class ShipAiEnemySearchManager
{
public:
	static void update();
	static void add(ShipObject &ship);
};

// ======================================================================

#endif // INCLUDED_ShipAiEnemySearchManager_H

