//========================================================================
//
// SharedDraftSchematicObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedDraftSchematicObjectTemplate_H
#define _INCLUDED_SharedDraftSchematicObjectTemplate_H

#include "SharedIntangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class SharedDraftSchematicObjectTemplate : public SharedIntangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedDraftSchematicObjectTemplate_tag = TAG(S,D,S,C)
	};
//@END TFD ID
public:
	         SharedDraftSchematicObjectTemplate(const std::string & filename);
	virtual ~SharedDraftSchematicObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(void);

//@BEGIN TFD
public:
	enum ArmorRating
	{
		AR_armorNone,
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
	struct IngredientSlot
	{
		StringId               name;
		std::string            hardpoint;
	};

	struct SchematicAttribute
	{
		StringId               name;
		StringId               experiment;
		int                    value;
	};

protected:
	class _IngredientSlot : public TpfTemplate
	{
		friend class SharedDraftSchematicObjectTemplate;
	public:
		enum
		{
			_IngredientSlot_tag = TAG(S,I,S,S)
		};

	public:
		         _IngredientSlot(const std::string & filename);
		virtual ~_IngredientSlot();

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
	StringIdParam m_name;		// slot name
	StringParam m_hardpoint;		// UI hardpoint for this slot
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_IngredientSlot(const _IngredientSlot &);
		_IngredientSlot & operator =(const _IngredientSlot &);
	};
	friend class SharedDraftSchematicObjectTemplate::_IngredientSlot;

	class _SchematicAttribute : public TpfTemplate
	{
		friend class SharedDraftSchematicObjectTemplate;
	public:
		enum
		{
			_SchematicAttribute_tag = TAG(D,S,S,A)
		};

	public:
		         _SchematicAttribute(const std::string & filename);
		virtual ~_SchematicAttribute();

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
	StringIdParam m_name;		// name of the attribute
	StringIdParam m_experiment;		// name of the experimental attribute associated with the attribute
	CompilerIntegerParam m_value;		// value of the attribute - a range for draft schematics and a single value for manufacture schematics
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_SchematicAttribute(const _SchematicAttribute &);
		_SchematicAttribute & operator =(const _SchematicAttribute &);
	};
	friend class SharedDraftSchematicObjectTemplate::_SchematicAttribute;

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
	// this enum is also defined in the server object_template.tdf file
	// this enum is also defined in the server object_template.tdf file
	std::vector<StructParamOT *> m_slots;		// ingredient slots
	bool m_slotsLoaded;
	bool m_slotsAppend;
	std::vector<StructParamOT *> m_attributes;		// what attributes the schematic can affect
	bool m_attributesLoaded;
	bool m_attributesAppend;
	StringParam m_craftedSharedTemplate;
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedDraftSchematicObjectTemplate(const SharedDraftSchematicObjectTemplate &);
	SharedDraftSchematicObjectTemplate & operator =(const SharedDraftSchematicObjectTemplate &);
};


inline void SharedDraftSchematicObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	SharedDraftSchematicObjectTemplate::registerMe();
	SharedDraftSchematicObjectTemplate::_IngredientSlot::registerMe();
	SharedDraftSchematicObjectTemplate::_SchematicAttribute::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedDraftSchematicObjectTemplate_H
