//========================================================================
//
// SharedManufactureSchematicObjectTemplate.cpp
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
#include "SharedManufactureSchematicObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedManufactureSchematicObjectTemplate::SharedManufactureSchematicObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedIntangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedManufactureSchematicObjectTemplate::SharedManufactureSchematicObjectTemplate

/**
 * Class destructor.
 */
SharedManufactureSchematicObjectTemplate::~SharedManufactureSchematicObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedManufactureSchematicObjectTemplate::~SharedManufactureSchematicObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedManufactureSchematicObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedManufactureSchematicObjectTemplate_tag, create);
}	// SharedManufactureSchematicObjectTemplate::registerMe

/**
 * Creates a SharedManufactureSchematicObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedManufactureSchematicObjectTemplate::create(const std::string & filename)
{
	return new SharedManufactureSchematicObjectTemplate(filename);
}	// SharedManufactureSchematicObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedManufactureSchematicObjectTemplate::getId(void) const
{
	return SharedManufactureSchematicObjectTemplate_tag;
}	// SharedManufactureSchematicObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedManufactureSchematicObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedManufactureSchematicObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedManufactureSchematicObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedManufactureSchematicObjectTemplate * base = dynamic_cast<const SharedManufactureSchematicObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedManufactureSchematicObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedManufactureSchematicObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getCompilerIntegerParam

FloatParam * SharedManufactureSchematicObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getFloatParam

BoolParam * SharedManufactureSchematicObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getBoolParam

StringParam * SharedManufactureSchematicObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getStringParam

StringIdParam * SharedManufactureSchematicObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getStringIdParam

VectorParam * SharedManufactureSchematicObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getVectorParam

DynamicVariableParam * SharedManufactureSchematicObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getDynamicVariableParam

StructParamOT * SharedManufactureSchematicObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedManufactureSchematicObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedManufactureSchematicObjectTemplate::getTriggerVolumeParam

void SharedManufactureSchematicObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedIntangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedManufactureSchematicObjectTemplate::initStructParamOT

void SharedManufactureSchematicObjectTemplate::setAsEmptyList(const char *name)
{
	SharedIntangibleObjectTemplate::setAsEmptyList(name);
}	// SharedManufactureSchematicObjectTemplate::setAsEmptyList

void SharedManufactureSchematicObjectTemplate::setAppend(const char *name)
{
	SharedIntangibleObjectTemplate::setAppend(name);
}	// SharedManufactureSchematicObjectTemplate::setAppend

bool SharedManufactureSchematicObjectTemplate::isAppend(const char *name) const
{
	return SharedIntangibleObjectTemplate::isAppend(name);
}	// SharedManufactureSchematicObjectTemplate::isAppend


int SharedManufactureSchematicObjectTemplate::getListLength(const char *name) const
{
	return SharedIntangibleObjectTemplate::getListLength(name);
}	// SharedManufactureSchematicObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedManufactureSchematicObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedManufactureSchematicObjectTemplate_tag)
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
}	// SharedManufactureSchematicObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedManufactureSchematicObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedManufactureSchematicObjectTemplate_tag);
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
}	// SharedManufactureSchematicObjectTemplate::save

//@END TFD
