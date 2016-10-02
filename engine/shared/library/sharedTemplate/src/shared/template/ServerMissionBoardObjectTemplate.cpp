//========================================================================
//
// ServerMissionBoardObjectTemplate.cpp
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
#include "ServerMissionBoardObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerMissionBoardObjectTemplate::ServerMissionBoardObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerMissionBoardObjectTemplate::ServerMissionBoardObjectTemplate

/**
 * Class destructor.
 */
ServerMissionBoardObjectTemplate::~ServerMissionBoardObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerMissionBoardObjectTemplate::~ServerMissionBoardObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerMissionBoardObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerMissionBoardObjectTemplate_tag, create);
}	// ServerMissionBoardObjectTemplate::registerMe

/**
 * Creates a ServerMissionBoardObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerMissionBoardObjectTemplate::create(const std::string & filename)
{
	return new ServerMissionBoardObjectTemplate(filename);
}	// ServerMissionBoardObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerMissionBoardObjectTemplate::getId(void) const
{
	return ServerMissionBoardObjectTemplate_tag;
}	// ServerMissionBoardObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerMissionBoardObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerMissionBoardObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerMissionBoardObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const ServerMissionBoardObjectTemplate * base = dynamic_cast<const ServerMissionBoardObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerMissionBoardObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerMissionBoardObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getCompilerIntegerParam

FloatParam * ServerMissionBoardObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getFloatParam

BoolParam * ServerMissionBoardObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getBoolParam

StringParam * ServerMissionBoardObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getStringParam

StringIdParam * ServerMissionBoardObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getStringIdParam

VectorParam * ServerMissionBoardObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getVectorParam

DynamicVariableParam * ServerMissionBoardObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getDynamicVariableParam

StructParamOT * ServerMissionBoardObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerMissionBoardObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerMissionBoardObjectTemplate::getTriggerVolumeParam

void ServerMissionBoardObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerUniverseObjectTemplate::initStructParamOT(param, name);
}	// ServerMissionBoardObjectTemplate::initStructParamOT

void ServerMissionBoardObjectTemplate::setAsEmptyList(const char *name)
{
	ServerUniverseObjectTemplate::setAsEmptyList(name);
}	// ServerMissionBoardObjectTemplate::setAsEmptyList

void ServerMissionBoardObjectTemplate::setAppend(const char *name)
{
	ServerUniverseObjectTemplate::setAppend(name);
}	// ServerMissionBoardObjectTemplate::setAppend

bool ServerMissionBoardObjectTemplate::isAppend(const char *name) const
{
	return ServerUniverseObjectTemplate::isAppend(name);
}	// ServerMissionBoardObjectTemplate::isAppend


int ServerMissionBoardObjectTemplate::getListLength(const char *name) const
{
	return ServerUniverseObjectTemplate::getListLength(name);
}	// ServerMissionBoardObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerMissionBoardObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerMissionBoardObjectTemplate_tag)
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
}	// ServerMissionBoardObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerMissionBoardObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerMissionBoardObjectTemplate_tag);
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
}	// ServerMissionBoardObjectTemplate::save

//@END TFD
