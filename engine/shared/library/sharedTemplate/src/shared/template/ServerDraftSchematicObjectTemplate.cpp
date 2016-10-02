//========================================================================
//
// ServerDraftSchematicObjectTemplate.cpp
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
#include "ServerDraftSchematicObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerDraftSchematicObjectTemplate::ServerDraftSchematicObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
	,m_slotsLoaded(false)
	,m_slotsAppend(false)
	,m_skillCommandsLoaded(false)
	,m_skillCommandsAppend(false)
	,m_manufactureScriptsLoaded(false)
	,m_manufactureScriptsAppend(false)
//@END TFD INIT
{
}	// ServerDraftSchematicObjectTemplate::ServerDraftSchematicObjectTemplate

/**
 * Class destructor.
 */
ServerDraftSchematicObjectTemplate::~ServerDraftSchematicObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_slots.begin(); iter != m_slots.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_slots.clear();
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_skillCommands.begin(); iter != m_skillCommands.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_skillCommands.clear();
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_manufactureScripts.begin(); iter != m_manufactureScripts.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_manufactureScripts.clear();
	}
//@END TFD CLEANUP
}	// ServerDraftSchematicObjectTemplate::~ServerDraftSchematicObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerDraftSchematicObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerDraftSchematicObjectTemplate_tag, create);
}	// ServerDraftSchematicObjectTemplate::registerMe

/**
 * Creates a ServerDraftSchematicObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerDraftSchematicObjectTemplate::create(const std::string & filename)
{
	return new ServerDraftSchematicObjectTemplate(filename);
}	// ServerDraftSchematicObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerDraftSchematicObjectTemplate::getId(void) const
{
	return ServerDraftSchematicObjectTemplate_tag;
}	// ServerDraftSchematicObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerDraftSchematicObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerDraftSchematicObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerDraftSchematicObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerDraftSchematicObjectTemplate * base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerDraftSchematicObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerDraftSchematicObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "category") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_category;
		}
		fprintf(stderr, "trying to access single-parameter \"category\" as an array\n");
	}
	else if (strcmp(name, "itemsPerContainer") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_itemsPerContainer;
		}
		fprintf(stderr, "trying to access single-parameter \"itemsPerContainer\" as an array\n");
	}
	else
		return ServerIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerDraftSchematicObjectTemplate::getCompilerIntegerParam

FloatParam * ServerDraftSchematicObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "manufactureTime") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_manufactureTime;
		}
		fprintf(stderr, "trying to access single-parameter \"manufactureTime\" as an array\n");
	}
	else if (strcmp(name, "prototypeTime") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_prototypeTime;
		}
		fprintf(stderr, "trying to access single-parameter \"prototypeTime\" as an array\n");
	}
	else
		return ServerIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//ServerDraftSchematicObjectTemplate::getFloatParam

BoolParam * ServerDraftSchematicObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "destroyIngredients") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_destroyIngredients;
		}
		fprintf(stderr, "trying to access single-parameter \"destroyIngredients\" as an array\n");
	}
	else
		return ServerIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//ServerDraftSchematicObjectTemplate::getBoolParam

StringParam * ServerDraftSchematicObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "craftedObjectTemplate") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_craftedObjectTemplate;
		}
		fprintf(stderr, "trying to access single-parameter \"craftedObjectTemplate\" as an array\n");
	}
	else if (strcmp(name, "crateObjectTemplate") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_crateObjectTemplate;
		}
		fprintf(stderr, "trying to access single-parameter \"crateObjectTemplate\" as an array\n");
	}
	else if (strcmp(name, "skillCommands") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_skillCommands.size()))
			return m_skillCommands[index];
		if (index == static_cast<int>(m_skillCommands.size()))
		{
			StringParam *temp = new StringParam();
			m_skillCommands.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"skillCommands\" out of bounds\n");
	}
	else if (strcmp(name, "manufactureScripts") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_manufactureScripts.size()))
			return m_manufactureScripts[index];
		if (index == static_cast<int>(m_manufactureScripts.size()))
		{
			StringParam *temp = new StringParam();
			m_manufactureScripts.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"manufactureScripts\" out of bounds\n");
	}
	else
		return ServerIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerDraftSchematicObjectTemplate::getStringParam

StringIdParam * ServerDraftSchematicObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerDraftSchematicObjectTemplate::getStringIdParam

VectorParam * ServerDraftSchematicObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerDraftSchematicObjectTemplate::getVectorParam

DynamicVariableParam * ServerDraftSchematicObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerDraftSchematicObjectTemplate::getDynamicVariableParam

StructParamOT * ServerDraftSchematicObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "slots") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_slots.size()))
			return m_slots[index];
		if (index == static_cast<int>(m_slots.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_slots.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"slots\" out of bounds\n");
	}
	else
		return ServerIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
	return nullptr;
}	//ServerDraftSchematicObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerDraftSchematicObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerDraftSchematicObjectTemplate::getTriggerVolumeParam

void ServerDraftSchematicObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "slots") == 0)
		param.setValue(new _IngredientSlot(""));
	else
		ServerIntangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerDraftSchematicObjectTemplate::initStructParamOT

void ServerDraftSchematicObjectTemplate::setAsEmptyList(const char *name)
{
	if (strcmp(name, "slots") == 0)
	{
		m_slots.clear();
		m_slotsLoaded = true;
	}
	else if (strcmp(name, "skillCommands") == 0)
	{
		m_skillCommands.clear();
		m_skillCommandsLoaded = true;
	}
	else if (strcmp(name, "manufactureScripts") == 0)
	{
		m_manufactureScripts.clear();
		m_manufactureScriptsLoaded = true;
	}
	else
		ServerIntangibleObjectTemplate::setAsEmptyList(name);
}	// ServerDraftSchematicObjectTemplate::setAsEmptyList

void ServerDraftSchematicObjectTemplate::setAppend(const char *name)
{
	if (strcmp(name, "slots") == 0)
		m_slotsAppend = true;
	else if (strcmp(name, "skillCommands") == 0)
		m_skillCommandsAppend = true;
	else if (strcmp(name, "manufactureScripts") == 0)
		m_manufactureScriptsAppend = true;
	else
		ServerIntangibleObjectTemplate::setAppend(name);
}	// ServerDraftSchematicObjectTemplate::setAppend

bool ServerDraftSchematicObjectTemplate::isAppend(const char *name) const
{
	if (strcmp(name, "slots") == 0)
		return m_slotsAppend;
	else if (strcmp(name, "skillCommands") == 0)
		return m_skillCommandsAppend;
	else if (strcmp(name, "manufactureScripts") == 0)
		return m_manufactureScriptsAppend;
	else
		return ServerIntangibleObjectTemplate::isAppend(name);
}	// ServerDraftSchematicObjectTemplate::isAppend


int ServerDraftSchematicObjectTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "slots") == 0)
	{
		return m_slots.size();
	}
	else if (strcmp(name, "skillCommands") == 0)
	{
		return m_skillCommands.size();
	}
	else if (strcmp(name, "manufactureScripts") == 0)
	{
		return m_manufactureScripts.size();
	}
	else
		return ServerIntangibleObjectTemplate::getListLength(name);
}	// ServerDraftSchematicObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerDraftSchematicObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerDraftSchematicObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,7))
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
		if (strcmp(paramName, "category") == 0)
			m_category.loadFromIff(file);
		else if (strcmp(paramName, "craftedObjectTemplate") == 0)
			m_craftedObjectTemplate.loadFromIff(file);
		else if (strcmp(paramName, "crateObjectTemplate") == 0)
			m_crateObjectTemplate.loadFromIff(file);
		else if (strcmp(paramName, "slots") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_slots.begin(); iter != m_slots.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_slots.clear();
			m_slotsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_slots.push_back(newData);
			}
			m_slotsLoaded = true;
		}
		else if (strcmp(paramName, "skillCommands") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_skillCommands.begin(); iter != m_skillCommands.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_skillCommands.clear();
			m_skillCommandsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_skillCommands.push_back(newData);
			}
			m_skillCommandsLoaded = true;
		}
		else if (strcmp(paramName, "destroyIngredients") == 0)
			m_destroyIngredients.loadFromIff(file);
		else if (strcmp(paramName, "manufactureScripts") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_manufactureScripts.begin(); iter != m_manufactureScripts.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_manufactureScripts.clear();
			m_manufactureScriptsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_manufactureScripts.push_back(newData);
			}
			m_manufactureScriptsLoaded = true;
		}
		else if (strcmp(paramName, "itemsPerContainer") == 0)
			m_itemsPerContainer.loadFromIff(file);
		else if (strcmp(paramName, "manufactureTime") == 0)
			m_manufactureTime.loadFromIff(file);
		else if (strcmp(paramName, "prototypeTime") == 0)
			m_prototypeTime.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerIntangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerDraftSchematicObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerDraftSchematicObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerDraftSchematicObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,7));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save category
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("category");
	m_category.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save craftedObjectTemplate
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("craftedObjectTemplate");
	m_craftedObjectTemplate.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save crateObjectTemplate
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("crateObjectTemplate");
	m_crateObjectTemplate.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_slotsLoaded)
	{
		// mark the list as empty and extending the base list
		m_slotsAppend = true;
	}
	// save slots
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("slots");
	file.insertChunkData(&m_slotsAppend, sizeof(bool));
	count = m_slots.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_slots[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_skillCommandsLoaded)
	{
		// mark the list as empty and extending the base list
		m_skillCommandsAppend = true;
	}
	// save skillCommands
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("skillCommands");
	file.insertChunkData(&m_skillCommandsAppend, sizeof(bool));
	count = m_skillCommands.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_skillCommands[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save destroyIngredients
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("destroyIngredients");
	m_destroyIngredients.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_manufactureScriptsLoaded)
	{
		// mark the list as empty and extending the base list
		m_manufactureScriptsAppend = true;
	}
	// save manufactureScripts
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("manufactureScripts");
	file.insertChunkData(&m_manufactureScriptsAppend, sizeof(bool));
	count = m_manufactureScripts.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_manufactureScripts[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save itemsPerContainer
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("itemsPerContainer");
	m_itemsPerContainer.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save manufactureTime
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("manufactureTime");
	m_manufactureTime.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save prototypeTime
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("prototypeTime");
	m_prototypeTime.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	ServerIntangibleObjectTemplate::save(file);
	file.exitForm();
}	// ServerDraftSchematicObjectTemplate::save


//=============================================================================
// class ServerDraftSchematicObjectTemplate::_IngredientSlot

/**
 * Class constructor.
 */
ServerDraftSchematicObjectTemplate::_IngredientSlot::_IngredientSlot(const std::string & filename)
	: TpfTemplate(filename)
	,m_optionsLoaded(false)
	,m_optionsAppend(false)
{
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::_IngredientSlot

/**
 * Class destructor.
 */
ServerDraftSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot()
{
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_options.begin(); iter != m_options.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_options.clear();
	}
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot

/**
 * Static function used to register this template.
 */
void ServerDraftSchematicObjectTemplate::_IngredientSlot::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_IngredientSlot_tag, create);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::registerMe

/**
 * Creates a ServerDraftSchematicObjectTemplate::_IngredientSlot template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerDraftSchematicObjectTemplate::_IngredientSlot::create(const std::string & filename)
{
	return new ServerDraftSchematicObjectTemplate::_IngredientSlot(filename);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerDraftSchematicObjectTemplate::_IngredientSlot::getId(void) const
{
	return _IngredientSlot_tag;
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getId

CompilerIntegerParam * ServerDraftSchematicObjectTemplate::_IngredientSlot::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getCompilerIntegerParam

FloatParam * ServerDraftSchematicObjectTemplate::_IngredientSlot::getFloatParam(const char *name, bool deepCheck, int index)
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
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getFloatParam

BoolParam * ServerDraftSchematicObjectTemplate::_IngredientSlot::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "optional") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_optional;
		}
		fprintf(stderr, "trying to access single-parameter \"optional\" as an array\n");
	}
	else
		return TpfTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getBoolParam

StringParam * ServerDraftSchematicObjectTemplate::_IngredientSlot::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "optionalSkillCommand") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_optionalSkillCommand;
		}
		fprintf(stderr, "trying to access single-parameter \"optionalSkillCommand\" as an array\n");
	}
	else if (strcmp(name, "appearance") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_appearance;
		}
		fprintf(stderr, "trying to access single-parameter \"appearance\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getStringParam

StringIdParam * ServerDraftSchematicObjectTemplate::_IngredientSlot::getStringIdParam(const char *name, bool deepCheck, int index)
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
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getStringIdParam

VectorParam * ServerDraftSchematicObjectTemplate::_IngredientSlot::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getVectorParam

DynamicVariableParam * ServerDraftSchematicObjectTemplate::_IngredientSlot::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getDynamicVariableParam

StructParamOT * ServerDraftSchematicObjectTemplate::_IngredientSlot::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "options") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_options.size()))
			return m_options[index];
		if (index == static_cast<int>(m_options.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_options.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"options\" out of bounds\n");
	}
	else
		return TpfTemplate::getStructParamOT(name, deepCheck, index);
	return nullptr;
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getStructParamOT

TriggerVolumeParam * ServerDraftSchematicObjectTemplate::_IngredientSlot::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerDraftSchematicObjectTemplate::_IngredientSlot::getTriggerVolumeParam

void ServerDraftSchematicObjectTemplate::_IngredientSlot::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "options") == 0)
		param.setValue(new _Ingredient(""));
	else
		TpfTemplate::initStructParamOT(param, name);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::initStructParamOT

void ServerDraftSchematicObjectTemplate::_IngredientSlot::setAsEmptyList(const char *name)
{
	if (strcmp(name, "options") == 0)
	{
		m_options.clear();
		m_optionsLoaded = true;
	}
	else
		TpfTemplate::setAsEmptyList(name);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::setAsEmptyList

void ServerDraftSchematicObjectTemplate::_IngredientSlot::setAppend(const char *name)
{
	if (strcmp(name, "options") == 0)
		m_optionsAppend = true;
	else
		TpfTemplate::setAppend(name);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::setAppend

bool ServerDraftSchematicObjectTemplate::_IngredientSlot::isAppend(const char *name) const
{
	if (strcmp(name, "options") == 0)
		return m_optionsAppend;
	else
		return TpfTemplate::isAppend(name);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::isAppend


int ServerDraftSchematicObjectTemplate::_IngredientSlot::getListLength(const char *name) const
{
	if (strcmp(name, "options") == 0)
	{
		return m_options.size();
	}
	else
		return TpfTemplate::getListLength(name);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerDraftSchematicObjectTemplate::_IngredientSlot::load(Iff &file)
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
		if (strcmp(paramName, "optional") == 0)
			m_optional.loadFromIff(file);
		else if (strcmp(paramName, "name") == 0)
			m_name.loadFromIff(file);
		else if (strcmp(paramName, "options") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_options.begin(); iter != m_options.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_options.clear();
			m_optionsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_options.push_back(newData);
			}
			m_optionsLoaded = true;
		}
		else if (strcmp(paramName, "optionalSkillCommand") == 0)
			m_optionalSkillCommand.loadFromIff(file);
		else if (strcmp(paramName, "complexity") == 0)
			m_complexity.loadFromIff(file);
		else if (strcmp(paramName, "appearance") == 0)
			m_appearance.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerDraftSchematicObjectTemplate::_IngredientSlot::save(Iff &file)
{
int count;

	file.insertForm(_IngredientSlot_tag);

	int paramCount = 0;

	// save optional
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("optional");
	m_optional.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save name
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("name");
	m_name.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_optionsLoaded)
	{
		// mark the list as empty and extending the base list
		m_optionsAppend = true;
	}
	// save options
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("options");
	file.insertChunkData(&m_optionsAppend, sizeof(bool));
	count = m_options.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_options[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save optionalSkillCommand
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("optionalSkillCommand");
	m_optionalSkillCommand.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save complexity
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("complexity");
	m_complexity.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save appearance
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("appearance");
	m_appearance.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::save

//@END TFD
