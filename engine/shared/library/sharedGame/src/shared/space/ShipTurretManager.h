// ======================================================================
//
// ShipTurretManager.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipTurretManager_H
#define INCLUDED_ShipTurretManager_H

// ======================================================================

class ShipTurretManager
{
public:
	static void install();

	static bool isTurret(uint32 chassisCrc, int weaponIndex);
	static float getTurretMinYaw(uint32 chassisCrc, int weaponIndex);
	static float getTurretMaxYaw(uint32 chassisCrc, int weaponIndex);
	static float getTurretMinPitch(uint32 chassisCrc, int weaponIndex);
	static float getTurretMaxPitch(uint32 chassisCrc, int weaponIndex);
};

// ======================================================================

#endif // INCLUDED_ShipTurretManager_H
