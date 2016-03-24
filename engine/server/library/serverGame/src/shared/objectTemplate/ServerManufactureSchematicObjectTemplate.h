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
#include "sharedUtility/TemplateParameter.h"


class DraftSchematicObject;
class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


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
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	struct IngredientSlot
	{
		StringId               name;
		Ingredient ingredient;
	};

protected:
	class _IngredientSlot : public ObjectTemplate
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
	const StringId         getName(bool versionOk) const;
	void              getIngredient(Ingredient &data, bool versionOk) const;
	void              getIngredientMin(Ingredient &data, bool versionOk) const;
	void              getIngredientMax(Ingredient &data, bool versionOk) const;


protected:
	virtual void load(Iff &file);

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
	const std::string &    getDraftSchematic() const;
	const std::string &    getCreator() const;
	void              getIngredients(IngredientSlot &data, int index) const;
	void              getIngredientsMin(IngredientSlot &data, int index) const;
	void              getIngredientsMax(IngredientSlot &data, int index) const;
	size_t            getIngredientsCount(void) const;
	int                    getItemCount() const;
	int                    getItemCountMin() const;
	int                    getItemCountMax() const;
	void              getAttributes(SchematicAttribute &data, int index) const;
	void              getAttributesMin(SchematicAttribute &data, int index) const;
	void              getAttributesMax(SchematicAttribute &data, int index) const;
	size_t            getAttributesCount(void) const;


protected:
	virtual void load(Iff &file);

private:
	StringParam m_draftSchematic;		// source draft schematic
	StringParam m_creator;		// who created me
	stdvector<StructParamOT *>::fwd m_ingredients;		// ingredients needed to create
	bool m_ingredientsLoaded;
	bool m_ingredientsAppend;
	IntegerParam m_itemCount;		// number of items this schematic can make
	stdvector<StructParamOT *>::fwd m_attributes;		// values for the attributes the schematic affects
	bool m_attributesLoaded;
	bool m_attributesAppend;
//@END TFD

public:
	// user functions
	virtual Object * createObject(void) const;
	static  Object * createObject(const char *const fileName, const DraftSchematicObject &);
	        Object * createObject(const DraftSchematicObject &) const;

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerManufactureSchematicObjectTemplate(const ServerManufactureSchematicObjectTemplate &);
	ServerManufactureSchematicObjectTemplate & operator =(const ServerManufactureSchematicObjectTemplate &);
};


inline void ServerManufactureSchematicObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerManufactureSchematicObjectTemplate::registerMe();
	ServerManufactureSchematicObjectTemplate::_IngredientSlot::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerManufactureSchematicObjectTemplate_H
