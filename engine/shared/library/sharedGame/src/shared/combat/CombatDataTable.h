// ======================================================================
//
// CombatDataTable.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_CombatDataTable_H
#define INCLUDED_CombatDataTable_H

// ======================================================================

class CombatDataTable // static class
{
public:
	// the order and value of these are tied to the order and value that
	// are displayed in the drop down list in the chat option window
	enum CombatSpamFilterType
	{
		CSFT_All = 0,
		CSFT_Self,
		CSFT_Group,
		CSFT_None
	};

	enum ValidTargetType
	{
		VTT_none       =-1,
		VTT_standard   = 0,
		VTT_mob        = 1,
		VTT_creature   = 2,
		VTT_npc        = 3,
		VTT_droid      = 4,
		VTT_pvp        = 5,
		VTT_jedi       = 6,
		VTT_dead       = 7,
		VTT_friend     = 8
	};

	enum AttackType
	{
		AT_none            =-1,
		AT_cone            = 0,
		AT_singleTarget    = 1,
		AT_area            = 2,
		AT_targetArea      = 3,
		AT_dualWield       = 4
	};

	enum HitType
	{
		HT_attack           =-1,
		HT_nonAttack        = 0,
		HT_nonDamageAttack  = 4,
		HT_heal             = 5,
		HT_delayAttack      = 6,
		HT_revive           = 7
	};

	static void           install();

	static bool           isActionClientAnim(uint32 commandHash);
	static std::string    getActionAnimationData(uint32 commandHash, char const * const weaponType);
	static bool           actionForcesCharacterIntoCombat(uint32 commandHash);
	static int32          getAttackType(uint32 commandHash);
	static int32          getHitType(uint32 commandHash);
	static int32          getValidTarget(uint32 commandHash);
	static float          getMinRange(uint32 commandHash);
	static float          getMaxRange(uint32 commandHash);
	static float          getActionCost(uint32 commandHash);
	static float          getMindCost(uint32 commandHash);
	static bool           getCancelsAutoAttack(uint32 commandHash);
	static float          getPercentAddFromWeapon(uint32 commandHash);
	static std::string    getSpecialLine(uint32 commandHash);
	static int            getMinInvisLevelRequired(uint32 commandHash);
	static int			  getTrailBits(uint32 commandHash);

private: // disabled

	CombatDataTable();
	CombatDataTable(CombatDataTable const &);
	CombatDataTable &operator =(CombatDataTable const &);
};

// ======================================================================

#endif // INCLUDED_CombatDataTable_H
