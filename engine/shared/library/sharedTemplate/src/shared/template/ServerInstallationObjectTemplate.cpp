//========================================================================
//
// ServerInstallationObjectTemplate.cpp
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
#include "ServerInstallationObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerInstallationObjectTemplate::ServerInstallationObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerInstallationObjectTemplate::ServerInstallationObjectTemplate

/**
 * Class destructor.
 */
ServerInstallationObjectTemplate::~ServerInstallationObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerInstallationObjectTemplate::~ServerInstallationObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerInstallationObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerInstallationObjectTemplate_tag, create);
}	// ServerInstallationObjectTemplate::registerMe

/**
 * Creates a ServerInstallationObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerInstallationObjectTemplate::create(const std::string & filename)
{
	return new ServerInstallationObjectTemplate(filename);
}	// ServerInstallationObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerInstallationObjectTemplate::getId(void) const
{
	return ServerInstallationObjectTemplate_tag;
}	// ServerInstallationObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerInstallationObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerInstallationObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerInstallationObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerInstallationObjectTemplate * base = dynamic_cast<const ServerInstallationObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerInstallationObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerInstallationObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getCompilerIntegerParam

FloatParam * ServerInstallationObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getFloatParam

BoolParam * ServerInstallationObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getBoolParam

StringParam * ServerInstallationObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getStringParam

StringIdParam * ServerInstallationObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getStringIdParam

VectorParam * ServerInstallationObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getVectorParam

DynamicVariableParam * ServerInstallationObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getDynamicVariableParam

StructParamOT * ServerInstallationObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerInstallationObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerInstallationObjectTemplate::getTriggerVolumeParam

void ServerInstallationObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerInstallationObjectTemplate::initStructParamOT

void ServerInstallationObjectTemplate::setAsEmptyList(const char *name)
{
	ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerInstallationObjectTemplate::setAsEmptyList

void ServerInstallationObjectTemplate::setAppend(const char *name)
{
	ServerTangibleObjectTemplate::setAppend(name);
}	// ServerInstallationObjectTemplate::setAppend

bool ServerInstallationObjectTemplate::isAppend(const char *name) const
{
	return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerInstallationObjectTemplate::isAppend


int ServerInstallationObjectTemplate::getListLength(const char *name) const
{
	return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerInstallationObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerInstallationObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerInstallationObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,1))
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
}	// ServerInstallationObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerInstallationObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerInstallationObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,1));
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
}	// ServerInstallationObjectTemplate::save

//@END TFD
