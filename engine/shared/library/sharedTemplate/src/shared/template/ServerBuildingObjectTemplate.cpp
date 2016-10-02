//========================================================================
//
// ServerBuildingObjectTemplate.cpp
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
#include "ServerBuildingObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerBuildingObjectTemplate::ServerBuildingObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerBuildingObjectTemplate::ServerBuildingObjectTemplate

/**
 * Class destructor.
 */
ServerBuildingObjectTemplate::~ServerBuildingObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerBuildingObjectTemplate::~ServerBuildingObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerBuildingObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerBuildingObjectTemplate_tag, create);
}	// ServerBuildingObjectTemplate::registerMe

/**
 * Creates a ServerBuildingObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerBuildingObjectTemplate::create(const std::string & filename)
{
	return new ServerBuildingObjectTemplate(filename);
}	// ServerBuildingObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerBuildingObjectTemplate::getId(void) const
{
	return ServerBuildingObjectTemplate_tag;
}	// ServerBuildingObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerBuildingObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerBuildingObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerBuildingObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerBuildingObjectTemplate * base = dynamic_cast<const ServerBuildingObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerBuildingObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerBuildingObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "maintenanceCost") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maintenanceCost;
		}
		fprintf(stderr, "trying to access single-parameter \"maintenanceCost\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerBuildingObjectTemplate::getCompilerIntegerParam

FloatParam * ServerBuildingObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//ServerBuildingObjectTemplate::getFloatParam

BoolParam * ServerBuildingObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "isPublic") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_isPublic;
		}
		fprintf(stderr, "trying to access single-parameter \"isPublic\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//ServerBuildingObjectTemplate::getBoolParam

StringParam * ServerBuildingObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerBuildingObjectTemplate::getStringParam

StringIdParam * ServerBuildingObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerBuildingObjectTemplate::getStringIdParam

VectorParam * ServerBuildingObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerBuildingObjectTemplate::getVectorParam

DynamicVariableParam * ServerBuildingObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerBuildingObjectTemplate::getDynamicVariableParam

StructParamOT * ServerBuildingObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerBuildingObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerBuildingObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerBuildingObjectTemplate::getTriggerVolumeParam

void ServerBuildingObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerBuildingObjectTemplate::initStructParamOT

void ServerBuildingObjectTemplate::setAsEmptyList(const char *name)
{
	ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerBuildingObjectTemplate::setAsEmptyList

void ServerBuildingObjectTemplate::setAppend(const char *name)
{
	ServerTangibleObjectTemplate::setAppend(name);
}	// ServerBuildingObjectTemplate::setAppend

bool ServerBuildingObjectTemplate::isAppend(const char *name) const
{
	return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerBuildingObjectTemplate::isAppend


int ServerBuildingObjectTemplate::getListLength(const char *name) const
{
	return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerBuildingObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerBuildingObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerBuildingObjectTemplate_tag)
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
		if (strcmp(paramName, "maintenanceCost") == 0)
			m_maintenanceCost.loadFromIff(file);
		else if (strcmp(paramName, "isPublic") == 0)
			m_isPublic.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerBuildingObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerBuildingObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerBuildingObjectTemplate_tag);
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

	// save maintenanceCost
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maintenanceCost");
	m_maintenanceCost.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save isPublic
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("isPublic");
	m_isPublic.saveToIff(file);
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
}	// ServerBuildingObjectTemplate::save

//@END TFD
