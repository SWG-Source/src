//========================================================================
//
// SharedJediManagerObjectTemplate.cpp
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
#include "SharedJediManagerObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedJediManagerObjectTemplate::SharedJediManagerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedUniverseObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedJediManagerObjectTemplate::SharedJediManagerObjectTemplate

/**
 * Class destructor.
 */
SharedJediManagerObjectTemplate::~SharedJediManagerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedJediManagerObjectTemplate::~SharedJediManagerObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedJediManagerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedJediManagerObjectTemplate_tag, create);
}	// SharedJediManagerObjectTemplate::registerMe

/**
 * Creates a SharedJediManagerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedJediManagerObjectTemplate::create(const std::string & filename)
{
	return new SharedJediManagerObjectTemplate(filename);
}	// SharedJediManagerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedJediManagerObjectTemplate::getId(void) const
{
	return SharedJediManagerObjectTemplate_tag;
}	// SharedJediManagerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedJediManagerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedJediManagerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedJediManagerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedJediManagerObjectTemplate * base = dynamic_cast<const SharedJediManagerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedJediManagerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedJediManagerObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getCompilerIntegerParam

FloatParam * SharedJediManagerObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getFloatParam

BoolParam * SharedJediManagerObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getBoolParam

StringParam * SharedJediManagerObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getStringParam

StringIdParam * SharedJediManagerObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getStringIdParam

VectorParam * SharedJediManagerObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getVectorParam

DynamicVariableParam * SharedJediManagerObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getDynamicVariableParam

StructParamOT * SharedJediManagerObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedJediManagerObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedUniverseObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedJediManagerObjectTemplate::getTriggerVolumeParam

void SharedJediManagerObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedUniverseObjectTemplate::initStructParamOT(param, name);
}	// SharedJediManagerObjectTemplate::initStructParamOT

void SharedJediManagerObjectTemplate::setAsEmptyList(const char *name)
{
	SharedUniverseObjectTemplate::setAsEmptyList(name);
}	// SharedJediManagerObjectTemplate::setAsEmptyList

void SharedJediManagerObjectTemplate::setAppend(const char *name)
{
	SharedUniverseObjectTemplate::setAppend(name);
}	// SharedJediManagerObjectTemplate::setAppend

bool SharedJediManagerObjectTemplate::isAppend(const char *name) const
{
	return SharedUniverseObjectTemplate::isAppend(name);
}	// SharedJediManagerObjectTemplate::isAppend


int SharedJediManagerObjectTemplate::getListLength(const char *name) const
{
	return SharedUniverseObjectTemplate::getListLength(name);
}	// SharedJediManagerObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedJediManagerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedJediManagerObjectTemplate_tag)
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
}	// SharedJediManagerObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedJediManagerObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedJediManagerObjectTemplate_tag);
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
}	// SharedJediManagerObjectTemplate::save

//@END TFD
