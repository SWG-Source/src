//========================================================================
//
// SharedBuildingObjectTemplate.cpp
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
#include "SharedBuildingObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedBuildingObjectTemplate::SharedBuildingObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedBuildingObjectTemplate::SharedBuildingObjectTemplate

/**
 * Class destructor.
 */
SharedBuildingObjectTemplate::~SharedBuildingObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedBuildingObjectTemplate::~SharedBuildingObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedBuildingObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedBuildingObjectTemplate_tag, create);
}	// SharedBuildingObjectTemplate::registerMe

/**
 * Creates a SharedBuildingObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedBuildingObjectTemplate::create(const std::string & filename)
{
	return new SharedBuildingObjectTemplate(filename);
}	// SharedBuildingObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedBuildingObjectTemplate::getId(void) const
{
	return SharedBuildingObjectTemplate_tag;
}	// SharedBuildingObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedBuildingObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedBuildingObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedBuildingObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedBuildingObjectTemplate * base = dynamic_cast<const SharedBuildingObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedBuildingObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedBuildingObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedBuildingObjectTemplate::getCompilerIntegerParam

FloatParam * SharedBuildingObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedBuildingObjectTemplate::getFloatParam

BoolParam * SharedBuildingObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedBuildingObjectTemplate::getBoolParam

StringParam * SharedBuildingObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "terrainModificationFileName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_terrainModificationFileName;
		}
		fprintf(stderr, "trying to access single-parameter \"terrainModificationFileName\" as an array\n");
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
}	//SharedBuildingObjectTemplate::getStringParam

StringIdParam * SharedBuildingObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedBuildingObjectTemplate::getStringIdParam

VectorParam * SharedBuildingObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedBuildingObjectTemplate::getVectorParam

DynamicVariableParam * SharedBuildingObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedBuildingObjectTemplate::getDynamicVariableParam

StructParamOT * SharedBuildingObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedBuildingObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedBuildingObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedBuildingObjectTemplate::getTriggerVolumeParam

void SharedBuildingObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedTangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedBuildingObjectTemplate::initStructParamOT

void SharedBuildingObjectTemplate::setAsEmptyList(const char *name)
{
	SharedTangibleObjectTemplate::setAsEmptyList(name);
}	// SharedBuildingObjectTemplate::setAsEmptyList

void SharedBuildingObjectTemplate::setAppend(const char *name)
{
	SharedTangibleObjectTemplate::setAppend(name);
}	// SharedBuildingObjectTemplate::setAppend

bool SharedBuildingObjectTemplate::isAppend(const char *name) const
{
	return SharedTangibleObjectTemplate::isAppend(name);
}	// SharedBuildingObjectTemplate::isAppend


int SharedBuildingObjectTemplate::getListLength(const char *name) const
{
	return SharedTangibleObjectTemplate::getListLength(name);
}	// SharedBuildingObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedBuildingObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedBuildingObjectTemplate_tag)
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
		if (strcmp(paramName, "terrainModificationFileName") == 0)
			m_terrainModificationFileName.loadFromIff(file);
		else if (strcmp(paramName, "interiorLayoutFileName") == 0)
			m_interiorLayoutFileName.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedBuildingObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedBuildingObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedBuildingObjectTemplate_tag);
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

	// save terrainModificationFileName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("terrainModificationFileName");
	m_terrainModificationFileName.saveToIff(file);
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
}	// SharedBuildingObjectTemplate::save

//@END TFD
