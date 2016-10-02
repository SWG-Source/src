//========================================================================
//
// SharedMissionObjectTemplate.cpp
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
#include "SharedMissionObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedMissionObjectTemplate::SharedMissionObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedMissionObjectTemplate::SharedMissionObjectTemplate

/**
 * Class destructor.
 */
SharedMissionObjectTemplate::~SharedMissionObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedMissionObjectTemplate::~SharedMissionObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedMissionObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedMissionObjectTemplate_tag, create);
}	// SharedMissionObjectTemplate::registerMe

/**
 * Creates a SharedMissionObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedMissionObjectTemplate::create(const std::string & filename)
{
	return new SharedMissionObjectTemplate(filename);
}	// SharedMissionObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedMissionObjectTemplate::getId(void) const
{
	return SharedMissionObjectTemplate_tag;
}	// SharedMissionObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedMissionObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedMissionObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedMissionObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedMissionObjectTemplate * base = dynamic_cast<const SharedMissionObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedMissionObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedMissionObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getCompilerIntegerParam

FloatParam * SharedMissionObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getFloatParam

BoolParam * SharedMissionObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getBoolParam

StringParam * SharedMissionObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getStringParam

StringIdParam * SharedMissionObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getStringIdParam

VectorParam * SharedMissionObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getVectorParam

DynamicVariableParam * SharedMissionObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getDynamicVariableParam

StructParamOT * SharedMissionObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedMissionObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedMissionObjectTemplate::getTriggerVolumeParam

void SharedMissionObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedIntangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedMissionObjectTemplate::initStructParamOT

void SharedMissionObjectTemplate::setAsEmptyList(const char *name)
{
	SharedIntangibleObjectTemplate::setAsEmptyList(name);
}	// SharedMissionObjectTemplate::setAsEmptyList

void SharedMissionObjectTemplate::setAppend(const char *name)
{
	SharedIntangibleObjectTemplate::setAppend(name);
}	// SharedMissionObjectTemplate::setAppend

bool SharedMissionObjectTemplate::isAppend(const char *name) const
{
	return SharedIntangibleObjectTemplate::isAppend(name);
}	// SharedMissionObjectTemplate::isAppend


int SharedMissionObjectTemplate::getListLength(const char *name) const
{
	return SharedIntangibleObjectTemplate::getListLength(name);
}	// SharedMissionObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedMissionObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedMissionObjectTemplate_tag)
	{
		SharedIntangibleObjectTemplate::load(file);
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
	SharedIntangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedMissionObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedMissionObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedMissionObjectTemplate_tag);
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
	SharedIntangibleObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// SharedMissionObjectTemplate::save

//@END TFD
