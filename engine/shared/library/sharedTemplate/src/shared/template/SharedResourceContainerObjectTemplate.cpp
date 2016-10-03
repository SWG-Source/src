//========================================================================
//
// SharedResourceContainerObjectTemplate.cpp
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
#include "SharedResourceContainerObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate

/**
 * Class destructor.
 */
SharedResourceContainerObjectTemplate::~SharedResourceContainerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedResourceContainerObjectTemplate::~SharedResourceContainerObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedResourceContainerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedResourceContainerObjectTemplate_tag, create);
}	// SharedResourceContainerObjectTemplate::registerMe

/**
 * Creates a SharedResourceContainerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedResourceContainerObjectTemplate::create(const std::string & filename)
{
	return new SharedResourceContainerObjectTemplate(filename);
}	// SharedResourceContainerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedResourceContainerObjectTemplate::getId(void) const
{
	return SharedResourceContainerObjectTemplate_tag;
}	// SharedResourceContainerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedResourceContainerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedResourceContainerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedResourceContainerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedResourceContainerObjectTemplate * base = dynamic_cast<const SharedResourceContainerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedResourceContainerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedResourceContainerObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getCompilerIntegerParam

FloatParam * SharedResourceContainerObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getFloatParam

BoolParam * SharedResourceContainerObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getBoolParam

StringParam * SharedResourceContainerObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getStringParam

StringIdParam * SharedResourceContainerObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getStringIdParam

VectorParam * SharedResourceContainerObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getVectorParam

DynamicVariableParam * SharedResourceContainerObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getDynamicVariableParam

StructParamOT * SharedResourceContainerObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedResourceContainerObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedResourceContainerObjectTemplate::getTriggerVolumeParam

void SharedResourceContainerObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedTangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedResourceContainerObjectTemplate::initStructParamOT

void SharedResourceContainerObjectTemplate::setAsEmptyList(const char *name)
{
	SharedTangibleObjectTemplate::setAsEmptyList(name);
}	// SharedResourceContainerObjectTemplate::setAsEmptyList

void SharedResourceContainerObjectTemplate::setAppend(const char *name)
{
	SharedTangibleObjectTemplate::setAppend(name);
}	// SharedResourceContainerObjectTemplate::setAppend

bool SharedResourceContainerObjectTemplate::isAppend(const char *name) const
{
	return SharedTangibleObjectTemplate::isAppend(name);
}	// SharedResourceContainerObjectTemplate::isAppend


int SharedResourceContainerObjectTemplate::getListLength(const char *name) const
{
	return SharedTangibleObjectTemplate::getListLength(name);
}	// SharedResourceContainerObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedResourceContainerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedResourceContainerObjectTemplate_tag)
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
}	// SharedResourceContainerObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedResourceContainerObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedResourceContainerObjectTemplate_tag);
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
}	// SharedResourceContainerObjectTemplate::save

//@END TFD
