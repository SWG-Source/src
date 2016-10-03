//========================================================================
//
// ServerPlayerQuestObjectTemplate.cpp
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
#include "ServerPlayerQuestObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerPlayerQuestObjectTemplate::ServerPlayerQuestObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerPlayerQuestObjectTemplate::ServerPlayerQuestObjectTemplate

/**
 * Class destructor.
 */
ServerPlayerQuestObjectTemplate::~ServerPlayerQuestObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerPlayerQuestObjectTemplate::~ServerPlayerQuestObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerPlayerQuestObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerPlayerQuestObjectTemplate_tag, create);
}	// ServerPlayerQuestObjectTemplate::registerMe

/**
 * Creates a ServerPlayerQuestObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerPlayerQuestObjectTemplate::create(const std::string & filename)
{
	return new ServerPlayerQuestObjectTemplate(filename);
}	// ServerPlayerQuestObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerPlayerQuestObjectTemplate::getId(void) const
{
	return ServerPlayerQuestObjectTemplate_tag;
}	// ServerPlayerQuestObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerPlayerQuestObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerPlayerQuestObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerPlayerQuestObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerPlayerQuestObjectTemplate * base = dynamic_cast<const ServerPlayerQuestObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerPlayerQuestObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerPlayerQuestObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getCompilerIntegerParam

FloatParam * ServerPlayerQuestObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getFloatParam

BoolParam * ServerPlayerQuestObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getBoolParam

StringParam * ServerPlayerQuestObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getStringParam

StringIdParam * ServerPlayerQuestObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getStringIdParam

VectorParam * ServerPlayerQuestObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getVectorParam

DynamicVariableParam * ServerPlayerQuestObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getDynamicVariableParam

StructParamOT * ServerPlayerQuestObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerPlayerQuestObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerPlayerQuestObjectTemplate::getTriggerVolumeParam

void ServerPlayerQuestObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerPlayerQuestObjectTemplate::initStructParamOT

void ServerPlayerQuestObjectTemplate::setAsEmptyList(const char *name)
{
	ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerPlayerQuestObjectTemplate::setAsEmptyList

void ServerPlayerQuestObjectTemplate::setAppend(const char *name)
{
	ServerTangibleObjectTemplate::setAppend(name);
}	// ServerPlayerQuestObjectTemplate::setAppend

bool ServerPlayerQuestObjectTemplate::isAppend(const char *name) const
{
	return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerPlayerQuestObjectTemplate::isAppend


int ServerPlayerQuestObjectTemplate::getListLength(const char *name) const
{
	return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerPlayerQuestObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerPlayerQuestObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerPlayerQuestObjectTemplate_tag)
	{
		ServerTangibleObjectTemplate::load(file);
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
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerPlayerQuestObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerPlayerQuestObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerPlayerQuestObjectTemplate_tag);
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
	ServerTangibleObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerPlayerQuestObjectTemplate::save

//@END TFD
