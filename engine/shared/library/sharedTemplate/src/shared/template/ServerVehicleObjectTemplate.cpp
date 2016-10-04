//========================================================================
//
// ServerVehicleObjectTemplate.cpp
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
#include "ServerVehicleObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerVehicleObjectTemplate::ServerVehicleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerVehicleObjectTemplate::ServerVehicleObjectTemplate

/**
 * Class destructor.
 */
ServerVehicleObjectTemplate::~ServerVehicleObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerVehicleObjectTemplate::~ServerVehicleObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerVehicleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerVehicleObjectTemplate_tag, create);
}	// ServerVehicleObjectTemplate::registerMe

/**
 * Creates a ServerVehicleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerVehicleObjectTemplate::create(const std::string & filename)
{
	return new ServerVehicleObjectTemplate(filename);
}	// ServerVehicleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerVehicleObjectTemplate::getId(void) const
{
	return ServerVehicleObjectTemplate_tag;
}	// ServerVehicleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerVehicleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerVehicleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerVehicleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerVehicleObjectTemplate * base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerVehicleObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerVehicleObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerVehicleObjectTemplate::getCompilerIntegerParam

FloatParam * ServerVehicleObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "currentFuel") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_currentFuel;
		}
		fprintf(stderr, "trying to access single-parameter \"currentFuel\" as an array\n");
	}
	else if (strcmp(name, "maxFuel") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxFuel;
		}
		fprintf(stderr, "trying to access single-parameter \"maxFuel\" as an array\n");
	}
	else if (strcmp(name, "consumpsion") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_consumpsion;
		}
		fprintf(stderr, "trying to access single-parameter \"consumpsion\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//ServerVehicleObjectTemplate::getFloatParam

BoolParam * ServerVehicleObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerVehicleObjectTemplate::getBoolParam

StringParam * ServerVehicleObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "fuelType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_fuelType;
		}
		fprintf(stderr, "trying to access single-parameter \"fuelType\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerVehicleObjectTemplate::getStringParam

StringIdParam * ServerVehicleObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerVehicleObjectTemplate::getStringIdParam

VectorParam * ServerVehicleObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerVehicleObjectTemplate::getVectorParam

DynamicVariableParam * ServerVehicleObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerVehicleObjectTemplate::getDynamicVariableParam

StructParamOT * ServerVehicleObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerVehicleObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerVehicleObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerVehicleObjectTemplate::getTriggerVolumeParam

void ServerVehicleObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerVehicleObjectTemplate::initStructParamOT

void ServerVehicleObjectTemplate::setAsEmptyList(const char *name)
{
	ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerVehicleObjectTemplate::setAsEmptyList

void ServerVehicleObjectTemplate::setAppend(const char *name)
{
	ServerTangibleObjectTemplate::setAppend(name);
}	// ServerVehicleObjectTemplate::setAppend

bool ServerVehicleObjectTemplate::isAppend(const char *name) const
{
	return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerVehicleObjectTemplate::isAppend


int ServerVehicleObjectTemplate::getListLength(const char *name) const
{
	return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerVehicleObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerVehicleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerVehicleObjectTemplate_tag)
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
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "fuelType") == 0)
			m_fuelType.loadFromIff(file);
		else if (strcmp(paramName, "currentFuel") == 0)
			m_currentFuel.loadFromIff(file);
		else if (strcmp(paramName, "maxFuel") == 0)
			m_maxFuel.loadFromIff(file);
		else if (strcmp(paramName, "consumpsion") == 0)
			m_consumpsion.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerVehicleObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerVehicleObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerVehicleObjectTemplate_tag);
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

	// save fuelType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("fuelType");
	m_fuelType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save currentFuel
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("currentFuel");
	m_currentFuel.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxFuel
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxFuel");
	m_maxFuel.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save consumpsion
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("consumpsion");
	m_consumpsion.saveToIff(file);
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
}	// ServerVehicleObjectTemplate::save

//@END TFD
