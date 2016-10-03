//========================================================================
//
// ServerResourceTypeObjectTemplate.cpp
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
#include "ServerResourceTypeObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerResourceTypeObjectTemplate::ServerResourceTypeObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerResourceTypeObjectTemplate::ServerResourceTypeObjectTemplate

/**
 * Class destructor.
 */
ServerResourceTypeObjectTemplate::~ServerResourceTypeObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerResourceTypeObjectTemplate::~ServerResourceTypeObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerResourceTypeObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerResourceTypeObjectTemplate_tag, create);
}	// ServerResourceTypeObjectTemplate::registerMe

/**
 * Creates a ServerResourceTypeObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerResourceTypeObjectTemplate::create(const std::string & filename)
{
	return new ServerResourceTypeObjectTemplate(filename);
}	// ServerResourceTypeObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerResourceTypeObjectTemplate::getId(void) const
{
	return ServerResourceTypeObjectTemplate_tag;
}	// ServerResourceTypeObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerResourceTypeObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerResourceTypeObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerResourceTypeObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerResourceTypeObjectTemplate * base = dynamic_cast<const ServerResourceTypeObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerResourceTypeObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerResourceTypeObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerResourceTypeObjectTemplate::getCompilerIntegerParam

FloatParam * ServerResourceTypeObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerResourceTypeObjectTemplate::getFloatParam

BoolParam * ServerResourceTypeObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerResourceTypeObjectTemplate::getBoolParam

StringParam * ServerResourceTypeObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "resourceName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_resourceName;
		}
		fprintf(stderr, "trying to access single-parameter \"resourceName\" as an array\n");
	}
	else
		return ServerUniverseObjectTemplate::getStringParam(name, deepCheck, index);
	return NULL;
}	//ServerResourceTypeObjectTemplate::getStringParam

StringIdParam * ServerResourceTypeObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerResourceTypeObjectTemplate::getStringIdParam

VectorParam * ServerResourceTypeObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerResourceTypeObjectTemplate::getVectorParam

DynamicVariableParam * ServerResourceTypeObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerResourceTypeObjectTemplate::getDynamicVariableParam

StructParamOT * ServerResourceTypeObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerResourceTypeObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerResourceTypeObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerResourceTypeObjectTemplate::getTriggerVolumeParam

void ServerResourceTypeObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerUniverseObjectTemplate::initStructParamOT(param, name);
}	// ServerResourceTypeObjectTemplate::initStructParamOT

void ServerResourceTypeObjectTemplate::setAsEmptyList(const char *name)
{
	ServerUniverseObjectTemplate::setAsEmptyList(name);
}	// ServerResourceTypeObjectTemplate::setAsEmptyList

void ServerResourceTypeObjectTemplate::setAppend(const char *name)
{
	ServerUniverseObjectTemplate::setAppend(name);
}	// ServerResourceTypeObjectTemplate::setAppend

bool ServerResourceTypeObjectTemplate::isAppend(const char *name) const
{
	return ServerUniverseObjectTemplate::isAppend(name);
}	// ServerResourceTypeObjectTemplate::isAppend


int ServerResourceTypeObjectTemplate::getListLength(const char *name) const
{
	return ServerUniverseObjectTemplate::getListLength(name);
}	// ServerResourceTypeObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerResourceTypeObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerResourceTypeObjectTemplate_tag)
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
		if (strcmp(paramName, "resourceName") == 0)
			m_resourceName.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerUniverseObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerResourceTypeObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerResourceTypeObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerResourceTypeObjectTemplate_tag);
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

	// save resourceName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("resourceName");
	m_resourceName.saveToIff(file);
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
}	// ServerResourceTypeObjectTemplate::save

//@END TFD
