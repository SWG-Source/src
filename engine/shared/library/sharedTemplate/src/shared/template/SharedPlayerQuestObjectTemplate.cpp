//========================================================================
//
// SharedPlayerQuestObjectTemplate.cpp
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
#include "SharedPlayerQuestObjectTemplate.h"
#include "SharedTangibleObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedPlayerQuestObjectTemplate::SharedPlayerQuestObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedPlayerQuestObjectTemplate::SharedPlayerQuestObjectTemplate

/**
 * Class destructor.
 */
SharedPlayerQuestObjectTemplate::~SharedPlayerQuestObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedPlayerQuestObjectTemplate::~SharedPlayerQuestObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedPlayerQuestObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedPlayerQuestObjectTemplate_tag, create);
}	// SharedPlayerQuestObjectTemplate::registerMe

/**
 * Creates a SharedPlayerQuestObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedPlayerQuestObjectTemplate::create(const std::string & filename)
{
	return new SharedPlayerQuestObjectTemplate(filename);
}	// SharedPlayerQuestObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedPlayerQuestObjectTemplate::getId(void) const
{
	return SharedPlayerQuestObjectTemplate_tag;
}	// SharedPlayerQuestObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedPlayerQuestObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedPlayerQuestObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedPlayerQuestObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedPlayerQuestObjectTemplate * base = dynamic_cast<const SharedPlayerQuestObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedPlayerQuestObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedPlayerQuestObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getCompilerIntegerParam

FloatParam * SharedPlayerQuestObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getFloatParam

BoolParam * SharedPlayerQuestObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getBoolParam

StringParam * SharedPlayerQuestObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getStringParam

StringIdParam * SharedPlayerQuestObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getStringIdParam

VectorParam * SharedPlayerQuestObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getVectorParam

DynamicVariableParam * SharedPlayerQuestObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getDynamicVariableParam

StructParamOT * SharedPlayerQuestObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedPlayerQuestObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedPlayerQuestObjectTemplate::getTriggerVolumeParam

void SharedPlayerQuestObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedTangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedPlayerQuestObjectTemplate::initStructParamOT

void SharedPlayerQuestObjectTemplate::setAsEmptyList(const char *name)
{
	SharedTangibleObjectTemplate::setAsEmptyList(name);
}	// SharedPlayerQuestObjectTemplate::setAsEmptyList

void SharedPlayerQuestObjectTemplate::setAppend(const char *name)
{
	SharedTangibleObjectTemplate::setAppend(name);
}	// SharedPlayerQuestObjectTemplate::setAppend

bool SharedPlayerQuestObjectTemplate::isAppend(const char *name) const
{
	return SharedTangibleObjectTemplate::isAppend(name);
}	// SharedPlayerQuestObjectTemplate::isAppend


int SharedPlayerQuestObjectTemplate::getListLength(const char *name) const
{
	return SharedTangibleObjectTemplate::getListLength(name);
}	// SharedPlayerQuestObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedPlayerQuestObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedPlayerQuestObjectTemplate_tag)
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
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedPlayerQuestObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedPlayerQuestObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedPlayerQuestObjectTemplate_tag);
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
	SharedTangibleObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// SharedPlayerQuestObjectTemplate::save

//@END TFD
