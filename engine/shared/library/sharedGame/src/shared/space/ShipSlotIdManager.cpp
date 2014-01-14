// ======================================================================
//
// ShipSlotIdManager.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipSlotIdManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"

// ======================================================================

namespace ShipSlotIdManagerNamespace
{

	SlotId s_shipPilotSlotId;
	SlotId s_pobShipPilotSlotId;
	SlotId s_shipOperationsSlotId;
	SlotId s_pobShipOperationsSlotId;
	SlotId s_shipGunnerSlotIds[ShipChassisSlotType::cms_numWeaponIndices];
	SlotId s_pobShipGunnerSlotIds[ShipChassisSlotType::cms_numWeaponIndices];
	SlotId s_shipDroidSlotId;

	bool s_installed = false;

}

using namespace ShipSlotIdManagerNamespace;

// ======================================================================

void ShipSlotIdManager::install()
{
	InstallTimer const installTimer("ShipSlotIdManager::install");

	if (SlotIdManager::isInstalled())
	{
		s_shipPilotSlotId = SlotIdManager::findSlotId(ConstCharCrcLowerString("ship_pilot"));
		s_pobShipPilotSlotId = SlotIdManager::findSlotId(ConstCharCrcLowerString("ship_pilot_pob"));
		s_shipOperationsSlotId = SlotIdManager::findSlotId(ConstCharCrcLowerString("ship_operations"));
		s_pobShipOperationsSlotId = SlotIdManager::findSlotId(ConstCharCrcLowerString("ship_operations_pob"));

		for (int i = 0; i < ShipChassisSlotType::cms_numWeaponIndices; ++i)
		{
			char buf[32];
			IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "ship_gunner%d", i));
			buf[sizeof(buf)-1] = '\0';
			s_shipGunnerSlotIds[i] = SlotIdManager::findSlotId(ConstCharCrcLowerString(buf));
			IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "ship_gunner%d_pob", i));
			buf[sizeof(buf)-1] = '\0';
			s_pobShipGunnerSlotIds[i] = SlotIdManager::findSlotId(ConstCharCrcLowerString(buf));
		}

		s_shipDroidSlotId = SlotIdManager::findSlotId(ConstCharCrcLowerString("ship_droid"));

		s_installed = true;
	}
}

// ----------------------------------------------------------------------

SlotId const &ShipSlotIdManager::getShipPilotSlotId()
{
	DEBUG_FATAL(!s_installed, ("ShipSlotIdManager was not installed, or sharedGame was installed before SharedObject"));
	return s_shipPilotSlotId;
}

// ----------------------------------------------------------------------

SlotId const &ShipSlotIdManager::getPobShipPilotSlotId()
{
	DEBUG_FATAL(!s_installed, ("ShipSlotIdManager was not installed, or sharedGame was installed before SharedObject"));
	return s_pobShipPilotSlotId;
}

// ----------------------------------------------------------------------

SlotId const &ShipSlotIdManager::getShipOperationsSlotId()
{
	DEBUG_FATAL(!s_installed, ("ShipSlotIdManager was not installed, or sharedGame was installed before SharedObject"));
	return s_shipOperationsSlotId;
}

// ----------------------------------------------------------------------

SlotId const &ShipSlotIdManager::getPobShipOperationsSlotId()
{
	DEBUG_FATAL(!s_installed, ("ShipSlotIdManager was not installed, or sharedGame was installed before SharedObject"));
	return s_pobShipOperationsSlotId;
}

// ----------------------------------------------------------------------

SlotId const &ShipSlotIdManager::getShipDroidSlotId()
{
	DEBUG_FATAL(!s_installed, ("ShipSlotIdManager was not installed, or sharedGame was installed before SharedObject"));
	return s_shipDroidSlotId;
}

// ----------------------------------------------------------------------

SlotId const &ShipSlotIdManager::getShipGunnerSlotId(int weaponIndex)
{
	DEBUG_FATAL(!s_installed, ("ShipSlotIdManager was not installed, or sharedGame was installed before SharedObject"));
	FATAL(weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices, ("tried to get gunner slot id for invalid weaponIndex"));
	return s_shipGunnerSlotIds[weaponIndex];
}

// ----------------------------------------------------------------------

SlotId const &ShipSlotIdManager::getPobShipGunnerSlotId(int weaponIndex)
{
	DEBUG_FATAL(!s_installed, ("ShipSlotIdManager was not installed, or sharedGame was installed before SharedObject"));
	FATAL(weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices, ("tried to get gunner slot id for invalid weaponIndex"));
	return s_pobShipGunnerSlotIds[weaponIndex];
}

// ----------------------------------------------------------------------

int ShipSlotIdManager::getGunnerSlotWeaponIndex(SlotId const &slotId)
{
	DEBUG_FATAL(!s_installed, ("ShipSlotIdManager was not installed, or sharedGame was installed before SharedObject"));
	for (int i = 0; i < ShipChassisSlotType::cms_numWeaponIndices; ++i)
		if (s_shipGunnerSlotIds[i] == slotId || s_pobShipGunnerSlotIds[i] == slotId)
			return i;
	return -1;
}

// ----------------------------------------------------------------------

bool ShipSlotIdManager::isGunnerSlot(SlotId const &slotId)
{
	return getGunnerSlotWeaponIndex(slotId) != -1;
}

// ======================================================================

