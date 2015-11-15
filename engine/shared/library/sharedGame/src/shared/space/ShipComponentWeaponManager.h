//======================================================================
//
// ShipComponentWeaponManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentWeaponManager_H
#define INCLUDED_ShipComponentWeaponManager_H

//======================================================================

class ShipComponentWeaponManager
{
public:

	enum Flags
	{
		F_ammoConsuming   = 0x01,
		F_missile         = 0x02,
		F_countermeasure  = 0x04,
		F_mining          = 0x08,
		F_tractor         = 0x10,
		F_beam            = 0x20
	};

	static void install();
	static void remove();

	static int getProjectileIndex(uint32 componentCrc);
	static bool isAmmoConsuming(uint32 componentCrc);
	static bool isMissile(uint32 componentCrc);
	static bool isCountermeasure(uint32 componentCrc);
	static bool isMining(uint32 componentCrc);
	static bool isTractor(uint32 componentCrc);
	
	static bool hasFlags(uint32 componentCrc, int flags);

	static float getRange(uint32 componentCrc);
	static float getProjectileSpeed(uint32 componentCrc);
};

//======================================================================

#endif
