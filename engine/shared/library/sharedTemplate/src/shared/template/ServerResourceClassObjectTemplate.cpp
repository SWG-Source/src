//========================================================================
//
// ServerResourceClassObjectTemplate.cpp
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
#include "ServerResourceClassObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerResourceClassObjectTemplate::ServerResourceClassObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerResourceClassObjectTemplate::ServerResourceClassObjectTemplate

/**
 * Class destructor.
 */
ServerResourceClassObjectTemplate::~ServerResourceClassObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerResourceClassObjectTemplate::~ServerResourceClassObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerResourceClassObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerResourceClassObjectTemplate_tag, create);
}	// ServerResourceClassObjectTemplate::registerMe

/**
 * Creates a ServerResourceClassObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerResourceClassObjectTemplate::create(const std::string & filename)
{
	return new ServerResourceClassObjectTemplate(filename);
}	// ServerResourceClassObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerResourceClassObjectTemplate::getId(void) const
{
	return ServerResourceClassObjectTemplate_tag;
}	// ServerResourceClassObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerResourceClassObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerResourceClassObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerResourceClassObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerResourceClassObjectTemplate * base = dynamic_cast<const ServerResourceClassObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerResourceClassObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerResourceClassObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "numTypes") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_numTypes;
		}
		fprintf(stderr, "trying to access single-parameter \"numTypes\" as an array\n");
	}
	else if (strcmp(name, "minTypes") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_minTypes;
		}
		fprintf(stderr, "trying to access single-parameter \"minTypes\" as an array\n");
	}
	else if (strcmp(name, "maxTypes") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_maxTypes;
		}
		fprintf(stderr, "trying to access single-parameter \"maxTypes\" as an array\n");
	}
	else
		return ServerUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return NULL;
}	//ServerResourceClassObjectTemplate::getCompilerIntegerParam

FloatParam * ServerResourceClassObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerResourceClassObjectTemplate::getFloatParam

BoolParam * ServerResourceClassObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerResourceClassObjectTemplate::getBoolParam

StringParam * ServerResourceClassObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "resourceClassName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_resourceClassName;
		}
		fprintf(stderr, "trying to access single-parameter \"resourceClassName\" as an array\n");
	}
	else if (strcmp(name, "parentClass") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_parentClass;
		}
		fprintf(stderr, "trying to access single-parameter \"parentClass\" as an array\n");
	}
	else
		return ServerUniverseObjectTemplate::getStringParam(name, deepCheck, index);
	return NULL;
}	//ServerResourceClassObjectTemplate::getStringParam

StringIdParam * ServerResourceClassObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerResourceClassObjectTemplate::getStringIdParam

VectorParam * ServerResourceClassObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerResourceClassObjectTemplate::getVectorParam

DynamicVariableParam * ServerResourceClassObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerResourceClassObjectTemplate::getDynamicVariableParam

StructParamOT * ServerResourceClassObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerResourceClassObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerResourceClassObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerResourceClassObjectTemplate::getTriggerVolumeParam

void ServerResourceClassObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerUniverseObjectTemplate::initStructParamOT(param, name);
}	// ServerResourceClassObjectTemplate::initStructParamOT

void ServerResourceClassObjectTemplate::setAsEmptyList(const char *name)
{
	ServerUniverseObjectTemplate::setAsEmptyList(name);
}	// ServerResourceClassObjectTemplate::setAsEmptyList

void ServerResourceClassObjectTemplate::setAppend(const char *name)
{
	ServerUniverseObjectTemplate::setAppend(name);
}	// ServerResourceClassObjectTemplate::setAppend

bool ServerResourceClassObjectTemplate::isAppend(const char *name) const
{
	return ServerUniverseObjectTemplate::isAppend(name);
}	// ServerResourceClassObjectTemplate::isAppend


int ServerResourceClassObjectTemplate::getListLength(const char *name) const
{
	return ServerUniverseObjectTemplate::getListLength(name);
}	// ServerResourceClassObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerResourceClassObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerResourceClassObjectTemplate_tag)
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
		if (strcmp(paramName, "resourceClassName") == 0)
			m_resourceClassName.loadFromIff(file);
		else if (strcmp(paramName, "numTypes") == 0)
			m_numTypes.loadFromIff(file);
		else if (strcmp(paramName, "minTypes") == 0)
			m_minTypes.loadFromIff(file);
		else if (strcmp(paramName, "maxTypes") == 0)
			m_maxTypes.loadFromIff(file);
		else if (strcmp(paramName, "parentClass") == 0)
			m_parentClass.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerUniverseObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerResourceClassObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerResourceClassObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerResourceClassObjectTemplate_tag);
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

	// save resourceClassName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("resourceClassName");
	m_resourceClassName.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save numTypes
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("numTypes");
	m_numTypes.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save minTypes
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("minTypes");
	m_minTypes.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxTypes
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxTypes");
	m_maxTypes.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save parentClass
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("parentClass");
	m_parentClass.saveToIff(file);
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
}	// ServerResourceClassObjectTemplate::save

//@END TFD
