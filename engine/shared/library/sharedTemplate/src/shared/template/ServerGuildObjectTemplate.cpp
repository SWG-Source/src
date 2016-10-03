//========================================================================
//
// ServerGuildObjectTemplate.cpp
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
#include "ServerGuildObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerGuildObjectTemplate::ServerGuildObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerGuildObjectTemplate::ServerGuildObjectTemplate

/**
 * Class destructor.
 */
ServerGuildObjectTemplate::~ServerGuildObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerGuildObjectTemplate::~ServerGuildObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerGuildObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerGuildObjectTemplate_tag, create);
}	// ServerGuildObjectTemplate::registerMe

/**
 * Creates a ServerGuildObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerGuildObjectTemplate::create(const std::string & filename)
{
	return new ServerGuildObjectTemplate(filename);
}	// ServerGuildObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerGuildObjectTemplate::getId(void) const
{
	return ServerGuildObjectTemplate_tag;
}	// ServerGuildObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerGuildObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerGuildObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerGuildObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerGuildObjectTemplate * base = dynamic_cast<const ServerGuildObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerGuildObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerGuildObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getCompilerIntegerParam

FloatParam * ServerGuildObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getFloatParam

BoolParam * ServerGuildObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getBoolParam

StringParam * ServerGuildObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getStringParam

StringIdParam * ServerGuildObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getStringIdParam

VectorParam * ServerGuildObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getVectorParam

DynamicVariableParam * ServerGuildObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getDynamicVariableParam

StructParamOT * ServerGuildObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerGuildObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerGuildObjectTemplate::getTriggerVolumeParam

void ServerGuildObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerUniverseObjectTemplate::initStructParamOT(param, name);
}	// ServerGuildObjectTemplate::initStructParamOT

void ServerGuildObjectTemplate::setAsEmptyList(const char *name)
{
	ServerUniverseObjectTemplate::setAsEmptyList(name);
}	// ServerGuildObjectTemplate::setAsEmptyList

void ServerGuildObjectTemplate::setAppend(const char *name)
{
	ServerUniverseObjectTemplate::setAppend(name);
}	// ServerGuildObjectTemplate::setAppend

bool ServerGuildObjectTemplate::isAppend(const char *name) const
{
	return ServerUniverseObjectTemplate::isAppend(name);
}	// ServerGuildObjectTemplate::isAppend


int ServerGuildObjectTemplate::getListLength(const char *name) const
{
	return ServerUniverseObjectTemplate::getListLength(name);
}	// ServerGuildObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerGuildObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerGuildObjectTemplate_tag)
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
	UNREF(paramName);
	UNREF(paramCount);

	file.exitForm();
	ServerUniverseObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerGuildObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerGuildObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerGuildObjectTemplate_tag);
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


	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	ServerUniverseObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerGuildObjectTemplate::save

//@END TFD
