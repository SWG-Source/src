//========================================================================
//
// SharedStaticObjectTemplate.cpp
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
#include "SharedStaticObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedStaticObjectTemplate::SharedStaticObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedStaticObjectTemplate::SharedStaticObjectTemplate

/**
 * Class destructor.
 */
SharedStaticObjectTemplate::~SharedStaticObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedStaticObjectTemplate::~SharedStaticObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedStaticObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedStaticObjectTemplate_tag, create);
}	// SharedStaticObjectTemplate::registerMe

/**
 * Creates a SharedStaticObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedStaticObjectTemplate::create(const std::string & filename)
{
	return new SharedStaticObjectTemplate(filename);
}	// SharedStaticObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedStaticObjectTemplate::getId(void) const
{
	return SharedStaticObjectTemplate_tag;
}	// SharedStaticObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedStaticObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedStaticObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedStaticObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedStaticObjectTemplate * base = dynamic_cast<const SharedStaticObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedStaticObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedStaticObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getCompilerIntegerParam

FloatParam * SharedStaticObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getFloatParam

BoolParam * SharedStaticObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getBoolParam

StringParam * SharedStaticObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getStringParam

StringIdParam * SharedStaticObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getStringIdParam

VectorParam * SharedStaticObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getVectorParam

DynamicVariableParam * SharedStaticObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getDynamicVariableParam

StructParamOT * SharedStaticObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedStaticObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedStaticObjectTemplate::getTriggerVolumeParam

void SharedStaticObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedObjectTemplate::initStructParamOT(param, name);
}	// SharedStaticObjectTemplate::initStructParamOT

void SharedStaticObjectTemplate::setAsEmptyList(const char *name)
{
	SharedObjectTemplate::setAsEmptyList(name);
}	// SharedStaticObjectTemplate::setAsEmptyList

void SharedStaticObjectTemplate::setAppend(const char *name)
{
	SharedObjectTemplate::setAppend(name);
}	// SharedStaticObjectTemplate::setAppend

bool SharedStaticObjectTemplate::isAppend(const char *name) const
{
	return SharedObjectTemplate::isAppend(name);
}	// SharedStaticObjectTemplate::isAppend


int SharedStaticObjectTemplate::getListLength(const char *name) const
{
	return SharedObjectTemplate::getListLength(name);
}	// SharedStaticObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedStaticObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedStaticObjectTemplate_tag)
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
}	// SharedStaticObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedStaticObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedStaticObjectTemplate_tag);
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
}	// SharedStaticObjectTemplate::save

//@END TFD
