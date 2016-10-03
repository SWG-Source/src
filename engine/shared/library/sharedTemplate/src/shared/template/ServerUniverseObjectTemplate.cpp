//========================================================================
//
// ServerUniverseObjectTemplate.cpp
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
#include "ServerUniverseObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerUniverseObjectTemplate::ServerUniverseObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerUniverseObjectTemplate::ServerUniverseObjectTemplate

/**
 * Class destructor.
 */
ServerUniverseObjectTemplate::~ServerUniverseObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerUniverseObjectTemplate::~ServerUniverseObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerUniverseObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerUniverseObjectTemplate_tag, create);
}	// ServerUniverseObjectTemplate::registerMe

/**
 * Creates a ServerUniverseObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerUniverseObjectTemplate::create(const std::string & filename)
{
	return new ServerUniverseObjectTemplate(filename);
}	// ServerUniverseObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerUniverseObjectTemplate::getId(void) const
{
	return ServerUniverseObjectTemplate_tag;
}	// ServerUniverseObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerUniverseObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerUniverseObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerUniverseObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerUniverseObjectTemplate * base = dynamic_cast<const ServerUniverseObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerUniverseObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerUniverseObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getCompilerIntegerParam

FloatParam * ServerUniverseObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getFloatParam

BoolParam * ServerUniverseObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getBoolParam

StringParam * ServerUniverseObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getStringParam

StringIdParam * ServerUniverseObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getStringIdParam

VectorParam * ServerUniverseObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getVectorParam

DynamicVariableParam * ServerUniverseObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getDynamicVariableParam

StructParamOT * ServerUniverseObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerUniverseObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerUniverseObjectTemplate::getTriggerVolumeParam

void ServerUniverseObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerObjectTemplate::initStructParamOT(param, name);
}	// ServerUniverseObjectTemplate::initStructParamOT

void ServerUniverseObjectTemplate::setAsEmptyList(const char *name)
{
	ServerObjectTemplate::setAsEmptyList(name);
}	// ServerUniverseObjectTemplate::setAsEmptyList

void ServerUniverseObjectTemplate::setAppend(const char *name)
{
	ServerObjectTemplate::setAppend(name);
}	// ServerUniverseObjectTemplate::setAppend

bool ServerUniverseObjectTemplate::isAppend(const char *name) const
{
	return ServerObjectTemplate::isAppend(name);
}	// ServerUniverseObjectTemplate::isAppend


int ServerUniverseObjectTemplate::getListLength(const char *name) const
{
	return ServerObjectTemplate::getListLength(name);
}	// ServerUniverseObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerUniverseObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerUniverseObjectTemplate_tag)
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
	ServerObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerUniverseObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerUniverseObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerUniverseObjectTemplate_tag);
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
	ServerObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerUniverseObjectTemplate::save

//@END TFD
