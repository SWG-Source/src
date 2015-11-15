// ======================================================================
//
// ShipClientUpdateTracker.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipClientUpdateTracker_H
#define INCLUDED_ShipClientUpdateTracker_H

// ======================================================================

class Client;
class ShipObject;

// ======================================================================

class ShipClientUpdateTracker
{
public:
	static void install();
	static void update(float elapsedTime);
	static void queueForUpdate(Client &client, ShipObject const &ship);
	static void onClientDestroyed(Client &client);
	static unsigned int getShipUpdatePriorityValue(Client const &client, ShipObject const &ship);
};

// ======================================================================

#endif // INCLUDED_ShipClientUpdateTracker_H

