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

#include "sharedTemplate/FirstSharedTemplate.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateGlobals.h"
#include "ServerIntangibleObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerIntangibleObjectTemplate::ServerIntangibleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
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

//@BEGIN TFD
CompilerIntegerParam * ServerIntangibleObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "count") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_count;
		}
		fprintf(stderr, "trying to access single-parameter \"count\" as an array\n");
	}
	else
		return ServerObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::getCompilerIntegerParam

FloatParam * ServerIntangibleObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::getFloatParam

BoolParam * ServerIntangibleObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::getBoolParam

StringParam * ServerIntangibleObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::getStringParam

StringIdParam * ServerIntangibleObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::getStringIdParam

VectorParam * ServerIntangibleObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::getVectorParam

DynamicVariableParam * ServerIntangibleObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::getDynamicVariableParam

StructParamOT * ServerIntangibleObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerIntangibleObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::getTriggerVolumeParam

void ServerIntangibleObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerObjectTemplate::initStructParamOT(param, name);
}	// ServerIntangibleObjectTemplate::initStructParamOT

void ServerIntangibleObjectTemplate::setAsEmptyList(const char *name)
{
	ServerObjectTemplate::setAsEmptyList(name);
}	// ServerIntangibleObjectTemplate::setAsEmptyList

void ServerIntangibleObjectTemplate::setAppend(const char *name)
{
	ServerObjectTemplate::setAppend(name);
}	// ServerIntangibleObjectTemplate::setAppend

bool ServerIntangibleObjectTemplate::isAppend(const char *name) const
{
	return ServerObjectTemplate::isAppend(name);
}	// ServerIntangibleObjectTemplate::isAppend


int ServerIntangibleObjectTemplate::getListLength(const char *name) const
{
	return ServerObjectTemplate::getListLength(name);
}	// ServerIntangibleObjectTemplate::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerIntangibleObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerIntangibleObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,1));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save count
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("count");
	m_count.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	ServerObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerIntangibleObjectTemplate::save


//=============================================================================
// class ServerIntangibleObjectTemplate::_Ingredient

/**
 * Class constructor.
 */
ServerIntangibleObjectTemplate::_Ingredient::_Ingredient(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * ServerIntangibleObjectTemplate::_Ingredient::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "ingredientType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_ingredientType;
		}
		fprintf(stderr, "trying to access single-parameter \"ingredientType\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_Ingredient::getCompilerIntegerParam

FloatParam * ServerIntangibleObjectTemplate::_Ingredient::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "complexity") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_complexity;
		}
		fprintf(stderr, "trying to access single-parameter \"complexity\" as an array\n");
	}
	else
		return TpfTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_Ingredient::getFloatParam

BoolParam * ServerIntangibleObjectTemplate::_Ingredient::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_Ingredient::getBoolParam

StringParam * ServerIntangibleObjectTemplate::_Ingredient::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "skillCommand") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_skillCommand;
		}
		fprintf(stderr, "trying to access single-parameter \"skillCommand\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_Ingredient::getStringParam

StringIdParam * ServerIntangibleObjectTemplate::_Ingredient::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_Ingredient::getStringIdParam

VectorParam * ServerIntangibleObjectTemplate::_Ingredient::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_Ingredient::getVectorParam

DynamicVariableParam * ServerIntangibleObjectTemplate::_Ingredient::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_Ingredient::getDynamicVariableParam

StructParamOT * ServerIntangibleObjectTemplate::_Ingredient::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "ingredients") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_ingredients.size()))
			return m_ingredients[index];
		if (index == static_cast<int>(m_ingredients.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_ingredients.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"ingredients\" out of bounds\n");
	}
	else
		return TpfTemplate::getStructParamOT(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_Ingredient::getStructParamOT

TriggerVolumeParam * ServerIntangibleObjectTemplate::_Ingredient::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_Ingredient::getTriggerVolumeParam

void ServerIntangibleObjectTemplate::_Ingredient::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "ingredients") == 0)
		param.setValue(new _SimpleIngredient(""));
	else
		TpfTemplate::initStructParamOT(param, name);
}	// ServerIntangibleObjectTemplate::_Ingredient::initStructParamOT

void ServerIntangibleObjectTemplate::_Ingredient::setAsEmptyList(const char *name)
{
	if (strcmp(name, "ingredients") == 0)
	{
		m_ingredients.clear();
		m_ingredientsLoaded = true;
	}
	else
		TpfTemplate::setAsEmptyList(name);
}	// ServerIntangibleObjectTemplate::_Ingredient::setAsEmptyList

void ServerIntangibleObjectTemplate::_Ingredient::setAppend(const char *name)
{
	if (strcmp(name, "ingredients") == 0)
		m_ingredientsAppend = true;
	else
		TpfTemplate::setAppend(name);
}	// ServerIntangibleObjectTemplate::_Ingredient::setAppend

bool ServerIntangibleObjectTemplate::_Ingredient::isAppend(const char *name) const
{
	if (strcmp(name, "ingredients") == 0)
		return m_ingredientsAppend;
	else
		return TpfTemplate::isAppend(name);
}	// ServerIntangibleObjectTemplate::_Ingredient::isAppend


int ServerIntangibleObjectTemplate::_Ingredient::getListLength(const char *name) const
{
	if (strcmp(name, "ingredients") == 0)
	{
		return m_ingredients.size();
	}
	else
		return TpfTemplate::getListLength(name);
}	// ServerIntangibleObjectTemplate::_Ingredient::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerIntangibleObjectTemplate::_Ingredient::save(Iff &file)
{
int count;

	file.insertForm(_Ingredient_tag);

	int paramCount = 0;

	// save ingredientType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("ingredientType");
	m_ingredientType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_ingredientsLoaded)
	{
		// mark the list as empty and extending the base list
		m_ingredientsAppend = true;
	}
	// save ingredients
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("ingredients");
	file.insertChunkData(&m_ingredientsAppend, sizeof(bool));
	count = m_ingredients.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_ingredients[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save complexity
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("complexity");
	m_complexity.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save skillCommand
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("skillCommand");
	m_skillCommand.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
}	// ServerIntangibleObjectTemplate::_Ingredient::save


//=============================================================================
// class ServerIntangibleObjectTemplate::_SchematicAttribute

/**
 * Class constructor.
 */
ServerIntangibleObjectTemplate::_SchematicAttribute::_SchematicAttribute(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * ServerIntangibleObjectTemplate::_SchematicAttribute::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "value") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_value;
		}
		fprintf(stderr, "trying to access single-parameter \"value\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getCompilerIntegerParam

FloatParam * ServerIntangibleObjectTemplate::_SchematicAttribute::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getFloatParam

BoolParam * ServerIntangibleObjectTemplate::_SchematicAttribute::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getBoolParam

StringParam * ServerIntangibleObjectTemplate::_SchematicAttribute::getStringParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getStringParam

StringIdParam * ServerIntangibleObjectTemplate::_SchematicAttribute::getStringIdParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "name") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_name;
		}
		fprintf(stderr, "trying to access single-parameter \"name\" as an array\n");
	}
	else
		return TpfTemplate::getStringIdParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getStringIdParam

VectorParam * ServerIntangibleObjectTemplate::_SchematicAttribute::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getVectorParam

DynamicVariableParam * ServerIntangibleObjectTemplate::_SchematicAttribute::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getDynamicVariableParam

StructParamOT * ServerIntangibleObjectTemplate::_SchematicAttribute::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getStructParamOT

TriggerVolumeParam * ServerIntangibleObjectTemplate::_SchematicAttribute::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SchematicAttribute::getTriggerVolumeParam

void ServerIntangibleObjectTemplate::_SchematicAttribute::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::initStructParamOT

void ServerIntangibleObjectTemplate::_SchematicAttribute::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::setAsEmptyList

void ServerIntangibleObjectTemplate::_SchematicAttribute::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::setAppend

bool ServerIntangibleObjectTemplate::_SchematicAttribute::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::isAppend


int ServerIntangibleObjectTemplate::_SchematicAttribute::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerIntangibleObjectTemplate::_SchematicAttribute::save(Iff &file)
{
int count;

	file.insertForm(_SchematicAttribute_tag);

	int paramCount = 0;

	// save name
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("name");
	m_name.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save value
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("value");
	m_value.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// ServerIntangibleObjectTemplate::_SchematicAttribute::save


//=============================================================================
// class ServerIntangibleObjectTemplate::_SimpleIngredient

/**
 * Class constructor.
 */
ServerIntangibleObjectTemplate::_SimpleIngredient::_SimpleIngredient(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * ServerIntangibleObjectTemplate::_SimpleIngredient::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "count") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_count;
		}
		fprintf(stderr, "trying to access single-parameter \"count\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getCompilerIntegerParam

FloatParam * ServerIntangibleObjectTemplate::_SimpleIngredient::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getFloatParam

BoolParam * ServerIntangibleObjectTemplate::_SimpleIngredient::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getBoolParam

StringParam * ServerIntangibleObjectTemplate::_SimpleIngredient::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "ingredient") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_ingredient;
		}
		fprintf(stderr, "trying to access single-parameter \"ingredient\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getStringParam

StringIdParam * ServerIntangibleObjectTemplate::_SimpleIngredient::getStringIdParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "name") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_name;
		}
		fprintf(stderr, "trying to access single-parameter \"name\" as an array\n");
	}
	else
		return TpfTemplate::getStringIdParam(name, deepCheck, index);
	return nullptr;
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getStringIdParam

VectorParam * ServerIntangibleObjectTemplate::_SimpleIngredient::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getVectorParam

DynamicVariableParam * ServerIntangibleObjectTemplate::_SimpleIngredient::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getDynamicVariableParam

StructParamOT * ServerIntangibleObjectTemplate::_SimpleIngredient::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getStructParamOT

TriggerVolumeParam * ServerIntangibleObjectTemplate::_SimpleIngredient::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerIntangibleObjectTemplate::_SimpleIngredient::getTriggerVolumeParam

void ServerIntangibleObjectTemplate::_SimpleIngredient::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::initStructParamOT

void ServerIntangibleObjectTemplate::_SimpleIngredient::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::setAsEmptyList

void ServerIntangibleObjectTemplate::_SimpleIngredient::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::setAppend

bool ServerIntangibleObjectTemplate::_SimpleIngredient::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::isAppend


int ServerIntangibleObjectTemplate::_SimpleIngredient::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerIntangibleObjectTemplate::_SimpleIngredient::save(Iff &file)
{
int count;

	file.insertForm(_SimpleIngredient_tag);

	int paramCount = 0;

	// save name
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("name");
	m_name.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save ingredient
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("ingredient");
	m_ingredient.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save count
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("count");
	m_count.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// ServerIntangibleObjectTemplate::_SimpleIngredient::save

//@END TFD
