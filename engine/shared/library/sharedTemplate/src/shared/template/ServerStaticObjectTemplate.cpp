//========================================================================
//
// ServerStaticObjectTemplate.cpp
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
#include "ServerStaticObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerStaticObjectTemplate::ServerStaticObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerStaticObjectTemplate::ServerStaticObjectTemplate

/**
 * Class destructor.
 */
ServerStaticObjectTemplate::~ServerStaticObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerStaticObjectTemplate::~ServerStaticObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerStaticObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerStaticObjectTemplate_tag, create);
}	// ServerStaticObjectTemplate::registerMe

/**
 * Creates a ServerStaticObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerStaticObjectTemplate::create(const std::string & filename)
{
	return new ServerStaticObjectTemplate(filename);
}	// ServerStaticObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerStaticObjectTemplate::getId(void) const
{
	return ServerStaticObjectTemplate_tag;
}	// ServerStaticObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerStaticObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerStaticObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerStaticObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerStaticObjectTemplate * base = dynamic_cast<const ServerStaticObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerStaticObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerStaticObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerStaticObjectTemplate::getCompilerIntegerParam

FloatParam * ServerStaticObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerStaticObjectTemplate::getFloatParam

BoolParam * ServerStaticObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "clientOnlyBuildout") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_clientOnlyBuildout;
		}
		fprintf(stderr, "trying to access single-parameter \"clientOnlyBuildout\" as an array\n");
	}
	else
		return ServerObjectTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//ServerStaticObjectTemplate::getBoolParam

StringParam * ServerStaticObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerStaticObjectTemplate::getStringParam

StringIdParam * ServerStaticObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerStaticObjectTemplate::getStringIdParam

VectorParam * ServerStaticObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerStaticObjectTemplate::getVectorParam

DynamicVariableParam * ServerStaticObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerStaticObjectTemplate::getDynamicVariableParam

StructParamOT * ServerStaticObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerStaticObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerStaticObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerStaticObjectTemplate::getTriggerVolumeParam

void ServerStaticObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerObjectTemplate::initStructParamOT(param, name);
}	// ServerStaticObjectTemplate::initStructParamOT

void ServerStaticObjectTemplate::setAsEmptyList(const char *name)
{
	ServerObjectTemplate::setAsEmptyList(name);
}	// ServerStaticObjectTemplate::setAsEmptyList

void ServerStaticObjectTemplate::setAppend(const char *name)
{
	ServerObjectTemplate::setAppend(name);
}	// ServerStaticObjectTemplate::setAppend

bool ServerStaticObjectTemplate::isAppend(const char *name) const
{
	return ServerObjectTemplate::isAppend(name);
}	// ServerStaticObjectTemplate::isAppend


int ServerStaticObjectTemplate::getListLength(const char *name) const
{
	return ServerObjectTemplate::getListLength(name);
}	// ServerStaticObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerStaticObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerStaticObjectTemplate_tag)
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
		if (strcmp(paramName, "clientOnlyBuildout") == 0)
			m_clientOnlyBuildout.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerStaticObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerStaticObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerStaticObjectTemplate_tag);
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

	// save clientOnlyBuildout
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("clientOnlyBuildout");
	m_clientOnlyBuildout.saveToIff(file);
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
}	// ServerStaticObjectTemplate::save

//@END TFD
