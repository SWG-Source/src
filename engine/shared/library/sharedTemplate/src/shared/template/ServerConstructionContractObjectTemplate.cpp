//========================================================================
//
// ServerConstructionContractObjectTemplate.cpp
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
#include "ServerConstructionContractObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerConstructionContractObjectTemplate::ServerConstructionContractObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerConstructionContractObjectTemplate::ServerConstructionContractObjectTemplate

/**
 * Class destructor.
 */
ServerConstructionContractObjectTemplate::~ServerConstructionContractObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerConstructionContractObjectTemplate::~ServerConstructionContractObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerConstructionContractObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerConstructionContractObjectTemplate_tag, create);
}	// ServerConstructionContractObjectTemplate::registerMe

/**
 * Creates a ServerConstructionContractObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerConstructionContractObjectTemplate::create(const std::string & filename)
{
	return new ServerConstructionContractObjectTemplate(filename);
}	// ServerConstructionContractObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerConstructionContractObjectTemplate::getId(void) const
{
	return ServerConstructionContractObjectTemplate_tag;
}	// ServerConstructionContractObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerConstructionContractObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerConstructionContractObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerConstructionContractObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerConstructionContractObjectTemplate * base = dynamic_cast<const ServerConstructionContractObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerConstructionContractObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerConstructionContractObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getCompilerIntegerParam

FloatParam * ServerConstructionContractObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getFloatParam

BoolParam * ServerConstructionContractObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getBoolParam

StringParam * ServerConstructionContractObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getStringParam

StringIdParam * ServerConstructionContractObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getStringIdParam

VectorParam * ServerConstructionContractObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getVectorParam

DynamicVariableParam * ServerConstructionContractObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getDynamicVariableParam

StructParamOT * ServerConstructionContractObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerConstructionContractObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerConstructionContractObjectTemplate::getTriggerVolumeParam

void ServerConstructionContractObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerIntangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerConstructionContractObjectTemplate::initStructParamOT

void ServerConstructionContractObjectTemplate::setAsEmptyList(const char *name)
{
	ServerIntangibleObjectTemplate::setAsEmptyList(name);
}	// ServerConstructionContractObjectTemplate::setAsEmptyList

void ServerConstructionContractObjectTemplate::setAppend(const char *name)
{
	ServerIntangibleObjectTemplate::setAppend(name);
}	// ServerConstructionContractObjectTemplate::setAppend

bool ServerConstructionContractObjectTemplate::isAppend(const char *name) const
{
	return ServerIntangibleObjectTemplate::isAppend(name);
}	// ServerConstructionContractObjectTemplate::isAppend


int ServerConstructionContractObjectTemplate::getListLength(const char *name) const
{
	return ServerIntangibleObjectTemplate::getListLength(name);
}	// ServerConstructionContractObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerConstructionContractObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerConstructionContractObjectTemplate_tag)
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
}	// ServerConstructionContractObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerConstructionContractObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerConstructionContractObjectTemplate_tag);
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
}	// ServerConstructionContractObjectTemplate::save

//@END TFD
