//========================================================================
//
// ServerResourcePoolObjectTemplate.cpp
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
#include "ServerResourcePoolObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerResourcePoolObjectTemplate::ServerResourcePoolObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerResourcePoolObjectTemplate::ServerResourcePoolObjectTemplate

/**
 * Class destructor.
 */
ServerResourcePoolObjectTemplate::~ServerResourcePoolObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerResourcePoolObjectTemplate::~ServerResourcePoolObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerResourcePoolObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerResourcePoolObjectTemplate_tag, create);
}	// ServerResourcePoolObjectTemplate::registerMe

/**
 * Creates a ServerResourcePoolObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerResourcePoolObjectTemplate::create(const std::string & filename)
{
	return new ServerResourcePoolObjectTemplate(filename);
}	// ServerResourcePoolObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerResourcePoolObjectTemplate::getId(void) const
{
	return ServerResourcePoolObjectTemplate_tag;
}	// ServerResourcePoolObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerResourcePoolObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerResourcePoolObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerResourcePoolObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerResourcePoolObjectTemplate * base = dynamic_cast<const ServerResourcePoolObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerResourcePoolObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerResourcePoolObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "mapSeed") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_mapSeed;
		}
		fprintf(stderr, "trying to access single-parameter \"mapSeed\" as an array\n");
	}
	else if (strcmp(name, "amountRemaining") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_amountRemaining;
		}
		fprintf(stderr, "trying to access single-parameter \"amountRemaining\" as an array\n");
	}
	else
		return ServerUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return NULL;
}	//ServerResourcePoolObjectTemplate::getCompilerIntegerParam

FloatParam * ServerResourcePoolObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerResourcePoolObjectTemplate::getFloatParam

BoolParam * ServerResourcePoolObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerResourcePoolObjectTemplate::getBoolParam

StringParam * ServerResourcePoolObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerResourcePoolObjectTemplate::getStringParam

StringIdParam * ServerResourcePoolObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerResourcePoolObjectTemplate::getStringIdParam

VectorParam * ServerResourcePoolObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerResourcePoolObjectTemplate::getVectorParam

DynamicVariableParam * ServerResourcePoolObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerResourcePoolObjectTemplate::getDynamicVariableParam

StructParamOT * ServerResourcePoolObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerResourcePoolObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerResourcePoolObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerResourcePoolObjectTemplate::getTriggerVolumeParam

void ServerResourcePoolObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerUniverseObjectTemplate::initStructParamOT(param, name);
}	// ServerResourcePoolObjectTemplate::initStructParamOT

void ServerResourcePoolObjectTemplate::setAsEmptyList(const char *name)
{
	ServerUniverseObjectTemplate::setAsEmptyList(name);
}	// ServerResourcePoolObjectTemplate::setAsEmptyList

void ServerResourcePoolObjectTemplate::setAppend(const char *name)
{
	ServerUniverseObjectTemplate::setAppend(name);
}	// ServerResourcePoolObjectTemplate::setAppend

bool ServerResourcePoolObjectTemplate::isAppend(const char *name) const
{
	return ServerUniverseObjectTemplate::isAppend(name);
}	// ServerResourcePoolObjectTemplate::isAppend


int ServerResourcePoolObjectTemplate::getListLength(const char *name) const
{
	return ServerUniverseObjectTemplate::getListLength(name);
}	// ServerResourcePoolObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerResourcePoolObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerResourcePoolObjectTemplate_tag)
	{
		ServerUniverseObjectTemplate::load(file);
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
		DEBUG_WARNING(base == NULL, ("was unable to load base template %s", baseFilename.c_str()));
		if (m_baseData == base && base != NULL)
			base->releaseReference();
		else
		{
			if (m_baseData != NULL)
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
	}

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "mapSeed") == 0)
			m_mapSeed.loadFromIff(file);
		else if (strcmp(paramName, "amountRemaining") == 0)
			m_amountRemaining.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerUniverseObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerResourcePoolObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerResourcePoolObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerResourcePoolObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,0));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save mapSeed
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("mapSeed");
	m_mapSeed.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save amountRemaining
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("amountRemaining");
	m_amountRemaining.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	ServerUniverseObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerResourcePoolObjectTemplate::save

//@END TFD
