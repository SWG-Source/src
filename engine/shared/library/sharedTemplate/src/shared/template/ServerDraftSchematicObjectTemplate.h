//========================================================================
//
// ServerDraftSchematicObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerDraftSchematicObjectTemplate_H
#define _INCLUDED_ServerDraftSchematicObjectTemplate_H

#include "ServerIntangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class ServerDraftSchematicObjectTemplate : public ServerIntangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerDraftSchematicObjectTemplate_tag = TAG(D,S,C,O)
	};
//@END TFD ID
public:
	         ServerDraftSchematicObjectTemplate(const std::string & filename);
	virtual ~ServerDraftSchematicObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(void);

//@BEGIN TFD
public:
	struct IngredientSlot
	{
		bool                   optional;
		StringId               name;
		std::vector<Ingredient> options;
		std::string            optionalSkillCommand;
		float                  complexity;
		std::string            appearance;
	};

protected:
	class _IngredientSlot : public TpfTemplate
	{
		friend class ServerDraftSchematicObjectTemplate;
	public:
		enum
		{
			_IngredientSlot_tag = TAG(D,I,N,S)
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
	BoolParam m_optional;		// is the slot optional
	StringIdParam m_name;		// slot name
	std::vector<StructParamOT *> m_options;		// possible ingredients that can be used to fill the slot
	bool m_optionsLoaded;
	bool m_optionsAppend;
	StringParam m_optionalSkillCommand;		// skill commands needed to access this slot if it is optional (ignored for required slots)
	FloatParam m_complexity;		// adjustment to complexity by using this slot
	StringParam m_appearance;		// if the slot is a component, the name of the hardpoint associated with the slot; if the slot is a resource, a string used to build an appearance file name
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_IngredientSlot(const _IngredientSlot &);
		_IngredientSlot & operator =(const _IngredientSlot &);
	};
	friend class ServerDraftSchematicObjectTemplate::_IngredientSlot;

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
	CompilerIntegerParam m_category;		// schematic category (food, weapon, etc)
	StringParam m_craftedObjectTemplate;		// what object we create
	StringParam m_crateObjectTemplate;		// the "crate" to use when manufacturing multiple copies of the object
	std::vector<StructParamOT *> m_slots;		// ingredient slots
	bool m_slotsLoaded;
	bool m_slotsAppend;
	std::vector<StringParam *> m_skillCommands;		// skill commands needed to access this schematic and required slots
	bool m_skillCommandsLoaded;
	bool m_skillCommandsAppend;
	BoolParam m_destroyIngredients;		// flag that the ingredients used in the design stage should be destroyed
	std::vector<StringParam *> m_manufactureScripts;		// scripts that will be attached to a manufacturing schematic created from this schematic
	bool m_manufactureScriptsLoaded;
	bool m_manufactureScriptsAppend;
	CompilerIntegerParam m_itemsPerContainer;		// when manufacturing, how many items will be put in a container (1 = items not in a container)
	FloatParam m_manufactureTime;		// time to manufacture an item (in secs) per complexity point
	FloatParam m_prototypeTime;		// time to create a prototype (in secs) per complexity point
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerDraftSchematicObjectTemplate(const ServerDraftSchematicObjectTemplate &);
	ServerDraftSchematicObjectTemplate & operator =(const ServerDraftSchematicObjectTemplate &);
};


inline void ServerDraftSchematicObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	ServerDraftSchematicObjectTemplate::registerMe();
	ServerDraftSchematicObjectTemplate::_IngredientSlot::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerDraftSchematicObjectTemplate_H
