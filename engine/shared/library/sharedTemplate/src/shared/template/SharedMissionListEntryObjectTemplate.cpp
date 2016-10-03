//========================================================================
//
// SharedMissionListEntryObjectTemplate.cpp
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
#include "SharedMissionListEntryObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedMissionListEntryObjectTemplate::SharedMissionListEntryObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedMissionListEntryObjectTemplate::SharedMissionListEntryObjectTemplate

/**
 * Class destructor.
 */
SharedMissionListEntryObjectTemplate::~SharedMissionListEntryObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedMissionListEntryObjectTemplate::~SharedMissionListEntryObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedMissionListEntryObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedMissionListEntryObjectTemplate_tag, create);
}	// SharedMissionListEntryObjectTemplate::registerMe

/**
 * Creates a SharedMissionListEntryObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedMissionListEntryObjectTemplate::create(const std::string & filename)
{
	return new SharedMissionListEntryObjectTemplate(filename);
}	// SharedMissionListEntryObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedMissionListEntryObjectTemplate::getId(void) const
{
	return SharedMissionListEntryObjectTemplate_tag;
}	// SharedMissionListEntryObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedMissionListEntryObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedMissionListEntryObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedMissionListEntryObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedMissionListEntryObjectTemplate * base = dynamic_cast<const SharedMissionListEntryObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedMissionListEntryObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedMissionListEntryObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getCompilerIntegerParam

FloatParam * SharedMissionListEntryObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getFloatParam

BoolParam * SharedMissionListEntryObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getBoolParam

StringParam * SharedMissionListEntryObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getStringParam

StringIdParam * SharedMissionListEntryObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getStringIdParam

VectorParam * SharedMissionListEntryObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getVectorParam

DynamicVariableParam * SharedMissionListEntryObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getDynamicVariableParam

StructParamOT * SharedMissionListEntryObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedMissionListEntryObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedMissionListEntryObjectTemplate::getTriggerVolumeParam

void SharedMissionListEntryObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedIntangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedMissionListEntryObjectTemplate::initStructParamOT

void SharedMissionListEntryObjectTemplate::setAsEmptyList(const char *name)
{
	SharedIntangibleObjectTemplate::setAsEmptyList(name);
}	// SharedMissionListEntryObjectTemplate::setAsEmptyList

void SharedMissionListEntryObjectTemplate::setAppend(const char *name)
{
	SharedIntangibleObjectTemplate::setAppend(name);
}	// SharedMissionListEntryObjectTemplate::setAppend

bool SharedMissionListEntryObjectTemplate::isAppend(const char *name) const
{
	return SharedIntangibleObjectTemplate::isAppend(name);
}	// SharedMissionListEntryObjectTemplate::isAppend


int SharedMissionListEntryObjectTemplate::getListLength(const char *name) const
{
	return SharedIntangibleObjectTemplate::getListLength(name);
}	// SharedMissionListEntryObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedMissionListEntryObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedMissionListEntryObjectTemplate_tag)
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
}	// SharedMissionListEntryObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedMissionListEntryObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedMissionListEntryObjectTemplate_tag);
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
}	// SharedMissionListEntryObjectTemplate::save

//@END TFD
