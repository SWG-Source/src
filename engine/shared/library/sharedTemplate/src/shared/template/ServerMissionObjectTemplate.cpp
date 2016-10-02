//========================================================================
//
// ServerMissionObjectTemplate.cpp
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
#include "ServerMissionObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerMissionObjectTemplate::ServerMissionObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerMissionObjectTemplate::ServerMissionObjectTemplate

/**
 * Class destructor.
 */
ServerMissionObjectTemplate::~ServerMissionObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerMissionObjectTemplate::~ServerMissionObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerMissionObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerMissionObjectTemplate_tag, create);
}	// ServerMissionObjectTemplate::registerMe

/**
 * Creates a ServerMissionObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerMissionObjectTemplate::create(const std::string & filename)
{
	return new ServerMissionObjectTemplate(filename);
}	// ServerMissionObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerMissionObjectTemplate::getId(void) const
{
	return ServerMissionObjectTemplate_tag;
}	// ServerMissionObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerMissionObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerMissionObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerMissionObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerMissionObjectTemplate * base = dynamic_cast<const ServerMissionObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerMissionObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerMissionObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getCompilerIntegerParam

FloatParam * ServerMissionObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getFloatParam

BoolParam * ServerMissionObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getBoolParam

StringParam * ServerMissionObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getStringParam

StringIdParam * ServerMissionObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getStringIdParam

VectorParam * ServerMissionObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getVectorParam

DynamicVariableParam * ServerMissionObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getDynamicVariableParam

StructParamOT * ServerMissionObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerMissionObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerMissionObjectTemplate::getTriggerVolumeParam

void ServerMissionObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerIntangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerMissionObjectTemplate::initStructParamOT

void ServerMissionObjectTemplate::setAsEmptyList(const char *name)
{
	ServerIntangibleObjectTemplate::setAsEmptyList(name);
}	// ServerMissionObjectTemplate::setAsEmptyList

void ServerMissionObjectTemplate::setAppend(const char *name)
{
	ServerIntangibleObjectTemplate::setAppend(name);
}	// ServerMissionObjectTemplate::setAppend

bool ServerMissionObjectTemplate::isAppend(const char *name) const
{
	return ServerIntangibleObjectTemplate::isAppend(name);
}	// ServerMissionObjectTemplate::isAppend


int ServerMissionObjectTemplate::getListLength(const char *name) const
{
	return ServerIntangibleObjectTemplate::getListLength(name);
}	// ServerMissionObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerMissionObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerMissionObjectTemplate_tag)
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
	ServerIntangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerMissionObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerMissionObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerMissionObjectTemplate_tag);
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
	ServerIntangibleObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerMissionObjectTemplate::save

//@END TFD
