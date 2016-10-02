//========================================================================
//
// SharedConstructionContractObjectTemplate.cpp
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
#include "SharedConstructionContractObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedConstructionContractObjectTemplate::SharedConstructionContractObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedConstructionContractObjectTemplate::SharedConstructionContractObjectTemplate

/**
 * Class destructor.
 */
SharedConstructionContractObjectTemplate::~SharedConstructionContractObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedConstructionContractObjectTemplate::~SharedConstructionContractObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedConstructionContractObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedConstructionContractObjectTemplate_tag, create);
}	// SharedConstructionContractObjectTemplate::registerMe

/**
 * Creates a SharedConstructionContractObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedConstructionContractObjectTemplate::create(const std::string & filename)
{
	return new SharedConstructionContractObjectTemplate(filename);
}	// SharedConstructionContractObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedConstructionContractObjectTemplate::getId(void) const
{
	return SharedConstructionContractObjectTemplate_tag;
}	// SharedConstructionContractObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedConstructionContractObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedConstructionContractObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedConstructionContractObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedConstructionContractObjectTemplate * base = dynamic_cast<const SharedConstructionContractObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedConstructionContractObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedConstructionContractObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getCompilerIntegerParam

FloatParam * SharedConstructionContractObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getFloatParam

BoolParam * SharedConstructionContractObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getBoolParam

StringParam * SharedConstructionContractObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getStringParam

StringIdParam * SharedConstructionContractObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getStringIdParam

VectorParam * SharedConstructionContractObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getVectorParam

DynamicVariableParam * SharedConstructionContractObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getDynamicVariableParam

StructParamOT * SharedConstructionContractObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedConstructionContractObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedConstructionContractObjectTemplate::getTriggerVolumeParam

void SharedConstructionContractObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedIntangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedConstructionContractObjectTemplate::initStructParamOT

void SharedConstructionContractObjectTemplate::setAsEmptyList(const char *name)
{
	SharedIntangibleObjectTemplate::setAsEmptyList(name);
}	// SharedConstructionContractObjectTemplate::setAsEmptyList

void SharedConstructionContractObjectTemplate::setAppend(const char *name)
{
	SharedIntangibleObjectTemplate::setAppend(name);
}	// SharedConstructionContractObjectTemplate::setAppend

bool SharedConstructionContractObjectTemplate::isAppend(const char *name) const
{
	return SharedIntangibleObjectTemplate::isAppend(name);
}	// SharedConstructionContractObjectTemplate::isAppend


int SharedConstructionContractObjectTemplate::getListLength(const char *name) const
{
	return SharedIntangibleObjectTemplate::getListLength(name);
}	// SharedConstructionContractObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedConstructionContractObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedConstructionContractObjectTemplate_tag)
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
}	// SharedConstructionContractObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedConstructionContractObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedConstructionContractObjectTemplate_tag);
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
}	// SharedConstructionContractObjectTemplate::save

//@END TFD
