//========================================================================
//
// ServerManufactureInstallationObjectTemplate.cpp
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
#include "ServerManufactureInstallationObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerInstallationObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate

/**
 * Class destructor.
 */
ServerManufactureInstallationObjectTemplate::~ServerManufactureInstallationObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerManufactureInstallationObjectTemplate::~ServerManufactureInstallationObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerManufactureInstallationObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerManufactureInstallationObjectTemplate_tag, create);
}	// ServerManufactureInstallationObjectTemplate::registerMe

/**
 * Creates a ServerManufactureInstallationObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerManufactureInstallationObjectTemplate::create(const std::string & filename)
{
	return new ServerManufactureInstallationObjectTemplate(filename);
}	// ServerManufactureInstallationObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerManufactureInstallationObjectTemplate::getId(void) const
{
	return ServerManufactureInstallationObjectTemplate_tag;
}	// ServerManufactureInstallationObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerManufactureInstallationObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerManufactureInstallationObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerManufactureInstallationObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerManufactureInstallationObjectTemplate * base = dynamic_cast<const ServerManufactureInstallationObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerManufactureInstallationObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerManufactureInstallationObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getCompilerIntegerParam

FloatParam * ServerManufactureInstallationObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getFloatParam

BoolParam * ServerManufactureInstallationObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getBoolParam

StringParam * ServerManufactureInstallationObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getStringParam

StringIdParam * ServerManufactureInstallationObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getStringIdParam

VectorParam * ServerManufactureInstallationObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getVectorParam

DynamicVariableParam * ServerManufactureInstallationObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getDynamicVariableParam

StructParamOT * ServerManufactureInstallationObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerManufactureInstallationObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerInstallationObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerManufactureInstallationObjectTemplate::getTriggerVolumeParam

void ServerManufactureInstallationObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerInstallationObjectTemplate::initStructParamOT(param, name);
}	// ServerManufactureInstallationObjectTemplate::initStructParamOT

void ServerManufactureInstallationObjectTemplate::setAsEmptyList(const char *name)
{
	ServerInstallationObjectTemplate::setAsEmptyList(name);
}	// ServerManufactureInstallationObjectTemplate::setAsEmptyList

void ServerManufactureInstallationObjectTemplate::setAppend(const char *name)
{
	ServerInstallationObjectTemplate::setAppend(name);
}	// ServerManufactureInstallationObjectTemplate::setAppend

bool ServerManufactureInstallationObjectTemplate::isAppend(const char *name) const
{
	return ServerInstallationObjectTemplate::isAppend(name);
}	// ServerManufactureInstallationObjectTemplate::isAppend


int ServerManufactureInstallationObjectTemplate::getListLength(const char *name) const
{
	return ServerInstallationObjectTemplate::getListLength(name);
}	// ServerManufactureInstallationObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerManufactureInstallationObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerManufactureInstallationObjectTemplate_tag)
	{
		ServerInstallationObjectTemplate::load(file);
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
	ServerInstallationObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerManufactureInstallationObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerManufactureInstallationObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerManufactureInstallationObjectTemplate_tag);
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
	ServerInstallationObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerManufactureInstallationObjectTemplate::save

//@END TFD
