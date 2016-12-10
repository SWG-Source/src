//========================================================================
//
// ServerIntangibleObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerIntangibleObjectTemplate_H
#define _INCLUDED_ServerIntangibleObjectTemplate_H

#include "ServerObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class ServerIntangibleObjectTemplate : public ServerObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerIntangibleObjectTemplate_tag = TAG(I,T,N,O)
	};
//@END TFD ID
public:
	         ServerIntangibleObjectTemplate(const std::string & filename);
	virtual ~ServerIntangibleObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(void);

//@BEGIN TFD
public:
	enum IngredientType
	{
		IT_none,		// no item (for empty/unused slots)
		IT_item,		// a specific item (Skywalker barrel mark V)
		IT_template,		// any item created from a template (any small_blaster_barrel)
		IT_resourceType,		// a specific resource type (iron type 5)
		IT_resourceClass,		// any resource of a resource class (ferrous metal)
		IT_templateGeneric,		// same as IT_template, but if multiple components are required, they don't have to be the same exact type as the first component used
		IT_schematic,		// item crafted from a draft schematic
		IT_schematicGeneric,		// same as IT_schematic, but if multiple components are required, they don't have to be the same exact type as the first component used
		IngredientType_Last = IT_schematicGeneric,
	};

public:
	struct SimpleIngredient
	{
		StringId               name;
		std::string            ingredient;
		int                    count;
	};

	struct Ingredient
	{
		enum IngredientType ingredientType;
		std::vector<SimpleIngredient> ingredients;
		float                  complexity;
		std::string            skillCommand;
	};

	struct SchematicAttribute
	{
		StringId               name;
		int                    value;
	};

protected:
	class _SimpleIngredient : public TpfTemplate
	{
		friend class ServerIntangibleObjectTemplate;
	public:
		enum
		{
			_SimpleIngredient_tag = TAG(S,I,N,G)
		};

	public:
		         _SimpleIngredient(const std::string & filename);
		virtual ~_SimpleIngredient();

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
	StringIdParam m_name;		// name of ingredient that is given to players
	StringParam m_ingredient;		// name of ingredient used to match the resource/component name
	CompilerIntegerParam m_count;		// number of ingredients
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_SimpleIngredient(const _SimpleIngredient &);
		_SimpleIngredient & operator =(const _SimpleIngredient &);
	};
	friend class ServerIntangibleObjectTemplate::_SimpleIngredient;

	class _Ingredient : public TpfTemplate
	{
		friend class ServerIntangibleObjectTemplate;
	public:
		enum
		{
			_Ingredient_tag = TAG(I,N,G,R)
		};

	public:
		         _Ingredient(const std::string & filename);
		virtual ~_Ingredient();

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
	CompilerIntegerParam m_ingredientType;		// type of ingredient required
	std::vector<StructParamOT *> m_ingredients;		// ingredients to be used/being used
	bool m_ingredientsLoaded;
	bool m_ingredientsAppend;
	FloatParam m_complexity;		// adjustment to complexity by using this ingredient
	StringParam m_skillCommand;		// skill command needed to use this ingredient
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_Ingredient(const _Ingredient &);
		_Ingredient & operator =(const _Ingredient &);
	};
	friend class ServerIntangibleObjectTemplate::_Ingredient;

	class _SchematicAttribute : public TpfTemplate
	{
		friend class ServerIntangibleObjectTemplate;
	public:
		enum
		{
			_SchematicAttribute_tag = TAG(I,T,A,T)
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
	CompilerIntegerParam m_value;		// value of the attribute - a range for draft schematics and a single value for manufacture schematics
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_SchematicAttribute(const _SchematicAttribute &);
		_SchematicAttribute & operator =(const _SchematicAttribute &);
	};
	friend class ServerIntangibleObjectTemplate::_SchematicAttribute;

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
	//
	// shared info needed by draft and manufacturing schematics
	//
	// this enum list must be reflected in MessageQueueIngredients.h
	CompilerIntegerParam m_count;		// generic counter
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerIntangibleObjectTemplate(const ServerIntangibleObjectTemplate &);
	ServerIntangibleObjectTemplate & operator =(const ServerIntangibleObjectTemplate &);
};


inline void ServerIntangibleObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	ServerIntangibleObjectTemplate::registerMe();
	ServerIntangibleObjectTemplate::_Ingredient::registerMe();
	ServerIntangibleObjectTemplate::_SchematicAttribute::registerMe();
	ServerIntangibleObjectTemplate::_SimpleIngredient::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerIntangibleObjectTemplate_H
