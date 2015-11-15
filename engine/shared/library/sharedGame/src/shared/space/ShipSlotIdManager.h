// ======================================================================
//
// ShipSlotIdManager.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipSlotIdManager_H
#define INCLUDED_ShipSlotIdManager_H

// ======================================================================

class SlotId;

// ======================================================================

class ShipSlotIdManager
{
public:
	static void install();

	static SlotId const &getShipPilotSlotId();
	static SlotId const &getPobShipPilotSlotId();
	static SlotId const &getShipOperationsSlotId();
	static SlotId const &getPobShipOperationsSlotId();
	static SlotId const &getShipDroidSlotId();
	static SlotId const &getShipGunnerSlotId(int weaponIndex);
	static SlotId const &getPobShipGunnerSlotId(int weaponIndex);
	static bool isGunnerSlot(SlotId const &slotId);
	static int getGunnerSlotWeaponIndex(SlotId const &slotId);
};

// ======================================================================

#endif // INCLUDED_ShipSlotIdManager_H

