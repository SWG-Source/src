//========================================================================
//
// SharedVehicleObjectTemplate.cpp
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
#include "SharedVehicleObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedVehicleObjectTemplate::SharedVehicleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedVehicleObjectTemplate::SharedVehicleObjectTemplate

/**
 * Class destructor.
 */
SharedVehicleObjectTemplate::~SharedVehicleObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedVehicleObjectTemplate::~SharedVehicleObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedVehicleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedVehicleObjectTemplate_tag, create);
}	// SharedVehicleObjectTemplate::registerMe

/**
 * Creates a SharedVehicleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedVehicleObjectTemplate::create(const std::string & filename)
{
	return new SharedVehicleObjectTemplate(filename);
}	// SharedVehicleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedVehicleObjectTemplate::getId(void) const
{
	return SharedVehicleObjectTemplate_tag;
}	// SharedVehicleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedVehicleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedVehicleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedVehicleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedVehicleObjectTemplate * base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedVehicleObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedVehicleObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedVehicleObjectTemplate::getCompilerIntegerParam

FloatParam * SharedVehicleObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "speed") == 0)
	{
		if (index >= 0 && index < 5)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_speed[index];
		}
		fprintf(stderr, "index for parameter \"speed\" out of bounds\n");
	}
	else if (strcmp(name, "slopeAversion") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_slopeAversion;
		}
		fprintf(stderr, "trying to access single-parameter \"slopeAversion\" as an array\n");
	}
	else if (strcmp(name, "hoverValue") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_hoverValue;
		}
		fprintf(stderr, "trying to access single-parameter \"hoverValue\" as an array\n");
	}
	else if (strcmp(name, "turnRate") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_turnRate;
		}
		fprintf(stderr, "trying to access single-parameter \"turnRate\" as an array\n");
	}
	else if (strcmp(name, "maxVelocity") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxVelocity;
		}
		fprintf(stderr, "trying to access single-parameter \"maxVelocity\" as an array\n");
	}
	else if (strcmp(name, "acceleration") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_acceleration;
		}
		fprintf(stderr, "trying to access single-parameter \"acceleration\" as an array\n");
	}
	else if (strcmp(name, "braking") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_braking;
		}
		fprintf(stderr, "trying to access single-parameter \"braking\" as an array\n");
	}
	else
		return SharedTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//SharedVehicleObjectTemplate::getFloatParam

BoolParam * SharedVehicleObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedVehicleObjectTemplate::getBoolParam

StringParam * SharedVehicleObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedVehicleObjectTemplate::getStringParam

StringIdParam * SharedVehicleObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedVehicleObjectTemplate::getStringIdParam

VectorParam * SharedVehicleObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedVehicleObjectTemplate::getVectorParam

DynamicVariableParam * SharedVehicleObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedVehicleObjectTemplate::getDynamicVariableParam

StructParamOT * SharedVehicleObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedVehicleObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedVehicleObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedVehicleObjectTemplate::getTriggerVolumeParam

void SharedVehicleObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedTangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedVehicleObjectTemplate::initStructParamOT

void SharedVehicleObjectTemplate::setAsEmptyList(const char *name)
{
	SharedTangibleObjectTemplate::setAsEmptyList(name);
}	// SharedVehicleObjectTemplate::setAsEmptyList

void SharedVehicleObjectTemplate::setAppend(const char *name)
{
	SharedTangibleObjectTemplate::setAppend(name);
}	// SharedVehicleObjectTemplate::setAppend

bool SharedVehicleObjectTemplate::isAppend(const char *name) const
{
	return SharedTangibleObjectTemplate::isAppend(name);
}	// SharedVehicleObjectTemplate::isAppend


int SharedVehicleObjectTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "speed") == 0)
	{
		return sizeof(m_speed) / sizeof(FloatParam);
	}
	else
		return SharedTangibleObjectTemplate::getListLength(name);
}	// SharedVehicleObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedVehicleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedVehicleObjectTemplate_tag)
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
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "speed") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 5, ("Template %s: read array size of %d for array \"speed\" of size 5, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 5 && j < listCount; ++j)
				m_speed[j].loadFromIff(file);
			// if there are more params for speed read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "slopeAversion") == 0)
			m_slopeAversion.loadFromIff(file);
		else if (strcmp(paramName, "hoverValue") == 0)
			m_hoverValue.loadFromIff(file);
		else if (strcmp(paramName, "turnRate") == 0)
			m_turnRate.loadFromIff(file);
		else if (strcmp(paramName, "maxVelocity") == 0)
			m_maxVelocity.loadFromIff(file);
		else if (strcmp(paramName, "acceleration") == 0)
			m_acceleration.loadFromIff(file);
		else if (strcmp(paramName, "braking") == 0)
			m_braking.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedVehicleObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedVehicleObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedVehicleObjectTemplate_tag);
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

	// save speed
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("speed");
	count = 5;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 5; ++i)
		m_speed[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save slopeAversion
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("slopeAversion");
	m_slopeAversion.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save hoverValue
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("hoverValue");
	m_hoverValue.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save turnRate
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("turnRate");
	m_turnRate.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxVelocity
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxVelocity");
	m_maxVelocity.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save acceleration
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("acceleration");
	m_acceleration.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save braking
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("braking");
	m_braking.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	SharedTangibleObjectTemplate::save(file);
	file.exitForm();
}	// SharedVehicleObjectTemplate::save

//@END TFD
