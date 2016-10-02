//========================================================================
//
// ServerXpManagerObjectTemplate.cpp
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
#include "ServerXpManagerObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerXpManagerObjectTemplate::ServerXpManagerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerXpManagerObjectTemplate::ServerXpManagerObjectTemplate

/**
 * Class destructor.
 */
ServerXpManagerObjectTemplate::~ServerXpManagerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerXpManagerObjectTemplate::~ServerXpManagerObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerXpManagerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerXpManagerObjectTemplate_tag, create);
}	// ServerXpManagerObjectTemplate::registerMe

/**
 * Creates a ServerXpManagerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerXpManagerObjectTemplate::create(const std::string & filename)
{
	return new ServerXpManagerObjectTemplate(filename);
}	// ServerXpManagerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerXpManagerObjectTemplate::getId(void) const
{
	return ServerXpManagerObjectTemplate_tag;
}	// ServerXpManagerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerXpManagerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerXpManagerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerXpManagerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerXpManagerObjectTemplate * base = dynamic_cast<const ServerXpManagerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerXpManagerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerXpManagerObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getCompilerIntegerParam

FloatParam * ServerXpManagerObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getFloatParam

BoolParam * ServerXpManagerObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getBoolParam

StringParam * ServerXpManagerObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getStringParam

StringIdParam * ServerXpManagerObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getStringIdParam

VectorParam * ServerXpManagerObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getVectorParam

DynamicVariableParam * ServerXpManagerObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getDynamicVariableParam

StructParamOT * ServerXpManagerObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerXpManagerObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerXpManagerObjectTemplate::getTriggerVolumeParam

void ServerXpManagerObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerUniverseObjectTemplate::initStructParamOT(param, name);
}	// ServerXpManagerObjectTemplate::initStructParamOT

void ServerXpManagerObjectTemplate::setAsEmptyList(const char *name)
{
	ServerUniverseObjectTemplate::setAsEmptyList(name);
}	// ServerXpManagerObjectTemplate::setAsEmptyList

void ServerXpManagerObjectTemplate::setAppend(const char *name)
{
	ServerUniverseObjectTemplate::setAppend(name);
}	// ServerXpManagerObjectTemplate::setAppend

bool ServerXpManagerObjectTemplate::isAppend(const char *name) const
{
	return ServerUniverseObjectTemplate::isAppend(name);
}	// ServerXpManagerObjectTemplate::isAppend


int ServerXpManagerObjectTemplate::getListLength(const char *name) const
{
	return ServerUniverseObjectTemplate::getListLength(name);
}	// ServerXpManagerObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerXpManagerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerXpManagerObjectTemplate_tag)
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
}	// ServerXpManagerObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerXpManagerObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerXpManagerObjectTemplate_tag);
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
}	// ServerXpManagerObjectTemplate::save

//@END TFD
