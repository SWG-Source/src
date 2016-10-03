//========================================================================
//
// ServerPlayerObjectTemplate.cpp
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
#include "ServerPlayerObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerPlayerObjectTemplate::ServerPlayerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerPlayerObjectTemplate::ServerPlayerObjectTemplate

/**
 * Class destructor.
 */
ServerPlayerObjectTemplate::~ServerPlayerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerPlayerObjectTemplate::~ServerPlayerObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerPlayerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerPlayerObjectTemplate_tag, create);
}	// ServerPlayerObjectTemplate::registerMe

/**
 * Creates a ServerPlayerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerPlayerObjectTemplate::create(const std::string & filename)
{
	return new ServerPlayerObjectTemplate(filename);
}	// ServerPlayerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerPlayerObjectTemplate::getId(void) const
{
	return ServerPlayerObjectTemplate_tag;
}	// ServerPlayerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerPlayerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerPlayerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerPlayerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerPlayerObjectTemplate * base = dynamic_cast<const ServerPlayerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerPlayerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerPlayerObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getCompilerIntegerParam

FloatParam * ServerPlayerObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getFloatParam

BoolParam * ServerPlayerObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getBoolParam

StringParam * ServerPlayerObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getStringParam

StringIdParam * ServerPlayerObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getStringIdParam

VectorParam * ServerPlayerObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getVectorParam

DynamicVariableParam * ServerPlayerObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getDynamicVariableParam

StructParamOT * ServerPlayerObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerPlayerObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerPlayerObjectTemplate::getTriggerVolumeParam

void ServerPlayerObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerIntangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerPlayerObjectTemplate::initStructParamOT

void ServerPlayerObjectTemplate::setAsEmptyList(const char *name)
{
	ServerIntangibleObjectTemplate::setAsEmptyList(name);
}	// ServerPlayerObjectTemplate::setAsEmptyList

void ServerPlayerObjectTemplate::setAppend(const char *name)
{
	ServerIntangibleObjectTemplate::setAppend(name);
}	// ServerPlayerObjectTemplate::setAppend

bool ServerPlayerObjectTemplate::isAppend(const char *name) const
{
	return ServerIntangibleObjectTemplate::isAppend(name);
}	// ServerPlayerObjectTemplate::isAppend


int ServerPlayerObjectTemplate::getListLength(const char *name) const
{
	return ServerIntangibleObjectTemplate::getListLength(name);
}	// ServerPlayerObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerPlayerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerPlayerObjectTemplate_tag)
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
}	// ServerPlayerObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerPlayerObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerPlayerObjectTemplate_tag);
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
}	// ServerPlayerObjectTemplate::save

//@END TFD
