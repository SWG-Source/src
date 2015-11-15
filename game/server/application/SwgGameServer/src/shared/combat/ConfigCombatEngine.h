//========================================================================
//
// ConfigCombatEngine.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef _INCLUDED_ConfigCombatEngine_H
#define _INCLUDED_ConfigCombatEngine_H

#include "serverGame/ServerArmorTemplate.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "swgSharedUtility/CombatEngineData.h"
#include "swgSharedUtility/Attributes.def"


//-------------------------------------------------------------------
// namespace ConfigCombatEngineData

namespace ConfigCombatEngineData
{
	struct BodyAttackMod
	{
		std::string  name;
		std::string  modelBoneName;			// name of a 3d-model bone to associate with
		uint32       combatSkeletonBone;
		int          toHitChance;
		float        toWoundBonus;
		float        damageBonus[Attributes::NumberOfAttributes];
	};
};//lint !e19 Useless declaration 


//-------------------------------------------------------------------
// class ConfigCombatEngine

class ConfigCombatEngine
{
public:

	struct AggressionMod
	{
		float time;
		int toHit;
	};

	struct SkeletonAttackMod
	{
		std::string name;
		int numHitLocations;
		std::string script;
		std::vector<ConfigCombatEngineData::BodyAttackMod> attackMods;
	};

    struct Data
    {
		int maxAims;
		int numberSkeletons;
		std::vector<SkeletonAttackMod> skeletonAttackMods;
		float kineticObjectDamage;
		float energyObjectDamage;
		float blastObjectDamage;
		float stunObjectDamage;
		float restraintObjectDamage;
		float elementalObjectDamage;
		float weaponLowerThanArmorRating;
		float weaponHigherThanArmorRating;
		float woundChance;
		float woundPercent;
		float shockWoundPercent;
		int numCombatStates;

		int debugQueues;
		int debugTargeting;
		int debugAttack;
		int debugDefense;
		int debugDamage;
		int debugExplosion;
    };

public:

    static void				install						(void);
    static void				remove						(void);

	static int                                    getMaxAims(void);
	static int                                    getNumberSkeletons(void);
	static const SkeletonAttackMod &              getSkeletonAttackMod(ServerTangibleObjectTemplate::CombatSkeleton skeleton);
	static float                                  getKineticObjectDamage(void);
	static float                                  getEnergyObjectDamage(void);
	static float                                  getBlastObjectDamage(void);
	static float                                  getStunObjectDamage(void);
	static float                                  getRestraintObjectDamage(void);
	static float                                  getElementalObjectDamage(void);
	static float                                  getObjectDamageSpread(ServerWeaponObjectTemplate::DamageType damageType);
	static float                                  getWoundChance(void);
	static float                                  getWoundPercent(void);
	static float                                  getShockWoundPercent(void);

	static int                                    getDebugQueues(void);
	static int                                    getDebugTargeting(void);
	static int                                    getDebugAttack(void);
	static int                                    getDebugDefense(void);
	static int                                    getDebugDamage(void);
	static int                                    getDebugExplosion(void);

private:

    static Data                                      m_data;
//	static ConfigCombatEngineData::BodyAttackMod     m_bodyAttackMod;
//	static SkeletonAttackMod                         m_skeletonAttackMod;
	static float                                     m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::ArmorRating_Last+1][ServerArmorTemplate::ArmorRating_Last+1]; //lint !e641 converting enum to int

	static void installFromConfigFile(void);
	static void parseVersion0(Iff & file);
	static void setupRatingDifferenceDamageReduction(void);
};


//-----------------------------------------------------------------------

inline int ConfigCombatEngine::getMaxAims(void)
{
	return m_data.maxAims;
}	// ConfigCombatEngine::getMaxAims

inline int ConfigCombatEngine::getNumberSkeletons(void)
{
	return m_data.numberSkeletons;
}	// ConfigCombatEngine::getNumberSkeletons

inline const ConfigCombatEngine::SkeletonAttackMod & 
	ConfigCombatEngine::getSkeletonAttackMod(
	ServerTangibleObjectTemplate::CombatSkeleton skeleton)
{
	int sk = static_cast<int>(skeleton);
	if (sk < 0 || static_cast<size_t>(skeleton) >= 
		m_data.skeletonAttackMods.size())
	{
		WARNING_STRICT_FATAL(true, ("ConfigCombatEngine::SkeletonAttackMod "
			"requested invalid skeleton %d", skeleton));
		// if we're not strict, use the default skeleton values
		skeleton = ServerTangibleObjectTemplate::CS_none;
	}
	return m_data.skeletonAttackMods[skeleton]; //lint !e641 Converting enum CombatSkeleton to int
}	// ConfigCombatEngine::getSkeletonAttackMods

inline float ConfigCombatEngine::getKineticObjectDamage(void)
{
	return m_data.kineticObjectDamage;
}	// ConfigCombatEngine::getKineticObjectDamage

inline float ConfigCombatEngine::getEnergyObjectDamage(void)
{
	return m_data.energyObjectDamage;
}	// ConfigCombatEngine::getEnergyObjectDamage

inline float ConfigCombatEngine::getBlastObjectDamage(void)
{
	return m_data.blastObjectDamage;
}	// ConfigCombatEngine::getBlastObjectDamage

inline float ConfigCombatEngine::getStunObjectDamage(void)
{
	return m_data.stunObjectDamage;
}	// ConfigCombatEngine::getStunObjectDamage

inline float ConfigCombatEngine::getRestraintObjectDamage(void)
{
	return m_data.restraintObjectDamage;
}	// ConfigCombatEngine::getRestraintObjectDamage

inline float ConfigCombatEngine::getElementalObjectDamage(void)
{
	return m_data.elementalObjectDamage;
}	// ConfigCombatEngine::getElementalObjectDamage

inline float ConfigCombatEngine::getWoundChance(void)
{
	return m_data.woundChance;
}	// ConfigCombatEngine::getWoundChance

inline float ConfigCombatEngine::getWoundPercent(void)
{
	return m_data.woundPercent;
}	// ConfigCombatEngine::getWoundPercent

inline float ConfigCombatEngine::getShockWoundPercent(void)
{
	return m_data.shockWoundPercent;
}	// ConfigCombatEngine::getShockWoundPercent

inline int ConfigCombatEngine::getDebugQueues(void)
{
	return m_data.debugQueues;
}	// ConfigCombatEngine::getDebugQueues

inline int ConfigCombatEngine::getDebugTargeting(void)
{
	return m_data.debugTargeting;
}	// ConfigCombatEngine::getDebugTargeting

inline int ConfigCombatEngine::getDebugAttack(void)
{
	return m_data.debugAttack;
}	// ConfigCombatEngine::getDebugAttack

inline int ConfigCombatEngine::getDebugDefense(void)
{
	return m_data.debugDefense;
}	// ConfigCombatEngine::getDebugDefense

inline int ConfigCombatEngine::getDebugDamage(void)
{
	return m_data.debugDamage;
}	// ConfigCombatEngine::getDebugDamage

inline int ConfigCombatEngine::getDebugExplosion(void)
{
	return m_data.debugExplosion;
}	// ConfigCombatEngine::getDebugExplosion

//-----------------------------------------------------------------------


#endif	// _INCLUDED_ConfigCombatEngine_H
