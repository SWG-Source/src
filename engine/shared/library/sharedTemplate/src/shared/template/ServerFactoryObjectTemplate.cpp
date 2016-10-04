//========================================================================
//
// ServerFactoryObjectTemplate.cpp
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
#include "ServerFactoryObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerFactoryObjectTemplate::ServerFactoryObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerFactoryObjectTemplate::ServerFactoryObjectTemplate

/**
 * Class destructor.
 */
ServerFactoryObjectTemplate::~ServerFactoryObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerFactoryObjectTemplate::~ServerFactoryObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerFactoryObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerFactoryObjectTemplate_tag, create);
}	// ServerFactoryObjectTemplate::registerMe

/**
 * Creates a ServerFactoryObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerFactoryObjectTemplate::create(const std::string & filename)
{
	return new ServerFactoryObjectTemplate(filename);
}	// ServerFactoryObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerFactoryObjectTemplate::getId(void) const
{
	return ServerFactoryObjectTemplate_tag;
}	// ServerFactoryObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerFactoryObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerFactoryObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerFactoryObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerFactoryObjectTemplate * base = dynamic_cast<const ServerFactoryObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerFactoryObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerFactoryObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getCompilerIntegerParam

FloatParam * ServerFactoryObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getFloatParam

BoolParam * ServerFactoryObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getBoolParam

StringParam * ServerFactoryObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getStringParam

StringIdParam * ServerFactoryObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getStringIdParam

VectorParam * ServerFactoryObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getVectorParam

DynamicVariableParam * ServerFactoryObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getDynamicVariableParam

StructParamOT * ServerFactoryObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerFactoryObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerFactoryObjectTemplate::getTriggerVolumeParam

void ServerFactoryObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerFactoryObjectTemplate::initStructParamOT

void ServerFactoryObjectTemplate::setAsEmptyList(const char *name)
{
	ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerFactoryObjectTemplate::setAsEmptyList

void ServerFactoryObjectTemplate::setAppend(const char *name)
{
	ServerTangibleObjectTemplate::setAppend(name);
}	// ServerFactoryObjectTemplate::setAppend

bool ServerFactoryObjectTemplate::isAppend(const char *name) const
{
	return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerFactoryObjectTemplate::isAppend


int ServerFactoryObjectTemplate::getListLength(const char *name) const
{
	return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerFactoryObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerFactoryObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerFactoryObjectTemplate_tag)
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
}	// ServerFactoryObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerFactoryObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerFactoryObjectTemplate_tag);
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
}	// ServerFactoryObjectTemplate::save

//@END TFD
