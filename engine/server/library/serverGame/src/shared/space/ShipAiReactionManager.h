// ======================================================================
//
// ShipAiReactionManager.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipAiReactionManager_H
#define INCLUDED_ShipAiReactionManager_H

// ======================================================================

class PersistentCrcString;
class ShipObject;

// ======================================================================

class ShipAiReactionManager
{
public:

	enum ShipClass
	{
		SC_fighter = 0,
		SC_bomber,
		SC_capitalShip,
		SC_transport,
		SC_invalid
	};

	static void install();

	static char const * getShipClassString(ShipClass const shipClass);
	static void setShipSpaceMobileType(ShipObject &ship, PersistentCrcString const &spaceMobileType);
	static bool isAlly(ShipObject const &actor, ShipObject const &target);
	static bool isEnemy(ShipObject const &actor, ShipObject const &target);
	static ShipClass getShipClass(PersistentCrcString const & spaceMobileType);
};

// ======================================================================

#endif // INCLUDED_ShipAiReactionManager_H

