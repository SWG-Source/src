//========================================================================
//
// SharedIntangibleObjectTemplate.cpp
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
#include "SharedIntangibleObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedIntangibleObjectTemplate::SharedIntangibleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedIntangibleObjectTemplate::SharedIntangibleObjectTemplate

/**
 * Class destructor.
 */
SharedIntangibleObjectTemplate::~SharedIntangibleObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedIntangibleObjectTemplate::~SharedIntangibleObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedIntangibleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedIntangibleObjectTemplate_tag, create);
}	// SharedIntangibleObjectTemplate::registerMe

/**
 * Creates a SharedIntangibleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedIntangibleObjectTemplate::create(const std::string & filename)
{
	return new SharedIntangibleObjectTemplate(filename);
}	// SharedIntangibleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedIntangibleObjectTemplate::getId(void) const
{
	return SharedIntangibleObjectTemplate_tag;
}	// SharedIntangibleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedIntangibleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedIntangibleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedIntangibleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedIntangibleObjectTemplate * base = dynamic_cast<const SharedIntangibleObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedIntangibleObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedIntangibleObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getCompilerIntegerParam

FloatParam * SharedIntangibleObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getFloatParam

BoolParam * SharedIntangibleObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getBoolParam

StringParam * SharedIntangibleObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getStringParam

StringIdParam * SharedIntangibleObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getStringIdParam

VectorParam * SharedIntangibleObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getVectorParam

DynamicVariableParam * SharedIntangibleObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getDynamicVariableParam

StructParamOT * SharedIntangibleObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedIntangibleObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedIntangibleObjectTemplate::getTriggerVolumeParam

void SharedIntangibleObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedObjectTemplate::initStructParamOT(param, name);
}	// SharedIntangibleObjectTemplate::initStructParamOT

void SharedIntangibleObjectTemplate::setAsEmptyList(const char *name)
{
	SharedObjectTemplate::setAsEmptyList(name);
}	// SharedIntangibleObjectTemplate::setAsEmptyList

void SharedIntangibleObjectTemplate::setAppend(const char *name)
{
	SharedObjectTemplate::setAppend(name);
}	// SharedIntangibleObjectTemplate::setAppend

bool SharedIntangibleObjectTemplate::isAppend(const char *name) const
{
	return SharedObjectTemplate::isAppend(name);
}	// SharedIntangibleObjectTemplate::isAppend


int SharedIntangibleObjectTemplate::getListLength(const char *name) const
{
	return SharedObjectTemplate::getListLength(name);
}	// SharedIntangibleObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedIntangibleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedIntangibleObjectTemplate_tag)
	{
		SharedObjectTemplate::load(file);
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
	SharedObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedIntangibleObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedIntangibleObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedIntangibleObjectTemplate_tag);
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
	SharedObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// SharedIntangibleObjectTemplate::save

//@END TFD
