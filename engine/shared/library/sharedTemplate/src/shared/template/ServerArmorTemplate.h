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

#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class ServerArmorTemplate : public TpfTemplate
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
	static void install(void);

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
	class _SpecialProtection : public TpfTemplate
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
	CompilerIntegerParam m_type;		// specific damage being protected from
	CompilerIntegerParam m_effectiveness;		// armor effectiveness ( <0 = no protection for this damage type, default protection will be ignored)
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_SpecialProtection(const _SpecialProtection &);
		_SpecialProtection & operator =(const _SpecialProtection &);
	};
	friend class ServerArmorTemplate::_SpecialProtection;

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
	CompilerIntegerParam m_rating;		// armor rating
	CompilerIntegerParam m_integrity;		// integrity
	CompilerIntegerParam m_effectiveness;		// default effectiveness (0 = only use special protection)
	std::vector<StructParamOT *> m_specialProtection;		// damage-type specific protection
	bool m_specialProtectionLoaded;
	bool m_specialProtectionAppend;
	CompilerIntegerParam m_vulnerability;		// damaga types that this armor doesn't protect against
	CompilerIntegerParam m_encumbrance[3];		// reduction to attributes from wearing this armor
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerArmorTemplate(const ServerArmorTemplate &);
	ServerArmorTemplate & operator =(const ServerArmorTemplate &);
};


inline void ServerArmorTemplate::install(void)
{
//@BEGIN TFD INSTALL
	ServerArmorTemplate::registerMe();
	ServerArmorTemplate::_SpecialProtection::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerArmorTemplate_H
