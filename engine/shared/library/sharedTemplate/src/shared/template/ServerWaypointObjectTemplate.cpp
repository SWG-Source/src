//========================================================================
//
// ServerWaypointObjectTemplate.cpp
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
#include "ServerWaypointObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerWaypointObjectTemplate::ServerWaypointObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerWaypointObjectTemplate::ServerWaypointObjectTemplate

/**
 * Class destructor.
 */
ServerWaypointObjectTemplate::~ServerWaypointObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerWaypointObjectTemplate::~ServerWaypointObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerWaypointObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerWaypointObjectTemplate_tag, create);
}	// ServerWaypointObjectTemplate::registerMe

/**
 * Creates a ServerWaypointObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerWaypointObjectTemplate::create(const std::string & filename)
{
	return new ServerWaypointObjectTemplate(filename);
}	// ServerWaypointObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerWaypointObjectTemplate::getId(void) const
{
	return ServerWaypointObjectTemplate_tag;
}	// ServerWaypointObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerWaypointObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerWaypointObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerWaypointObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerWaypointObjectTemplate * base = dynamic_cast<const ServerWaypointObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerWaypointObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerWaypointObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getCompilerIntegerParam

FloatParam * ServerWaypointObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getFloatParam

BoolParam * ServerWaypointObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getBoolParam

StringParam * ServerWaypointObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getStringParam

StringIdParam * ServerWaypointObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getStringIdParam

VectorParam * ServerWaypointObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getVectorParam

DynamicVariableParam * ServerWaypointObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getDynamicVariableParam

StructParamOT * ServerWaypointObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerWaypointObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerWaypointObjectTemplate::getTriggerVolumeParam

void ServerWaypointObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerIntangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerWaypointObjectTemplate::initStructParamOT

void ServerWaypointObjectTemplate::setAsEmptyList(const char *name)
{
	ServerIntangibleObjectTemplate::setAsEmptyList(name);
}	// ServerWaypointObjectTemplate::setAsEmptyList

void ServerWaypointObjectTemplate::setAppend(const char *name)
{
	ServerIntangibleObjectTemplate::setAppend(name);
}	// ServerWaypointObjectTemplate::setAppend

bool ServerWaypointObjectTemplate::isAppend(const char *name) const
{
	return ServerIntangibleObjectTemplate::isAppend(name);
}	// ServerWaypointObjectTemplate::isAppend


int ServerWaypointObjectTemplate::getListLength(const char *name) const
{
	return ServerIntangibleObjectTemplate::getListLength(name);
}	// ServerWaypointObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerWaypointObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerWaypointObjectTemplate_tag)
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
		DEBUG_WARNING(base == NULL, ("was unable to load base template %s", baseFilename.c_str()));
		if (m_baseData == base && base != NULL)
			base->releaseReference();
		else
		{
			if (m_baseData != NULL)
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
}	// ServerWaypointObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerWaypointObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerWaypointObjectTemplate_tag);
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
}	// ServerWaypointObjectTemplate::save

//@END TFD
