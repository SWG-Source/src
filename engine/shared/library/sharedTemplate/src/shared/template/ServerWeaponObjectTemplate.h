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
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


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
	static void install(void);

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
		WT_groundTargetting,
		WT_directionTargetting,
		WeaponType_Last = WT_directionTargetting,
	};

public:
	virtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);
	virtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);
	virtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);
	virtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);
	virtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);
	virtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);
	virtual void initStructParamOT(StructParamOT &param, const char *name);
	virtual void setAsEmptyList(const char *name);
	virtual void setAppend(const char *name);
	virtual bool isAppend(const char *name) const;
	virtual int getListLength(const char *name) const;

protected:
	virtual void load(Iff &file);
	virtual void save(Iff &file);

private:
	CompilerIntegerParam m_weaponType;		// General type of weapon
	CompilerIntegerParam m_attackType;		// Type of attack this weapon is used with.
	CompilerIntegerParam m_damageType;		// The kind of damage this weapon deals.
	CompilerIntegerParam m_elementalType;		// The kind of special elemental damage this weapon deals.
	CompilerIntegerParam m_elementalValue;		// The amount of special elemental damage this weapon deals.
	CompilerIntegerParam m_minDamageAmount;		// The amount of damage done by a weapon is between min-max evenly, randomly, distributed.
	CompilerIntegerParam m_maxDamageAmount;		// The amount of damage done by a weapon is between min-max evenly, randomly, distributed.
	FloatParam m_attackSpeed;		// The weapon's intrisic firing rate.
	FloatParam m_audibleRange;		// Distance message gets sent to NPCs when fired.
	FloatParam m_minRange;		// range where the min range mod is set
	FloatParam m_maxRange;		// range where the max range mod is set
	FloatParam m_damageRadius;		// Blast radius for area effect weapons
	FloatParam m_woundChance;		// Base % chance for a wound to occur on a successful attack
	CompilerIntegerParam m_attackCost;		// Amount of H/A/M drained by using the weapon
	CompilerIntegerParam m_accuracy;		// Accuracy bonus/penalty for this weapon
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerWeaponObjectTemplate(const ServerWeaponObjectTemplate &);
	ServerWeaponObjectTemplate & operator =(const ServerWeaponObjectTemplate &);
};


inline void ServerWeaponObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	ServerWeaponObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerWeaponObjectTemplate_H
