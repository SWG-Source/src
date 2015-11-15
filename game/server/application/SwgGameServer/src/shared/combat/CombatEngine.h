//========================================================================
//
// CombatEngine.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef _INCLUDED_CombatEngine_H
#define _INCLUDED_CombatEngine_H

#include "sharedGame/AttribMod.h"
#include "sharedObject/CachedNetworkId.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "swgSharedUtility/CombatEngineData.h"
#include <queue>

class Command;
class CreatureObject;


//-------------------------------------------------------------------
// class CombatEngine

class CombatEngine
{
public:
	typedef std::vector<AttribMod::AttribMod> DamageList;

public:

	static void install(void);
	static void remove(void);

	static bool reloadCombatData(void);

	// command queue hooks
	static void aim(const Command & command, const NetworkId & actor,
		const NetworkId & target, const Unicode::String & params);

	// queue actions
	static bool addTargetAction(TangibleObject & attacker, const CombatEngineData::TargetIdList & targets);
	static bool addAttackAction(TangibleObject & attacker, const NetworkId & weapon, int weaponMode);
	static bool addAimAction(TangibleObject & attacker);

	static bool onSuccessfulAttack(const TangibleObject & attacker, TangibleObject & defender, const WeaponObject & weapon, int damage, int hitLocation);
	static bool onSuccessfulAttack(const TangibleObject & attacker, TangibleObject & defender, int damage, int hitLocation);
	static bool damage(TangibleObject & defender, const WeaponObject & weapon, int damageAmount, int hitLocation);
	static void damage(TangibleObject & defender, ServerWeaponObjectTemplate::DamageType damageType, uint16 hitLocation, int damage);
	static void damage(const Vector &center, float radius, ServerWeaponObjectTemplate::DamageType damageType, int damage);
	static void alter(TangibleObject & object);

private:

	static uint16 ms_nextActionId;		// id to link attack messages with damage messages
	static uint16 getNextActionId(void);

	static void computeCreatureDamage(
		const ConfigCombatEngineData::BodyAttackMod *hitLocation,
		int damage, DamageList & damageList);
	static void computeObjectDamage(
		const ConfigCombatEngineData::BodyAttackMod *hitLocation,
		int damage, DamageList & damageList);
};


inline uint16 CombatEngine::getNextActionId(void)
{
	return ++ms_nextActionId;
}


#endif	// _INCLUDED_CombatEngine_H
