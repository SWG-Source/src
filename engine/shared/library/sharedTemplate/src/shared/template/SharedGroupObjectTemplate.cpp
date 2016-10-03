//========================================================================
//
// SharedGroupObjectTemplate.cpp
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
#include "SharedGroupObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedGroupObjectTemplate::SharedGroupObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedGroupObjectTemplate::SharedGroupObjectTemplate

/**
 * Class destructor.
 */
SharedGroupObjectTemplate::~SharedGroupObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedGroupObjectTemplate::~SharedGroupObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedGroupObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedGroupObjectTemplate_tag, create);
}	// SharedGroupObjectTemplate::registerMe

/**
 * Creates a SharedGroupObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedGroupObjectTemplate::create(const std::string & filename)
{
	return new SharedGroupObjectTemplate(filename);
}	// SharedGroupObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedGroupObjectTemplate::getId(void) const
{
	return SharedGroupObjectTemplate_tag;
}	// SharedGroupObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedGroupObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedGroupObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedGroupObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedGroupObjectTemplate * base = dynamic_cast<const SharedGroupObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedGroupObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedGroupObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getCompilerIntegerParam

FloatParam * SharedGroupObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getFloatParam

BoolParam * SharedGroupObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getBoolParam

StringParam * SharedGroupObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getStringParam

StringIdParam * SharedGroupObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getStringIdParam

VectorParam * SharedGroupObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getVectorParam

DynamicVariableParam * SharedGroupObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getDynamicVariableParam

StructParamOT * SharedGroupObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedGroupObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedGroupObjectTemplate::getTriggerVolumeParam

void SharedGroupObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedUniverseObjectTemplate::initStructParamOT(param, name);
}	// SharedGroupObjectTemplate::initStructParamOT

void SharedGroupObjectTemplate::setAsEmptyList(const char *name)
{
	SharedUniverseObjectTemplate::setAsEmptyList(name);
}	// SharedGroupObjectTemplate::setAsEmptyList

void SharedGroupObjectTemplate::setAppend(const char *name)
{
	SharedUniverseObjectTemplate::setAppend(name);
}	// SharedGroupObjectTemplate::setAppend

bool SharedGroupObjectTemplate::isAppend(const char *name) const
{
	return SharedUniverseObjectTemplate::isAppend(name);
}	// SharedGroupObjectTemplate::isAppend


int SharedGroupObjectTemplate::getListLength(const char *name) const
{
	return SharedUniverseObjectTemplate::getListLength(name);
}	// SharedGroupObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedGroupObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedGroupObjectTemplate_tag)
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
}	// SharedGroupObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedGroupObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedGroupObjectTemplate_tag);
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
}	// SharedGroupObjectTemplate::save

//@END TFD
