//========================================================================
//
// ServerManufactureSchematicObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerManufactureSchematicObjectTemplate.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "sharedFile/Iff.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <stdio.h>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerManufactureSchematicObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
	,m_ingredientsLoaded(false)
	,m_ingredientsAppend(false)
	,m_attributesLoaded(false)
	,m_attributesAppend(false)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate

/**
 * Class destructor.
 */
ServerManufactureSchematicObjectTemplate::~ServerManufactureSchematicObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_ingredients.begin(); iter != m_ingredients.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_ingredients.clear();
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_attributes.clear();
	}
//@END TFD CLEANUP
}	// ServerManufactureSchematicObjectTemplate::~ServerManufactureSchematicObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerManufactureSchematicObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerManufactureSchematicObjectTemplate_tag, create);
}	// ServerManufactureSchematicObjectTemplate::registerMe

/**
 * Creates a ServerManufactureSchematicObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerManufactureSchematicObjectTemplate::create(const std::string & filename)
{
	return new ServerManufactureSchematicObjectTemplate(filename);
}	// ServerManufactureSchematicObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerManufactureSchematicObjectTemplate::getId(void) const
{
	return ServerManufactureSchematicObjectTemplate_tag;
}	// ServerManufactureSchematicObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerManufactureSchematicObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerManufactureSchematicObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerManufactureSchematicObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerManufactureSchematicObjectTemplate * base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerManufactureSchematicObjectTemplate::getHighestTemplateVersion

/**
 * Called when the game tries to create a manf schematic via the default method.
 * Manf must be created via a draft schematic, so we always return NULL;
 *
 * @return the object
 */
Object * ServerManufactureSchematicObjectTemplate::createObject(void) const
{
//	WARNING(true, ("Someone is trying to create a manf schematic without a "
//		"draft schematic source!"));
	return new ManufactureSchematicObject(this);
}	// ServerManufactureSchematicObjectTemplate::createObject

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerManufactureSchematicObjectTemplate::createObject(const char *const fileName, const DraftSchematicObject & schematic)
{
	const ObjectTemplate *const objectTemplate = ObjectTemplateList::fetch(fileName);
	if (objectTemplate)
	{
		Object * object = NULL;
		const ServerManufactureSchematicObjectTemplate * const manfTemplate = 
			dynamic_cast<const ServerManufactureSchematicObjectTemplate *const>(
			objectTemplate);
		if (manfTemplate != NULL)
			object = manfTemplate->createObject(schematic);
		objectTemplate->releaseReference ();

		return object;
	}
	return NULL;
}	// ServerManufactureSchematicObjectTemplate::createObject

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerManufactureSchematicObjectTemplate::createObject(const DraftSchematicObject &) const
{
	return new ManufactureSchematicObject(this);
}	// ServerManufactureSchematicObjectTemplate::createObject

//@BEGIN TFD
const std::string & ServerManufactureSchematicObjectTemplate::getDraftSchematic(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDraftSchematic(true);
#endif
	}

	if (!m_draftSchematic.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter draftSchematic in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter draftSchematic has not been defined in template %s!", DataResource::getName()));
			return base->getDraftSchematic();
		}
	}

	const std::string & value = m_draftSchematic.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerManufactureSchematicObjectTemplate::getDraftSchematic

const std::string & ServerManufactureSchematicObjectTemplate::getCreator(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCreator(true);
#endif
	}

	if (!m_creator.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter creator in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter creator has not been defined in template %s!", DataResource::getName()));
			return base->getCreator();
		}
	}

	const std::string & value = m_creator.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerManufactureSchematicObjectTemplate::getCreator

void ServerManufactureSchematicObjectTemplate::getIngredients(IngredientSlot &data, int index) const
{
	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_ingredientsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredients in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredients has not been defined in template %s!", DataResource::getName()));
			base->getIngredients(data, index);
			return;
		}
	}

	if (m_ingredientsAppend && base != nullptr)
	{
		int baseCount = base->getIngredientsCount();
		if (index < baseCount)
			{
				base->getIngredients(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_ingredients.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_ingredients[index]).getValue();
	NOT_NULL(structTemplate);
	const _IngredientSlot *param = dynamic_cast<const _IngredientSlot *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	param->getIngredient(data.ingredient, m_versionOk);
}	// ServerManufactureSchematicObjectTemplate::getIngredients

void ServerManufactureSchematicObjectTemplate::getIngredientsMin(IngredientSlot &data, int index) const
{
	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_ingredientsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredients in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredients has not been defined in template %s!", DataResource::getName()));
			base->getIngredientsMin(data, index);
			return;
		}
	}

	if (m_ingredientsAppend && base != nullptr)
	{
		int baseCount = base->getIngredientsCount();
		if (index < baseCount)
			{
				base->getIngredientsMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_ingredients.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_ingredients[index]).getValue();
	NOT_NULL(structTemplate);
	const _IngredientSlot *param = dynamic_cast<const _IngredientSlot *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	param->getIngredientMin(data.ingredient, m_versionOk);
}	// ServerManufactureSchematicObjectTemplate::getIngredientsMin

void ServerManufactureSchematicObjectTemplate::getIngredientsMax(IngredientSlot &data, int index) const
{
	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_ingredientsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredients in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredients has not been defined in template %s!", DataResource::getName()));
			base->getIngredientsMax(data, index);
			return;
		}
	}

	if (m_ingredientsAppend && base != nullptr)
	{
		int baseCount = base->getIngredientsCount();
		if (index < baseCount)
			{
				base->getIngredientsMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_ingredients.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_ingredients[index]).getValue();
	NOT_NULL(structTemplate);
	const _IngredientSlot *param = dynamic_cast<const _IngredientSlot *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	param->getIngredientMax(data.ingredient, m_versionOk);
}	// ServerManufactureSchematicObjectTemplate::getIngredientsMax

size_t ServerManufactureSchematicObjectTemplate::getIngredientsCount(void) const
{
	if (!m_ingredientsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerManufactureSchematicObjectTemplate * base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getIngredientsCount();
	}

	size_t count = m_ingredients.size();

	// if we are extending our base template, add it's count
	if (m_ingredientsAppend && m_baseData != nullptr)
	{
		const ServerManufactureSchematicObjectTemplate * base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getIngredientsCount();
	}

	return count;
}	// ServerManufactureSchematicObjectTemplate::getIngredientsCount

int ServerManufactureSchematicObjectTemplate::getItemCount(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getItemCount(true);
#endif
	}

	if (!m_itemCount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter itemCount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter itemCount has not been defined in template %s!", DataResource::getName()));
			return base->getItemCount();
		}
	}

	int value = m_itemCount.getValue();
	char delta = m_itemCount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getItemCount();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerManufactureSchematicObjectTemplate::getItemCount

int ServerManufactureSchematicObjectTemplate::getItemCountMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getItemCountMin(true);
#endif
	}

	if (!m_itemCount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter itemCount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter itemCount has not been defined in template %s!", DataResource::getName()));
			return base->getItemCountMin();
		}
	}

	int value = m_itemCount.getMinValue();
	char delta = m_itemCount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getItemCountMin();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerManufactureSchematicObjectTemplate::getItemCountMin

int ServerManufactureSchematicObjectTemplate::getItemCountMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getItemCountMax(true);
#endif
	}

	if (!m_itemCount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter itemCount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter itemCount has not been defined in template %s!", DataResource::getName()));
			return base->getItemCountMax();
		}
	}

	int value = m_itemCount.getMaxValue();
	char delta = m_itemCount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getItemCountMax();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerManufactureSchematicObjectTemplate::getItemCountMax

void ServerManufactureSchematicObjectTemplate::getAttributes(SchematicAttribute &data, int index) const
{
	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_attributesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			base->getAttributes(data, index);
			return;
		}
	}

	if (m_attributesAppend && base != nullptr)
	{
		int baseCount = base->getAttributesCount();
		if (index < baseCount)
			{
				base->getAttributes(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attributes.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attributes[index]).getValue();
	NOT_NULL(structTemplate);
	const _SchematicAttribute *param = dynamic_cast<const _SchematicAttribute *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	data.value = param->getValue(m_versionOk);
}	// ServerManufactureSchematicObjectTemplate::getAttributes

void ServerManufactureSchematicObjectTemplate::getAttributesMin(SchematicAttribute &data, int index) const
{
	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_attributesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			base->getAttributesMin(data, index);
			return;
		}
	}

	if (m_attributesAppend && base != nullptr)
	{
		int baseCount = base->getAttributesCount();
		if (index < baseCount)
			{
				base->getAttributesMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attributes.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attributes[index]).getValue();
	NOT_NULL(structTemplate);
	const _SchematicAttribute *param = dynamic_cast<const _SchematicAttribute *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	data.value = param->getValueMin(m_versionOk);
}	// ServerManufactureSchematicObjectTemplate::getAttributesMin

void ServerManufactureSchematicObjectTemplate::getAttributesMax(SchematicAttribute &data, int index) const
{
	const ServerManufactureSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_attributesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			base->getAttributesMax(data, index);
			return;
		}
	}

	if (m_attributesAppend && base != nullptr)
	{
		int baseCount = base->getAttributesCount();
		if (index < baseCount)
			{
				base->getAttributesMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attributes.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attributes[index]).getValue();
	NOT_NULL(structTemplate);
	const _SchematicAttribute *param = dynamic_cast<const _SchematicAttribute *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	data.value = param->getValueMax(m_versionOk);
}	// ServerManufactureSchematicObjectTemplate::getAttributesMax

size_t ServerManufactureSchematicObjectTemplate::getAttributesCount(void) const
{
	if (!m_attributesLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerManufactureSchematicObjectTemplate * base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getAttributesCount();
	}

	size_t count = m_attributes.size();

	// if we are extending our base template, add it's count
	if (m_attributesAppend && m_baseData != nullptr)
	{
		const ServerManufactureSchematicObjectTemplate * base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getAttributesCount();
	}

	return count;
}	// ServerManufactureSchematicObjectTemplate::getAttributesCount

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerManufactureSchematicObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getDraftSchematic(true));
	IGNORE_RETURN(getCreator(true));
	IGNORE_RETURN(getItemCountMin(true));
	IGNORE_RETURN(getItemCountMax(true));
	ServerIntangibleObjectTemplate::testValues();
}	// ServerManufactureSchematicObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerManufactureSchematicObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerManufactureSchematicObjectTemplate_tag)
	{
		ServerIntangibleObjectTemplate::load(file);
		return;
	}

	file.enterForm();
	m_templateVersion = file.getCurrentName();
	if (m_templateVersion == TAG(D,E,R,V))
	{
		file.enterForm();
		file.enterChunk();
		std::string baseFilename;
		file.read_string(baseFilename);
		file.exitChunk();
		const ObjectTemplate *base = ObjectTemplateList::fetch(baseFilename);
		DEBUG_WARNING(base == nullptr, ("was unable to load base template %s", baseFilename.c_str()));
		if (m_baseData == base && base != nullptr)
			base->releaseReference();
		else
		{
			if (m_baseData != nullptr)
				m_baseData->releaseReference();
			m_baseData = base;
		}
		file.exitForm();
		m_templateVersion = file.getCurrentName();
	}
	if (getHighestTemplateVersion() != TAG(0,0,0,0))
	{
		if (DataLint::isEnabled())
			DEBUG_WARNING(true, ("template %s version out of date", file.getFileName()));
		m_versionOk = false;
	}

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "draftSchematic") == 0)
			m_draftSchematic.loadFromIff(file);
		else if (strcmp(paramName, "creator") == 0)
			m_creator.loadFromIff(file);
		else if (strcmp(paramName, "ingredients") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_ingredients.begin(); iter != m_ingredients.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_ingredients.clear();
			m_ingredientsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_ingredients.push_back(newData);
			}
			m_ingredientsLoaded = true;
		}
		else if (strcmp(paramName, "itemCount") == 0)
			m_itemCount.loadFromIff(file);
		else if (strcmp(paramName, "attributes") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_attributes.clear();
			m_attributesAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_attributes.push_back(newData);
			}
			m_attributesLoaded = true;
		}
		file.exitChunk(true);
	}

	file.exitForm();
	ServerIntangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerManufactureSchematicObjectTemplate::load


//=============================================================================
// class ServerManufactureSchematicObjectTemplate::_IngredientSlot

/**
 * Class constructor.
 */
ServerManufactureSchematicObjectTemplate::_IngredientSlot::_IngredientSlot(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::_IngredientSlot

/**
 * Class destructor.
 */
ServerManufactureSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot()
{
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot

/**
 * Static function used to register this template.
 */
void ServerManufactureSchematicObjectTemplate::_IngredientSlot::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_IngredientSlot_tag, create);
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::registerMe

/**
 * Creates a ServerManufactureSchematicObjectTemplate::_IngredientSlot template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerManufactureSchematicObjectTemplate::_IngredientSlot::create(const std::string & filename)
{
	return new ServerManufactureSchematicObjectTemplate::_IngredientSlot(filename);
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerManufactureSchematicObjectTemplate::_IngredientSlot::getId(void) const
{
	return _IngredientSlot_tag;
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::getId

const StringId ServerManufactureSchematicObjectTemplate::_IngredientSlot::getName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const ServerManufactureSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getName(true);
#endif
	}

	if (!m_name.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter name in template %s", DataResource::getName()));
			return DefaultStringId;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter name has not been defined in template %s!", DataResource::getName()));
			return base->getName(versionOk);
		}
	}

	const StringId value = m_name.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::getName

void ServerManufactureSchematicObjectTemplate::_IngredientSlot::getIngredient(Ingredient &data, bool versionOk) const
{
	const ServerManufactureSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
	}

	if (!m_ingredient.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredient in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredient has not been defined in template %s!", DataResource::getName()));
			base->getIngredient(data, versionOk);
			return;
		}
	}

	const ObjectTemplate * structTemplate = m_ingredient.getValue();
	NOT_NULL(structTemplate);
	const _Ingredient *param = dynamic_cast<const _Ingredient *>(structTemplate);
	NOT_NULL(param);
	data.ingredientType = param->getIngredientType(versionOk);
	{
		for (size_t i = 0; i < param->getIngredientsCount(); ++i)
		{
			data.ingredients.push_back(SimpleIngredient());
			param->getIngredients(data.ingredients.back(), i, versionOk);
		}
	}
	data.complexity = param->getComplexity(versionOk);
	data.skillCommand = param->getSkillCommand(versionOk);
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::getIngredient

void ServerManufactureSchematicObjectTemplate::_IngredientSlot::getIngredientMin(Ingredient &data, bool versionOk) const
{
	const ServerManufactureSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
	}

	if (!m_ingredient.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredient in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredient has not been defined in template %s!", DataResource::getName()));
			base->getIngredientMin(data, versionOk);
			return;
		}
	}

	const ObjectTemplate * structTemplate = m_ingredient.getValue();
	NOT_NULL(structTemplate);
	const _Ingredient *param = dynamic_cast<const _Ingredient *>(structTemplate);
	NOT_NULL(param);
	data.ingredientType = param->getIngredientType(versionOk);
	{
		for (size_t i = 0; i < param->getIngredientsCount(); ++i)
		{
			data.ingredients.push_back(SimpleIngredient());
			param->getIngredientsMin(data.ingredients.back(), i, versionOk);
		}
	}
	data.complexity = param->getComplexityMin(versionOk);
	data.skillCommand = param->getSkillCommand(versionOk);
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::getIngredientMin

void ServerManufactureSchematicObjectTemplate::_IngredientSlot::getIngredientMax(Ingredient &data, bool versionOk) const
{
	const ServerManufactureSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerManufactureSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
	}

	if (!m_ingredient.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredient in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredient has not been defined in template %s!", DataResource::getName()));
			base->getIngredientMax(data, versionOk);
			return;
		}
	}

	const ObjectTemplate * structTemplate = m_ingredient.getValue();
	NOT_NULL(structTemplate);
	const _Ingredient *param = dynamic_cast<const _Ingredient *>(structTemplate);
	NOT_NULL(param);
	data.ingredientType = param->getIngredientType(versionOk);
	{
		for (size_t i = 0; i < param->getIngredientsCount(); ++i)
		{
			data.ingredients.push_back(SimpleIngredient());
			param->getIngredientsMax(data.ingredients.back(), i, versionOk);
		}
	}
	data.complexity = param->getComplexityMax(versionOk);
	data.skillCommand = param->getSkillCommand(versionOk);
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::getIngredientMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerManufactureSchematicObjectTemplate::_IngredientSlot::testValues(void) const
{
	IGNORE_RETURN(getName(true));
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerManufactureSchematicObjectTemplate::_IngredientSlot::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "name") == 0)
			m_name.loadFromIff(file);
		else if (strcmp(paramName, "ingredient") == 0)
			m_ingredient.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerManufactureSchematicObjectTemplate::_IngredientSlot::load

//@END TFD
