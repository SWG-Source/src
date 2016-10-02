//========================================================================
//
// SharedShipObjectTemplate.cpp
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
#include "SharedShipObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedShipObjectTemplate::SharedShipObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedShipObjectTemplate::SharedShipObjectTemplate

/**
 * Class destructor.
 */
SharedShipObjectTemplate::~SharedShipObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedShipObjectTemplate::~SharedShipObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedShipObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedShipObjectTemplate_tag, create);
}	// SharedShipObjectTemplate::registerMe

/**
 * Creates a SharedShipObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedShipObjectTemplate::create(const std::string & filename)
{
	return new SharedShipObjectTemplate(filename);
}	// SharedShipObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedShipObjectTemplate::getId(void) const
{
	return SharedShipObjectTemplate_tag;
}	// SharedShipObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedShipObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedShipObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedShipObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedShipObjectTemplate * base = dynamic_cast<const SharedShipObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedShipObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedShipObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedShipObjectTemplate::getCompilerIntegerParam

FloatParam * SharedShipObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedShipObjectTemplate::getFloatParam

BoolParam * SharedShipObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "hasWings") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_hasWings;
		}
		fprintf(stderr, "trying to access single-parameter \"hasWings\" as an array\n");
	}
	else if (strcmp(name, "playerControlled") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_playerControlled;
		}
		fprintf(stderr, "trying to access single-parameter \"playerControlled\" as an array\n");
	}
	else
		return SharedTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//SharedShipObjectTemplate::getBoolParam

StringParam * SharedShipObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "cockpitFilename") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_cockpitFilename;
		}
		fprintf(stderr, "trying to access single-parameter \"cockpitFilename\" as an array\n");
	}
	else if (strcmp(name, "interiorLayoutFileName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_interiorLayoutFileName;
		}
		fprintf(stderr, "trying to access single-parameter \"interiorLayoutFileName\" as an array\n");
	}
	else
		return SharedTangibleObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedShipObjectTemplate::getStringParam

StringIdParam * SharedShipObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedShipObjectTemplate::getStringIdParam

VectorParam * SharedShipObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedShipObjectTemplate::getVectorParam

DynamicVariableParam * SharedShipObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedShipObjectTemplate::getDynamicVariableParam

StructParamOT * SharedShipObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedShipObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedShipObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedShipObjectTemplate::getTriggerVolumeParam

void SharedShipObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedTangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedShipObjectTemplate::initStructParamOT

void SharedShipObjectTemplate::setAsEmptyList(const char *name)
{
	SharedTangibleObjectTemplate::setAsEmptyList(name);
}	// SharedShipObjectTemplate::setAsEmptyList

void SharedShipObjectTemplate::setAppend(const char *name)
{
	SharedTangibleObjectTemplate::setAppend(name);
}	// SharedShipObjectTemplate::setAppend

bool SharedShipObjectTemplate::isAppend(const char *name) const
{
	return SharedTangibleObjectTemplate::isAppend(name);
}	// SharedShipObjectTemplate::isAppend


int SharedShipObjectTemplate::getListLength(const char *name) const
{
	return SharedTangibleObjectTemplate::getListLength(name);
}	// SharedShipObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedShipObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedShipObjectTemplate_tag)
	{
		SharedTangibleObjectTemplate::load(file);
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
	if (getHighestTemplateVersion() != TAG(0,0,0,4))
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
		if (strcmp(paramName, "cockpitFilename") == 0)
			m_cockpitFilename.loadFromIff(file);
		else if (strcmp(paramName, "hasWings") == 0)
			m_hasWings.loadFromIff(file);
		else if (strcmp(paramName, "playerControlled") == 0)
			m_playerControlled.loadFromIff(file);
		else if (strcmp(paramName, "interiorLayoutFileName") == 0)
			m_interiorLayoutFileName.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedShipObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedShipObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedShipObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,4));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save cockpitFilename
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("cockpitFilename");
	m_cockpitFilename.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save hasWings
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("hasWings");
	m_hasWings.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save playerControlled
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("playerControlled");
	m_playerControlled.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save interiorLayoutFileName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("interiorLayoutFileName");
	m_interiorLayoutFileName.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	SharedTangibleObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// SharedShipObjectTemplate::save

//@END TFD
