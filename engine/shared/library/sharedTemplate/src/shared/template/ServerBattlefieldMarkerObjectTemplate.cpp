//========================================================================
//
// ServerBattlefieldMarkerObjectTemplate.cpp
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
#include "ServerBattlefieldMarkerObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate

/**
 * Class destructor.
 */
ServerBattlefieldMarkerObjectTemplate::~ServerBattlefieldMarkerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerBattlefieldMarkerObjectTemplate::~ServerBattlefieldMarkerObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerBattlefieldMarkerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerBattlefieldMarkerObjectTemplate_tag, create);
}	// ServerBattlefieldMarkerObjectTemplate::registerMe

/**
 * Creates a ServerBattlefieldMarkerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerBattlefieldMarkerObjectTemplate::create(const std::string & filename)
{
	return new ServerBattlefieldMarkerObjectTemplate(filename);
}	// ServerBattlefieldMarkerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerBattlefieldMarkerObjectTemplate::getId(void) const
{
	return ServerBattlefieldMarkerObjectTemplate_tag;
}	// ServerBattlefieldMarkerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerBattlefieldMarkerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerBattlefieldMarkerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerBattlefieldMarkerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerBattlefieldMarkerObjectTemplate * base = dynamic_cast<const ServerBattlefieldMarkerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerBattlefieldMarkerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerBattlefieldMarkerObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getCompilerIntegerParam

FloatParam * ServerBattlefieldMarkerObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getFloatParam

BoolParam * ServerBattlefieldMarkerObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getBoolParam

StringParam * ServerBattlefieldMarkerObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getStringParam

StringIdParam * ServerBattlefieldMarkerObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getStringIdParam

VectorParam * ServerBattlefieldMarkerObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getVectorParam

DynamicVariableParam * ServerBattlefieldMarkerObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getDynamicVariableParam

StructParamOT * ServerBattlefieldMarkerObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerBattlefieldMarkerObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerBattlefieldMarkerObjectTemplate::getTriggerVolumeParam

void ServerBattlefieldMarkerObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerBattlefieldMarkerObjectTemplate::initStructParamOT

void ServerBattlefieldMarkerObjectTemplate::setAsEmptyList(const char *name)
{
	ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerBattlefieldMarkerObjectTemplate::setAsEmptyList

void ServerBattlefieldMarkerObjectTemplate::setAppend(const char *name)
{
	ServerTangibleObjectTemplate::setAppend(name);
}	// ServerBattlefieldMarkerObjectTemplate::setAppend

bool ServerBattlefieldMarkerObjectTemplate::isAppend(const char *name) const
{
	return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerBattlefieldMarkerObjectTemplate::isAppend


int ServerBattlefieldMarkerObjectTemplate::getListLength(const char *name) const
{
	return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerBattlefieldMarkerObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerBattlefieldMarkerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerBattlefieldMarkerObjectTemplate_tag)
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
}	// ServerBattlefieldMarkerObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerBattlefieldMarkerObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerBattlefieldMarkerObjectTemplate_tag);
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
}	// ServerBattlefieldMarkerObjectTemplate::save

//@END TFD
