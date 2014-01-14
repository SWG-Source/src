//========================================================================
//
// SharedWeaponObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedWeaponObjectTemplate_H
#define _INCLUDED_SharedWeaponObjectTemplate_H

#include "SharedTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class SharedWeaponObjectTemplate : public SharedTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedWeaponObjectTemplate_tag = TAG(S,W,O,T)
	};
//@END TFD ID
public:
	         SharedWeaponObjectTemplate(const std::string & filename);
	virtual ~SharedWeaponObjectTemplate();

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
		AT_ammo,
		AttackType_Last = AT_ammo,
	};

public:
	const std::string &    getWeaponEffect(bool testData = false) const;
	int                    getWeaponEffectIndex(bool testData = false) const;
	int                    getWeaponEffectIndexMin(bool testData = false) const;
	int                    getWeaponEffectIndexMax(bool testData = false) const;
	AttackType     getAttackType(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	StringParam m_weaponEffect;		// The id lookup into the weapon data table for the bolt and combat effects.
	IntegerParam m_weaponEffectIndex;		// The index 0-4 for a specific weaponEffect in the weapon data table.
	IntegerParam m_attackType;		// Type of attack this weapon is used with.
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedWeaponObjectTemplate(const SharedWeaponObjectTemplate &);
	SharedWeaponObjectTemplate & operator =(const SharedWeaponObjectTemplate &);
};


inline void SharedWeaponObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	SharedWeaponObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedWeaponObjectTemplate_H
