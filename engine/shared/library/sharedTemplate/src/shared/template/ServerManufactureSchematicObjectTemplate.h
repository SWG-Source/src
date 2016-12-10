//========================================================================
//
// ServerManufactureSchematicObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerManufactureSchematicObjectTemplate_H
#define _INCLUDED_ServerManufactureSchematicObjectTemplate_H

#include "ServerIntangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class ServerManufactureSchematicObjectTemplate : public ServerIntangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerManufactureSchematicObjectTemplate_tag = TAG(M,S,C,O)
	};
//@END TFD ID
public:
	         ServerManufactureSchematicObjectTemplate(const std::string & filename);
	virtual ~ServerManufactureSchematicObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(void);

//@BEGIN TFD
public:
	struct IngredientSlot
	{
		StringId               name;
		Ingredient ingredient;
	};

protected:
	class _IngredientSlot : public TpfTemplate
	{
		friend class ServerManufactureSchematicObjectTemplate;
	public:
		enum
		{
			_IngredientSlot_tag = TAG(M,I,N,S)
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
	StructParamOT m_ingredient;		// ingredient used to fill the slot
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_IngredientSlot(const _IngredientSlot &);
		_IngredientSlot & operator =(const _IngredientSlot &);
	};
	friend class ServerManufactureSchematicObjectTemplate::_IngredientSlot;

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
	StringParam m_draftSchematic;		// source draft schematic
	StringParam m_creator;		// who created me
	std::vector<StructParamOT *> m_ingredients;		// ingredients needed to create
	bool m_ingredientsLoaded;
	bool m_ingredientsAppend;
	CompilerIntegerParam m_itemCount;		// number of items this schematic can make
	std::vector<StructParamOT *> m_attributes;		// values for the attributes the schematic affects
	bool m_attributesLoaded;
	bool m_attributesAppend;
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerManufactureSchematicObjectTemplate(const ServerManufactureSchematicObjectTemplate &);
	ServerManufactureSchematicObjectTemplate & operator =(const ServerManufactureSchematicObjectTemplate &);
};


inline void ServerManufactureSchematicObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	ServerManufactureSchematicObjectTemplate::registerMe();
	ServerManufactureSchematicObjectTemplate::_IngredientSlot::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerManufactureSchematicObjectTemplate_H
