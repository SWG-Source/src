//========================================================================
//
// CombatEngineData.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_CombatEngineData_H
#define _INCLUDED_CombatEngineData_H

#include "sharedGame/AttribMod.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgSharedUtility/Attributes.def"
#include "swgSharedUtility/Postures.def"

#include <vector>

class MessageQueueCombatAction;
class TangibleObject;
class ServerObject;
class WeaponObject;

//-------------------------------------------------------------------

namespace ConfigCombatEngineData
{
	struct BodyAttackMod;
};

//-------------------------------------------------------------------
// CombatEngineData - data attached to every combatant

namespace CombatEngineData
{
	typedef std::vector<CachedNetworkId> TargetIdList;

	// results of a defense
	// NOTE: make sure CombatEngineData.cpp name-string table is updated when these values change.
	//       Also, the client combat manager needs to update a map if these values change.
	enum CombatDefense
	{
		CD_miss = 0,                // the defender didn't do anything special, the attack just missed
		CD_hit,                     // the defender was hit
		CD_block,                   // the defender blocked with his weapon/body
		CD_evade,                   // the defender moved out of the way
		CD_redirect,                // the defender redirected the attack out of the way
		CD_counterAttack,           // the defender counter attacked, results of attack to follow
		CD_fumble,                  // the attacker fumbled
		CD_lightsaberBlock,         // block caused by a lightsaber (deflection)
		CD_lightsaberCounter,       // counterattack caused by a lightsaber deflection.
		CD_lightsaberCounterTarget, // counterattack to jedi's target caused by a lightsaber deflection.

		CD_numCombatDefense
	};

	// the following enums are mirrored in ServerWeaponObjectTemplate
	enum AttackType
	{
		AT_melee,
		AT_ranged,
		AT_thrown,
		AT_ammo,
		AttackType_Last = AT_ammo,
	};

	enum DamageType
	{
		DT_none = 0x00000000,
		DT_kinetic = 0x00000001,
		DT_energy = 0x00000002,
		DT_blast = 0x00000004,
		DT_stun = 0x00000008,
		DT_restraint = 0x00000010,
		DT_elemental_heat = 0x00000020,
		DT_elemental_cold = 0x00000040,
		DT_elemental_acid = 0x00000080,
		DT_elemental_eletrical = 0x00000100,
		DT_environmental_heat = 0x00000200,
		DT_environmental_cold = 0x00000400,
		DT_environmental_acid = 0x00000800,
		DT_environmental_electrical = 0x00001000
	};

	struct ActionItem
	{
		enum Actions
		{
			none,
			target,
			attack,
			useSkill,
			aim,
			changePosture,
			changeAttitude,
			reloadWeapon,
			surrender
		};

		Actions type;			// what we are doing
		bool targetSelf;		// the action applies to me and not my target(s)
		union
		{
			struct
			{
				int numTargets;	// number of targets
				//@todo make these NetworkIds
				NetworkId::NetworkIdType target;		// if only one target is given
				NetworkId::NetworkIdType *targets;	// for multiple targets
			} targetData;

			struct
			{
				//@todo make this a NetworkId
				NetworkId::NetworkIdType weapon;		// if 0, use attacker's primary weapon
				int mode;		// 0 = primary, 1 = secondary, etc
			} attackData;

			int attitudeData;
			Postures::Enumerator postureData;
		} actionData;

		uint32 sequenceId;

		ActionItem(void);
		~ActionItem(void);
	};

	struct AttackData
	{
		int                   aims;         // number of aims taken
		AttackData(void);
	};

	struct DamageData
	{
		DamageData(void);

		std::vector<AttribMod::AttribMod> damage;// list of attribute modifiers this damage
												 // caused, pre armor effectiveness
		CachedNetworkId            attackerId;   // who caused the damage (nullptr for
												 // environmental effects, etc)
		NetworkId                  weaponId;     // id of the weapon used
		DamageType                 damageType;
		uint16                     hitLocationIndex;
		uint16                     actionId;
		bool                       wounded;
		bool                       ignoreInvulnerable;
		//		MessageQueueCombatAction * combatActionMessage;
	};

	struct DefenseData
	{
		std::vector<DamageData>  damage;       // list of damage I have taken this
											   // timeslice
	};

	struct CombatData
	{
		AttackData attackData;
		DefenseData defenseData;
	};

	//--------------------------------------------------
	// CombatEngineData inline functions

	inline ActionItem::ActionItem(void)
	{
		type = none;
		memset(&actionData, 0, sizeof(actionData));
		sequenceId = 0;
		targetSelf = 0;
	}	// ActionItem::ActionItem

	inline ActionItem::~ActionItem(void)
	{
		if (type == target && actionData.targetData.targets != nullptr)
		{
			delete[] actionData.targetData.targets;
			actionData.targetData.targets = nullptr;
		}
	}	// ActionItem::~ActionItem

	inline AttackData::AttackData(void) :
		aims(0)
	{
	}	// AttackData::AttackData

	inline DamageData::DamageData(void) :
		damage(),
		attackerId(),
		damageType(DT_kinetic),
		hitLocationIndex(0),
		actionId(0),
		wounded(false),
		ignoreInvulnerable(false)
		//		combatActionMessage(nullptr)
	{
	}

	const char *const getCombatDefenseName(CombatDefense combatDefense);
};

#endif	// _INCLUDED_CombatEngineData_H
