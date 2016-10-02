//========================================================================
//
// ServerIntangibleObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerIntangibleObjectTemplate.h"
#include "serverGame/IntangibleObject.h"
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

bool ServerIntangibleObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerIntangibleObjectTemplate::ServerIntangibleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerIntangibleObjectTemplate::ServerIntangibleObjectTemplate

/**
 * Class destructor.
 */
ServerIntangibleObjectTemplate::~ServerIntangibleObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerIntangibleObjectTemplate::~ServerIntangibleObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerIntangibleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerIntangibleObjectTemplate_tag, create);
}	// ServerIntangibleObjectTemplate::registerMe

/**
 * Creates a ServerIntangibleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerIntangibleObjectTemplate::create(const std::string & filename)
{
	return new ServerIntangibleObjectTemplate(filename);
}	// ServerIntangibleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerIntangibleObjectTemplate::getId(void) const
{
	return ServerIntangibleObjectTemplate_tag;
}	// ServerIntangibleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerIntangibleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerIntangibleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerIntangibleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerIntangibleObjectTemplate * base = dynamic_cast<const ServerIntangibleObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerIntangibleObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerIntangibleObjectTemplate::createObject(void) const
{
	return new IntangibleObject(this);
}	// ServerIntangibleObjectTemplate::createObject

//@BEGIN TFD
int ServerIntangibleObjectTemplate::getCount(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCount(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCount();
		}
	}

	int value = m_count.getValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCount();
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
}	// ServerIntangibleObjectTemplate::getCount

int ServerIntangibleObjectTemplate::getCountMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCountMin(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCountMin();
		}
	}

	int value = m_count.getMinValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCountMin();
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
}	// ServerIntangibleObjectTemplate::getCountMin

int ServerIntangibleObjectTemplate::getCountMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCountMax(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCountMax();
		}
	}

	int value = m_count.getMaxValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCountMax();
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
}	// ServerIntangibleObjectTemplate::getCountMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerIntangibleObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getCountMin(true));
	IGNORE_RETURN(getCountMax(true));
	ServerObjectTemplate::testValues();
}	// ServerIntangibleObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerIntangibleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerIntangibleObjectTemplate_tag)
	{
		ServerObjectTemplate::load(file);
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
	if (getHighestTemplateVersion() != TAG(0,0,0,1))
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
		if (strcmp(paramName, "count") == 0)
			m_count.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerIntangibleObjectTemplate::load


//=============================================================================
// class ServerIntangibleObjectTemplate::_Ingredient

/**
 * Class constructor.
 */
ServerIntangibleObjectTemplate::_Ingredient::_Ingredient(const std::string & filename)
	: ObjectTemplate(filename)
	,m_ingredientsLoaded(false)
	,m_ingredientsAppend(false)
{
}	// ServerIntangibleObjectTemplate::_Ingredient::_Ingredient

/**
 * Class destructor.
 */
ServerIntangibleObjectTemplate::_Ingredient::~_Ingredient()
{
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_ingredients.begin(); iter != m_ingredients.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_ingredients.clear();
	}
}	// ServerIntangibleObjectTemplate::_Ingredient::~_Ingredient

/**
 * Static function used to register this template.
 */
void ServerIntangibleObjectTemplate::_Ingredient::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_Ingredient_tag, create);
}	// ServerIntangibleObjectTemplate::_Ingredient::registerMe

/**
 * Creates a ServerIntangibleObjectTemplate::_Ingredient template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerIntangibleObjectTemplate::_Ingredient::create(const std::string & filename)
{
	return new ServerIntangibleObjectTemplate::_Ingredient(filename);
}	// ServerIntangibleObjectTemplate::_Ingredient::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerIntangibleObjectTemplate::_Ingredient::getId(void) const
{
	return _Ingredient_tag;
}	// ServerIntangibleObjectTemplate::_Ingredient::getId

ServerIntangibleObjectTemplate::IngredientType ServerIntangibleObjectTemplate::_Ingredient::getIngredientType(bool versionOk, bool testData) const
{
#ifdef _DEBUG
ServerIntangibleObjectTemplate::IngredientType testDataValue = static_cast<ServerIntangibleObjectTemplate::IngredientType>(0);
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_Ingredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getIngredientType(true);
#endif
	}

	if (!m_ingredientType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredientType in template %s", DataResource::getName()));
			return static_cast<IngredientType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredientType has not been defined in template %s!", DataResource::getName()));
			return base->getIngredientType(versionOk);
		}
	}

	IngredientType value = static_cast<IngredientType>(m_ingredientType.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerIntangibleObjectTemplate::_Ingredient::getIngredientType

void ServerIntangibleObjectTemplate::_Ingredient::getIngredients(SimpleIngredient &data, int index, bool versionOk) const
{
	const ServerIntangibleObjectTemplate::_Ingredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
	}

	if (!m_ingredientsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredients in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredients has not been defined in template %s!", DataResource::getName()));
			base->getIngredients(data, index, versionOk);
			return;
		}
	}

	if (m_ingredientsAppend && base != nullptr)
	{
		int baseCount = base->getIngredientsCount();
		if (index < baseCount)
			{
				base->getIngredients(data, index, versionOk);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_ingredients.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_ingredients[index]).getValue();
	NOT_NULL(structTemplate);
	const _SimpleIngredient *param = dynamic_cast<const _SimpleIngredient *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(versionOk);
	data.ingredient = param->getIngredient(versionOk);
	data.count = param->getCount(versionOk);
}	// ServerIntangibleObjectTemplate::_Ingredient::getIngredients

void ServerIntangibleObjectTemplate::_Ingredient::getIngredientsMin(SimpleIngredient &data, int index, bool versionOk) const
{
	const ServerIntangibleObjectTemplate::_Ingredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
	}

	if (!m_ingredientsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredients in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredients has not been defined in template %s!", DataResource::getName()));
			base->getIngredientsMin(data, index, versionOk);
			return;
		}
	}

	if (m_ingredientsAppend && base != nullptr)
	{
		int baseCount = base->getIngredientsCount();
		if (index < baseCount)
			{
				base->getIngredientsMin(data, index, versionOk);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_ingredients.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_ingredients[index]).getValue();
	NOT_NULL(structTemplate);
	const _SimpleIngredient *param = dynamic_cast<const _SimpleIngredient *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(versionOk);
	data.ingredient = param->getIngredient(versionOk);
	data.count = param->getCountMin(versionOk);
}	// ServerIntangibleObjectTemplate::_Ingredient::getIngredientsMin

void ServerIntangibleObjectTemplate::_Ingredient::getIngredientsMax(SimpleIngredient &data, int index, bool versionOk) const
{
	const ServerIntangibleObjectTemplate::_Ingredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
	}

	if (!m_ingredientsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredients in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredients has not been defined in template %s!", DataResource::getName()));
			base->getIngredientsMax(data, index, versionOk);
			return;
		}
	}

	if (m_ingredientsAppend && base != nullptr)
	{
		int baseCount = base->getIngredientsCount();
		if (index < baseCount)
			{
				base->getIngredientsMax(data, index, versionOk);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_ingredients.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_ingredients[index]).getValue();
	NOT_NULL(structTemplate);
	const _SimpleIngredient *param = dynamic_cast<const _SimpleIngredient *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(versionOk);
	data.ingredient = param->getIngredient(versionOk);
	data.count = param->getCountMax(versionOk);
}	// ServerIntangibleObjectTemplate::_Ingredient::getIngredientsMax

size_t ServerIntangibleObjectTemplate::_Ingredient::getIngredientsCount(void) const
{
	if (!m_ingredientsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerIntangibleObjectTemplate::_Ingredient * base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getIngredientsCount();
	}

	size_t count = m_ingredients.size();

	// if we are extending our base template, add it's count
	if (m_ingredientsAppend && m_baseData != nullptr)
	{
		const ServerIntangibleObjectTemplate::_Ingredient * base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
		if (base != nullptr)
			count += base->getIngredientsCount();
	}

	return count;
}	// ServerIntangibleObjectTemplate::_Ingredient::getIngredientsCount

float ServerIntangibleObjectTemplate::_Ingredient::getComplexity(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_Ingredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexity(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexity(versionOk);
		}
	}

	float value = m_complexity.getValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexity(versionOk);
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerIntangibleObjectTemplate::_Ingredient::getComplexity

float ServerIntangibleObjectTemplate::_Ingredient::getComplexityMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_Ingredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexityMin(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexityMin(versionOk);
		}
	}

	float value = m_complexity.getMinValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexityMin(versionOk);
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerIntangibleObjectTemplate::_Ingredient::getComplexityMin

float ServerIntangibleObjectTemplate::_Ingredient::getComplexityMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_Ingredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexityMax(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexityMax(versionOk);
		}
	}

	float value = m_complexity.getMaxValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexityMax(versionOk);
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerIntangibleObjectTemplate::_Ingredient::getComplexityMax

const std::string & ServerIntangibleObjectTemplate::_Ingredient::getSkillCommand(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_Ingredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_Ingredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSkillCommand(true);
#endif
	}

	if (!m_skillCommand.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter skillCommand in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter skillCommand has not been defined in template %s!", DataResource::getName()));
			return base->getSkillCommand(versionOk);
		}
	}

	const std::string & value = m_skillCommand.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerIntangibleObjectTemplate::_Ingredient::getSkillCommand

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerIntangibleObjectTemplate::_Ingredient::testValues(void) const
{
	IGNORE_RETURN(getIngredientType(true));
	IGNORE_RETURN(getComplexityMin(true));
	IGNORE_RETURN(getComplexityMax(true));
	IGNORE_RETURN(getSkillCommand(true));
}	// ServerIntangibleObjectTemplate::_Ingredient::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerIntangibleObjectTemplate::_Ingredient::load(Iff &file)
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
		if (strcmp(paramName, "ingredientType") == 0)
			m_ingredientType.loadFromIff(file);
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
		else if (strcmp(paramName, "complexity") == 0)
			m_complexity.loadFromIff(file);
		else if (strcmp(paramName, "skillCommand") == 0)
			m_skillCommand.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerIntangibleObjectTemplate::_Ingredient::load


//=============================================================================
// class ServerIntangibleObjectTemplate::_SchematicAttribute

/**
 * Class constructor.
 */
ServerIntangibleObjectTemplate::_SchematicAttribute::_SchematicAttribute(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::_SchematicAttribute

/**
 * Class destructor.
 */
ServerIntangibleObjectTemplate::_SchematicAttribute::~_SchematicAttribute()
{
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::~_SchematicAttribute

/**
 * Static function used to register this template.
 */
void ServerIntangibleObjectTemplate::_SchematicAttribute::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_SchematicAttribute_tag, create);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::registerMe

/**
 * Creates a ServerIntangibleObjectTemplate::_SchematicAttribute template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerIntangibleObjectTemplate::_SchematicAttribute::create(const std::string & filename)
{
	return new ServerIntangibleObjectTemplate::_SchematicAttribute(filename);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerIntangibleObjectTemplate::_SchematicAttribute::getId(void) const
{
	return _SchematicAttribute_tag;
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::getId

const StringId ServerIntangibleObjectTemplate::_SchematicAttribute::getName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SchematicAttribute *>(m_baseData);
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
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::getName

int ServerIntangibleObjectTemplate::_SchematicAttribute::getValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SchematicAttribute *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValue(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValue(versionOk);
		}
	}

	int value = m_value.getValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValue(versionOk);
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
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::getValue

int ServerIntangibleObjectTemplate::_SchematicAttribute::getValueMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SchematicAttribute *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMin(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMin(versionOk);
		}
	}

	int value = m_value.getMinValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMin(versionOk);
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
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::getValueMin

int ServerIntangibleObjectTemplate::_SchematicAttribute::getValueMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SchematicAttribute *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMax(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMax(versionOk);
		}
	}

	int value = m_value.getMaxValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMax(versionOk);
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
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::getValueMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerIntangibleObjectTemplate::_SchematicAttribute::testValues(void) const
{
	IGNORE_RETURN(getName(true));
	IGNORE_RETURN(getValueMin(true));
	IGNORE_RETURN(getValueMax(true));
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerIntangibleObjectTemplate::_SchematicAttribute::load(Iff &file)
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
		else if (strcmp(paramName, "value") == 0)
			m_value.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::load


//=============================================================================
// class ServerIntangibleObjectTemplate::_SimpleIngredient

/**
 * Class constructor.
 */
ServerIntangibleObjectTemplate::_SimpleIngredient::_SimpleIngredient(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::_SimpleIngredient

/**
 * Class destructor.
 */
ServerIntangibleObjectTemplate::_SimpleIngredient::~_SimpleIngredient()
{
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::~_SimpleIngredient

/**
 * Static function used to register this template.
 */
void ServerIntangibleObjectTemplate::_SimpleIngredient::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_SimpleIngredient_tag, create);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::registerMe

/**
 * Creates a ServerIntangibleObjectTemplate::_SimpleIngredient template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerIntangibleObjectTemplate::_SimpleIngredient::create(const std::string & filename)
{
	return new ServerIntangibleObjectTemplate::_SimpleIngredient(filename);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerIntangibleObjectTemplate::_SimpleIngredient::getId(void) const
{
	return _SimpleIngredient_tag;
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::getId

const StringId ServerIntangibleObjectTemplate::_SimpleIngredient::getName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SimpleIngredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SimpleIngredient *>(m_baseData);
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
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::getName

const std::string & ServerIntangibleObjectTemplate::_SimpleIngredient::getIngredient(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SimpleIngredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SimpleIngredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getIngredient(true);
#endif
	}

	if (!m_ingredient.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter ingredient in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter ingredient has not been defined in template %s!", DataResource::getName()));
			return base->getIngredient(versionOk);
		}
	}

	const std::string & value = m_ingredient.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::getIngredient

int ServerIntangibleObjectTemplate::_SimpleIngredient::getCount(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SimpleIngredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SimpleIngredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCount(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCount(versionOk);
		}
	}

	int value = m_count.getValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCount(versionOk);
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
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::getCount

int ServerIntangibleObjectTemplate::_SimpleIngredient::getCountMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SimpleIngredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SimpleIngredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCountMin(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCountMin(versionOk);
		}
	}

	int value = m_count.getMinValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCountMin(versionOk);
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
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::getCountMin

int ServerIntangibleObjectTemplate::_SimpleIngredient::getCountMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerIntangibleObjectTemplate::_SimpleIngredient * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerIntangibleObjectTemplate::_SimpleIngredient *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCountMax(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCountMax(versionOk);
		}
	}

	int value = m_count.getMaxValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCountMax(versionOk);
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
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::getCountMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerIntangibleObjectTemplate::_SimpleIngredient::testValues(void) const
{
	IGNORE_RETURN(getName(true));
	IGNORE_RETURN(getIngredient(true));
	IGNORE_RETURN(getCountMin(true));
	IGNORE_RETURN(getCountMax(true));
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerIntangibleObjectTemplate::_SimpleIngredient::load(Iff &file)
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
		else if (strcmp(paramName, "count") == 0)
			m_count.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::load

//@END TFD
