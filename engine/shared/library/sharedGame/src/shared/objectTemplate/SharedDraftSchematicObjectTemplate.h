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
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


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
	static void install(bool allowDefaultTemplateParams = true);

	const StringId               getCraftedName                     () const;
	const StringId               getCraftedDetailedDescription      () const;
	const StringId               getCraftedLookAtText               () const;
	const std::string            getCraftedAppearanceFilename       () const;
	const SharedObjectTemplate * fetchCraftedSharedObjectTemplate   () const;

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
	class _IngredientSlot : public ObjectTemplate
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
	const StringId         getName(bool versionOk, bool testData = false) const;
	const std::string &    getHardpoint(bool versionOk, bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

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

	class _SchematicAttribute : public ObjectTemplate
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
	const StringId         getName(bool versionOk, bool testData = false) const;
	const StringId         getExperiment(bool versionOk, bool testData = false) const;
	int                    getValue(bool versionOk, bool testData = false) const;
	int                    getValueMin(bool versionOk, bool testData = false) const;
	int                    getValueMax(bool versionOk, bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	StringIdParam m_name;		// name of the attribute
	StringIdParam m_experiment;		// name of the experimental attribute associated with the attribute
	IntegerParam m_value;		// value of the attribute - a range for draft schematics and a single value for manufacture schematics
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_SchematicAttribute(const _SchematicAttribute &);
		_SchematicAttribute & operator =(const _SchematicAttribute &);
	};
	friend class SharedDraftSchematicObjectTemplate::_SchematicAttribute;

public:
	void              getSlots(IngredientSlot &data, int index) const;
	void              getSlotsMin(IngredientSlot &data, int index) const;
	void              getSlotsMax(IngredientSlot &data, int index) const;
	size_t            getSlotsCount(void) const;
	void              getAttributes(SchematicAttribute &data, int index) const;
	void              getAttributesMin(SchematicAttribute &data, int index) const;
	void              getAttributesMax(SchematicAttribute &data, int index) const;
	size_t            getAttributesCount(void) const;
	const std::string &    getCraftedSharedTemplate(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

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
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedDraftSchematicObjectTemplate(const SharedDraftSchematicObjectTemplate &);
	SharedDraftSchematicObjectTemplate & operator =(const SharedDraftSchematicObjectTemplate &);
};


inline void SharedDraftSchematicObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	SharedDraftSchematicObjectTemplate::registerMe();
	SharedDraftSchematicObjectTemplate::_IngredientSlot::registerMe();
	SharedDraftSchematicObjectTemplate::_SchematicAttribute::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedDraftSchematicObjectTemplate_H
