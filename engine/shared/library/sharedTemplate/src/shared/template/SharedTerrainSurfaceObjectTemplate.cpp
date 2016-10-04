//========================================================================
//
// SharedTerrainSurfaceObjectTemplate.cpp
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
#include "SharedTerrainSurfaceObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedTerrainSurfaceObjectTemplate::SharedTerrainSurfaceObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: TpfTemplate(filename)
//@END TFD INIT
{
}	// SharedTerrainSurfaceObjectTemplate::SharedTerrainSurfaceObjectTemplate

/**
 * Class destructor.
 */
SharedTerrainSurfaceObjectTemplate::~SharedTerrainSurfaceObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedTerrainSurfaceObjectTemplate::~SharedTerrainSurfaceObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedTerrainSurfaceObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedTerrainSurfaceObjectTemplate_tag, create);
}	// SharedTerrainSurfaceObjectTemplate::registerMe

/**
 * Creates a SharedTerrainSurfaceObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedTerrainSurfaceObjectTemplate::create(const std::string & filename)
{
	return new SharedTerrainSurfaceObjectTemplate(filename);
}	// SharedTerrainSurfaceObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedTerrainSurfaceObjectTemplate::getId(void) const
{
	return SharedTerrainSurfaceObjectTemplate_tag;
}	// SharedTerrainSurfaceObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedTerrainSurfaceObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedTerrainSurfaceObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedTerrainSurfaceObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedTerrainSurfaceObjectTemplate * base = dynamic_cast<const SharedTerrainSurfaceObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedTerrainSurfaceObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedTerrainSurfaceObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedTerrainSurfaceObjectTemplate::getCompilerIntegerParam

FloatParam * SharedTerrainSurfaceObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "cover") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_cover;
		}
		fprintf(stderr, "trying to access single-parameter \"cover\" as an array\n");
	}
	else
		return TpfTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//SharedTerrainSurfaceObjectTemplate::getFloatParam

BoolParam * SharedTerrainSurfaceObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//SharedTerrainSurfaceObjectTemplate::getBoolParam

StringParam * SharedTerrainSurfaceObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "surfaceType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_surfaceType;
		}
		fprintf(stderr, "trying to access single-parameter \"surfaceType\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedTerrainSurfaceObjectTemplate::getStringParam

StringIdParam * SharedTerrainSurfaceObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedTerrainSurfaceObjectTemplate::getStringIdParam

VectorParam * SharedTerrainSurfaceObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//SharedTerrainSurfaceObjectTemplate::getVectorParam

DynamicVariableParam * SharedTerrainSurfaceObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedTerrainSurfaceObjectTemplate::getDynamicVariableParam

StructParamOT * SharedTerrainSurfaceObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedTerrainSurfaceObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedTerrainSurfaceObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedTerrainSurfaceObjectTemplate::getTriggerVolumeParam

void SharedTerrainSurfaceObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// SharedTerrainSurfaceObjectTemplate::initStructParamOT

void SharedTerrainSurfaceObjectTemplate::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// SharedTerrainSurfaceObjectTemplate::setAsEmptyList

void SharedTerrainSurfaceObjectTemplate::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// SharedTerrainSurfaceObjectTemplate::setAppend

bool SharedTerrainSurfaceObjectTemplate::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// SharedTerrainSurfaceObjectTemplate::isAppend


int SharedTerrainSurfaceObjectTemplate::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// SharedTerrainSurfaceObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedTerrainSurfaceObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedTerrainSurfaceObjectTemplate_tag)
	{
		TpfTemplate::load(file);
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
		if (strcmp(paramName, "cover") == 0)
			m_cover.loadFromIff(file);
		else if (strcmp(paramName, "surfaceType") == 0)
			m_surfaceType.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	TpfTemplate::load(file);
	file.exitForm();
	return;
}	// SharedTerrainSurfaceObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedTerrainSurfaceObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedTerrainSurfaceObjectTemplate_tag);
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

	// save cover
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("cover");
	m_cover.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save surfaceType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("surfaceType");
	m_surfaceType.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	TpfTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// SharedTerrainSurfaceObjectTemplate::save

//@END TFD
