//========================================================================
//
// ServerHarvesterInstallationObjectTemplate.cpp
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
#include "ServerHarvesterInstallationObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerInstallationObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate

/**
 * Class destructor.
 */
ServerHarvesterInstallationObjectTemplate::~ServerHarvesterInstallationObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerHarvesterInstallationObjectTemplate::~ServerHarvesterInstallationObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerHarvesterInstallationObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerHarvesterInstallationObjectTemplate_tag, create);
}	// ServerHarvesterInstallationObjectTemplate::registerMe

/**
 * Creates a ServerHarvesterInstallationObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerHarvesterInstallationObjectTemplate::create(const std::string & filename)
{
	return new ServerHarvesterInstallationObjectTemplate(filename);
}	// ServerHarvesterInstallationObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerHarvesterInstallationObjectTemplate::getId(void) const
{
	return ServerHarvesterInstallationObjectTemplate_tag;
}	// ServerHarvesterInstallationObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerHarvesterInstallationObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerHarvesterInstallationObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerHarvesterInstallationObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerHarvesterInstallationObjectTemplate * base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerHarvesterInstallationObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerHarvesterInstallationObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "maxExtractionRate") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxExtractionRate;
		}
		fprintf(stderr, "trying to access single-parameter \"maxExtractionRate\" as an array\n");
	}
	else if (strcmp(name, "currentExtractionRate") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_currentExtractionRate;
		}
		fprintf(stderr, "trying to access single-parameter \"currentExtractionRate\" as an array\n");
	}
	else if (strcmp(name, "maxHopperSize") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxHopperSize;
		}
		fprintf(stderr, "trying to access single-parameter \"maxHopperSize\" as an array\n");
	}
	else
		return ServerInstallationObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerHarvesterInstallationObjectTemplate::getCompilerIntegerParam

FloatParam * ServerHarvesterInstallationObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerHarvesterInstallationObjectTemplate::getFloatParam

BoolParam * ServerHarvesterInstallationObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerHarvesterInstallationObjectTemplate::getBoolParam

StringParam * ServerHarvesterInstallationObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "masterClassName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_masterClassName;
		}
		fprintf(stderr, "trying to access single-parameter \"masterClassName\" as an array\n");
	}
	else
		return ServerInstallationObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerHarvesterInstallationObjectTemplate::getStringParam

StringIdParam * ServerHarvesterInstallationObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerHarvesterInstallationObjectTemplate::getStringIdParam

VectorParam * ServerHarvesterInstallationObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerHarvesterInstallationObjectTemplate::getVectorParam

DynamicVariableParam * ServerHarvesterInstallationObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerHarvesterInstallationObjectTemplate::getDynamicVariableParam

StructParamOT * ServerHarvesterInstallationObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerHarvesterInstallationObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerHarvesterInstallationObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerHarvesterInstallationObjectTemplate::getTriggerVolumeParam

void ServerHarvesterInstallationObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerInstallationObjectTemplate::initStructParamOT(param, name);
}	// ServerHarvesterInstallationObjectTemplate::initStructParamOT

void ServerHarvesterInstallationObjectTemplate::setAsEmptyList(const char *name)
{
	ServerInstallationObjectTemplate::setAsEmptyList(name);
}	// ServerHarvesterInstallationObjectTemplate::setAsEmptyList

void ServerHarvesterInstallationObjectTemplate::setAppend(const char *name)
{
	ServerInstallationObjectTemplate::setAppend(name);
}	// ServerHarvesterInstallationObjectTemplate::setAppend

bool ServerHarvesterInstallationObjectTemplate::isAppend(const char *name) const
{
	return ServerInstallationObjectTemplate::isAppend(name);
}	// ServerHarvesterInstallationObjectTemplate::isAppend


int ServerHarvesterInstallationObjectTemplate::getListLength(const char *name) const
{
	return ServerInstallationObjectTemplate::getListLength(name);
}	// ServerHarvesterInstallationObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerHarvesterInstallationObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerHarvesterInstallationObjectTemplate_tag)
	{
		ServerInstallationObjectTemplate::load(file);
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
	}

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "maxExtractionRate") == 0)
			m_maxExtractionRate.loadFromIff(file);
		else if (strcmp(paramName, "currentExtractionRate") == 0)
			m_currentExtractionRate.loadFromIff(file);
		else if (strcmp(paramName, "maxHopperSize") == 0)
			m_maxHopperSize.loadFromIff(file);
		else if (strcmp(paramName, "masterClassName") == 0)
			m_masterClassName.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerInstallationObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerHarvesterInstallationObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerHarvesterInstallationObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerHarvesterInstallationObjectTemplate_tag);
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

	// save maxExtractionRate
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxExtractionRate");
	m_maxExtractionRate.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save currentExtractionRate
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("currentExtractionRate");
	m_currentExtractionRate.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxHopperSize
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxHopperSize");
	m_maxHopperSize.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save masterClassName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("masterClassName");
	m_masterClassName.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	ServerInstallationObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerHarvesterInstallationObjectTemplate::save

//@END TFD
