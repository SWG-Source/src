//========================================================================
//
// ServerArmorTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerArmorTemplate_H
#define _INCLUDED_ServerArmorTemplate_H

#include "sharedObject/ObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Armor;
class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class ServerArmorTemplate : public ObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerArmorTemplate_tag = TAG(A,R,M,O)
	};
//@END TFD ID
public:
	         ServerArmorTemplate(const std::string & filename);
	virtual ~ServerArmorTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	enum ArmorRating
	{
		AR_armorRealNone = -1,		// 			needed for internal reasons, do not use in templates! This means you!
		AR_armorNone = 0,
		AR_armorLight,
		AR_armorMedium,
		AR_armorHeavy,
		ArmorRating_Last = AR_armorHeavy,
	};

	enum DamageType
	{
		DT_kinetic = 0x00000001,
		DT_energy = 0x00000002,
		DT_blast = 0x00000004,
		DT_stun = 0x00000008,
		DT_restraint = 0x00000010,
		DT_elemental_heat = 0x00000020,
		DT_elemental_cold = 0x00000040,
		DT_elemental_acid = 0x00000080,
		DT_elemental_electrical = 0x00000100,
		DT_environmental_heat = 0x00000200,
		DT_environmental_cold = 0x00000400,
		DT_environmental_acid = 0x00000800,
		DT_environmental_electrical = 0x00001000,
		DamageType_Last = DT_environmental_electrical,
	};

public:
	struct SpecialProtection
	{
		enum DamageType type;
		int                    effectiveness;
	};

protected:
	class _SpecialProtection : public ObjectTemplate
	{
		friend class ServerArmorTemplate;
	public:
		enum
		{
			_SpecialProtection_tag = TAG(A,R,S,P)
		};

	public:
		         _SpecialProtection(const std::string & filename);
		virtual ~_SpecialProtection();

	virtual Tag getId(void) const;

public:
	DamageType     getType(bool versionOk, bool testData = false) const;
	int                    getEffectiveness(bool versionOk, bool testData = false) const;
	int                    getEffectivenessMin(bool versionOk, bool testData = false) const;
	int                    getEffectivenessMax(bool versionOk, bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_type;		// specific damage being protected from
	IntegerParam m_effectiveness;		// armor effectiveness ( <0 = no protection for this damage type, default protection will be ignored)
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_SpecialProtection(const _SpecialProtection &);
		_SpecialProtection & operator =(const _SpecialProtection &);
	};
	friend class ServerArmorTemplate::_SpecialProtection;

public:
	ArmorRating     getRating(bool testData = false) const;
	int                    getIntegrity(bool testData = false) const;
	int                    getIntegrityMin(bool testData = false) const;
	int                    getIntegrityMax(bool testData = false) const;
	int                    getEffectiveness(bool testData = false) const;
	int                    getEffectivenessMin(bool testData = false) const;
	int                    getEffectivenessMax(bool testData = false) const;
	void              getSpecialProtection(SpecialProtection &data, int index) const;
	void              getSpecialProtectionMin(SpecialProtection &data, int index) const;
	void              getSpecialProtectionMax(SpecialProtection &data, int index) const;
	size_t            getSpecialProtectionCount(void) const;
	int                    getVulnerability(bool testData = false) const;
	int                    getVulnerabilityMin(bool testData = false) const;
	int                    getVulnerabilityMax(bool testData = false) const;
	int                    getEncumbrance(int index) const;
	int                    getEncumbranceMin(int index) const;
	int                    getEncumbranceMax(int index) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_rating;		// armor rating
	IntegerParam m_integrity;		// integrity
	IntegerParam m_effectiveness;		// default effectiveness (0 = only use special protection)
	std::vector<StructParamOT *> m_specialProtection;		// damage-type specific protection
	bool m_specialProtectionLoaded;
	bool m_specialProtectionAppend;
	IntegerParam m_vulnerability;		// damaga types that this armor doesn't protect against
	IntegerParam m_encumbrance[3];		// reduction to attributes from wearing this armor
//@END TFD

public:
	// user functions


private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerArmorTemplate(const ServerArmorTemplate &);
	ServerArmorTemplate & operator =(const ServerArmorTemplate &);
};


inline void ServerArmorTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerArmorTemplate::registerMe();
	ServerArmorTemplate::_SpecialProtection::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerArmorTemplate_H
