//========================================================================
//
// SharedGuildObjectTemplate.cpp
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
#include "SharedGuildObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedGuildObjectTemplate::SharedGuildObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedGuildObjectTemplate::SharedGuildObjectTemplate

/**
 * Class destructor.
 */
SharedGuildObjectTemplate::~SharedGuildObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedGuildObjectTemplate::~SharedGuildObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedGuildObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedGuildObjectTemplate_tag, create);
}	// SharedGuildObjectTemplate::registerMe

/**
 * Creates a SharedGuildObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedGuildObjectTemplate::create(const std::string & filename)
{
	return new SharedGuildObjectTemplate(filename);
}	// SharedGuildObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedGuildObjectTemplate::getId(void) const
{
	return SharedGuildObjectTemplate_tag;
}	// SharedGuildObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedGuildObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedGuildObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedGuildObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedGuildObjectTemplate * base = dynamic_cast<const SharedGuildObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedGuildObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedGuildObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getCompilerIntegerParam

FloatParam * SharedGuildObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getFloatParam

BoolParam * SharedGuildObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getBoolParam

StringParam * SharedGuildObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getStringParam

StringIdParam * SharedGuildObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getStringIdParam

VectorParam * SharedGuildObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getVectorParam

DynamicVariableParam * SharedGuildObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getDynamicVariableParam

StructParamOT * SharedGuildObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedGuildObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedGuildObjectTemplate::getTriggerVolumeParam

void SharedGuildObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedUniverseObjectTemplate::initStructParamOT(param, name);
}	// SharedGuildObjectTemplate::initStructParamOT

void SharedGuildObjectTemplate::setAsEmptyList(const char *name)
{
	SharedUniverseObjectTemplate::setAsEmptyList(name);
}	// SharedGuildObjectTemplate::setAsEmptyList

void SharedGuildObjectTemplate::setAppend(const char *name)
{
	SharedUniverseObjectTemplate::setAppend(name);
}	// SharedGuildObjectTemplate::setAppend

bool SharedGuildObjectTemplate::isAppend(const char *name) const
{
	return SharedUniverseObjectTemplate::isAppend(name);
}	// SharedGuildObjectTemplate::isAppend


int SharedGuildObjectTemplate::getListLength(const char *name) const
{
	return SharedUniverseObjectTemplate::getListLength(name);
}	// SharedGuildObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedGuildObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedGuildObjectTemplate_tag)
	{
		SharedUniverseObjectTemplate::load(file);
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
	SharedUniverseObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedGuildObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedGuildObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedGuildObjectTemplate_tag);
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
	SharedUniverseObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// SharedGuildObjectTemplate::save

//@END TFD
