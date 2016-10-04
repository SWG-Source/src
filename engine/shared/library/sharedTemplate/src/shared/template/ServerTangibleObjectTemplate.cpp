//========================================================================
//
// ServerTangibleObjectTemplate.cpp
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
#include "ServerTangibleObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerTangibleObjectTemplate::ServerTangibleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
	,m_triggerVolumesLoaded(false)
	,m_triggerVolumesAppend(false)
//@END TFD INIT
{
}	// ServerTangibleObjectTemplate::ServerTangibleObjectTemplate

/**
 * Class destructor.
 */
ServerTangibleObjectTemplate::~ServerTangibleObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<TriggerVolumeParam *>::iterator iter;
		for (iter = m_triggerVolumes.begin(); iter != m_triggerVolumes.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_triggerVolumes.clear();
	}
//@END TFD CLEANUP
}	// ServerTangibleObjectTemplate::~ServerTangibleObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerTangibleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerTangibleObjectTemplate_tag, create);
}	// ServerTangibleObjectTemplate::registerMe

/**
 * Creates a ServerTangibleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerTangibleObjectTemplate::create(const std::string & filename)
{
	return new ServerTangibleObjectTemplate(filename);
}	// ServerTangibleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerTangibleObjectTemplate::getId(void) const
{
	return ServerTangibleObjectTemplate_tag;
}	// ServerTangibleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerTangibleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerTangibleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerTangibleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerTangibleObjectTemplate * base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerTangibleObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerTangibleObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "combatSkeleton") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_combatSkeleton;
		}
		fprintf(stderr, "trying to access single-parameter \"combatSkeleton\" as an array\n");
	}
	else if (strcmp(name, "maxHitPoints") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxHitPoints;
		}
		fprintf(stderr, "trying to access single-parameter \"maxHitPoints\" as an array\n");
	}
	else if (strcmp(name, "interestRadius") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_interestRadius;
		}
		fprintf(stderr, "trying to access single-parameter \"interestRadius\" as an array\n");
	}
	else if (strcmp(name, "count") == 0)
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
	else if (strcmp(name, "condition") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_condition;
		}
		fprintf(stderr, "trying to access single-parameter \"condition\" as an array\n");
	}
	else
		return ServerObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerTangibleObjectTemplate::getCompilerIntegerParam

FloatParam * ServerTangibleObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerTangibleObjectTemplate::getFloatParam

BoolParam * ServerTangibleObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "wantSawAttackTriggers") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_wantSawAttackTriggers;
		}
		fprintf(stderr, "trying to access single-parameter \"wantSawAttackTriggers\" as an array\n");
	}
	else
		return ServerObjectTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//ServerTangibleObjectTemplate::getBoolParam

StringParam * ServerTangibleObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "armor") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_armor;
		}
		fprintf(stderr, "trying to access single-parameter \"armor\" as an array\n");
	}
	else
		return ServerObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerTangibleObjectTemplate::getStringParam

StringIdParam * ServerTangibleObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerTangibleObjectTemplate::getStringIdParam

VectorParam * ServerTangibleObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerTangibleObjectTemplate::getVectorParam

DynamicVariableParam * ServerTangibleObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerTangibleObjectTemplate::getDynamicVariableParam

StructParamOT * ServerTangibleObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerTangibleObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerTangibleObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "triggerVolumes") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_triggerVolumes.size()))
			return m_triggerVolumes[index];
		if (index == static_cast<int>(m_triggerVolumes.size()))
		{
			TriggerVolumeParam *temp = new TriggerVolumeParam();
			m_triggerVolumes.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"triggerVolumes\" out of bounds\n");
	}
	else
		return ServerObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
	return nullptr;
}	//ServerTangibleObjectTemplate::getTriggerVolumeParam

void ServerTangibleObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerObjectTemplate::initStructParamOT(param, name);
}	// ServerTangibleObjectTemplate::initStructParamOT

void ServerTangibleObjectTemplate::setAsEmptyList(const char *name)
{
	if (strcmp(name, "triggerVolumes") == 0)
	{
		m_triggerVolumes.clear();
		m_triggerVolumesLoaded = true;
	}
	else
		ServerObjectTemplate::setAsEmptyList(name);
}	// ServerTangibleObjectTemplate::setAsEmptyList

void ServerTangibleObjectTemplate::setAppend(const char *name)
{
	if (strcmp(name, "triggerVolumes") == 0)
		m_triggerVolumesAppend = true;
	else
		ServerObjectTemplate::setAppend(name);
}	// ServerTangibleObjectTemplate::setAppend

bool ServerTangibleObjectTemplate::isAppend(const char *name) const
{
	if (strcmp(name, "triggerVolumes") == 0)
		return m_triggerVolumesAppend;
	else
		return ServerObjectTemplate::isAppend(name);
}	// ServerTangibleObjectTemplate::isAppend


int ServerTangibleObjectTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "triggerVolumes") == 0)
	{
		return m_triggerVolumes.size();
	}
	else
		return ServerObjectTemplate::getListLength(name);
}	// ServerTangibleObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerTangibleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerTangibleObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,4))
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
		if (strcmp(paramName, "triggerVolumes") == 0)
		{
			std::vector<TriggerVolumeParam *>::iterator iter;
			for (iter = m_triggerVolumes.begin(); iter != m_triggerVolumes.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_triggerVolumes.clear();
			m_triggerVolumesAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				TriggerVolumeParam * newData = new TriggerVolumeParam;
				newData->loadFromIff(file);
				m_triggerVolumes.push_back(newData);
			}
			m_triggerVolumesLoaded = true;
		}
		else if (strcmp(paramName, "combatSkeleton") == 0)
			m_combatSkeleton.loadFromIff(file);
		else if (strcmp(paramName, "maxHitPoints") == 0)
			m_maxHitPoints.loadFromIff(file);
		else if (strcmp(paramName, "armor") == 0)
			m_armor.loadFromIff(file);
		else if (strcmp(paramName, "interestRadius") == 0)
			m_interestRadius.loadFromIff(file);
		else if (strcmp(paramName, "count") == 0)
			m_count.loadFromIff(file);
		else if (strcmp(paramName, "condition") == 0)
			m_condition.loadFromIff(file);
		else if (strcmp(paramName, "wantSawAttackTriggers") == 0)
			m_wantSawAttackTriggers.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerTangibleObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerTangibleObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerTangibleObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,4));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	if (!m_triggerVolumesLoaded)
	{
		// mark the list as empty and extending the base list
		m_triggerVolumesAppend = true;
	}
	// save triggerVolumes
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("triggerVolumes");
	file.insertChunkData(&m_triggerVolumesAppend, sizeof(bool));
	count = m_triggerVolumes.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_triggerVolumes[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save combatSkeleton
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("combatSkeleton");
	m_combatSkeleton.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxHitPoints
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxHitPoints");
	m_maxHitPoints.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save armor
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("armor");
	m_armor.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save interestRadius
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("interestRadius");
	m_interestRadius.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save count
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("count");
	m_count.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save condition
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("condition");
	m_condition.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save wantSawAttackTriggers
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("wantSawAttackTriggers");
	m_wantSawAttackTriggers.saveToIff(file);
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
}	// ServerTangibleObjectTemplate::save

//@END TFD
