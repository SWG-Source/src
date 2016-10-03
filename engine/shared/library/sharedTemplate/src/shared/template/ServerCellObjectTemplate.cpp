//========================================================================
//
// ServerCellObjectTemplate.cpp
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
#include "ServerCellObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerCellObjectTemplate::ServerCellObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerCellObjectTemplate::ServerCellObjectTemplate

/**
 * Class destructor.
 */
ServerCellObjectTemplate::~ServerCellObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerCellObjectTemplate::~ServerCellObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerCellObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerCellObjectTemplate_tag, create);
}	// ServerCellObjectTemplate::registerMe

/**
 * Creates a ServerCellObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerCellObjectTemplate::create(const std::string & filename)
{
	return new ServerCellObjectTemplate(filename);
}	// ServerCellObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerCellObjectTemplate::getId(void) const
{
	return ServerCellObjectTemplate_tag;
}	// ServerCellObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerCellObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerCellObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerCellObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerCellObjectTemplate * base = dynamic_cast<const ServerCellObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerCellObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerCellObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerCellObjectTemplate::getCompilerIntegerParam

FloatParam * ServerCellObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerCellObjectTemplate::getFloatParam

BoolParam * ServerCellObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerCellObjectTemplate::getBoolParam

StringParam * ServerCellObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerCellObjectTemplate::getStringParam

StringIdParam * ServerCellObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerCellObjectTemplate::getStringIdParam

VectorParam * ServerCellObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerCellObjectTemplate::getVectorParam

DynamicVariableParam * ServerCellObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerCellObjectTemplate::getDynamicVariableParam

StructParamOT * ServerCellObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerCellObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerCellObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerCellObjectTemplate::getTriggerVolumeParam

void ServerCellObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerObjectTemplate::initStructParamOT(param, name);
}	// ServerCellObjectTemplate::initStructParamOT

void ServerCellObjectTemplate::setAsEmptyList(const char *name)
{
	ServerObjectTemplate::setAsEmptyList(name);
}	// ServerCellObjectTemplate::setAsEmptyList

void ServerCellObjectTemplate::setAppend(const char *name)
{
	ServerObjectTemplate::setAppend(name);
}	// ServerCellObjectTemplate::setAppend

bool ServerCellObjectTemplate::isAppend(const char *name) const
{
	return ServerObjectTemplate::isAppend(name);
}	// ServerCellObjectTemplate::isAppend


int ServerCellObjectTemplate::getListLength(const char *name) const
{
	return ServerObjectTemplate::getListLength(name);
}	// ServerCellObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerCellObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerCellObjectTemplate_tag)
	{
		ServerObjectTemplate::load(file);
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
	ServerObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerCellObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerCellObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerCellObjectTemplate_tag);
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
	ServerObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerCellObjectTemplate::save

//@END TFD
