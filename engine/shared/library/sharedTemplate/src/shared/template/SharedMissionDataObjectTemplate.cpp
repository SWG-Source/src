//========================================================================
//
// SharedMissionDataObjectTemplate.cpp
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
#include "SharedMissionDataObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedMissionDataObjectTemplate::SharedMissionDataObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedMissionDataObjectTemplate::SharedMissionDataObjectTemplate

/**
 * Class destructor.
 */
SharedMissionDataObjectTemplate::~SharedMissionDataObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedMissionDataObjectTemplate::~SharedMissionDataObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedMissionDataObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedMissionDataObjectTemplate_tag, create);
}	// SharedMissionDataObjectTemplate::registerMe

/**
 * Creates a SharedMissionDataObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedMissionDataObjectTemplate::create(const std::string & filename)
{
	return new SharedMissionDataObjectTemplate(filename);
}	// SharedMissionDataObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedMissionDataObjectTemplate::getId(void) const
{
	return SharedMissionDataObjectTemplate_tag;
}	// SharedMissionDataObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedMissionDataObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedMissionDataObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedMissionDataObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedMissionDataObjectTemplate * base = dynamic_cast<const SharedMissionDataObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedMissionDataObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedMissionDataObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getCompilerIntegerParam

FloatParam * SharedMissionDataObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getFloatParam

BoolParam * SharedMissionDataObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getBoolParam

StringParam * SharedMissionDataObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getStringParam

StringIdParam * SharedMissionDataObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getStringIdParam

VectorParam * SharedMissionDataObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getVectorParam

DynamicVariableParam * SharedMissionDataObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getDynamicVariableParam

StructParamOT * SharedMissionDataObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedMissionDataObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedMissionDataObjectTemplate::getTriggerVolumeParam

void SharedMissionDataObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedIntangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedMissionDataObjectTemplate::initStructParamOT

void SharedMissionDataObjectTemplate::setAsEmptyList(const char *name)
{
	SharedIntangibleObjectTemplate::setAsEmptyList(name);
}	// SharedMissionDataObjectTemplate::setAsEmptyList

void SharedMissionDataObjectTemplate::setAppend(const char *name)
{
	SharedIntangibleObjectTemplate::setAppend(name);
}	// SharedMissionDataObjectTemplate::setAppend

bool SharedMissionDataObjectTemplate::isAppend(const char *name) const
{
	return SharedIntangibleObjectTemplate::isAppend(name);
}	// SharedMissionDataObjectTemplate::isAppend


int SharedMissionDataObjectTemplate::getListLength(const char *name) const
{
	return SharedIntangibleObjectTemplate::getListLength(name);
}	// SharedMissionDataObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedMissionDataObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedMissionDataObjectTemplate_tag)
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
}	// SharedMissionDataObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedMissionDataObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedMissionDataObjectTemplate_tag);
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
}	// SharedMissionDataObjectTemplate::save

//@END TFD
