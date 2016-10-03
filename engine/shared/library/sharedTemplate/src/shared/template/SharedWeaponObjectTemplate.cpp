//========================================================================
//
// SharedWeaponObjectTemplate.cpp
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
#include "SharedWeaponObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedWeaponObjectTemplate::SharedWeaponObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedWeaponObjectTemplate::SharedWeaponObjectTemplate

/**
 * Class destructor.
 */
SharedWeaponObjectTemplate::~SharedWeaponObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedWeaponObjectTemplate::~SharedWeaponObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedWeaponObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedWeaponObjectTemplate_tag, create);
}	// SharedWeaponObjectTemplate::registerMe

/**
 * Creates a SharedWeaponObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedWeaponObjectTemplate::create(const std::string & filename)
{
	return new SharedWeaponObjectTemplate(filename);
}	// SharedWeaponObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedWeaponObjectTemplate::getId(void) const
{
	return SharedWeaponObjectTemplate_tag;
}	// SharedWeaponObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedWeaponObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedWeaponObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedWeaponObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedWeaponObjectTemplate * base = dynamic_cast<const SharedWeaponObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedWeaponObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedWeaponObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "weaponEffectIndex") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_weaponEffectIndex;
		}
		fprintf(stderr, "trying to access single-parameter \"weaponEffectIndex\" as an array\n");
	}
	else if (strcmp(name, "attackType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_attackType;
		}
		fprintf(stderr, "trying to access single-parameter \"attackType\" as an array\n");
	}
	else
		return SharedTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//SharedWeaponObjectTemplate::getCompilerIntegerParam

FloatParam * SharedWeaponObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedWeaponObjectTemplate::getFloatParam

BoolParam * SharedWeaponObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedWeaponObjectTemplate::getBoolParam

StringParam * SharedWeaponObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "weaponEffect") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_weaponEffect;
		}
		fprintf(stderr, "trying to access single-parameter \"weaponEffect\" as an array\n");
	}
	else
		return SharedTangibleObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedWeaponObjectTemplate::getStringParam

StringIdParam * SharedWeaponObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedWeaponObjectTemplate::getStringIdParam

VectorParam * SharedWeaponObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedWeaponObjectTemplate::getVectorParam

DynamicVariableParam * SharedWeaponObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedWeaponObjectTemplate::getDynamicVariableParam

StructParamOT * SharedWeaponObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedWeaponObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedWeaponObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedWeaponObjectTemplate::getTriggerVolumeParam

void SharedWeaponObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedTangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedWeaponObjectTemplate::initStructParamOT

void SharedWeaponObjectTemplate::setAsEmptyList(const char *name)
{
	SharedTangibleObjectTemplate::setAsEmptyList(name);
}	// SharedWeaponObjectTemplate::setAsEmptyList

void SharedWeaponObjectTemplate::setAppend(const char *name)
{
	SharedTangibleObjectTemplate::setAppend(name);
}	// SharedWeaponObjectTemplate::setAppend

bool SharedWeaponObjectTemplate::isAppend(const char *name) const
{
	return SharedTangibleObjectTemplate::isAppend(name);
}	// SharedWeaponObjectTemplate::isAppend


int SharedWeaponObjectTemplate::getListLength(const char *name) const
{
	return SharedTangibleObjectTemplate::getListLength(name);
}	// SharedWeaponObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedWeaponObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedWeaponObjectTemplate_tag)
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
		if (strcmp(paramName, "weaponEffect") == 0)
			m_weaponEffect.loadFromIff(file);
		else if (strcmp(paramName, "weaponEffectIndex") == 0)
			m_weaponEffectIndex.loadFromIff(file);
		else if (strcmp(paramName, "attackType") == 0)
			m_attackType.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedWeaponObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedWeaponObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedWeaponObjectTemplate_tag);
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

	// save weaponEffect
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("weaponEffect");
	m_weaponEffect.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save weaponEffectIndex
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("weaponEffectIndex");
	m_weaponEffectIndex.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save attackType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("attackType");
	m_attackType.saveToIff(file);
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
}	// SharedWeaponObjectTemplate::save

//@END TFD
