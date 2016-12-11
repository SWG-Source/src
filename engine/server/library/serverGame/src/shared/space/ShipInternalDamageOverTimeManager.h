//======================================================================
//
// ShipInternalDamageOverTimeManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipInternalDamageOverTimeManager_H
#define INCLUDED_ShipInternalDamageOverTimeManager_H

//======================================================================

class ShipInternalDamageOverTime;
class ShipObject;

//----------------------------------------------------------------------

class ShipInternalDamageOverTimeManager
{
public:

	typedef std::vector<ShipInternalDamageOverTime> IdotVector;

	static void install();
	static void remove();
	static void update(float elapsedTime);

	static bool setEntry(ShipObject const & ship, int chassisSlot, float damageRate, float damageThreshold);
	static bool removeEntry(ShipObject const & ship, int chassisSlot);
	static ShipInternalDamageOverTime const * const findEntry(ShipObject const & ship, int chassisSlot);

	static IdotVector const & getActiveShipInternalDamageOverTime();
};

//======================================================================

#endif
