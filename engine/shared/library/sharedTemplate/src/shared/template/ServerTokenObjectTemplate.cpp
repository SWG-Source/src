//========================================================================
//
// ServerTokenObjectTemplate.cpp
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
#include "ServerTokenObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerTokenObjectTemplate::ServerTokenObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerTokenObjectTemplate::ServerTokenObjectTemplate

/**
 * Class destructor.
 */
ServerTokenObjectTemplate::~ServerTokenObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerTokenObjectTemplate::~ServerTokenObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerTokenObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerTokenObjectTemplate_tag, create);
}	// ServerTokenObjectTemplate::registerMe

/**
 * Creates a ServerTokenObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerTokenObjectTemplate::create(const std::string & filename)
{
	return new ServerTokenObjectTemplate(filename);
}	// ServerTokenObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerTokenObjectTemplate::getId(void) const
{
	return ServerTokenObjectTemplate_tag;
}	// ServerTokenObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerTokenObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerTokenObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerTokenObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerTokenObjectTemplate * base = dynamic_cast<const ServerTokenObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerTokenObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerTokenObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getCompilerIntegerParam

FloatParam * ServerTokenObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getFloatParam

BoolParam * ServerTokenObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getBoolParam

StringParam * ServerTokenObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getStringParam

StringIdParam * ServerTokenObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getStringIdParam

VectorParam * ServerTokenObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getVectorParam

DynamicVariableParam * ServerTokenObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getDynamicVariableParam

StructParamOT * ServerTokenObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerTokenObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerTokenObjectTemplate::getTriggerVolumeParam

void ServerTokenObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerIntangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerTokenObjectTemplate::initStructParamOT

void ServerTokenObjectTemplate::setAsEmptyList(const char *name)
{
	ServerIntangibleObjectTemplate::setAsEmptyList(name);
}	// ServerTokenObjectTemplate::setAsEmptyList

void ServerTokenObjectTemplate::setAppend(const char *name)
{
	ServerIntangibleObjectTemplate::setAppend(name);
}	// ServerTokenObjectTemplate::setAppend

bool ServerTokenObjectTemplate::isAppend(const char *name) const
{
	return ServerIntangibleObjectTemplate::isAppend(name);
}	// ServerTokenObjectTemplate::isAppend


int ServerTokenObjectTemplate::getListLength(const char *name) const
{
	return ServerIntangibleObjectTemplate::getListLength(name);
}	// ServerTokenObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerTokenObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerTokenObjectTemplate_tag)
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
}	// ServerTokenObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerTokenObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerTokenObjectTemplate_tag);
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
}	// ServerTokenObjectTemplate::save

//@END TFD
