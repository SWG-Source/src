//========================================================================
//
// SharedUniverseObjectTemplate.cpp
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
#include "SharedUniverseObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedUniverseObjectTemplate::SharedUniverseObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedUniverseObjectTemplate::SharedUniverseObjectTemplate

/**
 * Class destructor.
 */
SharedUniverseObjectTemplate::~SharedUniverseObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedUniverseObjectTemplate::~SharedUniverseObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedUniverseObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedUniverseObjectTemplate_tag, create);
}	// SharedUniverseObjectTemplate::registerMe

/**
 * Creates a SharedUniverseObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedUniverseObjectTemplate::create(const std::string & filename)
{
	return new SharedUniverseObjectTemplate(filename);
}	// SharedUniverseObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedUniverseObjectTemplate::getId(void) const
{
	return SharedUniverseObjectTemplate_tag;
}	// SharedUniverseObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedUniverseObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedUniverseObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedUniverseObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedUniverseObjectTemplate * base = dynamic_cast<const SharedUniverseObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedUniverseObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedUniverseObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getCompilerIntegerParam

FloatParam * SharedUniverseObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getFloatParam

BoolParam * SharedUniverseObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getBoolParam

StringParam * SharedUniverseObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getStringParam

StringIdParam * SharedUniverseObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getStringIdParam

VectorParam * SharedUniverseObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getVectorParam

DynamicVariableParam * SharedUniverseObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getDynamicVariableParam

StructParamOT * SharedUniverseObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedUniverseObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedUniverseObjectTemplate::getTriggerVolumeParam

void SharedUniverseObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedObjectTemplate::initStructParamOT(param, name);
}	// SharedUniverseObjectTemplate::initStructParamOT

void SharedUniverseObjectTemplate::setAsEmptyList(const char *name)
{
	SharedObjectTemplate::setAsEmptyList(name);
}	// SharedUniverseObjectTemplate::setAsEmptyList

void SharedUniverseObjectTemplate::setAppend(const char *name)
{
	SharedObjectTemplate::setAppend(name);
}	// SharedUniverseObjectTemplate::setAppend

bool SharedUniverseObjectTemplate::isAppend(const char *name) const
{
	return SharedObjectTemplate::isAppend(name);
}	// SharedUniverseObjectTemplate::isAppend


int SharedUniverseObjectTemplate::getListLength(const char *name) const
{
	return SharedObjectTemplate::getListLength(name);
}	// SharedUniverseObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedUniverseObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedUniverseObjectTemplate_tag)
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
}	// SharedUniverseObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedUniverseObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedUniverseObjectTemplate_tag);
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
}	// SharedUniverseObjectTemplate::save

//@END TFD
