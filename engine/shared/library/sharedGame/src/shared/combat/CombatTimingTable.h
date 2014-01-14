// ======================================================================
//
// CombatTimingTable.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CombatTimingTable_H
#define INCLUDED_CombatTimingTable_H

// ======================================================================

class CombatTimingTable // static class
{
public:
	static int         getMaximumShotsTillWeaponReload(std::string const & weaponType);
	static bool        isContinuous(std::string const & weaponType);
	static float       getWeaponReloadTimeSeconds(std::string const & weaponType);
	static const char *getWeaponReloadClientEffect(std::string const & weaponType);
	static const char *getWeaponReloadClientAnimation(std::string const & weaponType);
};

// ======================================================================

#endif // INCLUDED_CombatTimingTable_H
