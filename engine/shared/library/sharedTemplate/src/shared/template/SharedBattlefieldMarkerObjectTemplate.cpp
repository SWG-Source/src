//========================================================================
//
// SharedBattlefieldMarkerObjectTemplate.cpp
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
#include "SharedBattlefieldMarkerObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedBattlefieldMarkerObjectTemplate::SharedBattlefieldMarkerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedBattlefieldMarkerObjectTemplate::SharedBattlefieldMarkerObjectTemplate

/**
 * Class destructor.
 */
SharedBattlefieldMarkerObjectTemplate::~SharedBattlefieldMarkerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedBattlefieldMarkerObjectTemplate::~SharedBattlefieldMarkerObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedBattlefieldMarkerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedBattlefieldMarkerObjectTemplate_tag, create);
}	// SharedBattlefieldMarkerObjectTemplate::registerMe

/**
 * Creates a SharedBattlefieldMarkerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedBattlefieldMarkerObjectTemplate::create(const std::string & filename)
{
	return new SharedBattlefieldMarkerObjectTemplate(filename);
}	// SharedBattlefieldMarkerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedBattlefieldMarkerObjectTemplate::getId(void) const
{
	return SharedBattlefieldMarkerObjectTemplate_tag;
}	// SharedBattlefieldMarkerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedBattlefieldMarkerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedBattlefieldMarkerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedBattlefieldMarkerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedBattlefieldMarkerObjectTemplate * base = dynamic_cast<const SharedBattlefieldMarkerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedBattlefieldMarkerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedBattlefieldMarkerObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "numberOfPoles") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_numberOfPoles;
		}
		fprintf(stderr, "trying to access single-parameter \"numberOfPoles\" as an array\n");
	}
	else
		return SharedTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//SharedBattlefieldMarkerObjectTemplate::getCompilerIntegerParam

FloatParam * SharedBattlefieldMarkerObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "radius") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_radius;
		}
		fprintf(stderr, "trying to access single-parameter \"radius\" as an array\n");
	}
	else
		return SharedTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//SharedBattlefieldMarkerObjectTemplate::getFloatParam

BoolParam * SharedBattlefieldMarkerObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedBattlefieldMarkerObjectTemplate::getBoolParam

StringParam * SharedBattlefieldMarkerObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedBattlefieldMarkerObjectTemplate::getStringParam

StringIdParam * SharedBattlefieldMarkerObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedBattlefieldMarkerObjectTemplate::getStringIdParam

VectorParam * SharedBattlefieldMarkerObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedBattlefieldMarkerObjectTemplate::getVectorParam

DynamicVariableParam * SharedBattlefieldMarkerObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedBattlefieldMarkerObjectTemplate::getDynamicVariableParam

StructParamOT * SharedBattlefieldMarkerObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedBattlefieldMarkerObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedBattlefieldMarkerObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedBattlefieldMarkerObjectTemplate::getTriggerVolumeParam

void SharedBattlefieldMarkerObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedTangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedBattlefieldMarkerObjectTemplate::initStructParamOT

void SharedBattlefieldMarkerObjectTemplate::setAsEmptyList(const char *name)
{
	SharedTangibleObjectTemplate::setAsEmptyList(name);
}	// SharedBattlefieldMarkerObjectTemplate::setAsEmptyList

void SharedBattlefieldMarkerObjectTemplate::setAppend(const char *name)
{
	SharedTangibleObjectTemplate::setAppend(name);
}	// SharedBattlefieldMarkerObjectTemplate::setAppend

bool SharedBattlefieldMarkerObjectTemplate::isAppend(const char *name) const
{
	return SharedTangibleObjectTemplate::isAppend(name);
}	// SharedBattlefieldMarkerObjectTemplate::isAppend


int SharedBattlefieldMarkerObjectTemplate::getListLength(const char *name) const
{
	return SharedTangibleObjectTemplate::getListLength(name);
}	// SharedBattlefieldMarkerObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedBattlefieldMarkerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedBattlefieldMarkerObjectTemplate_tag)
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
		if (strcmp(paramName, "numberOfPoles") == 0)
			m_numberOfPoles.loadFromIff(file);
		else if (strcmp(paramName, "radius") == 0)
			m_radius.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedBattlefieldMarkerObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedBattlefieldMarkerObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedBattlefieldMarkerObjectTemplate_tag);
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

	// save numberOfPoles
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("numberOfPoles");
	m_numberOfPoles.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save radius
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("radius");
	m_radius.saveToIff(file);
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
}	// SharedBattlefieldMarkerObjectTemplate::save

//@END TFD
