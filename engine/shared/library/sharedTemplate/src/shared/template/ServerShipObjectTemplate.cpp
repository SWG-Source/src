//========================================================================
//
// ServerShipObjectTemplate.cpp
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
#include "ServerShipObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerShipObjectTemplate::ServerShipObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerShipObjectTemplate::ServerShipObjectTemplate

/**
 * Class destructor.
 */
ServerShipObjectTemplate::~ServerShipObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerShipObjectTemplate::~ServerShipObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerShipObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerShipObjectTemplate_tag, create);
}	// ServerShipObjectTemplate::registerMe

/**
 * Creates a ServerShipObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerShipObjectTemplate::create(const std::string & filename)
{
	return new ServerShipObjectTemplate(filename);
}	// ServerShipObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerShipObjectTemplate::getId(void) const
{
	return ServerShipObjectTemplate_tag;
}	// ServerShipObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerShipObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerShipObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerShipObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerShipObjectTemplate * base = dynamic_cast<const ServerShipObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerShipObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerShipObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerShipObjectTemplate::getCompilerIntegerParam

FloatParam * ServerShipObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerShipObjectTemplate::getFloatParam

BoolParam * ServerShipObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerShipObjectTemplate::getBoolParam

StringParam * ServerShipObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "shipType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_shipType;
		}
		fprintf(stderr, "trying to access single-parameter \"shipType\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerShipObjectTemplate::getStringParam

StringIdParam * ServerShipObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerShipObjectTemplate::getStringIdParam

VectorParam * ServerShipObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerShipObjectTemplate::getVectorParam

DynamicVariableParam * ServerShipObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerShipObjectTemplate::getDynamicVariableParam

StructParamOT * ServerShipObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerShipObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerShipObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerShipObjectTemplate::getTriggerVolumeParam

void ServerShipObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerShipObjectTemplate::initStructParamOT

void ServerShipObjectTemplate::setAsEmptyList(const char *name)
{
	ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerShipObjectTemplate::setAsEmptyList

void ServerShipObjectTemplate::setAppend(const char *name)
{
	ServerTangibleObjectTemplate::setAppend(name);
}	// ServerShipObjectTemplate::setAppend

bool ServerShipObjectTemplate::isAppend(const char *name) const
{
	return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerShipObjectTemplate::isAppend


int ServerShipObjectTemplate::getListLength(const char *name) const
{
	return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerShipObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerShipObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerShipObjectTemplate_tag)
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
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "shipType") == 0)
			m_shipType.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerShipObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerShipObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerShipObjectTemplate_tag);
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

	// save shipType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("shipType");
	m_shipType.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	ServerTangibleObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerShipObjectTemplate::save

//@END TFD
