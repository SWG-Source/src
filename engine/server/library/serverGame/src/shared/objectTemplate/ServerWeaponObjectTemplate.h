//========================================================================
//
// ServerWeaponObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerWeaponObjectTemplate_H
#define _INCLUDED_ServerWeaponObjectTemplate_H

#include "ServerTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class ServerWeaponObjectTemplate : public ServerTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerWeaponObjectTemplate_tag = TAG(W,E,A,O)
	};
//@END TFD ID
public:
	         ServerWeaponObjectTemplate(const std::string & filename);
	virtual ~ServerWeaponObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	enum AttackType
	{
		AT_melee,
		AT_ranged,
		AT_thrown,
		AttackType_Last = AT_thrown,
	};

	enum AttribModDecaySpecial
	{
		AMDS_pool = -1,		//            use the attribute pool recovery rate (normal recovery)
		AMDS_wound = -2,		//            can only be healed by skill/item use, heal difficulty adjusted by shock wound value
		AMDS_antidote = -3,		//            used to clear all attrib mods for a given attribute
		AttribModDecaySpecial_Last = AMDS_antidote,
	};

	enum WeaponType
	{
		WT_rifle,
		WT_lightRifle,
		WT_pistol,
		WT_heavyWeapon,
		WT_1handMelee,
		WT_2handMelee,
		WT_unarmed,
		WT_polearm,
		WT_thrown,
		WT_1handLightsaber,
		WT_2handLightsaber,
		WT_polearmLightsaber,
		WeaponType_Last = WT_polearmLightsaber,
	};

public:
	WeaponType     getWeaponType() const;
	AttackType     getAttackType() const;
	DamageType     getDamageType() const;
	DamageType     getElementalType() const;
	int                    getElementalValue() const;
	int                    getElementalValueMin() const;
	int                    getElementalValueMax() const;
	int                    getMinDamageAmount() const;
	int                    getMinDamageAmountMin() const;
	int                    getMinDamageAmountMax() const;
	int                    getMaxDamageAmount() const;
	int                    getMaxDamageAmountMin() const;
	int                    getMaxDamageAmountMax() const;
	float                  getAttackSpeed() const;
	float                  getAttackSpeedMin() const;
	float                  getAttackSpeedMax() const;
	float                  getAudibleRange() const;
	float                  getAudibleRangeMin() const;
	float                  getAudibleRangeMax() const;
	float                  getMinRange() const;
	float                  getMinRangeMin() const;
	float                  getMinRangeMax() const;
	float                  getMaxRange() const;
	float                  getMaxRangeMin() const;
	float                  getMaxRangeMax() const;
	float                  getDamageRadius() const;
	float                  getDamageRadiusMin() const;
	float                  getDamageRadiusMax() const;
	float                  getWoundChance() const;
	float                  getWoundChanceMin() const;
	float                  getWoundChanceMax() const;
	int                    getAttackCost() const;
	int                    getAttackCostMin() const;
	int                    getAttackCostMax() const;
	int                    getAccuracy() const;
	int                    getAccuracyMin() const;
	int                    getAccuracyMax() const;


protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_weaponType;		// General type of weapon
	IntegerParam m_attackType;		// Type of attack this weapon is used with.
	IntegerParam m_damageType;		// The kind of damage this weapon deals.
	IntegerParam m_elementalType;		// The kind of special elemental damage this weapon deals.
	IntegerParam m_elementalValue;		// The amount of special elemental damage this weapon deals.
	IntegerParam m_minDamageAmount;		// The amount of damage done by a weapon is between min-max evenly, randomly, distributed.
	IntegerParam m_maxDamageAmount;		// The amount of damage done by a weapon is between min-max evenly, randomly, distributed.
	FloatParam m_attackSpeed;		// The weapon's intrisic firing rate.
	FloatParam m_audibleRange;		// Distance message gets sent to NPCs when fired.
	FloatParam m_minRange;		// range where the min range mod is set
	FloatParam m_maxRange;		// range where the max range mod is set
	FloatParam m_damageRadius;		// Blast radius for area effect weapons
	FloatParam m_woundChance;		// Base % chance for a wound to occur on a successful attack
	IntegerParam m_attackCost;		// Amount of H/A/M drained by using the weapon
	IntegerParam m_accuracy;		// Accuracy bonus/penalty for this weapon
//@END TFD

public:
	// user functions
	virtual Object * createObject(void) const;

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerWeaponObjectTemplate(const ServerWeaponObjectTemplate &);
	ServerWeaponObjectTemplate & operator =(const ServerWeaponObjectTemplate &);
};


inline void ServerWeaponObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerWeaponObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerWeaponObjectTemplate_H
