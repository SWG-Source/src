//========================================================================
//
// ServerWeaponObjectTemplate.cpp
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
#include "ServerWeaponObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerWeaponObjectTemplate::ServerWeaponObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
//@END TFD INIT
{
}	// ServerWeaponObjectTemplate::ServerWeaponObjectTemplate

/**
 * Class destructor.
 */
ServerWeaponObjectTemplate::~ServerWeaponObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerWeaponObjectTemplate::~ServerWeaponObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerWeaponObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerWeaponObjectTemplate_tag, create);
}	// ServerWeaponObjectTemplate::registerMe

/**
 * Creates a ServerWeaponObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerWeaponObjectTemplate::create(const std::string & filename)
{
	return new ServerWeaponObjectTemplate(filename);
}	// ServerWeaponObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerWeaponObjectTemplate::getId(void) const
{
	return ServerWeaponObjectTemplate_tag;
}	// ServerWeaponObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerWeaponObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerWeaponObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerWeaponObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerWeaponObjectTemplate * base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerWeaponObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerWeaponObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "weaponType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_weaponType;
		}
		fprintf(stderr, "trying to access single-parameter \"weaponType\" as an array\n");
	}
	else if (strcmp(name, "attackType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_attackType;
		}
		fprintf(stderr, "trying to access single-parameter \"attackType\" as an array\n");
	}
	else if (strcmp(name, "damageType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_damageType;
		}
		fprintf(stderr, "trying to access single-parameter \"damageType\" as an array\n");
	}
	else if (strcmp(name, "elementalType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_elementalType;
		}
		fprintf(stderr, "trying to access single-parameter \"elementalType\" as an array\n");
	}
	else if (strcmp(name, "elementalValue") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_elementalValue;
		}
		fprintf(stderr, "trying to access single-parameter \"elementalValue\" as an array\n");
	}
	else if (strcmp(name, "minDamageAmount") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_minDamageAmount;
		}
		fprintf(stderr, "trying to access single-parameter \"minDamageAmount\" as an array\n");
	}
	else if (strcmp(name, "maxDamageAmount") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxDamageAmount;
		}
		fprintf(stderr, "trying to access single-parameter \"maxDamageAmount\" as an array\n");
	}
	else if (strcmp(name, "attackCost") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_attackCost;
		}
		fprintf(stderr, "trying to access single-parameter \"attackCost\" as an array\n");
	}
	else if (strcmp(name, "accuracy") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_accuracy;
		}
		fprintf(stderr, "trying to access single-parameter \"accuracy\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerWeaponObjectTemplate::getCompilerIntegerParam

FloatParam * ServerWeaponObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "attackSpeed") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_attackSpeed;
		}
		fprintf(stderr, "trying to access single-parameter \"attackSpeed\" as an array\n");
	}
	else if (strcmp(name, "audibleRange") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_audibleRange;
		}
		fprintf(stderr, "trying to access single-parameter \"audibleRange\" as an array\n");
	}
	else if (strcmp(name, "minRange") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_minRange;
		}
		fprintf(stderr, "trying to access single-parameter \"minRange\" as an array\n");
	}
	else if (strcmp(name, "maxRange") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxRange;
		}
		fprintf(stderr, "trying to access single-parameter \"maxRange\" as an array\n");
	}
	else if (strcmp(name, "damageRadius") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_damageRadius;
		}
		fprintf(stderr, "trying to access single-parameter \"damageRadius\" as an array\n");
	}
	else if (strcmp(name, "woundChance") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_woundChance;
		}
		fprintf(stderr, "trying to access single-parameter \"woundChance\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//ServerWeaponObjectTemplate::getFloatParam

BoolParam * ServerWeaponObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//ServerWeaponObjectTemplate::getBoolParam

StringParam * ServerWeaponObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
}	//ServerWeaponObjectTemplate::getStringParam

StringIdParam * ServerWeaponObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerWeaponObjectTemplate::getStringIdParam

VectorParam * ServerWeaponObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerWeaponObjectTemplate::getVectorParam

DynamicVariableParam * ServerWeaponObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerWeaponObjectTemplate::getDynamicVariableParam

StructParamOT * ServerWeaponObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerWeaponObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerWeaponObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerWeaponObjectTemplate::getTriggerVolumeParam

void ServerWeaponObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerWeaponObjectTemplate::initStructParamOT

void ServerWeaponObjectTemplate::setAsEmptyList(const char *name)
{
	ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerWeaponObjectTemplate::setAsEmptyList

void ServerWeaponObjectTemplate::setAppend(const char *name)
{
	ServerTangibleObjectTemplate::setAppend(name);
}	// ServerWeaponObjectTemplate::setAppend

bool ServerWeaponObjectTemplate::isAppend(const char *name) const
{
	return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerWeaponObjectTemplate::isAppend


int ServerWeaponObjectTemplate::getListLength(const char *name) const
{
	return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerWeaponObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerWeaponObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerWeaponObjectTemplate_tag)
	{
		ServerTangibleObjectTemplate::load(file);
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
	if (getHighestTemplateVersion() != TAG(0,0,1,1))
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
		if (strcmp(paramName, "weaponType") == 0)
			m_weaponType.loadFromIff(file);
		else if (strcmp(paramName, "attackType") == 0)
			m_attackType.loadFromIff(file);
		else if (strcmp(paramName, "damageType") == 0)
			m_damageType.loadFromIff(file);
		else if (strcmp(paramName, "elementalType") == 0)
			m_elementalType.loadFromIff(file);
		else if (strcmp(paramName, "elementalValue") == 0)
			m_elementalValue.loadFromIff(file);
		else if (strcmp(paramName, "minDamageAmount") == 0)
			m_minDamageAmount.loadFromIff(file);
		else if (strcmp(paramName, "maxDamageAmount") == 0)
			m_maxDamageAmount.loadFromIff(file);
		else if (strcmp(paramName, "attackSpeed") == 0)
			m_attackSpeed.loadFromIff(file);
		else if (strcmp(paramName, "audibleRange") == 0)
			m_audibleRange.loadFromIff(file);
		else if (strcmp(paramName, "minRange") == 0)
			m_minRange.loadFromIff(file);
		else if (strcmp(paramName, "maxRange") == 0)
			m_maxRange.loadFromIff(file);
		else if (strcmp(paramName, "damageRadius") == 0)
			m_damageRadius.loadFromIff(file);
		else if (strcmp(paramName, "woundChance") == 0)
			m_woundChance.loadFromIff(file);
		else if (strcmp(paramName, "attackCost") == 0)
			m_attackCost.loadFromIff(file);
		else if (strcmp(paramName, "accuracy") == 0)
			m_accuracy.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerWeaponObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerWeaponObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerWeaponObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,1,1));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save weaponType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("weaponType");
	m_weaponType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save attackType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("attackType");
	m_attackType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save damageType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("damageType");
	m_damageType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save elementalType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("elementalType");
	m_elementalType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save elementalValue
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("elementalValue");
	m_elementalValue.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save minDamageAmount
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("minDamageAmount");
	m_minDamageAmount.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxDamageAmount
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxDamageAmount");
	m_maxDamageAmount.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save attackSpeed
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("attackSpeed");
	m_attackSpeed.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save audibleRange
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("audibleRange");
	m_audibleRange.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save minRange
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("minRange");
	m_minRange.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxRange
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxRange");
	m_maxRange.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save damageRadius
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("damageRadius");
	m_damageRadius.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save woundChance
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("woundChance");
	m_woundChance.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save attackCost
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("attackCost");
	m_attackCost.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save accuracy
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("accuracy");
	m_accuracy.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	ServerTangibleObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// ServerWeaponObjectTemplate::save

//@END TFD
