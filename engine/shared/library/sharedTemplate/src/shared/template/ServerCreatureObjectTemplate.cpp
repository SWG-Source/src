//========================================================================
//
// ServerCreatureObjectTemplate.cpp
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
#include "ServerCreatureObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerCreatureObjectTemplate::ServerCreatureObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_attribModsLoaded(false)
	,m_attribModsAppend(false)
//@END TFD INIT
{
}	// ServerCreatureObjectTemplate::ServerCreatureObjectTemplate

/**
 * Class destructor.
 */
ServerCreatureObjectTemplate::~ServerCreatureObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_attribMods.begin(); iter != m_attribMods.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_attribMods.clear();
	}
//@END TFD CLEANUP
}	// ServerCreatureObjectTemplate::~ServerCreatureObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerCreatureObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerCreatureObjectTemplate_tag, create);
}	// ServerCreatureObjectTemplate::registerMe

/**
 * Creates a ServerCreatureObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerCreatureObjectTemplate::create(const std::string & filename)
{
	return new ServerCreatureObjectTemplate(filename);
}	// ServerCreatureObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerCreatureObjectTemplate::getId(void) const
{
	return ServerCreatureObjectTemplate_tag;
}	// ServerCreatureObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerCreatureObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerCreatureObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerCreatureObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerCreatureObjectTemplate * base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerCreatureObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerCreatureObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "attributes") == 0)
	{
		if (index >= 0 && index < 6)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_attributes[index];
		}
		fprintf(stderr, "index for parameter \"attributes\" out of bounds\n");
	}
	else if (strcmp(name, "minAttributes") == 0)
	{
		if (index >= 0 && index < 6)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_minAttributes[index];
		}
		fprintf(stderr, "index for parameter \"minAttributes\" out of bounds\n");
	}
	else if (strcmp(name, "maxAttributes") == 0)
	{
		if (index >= 0 && index < 6)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxAttributes[index];
		}
		fprintf(stderr, "index for parameter \"maxAttributes\" out of bounds\n");
	}
	else if (strcmp(name, "shockWounds") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_shockWounds;
		}
		fprintf(stderr, "trying to access single-parameter \"shockWounds\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerCreatureObjectTemplate::getCompilerIntegerParam

FloatParam * ServerCreatureObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "minDrainModifier") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_minDrainModifier;
		}
		fprintf(stderr, "trying to access single-parameter \"minDrainModifier\" as an array\n");
	}
	else if (strcmp(name, "maxDrainModifier") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxDrainModifier;
		}
		fprintf(stderr, "trying to access single-parameter \"maxDrainModifier\" as an array\n");
	}
	else if (strcmp(name, "minFaucetModifier") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_minFaucetModifier;
		}
		fprintf(stderr, "trying to access single-parameter \"minFaucetModifier\" as an array\n");
	}
	else if (strcmp(name, "maxFaucetModifier") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxFaucetModifier;
		}
		fprintf(stderr, "trying to access single-parameter \"maxFaucetModifier\" as an array\n");
	}
	else if (strcmp(name, "approachTriggerRange") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_approachTriggerRange;
		}
		fprintf(stderr, "trying to access single-parameter \"approachTriggerRange\" as an array\n");
	}
	else if (strcmp(name, "maxMentalStates") == 0)
	{
		if (index >= 0 && index < 4)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxMentalStates[index];
		}
		fprintf(stderr, "index for parameter \"maxMentalStates\" out of bounds\n");
	}
	else if (strcmp(name, "mentalStatesDecay") == 0)
	{
		if (index >= 0 && index < 4)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_mentalStatesDecay[index];
		}
		fprintf(stderr, "index for parameter \"mentalStatesDecay\" out of bounds\n");
	}
	else
		return ServerTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//ServerCreatureObjectTemplate::getFloatParam

BoolParam * ServerCreatureObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "canCreateAvatar") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_canCreateAvatar;
		}
		fprintf(stderr, "trying to access single-parameter \"canCreateAvatar\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//ServerCreatureObjectTemplate::getBoolParam

StringParam * ServerCreatureObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "defaultWeapon") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_defaultWeapon;
		}
		fprintf(stderr, "trying to access single-parameter \"defaultWeapon\" as an array\n");
	}
	else if (strcmp(name, "nameGeneratorType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_nameGeneratorType;
		}
		fprintf(stderr, "trying to access single-parameter \"nameGeneratorType\" as an array\n");
	}
	else
		return ServerTangibleObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerCreatureObjectTemplate::getStringParam

StringIdParam * ServerCreatureObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerCreatureObjectTemplate::getStringIdParam

VectorParam * ServerCreatureObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//ServerCreatureObjectTemplate::getVectorParam

DynamicVariableParam * ServerCreatureObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerCreatureObjectTemplate::getDynamicVariableParam

StructParamOT * ServerCreatureObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "attribMods") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_attribMods.size()))
			return m_attribMods[index];
		if (index == static_cast<int>(m_attribMods.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_attribMods.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"attribMods\" out of bounds\n");
	}
	else
		return ServerTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
	return nullptr;
}	//ServerCreatureObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerCreatureObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return ServerTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerCreatureObjectTemplate::getTriggerVolumeParam

void ServerCreatureObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "attribMods") == 0)
		param.setValue(new _AttribMod(""));
	else
		ServerTangibleObjectTemplate::initStructParamOT(param, name);
}	// ServerCreatureObjectTemplate::initStructParamOT

void ServerCreatureObjectTemplate::setAsEmptyList(const char *name)
{
	if (strcmp(name, "attribMods") == 0)
	{
		m_attribMods.clear();
		m_attribModsLoaded = true;
	}
	else
		ServerTangibleObjectTemplate::setAsEmptyList(name);
}	// ServerCreatureObjectTemplate::setAsEmptyList

void ServerCreatureObjectTemplate::setAppend(const char *name)
{
	if (strcmp(name, "attribMods") == 0)
		m_attribModsAppend = true;
	else
		ServerTangibleObjectTemplate::setAppend(name);
}	// ServerCreatureObjectTemplate::setAppend

bool ServerCreatureObjectTemplate::isAppend(const char *name) const
{
	if (strcmp(name, "attribMods") == 0)
		return m_attribModsAppend;
	else
		return ServerTangibleObjectTemplate::isAppend(name);
}	// ServerCreatureObjectTemplate::isAppend


int ServerCreatureObjectTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "attributes") == 0)
	{
		return sizeof(m_attributes) / sizeof(CompilerIntegerParam);
	}
	else if (strcmp(name, "minAttributes") == 0)
	{
		return sizeof(m_minAttributes) / sizeof(CompilerIntegerParam);
	}
	else if (strcmp(name, "maxAttributes") == 0)
	{
		return sizeof(m_maxAttributes) / sizeof(CompilerIntegerParam);
	}
	else if (strcmp(name, "attribMods") == 0)
	{
		return m_attribMods.size();
	}
	else if (strcmp(name, "maxMentalStates") == 0)
	{
		return sizeof(m_maxMentalStates) / sizeof(FloatParam);
	}
	else if (strcmp(name, "mentalStatesDecay") == 0)
	{
		return sizeof(m_mentalStatesDecay) / sizeof(FloatParam);
	}
	else
		return ServerTangibleObjectTemplate::getListLength(name);
}	// ServerCreatureObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerCreatureObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerCreatureObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,5))
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
		if (strcmp(paramName, "defaultWeapon") == 0)
			m_defaultWeapon.loadFromIff(file);
		else if (strcmp(paramName, "attributes") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 6, ("Template %s: read array size of %d for array \"attributes\" of size 6, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 6 && j < listCount; ++j)
				m_attributes[j].loadFromIff(file);
			// if there are more params for attributes read and dump them
			for (; j < listCount; ++j)
			{
				CompilerIntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "minAttributes") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 6, ("Template %s: read array size of %d for array \"minAttributes\" of size 6, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 6 && j < listCount; ++j)
				m_minAttributes[j].loadFromIff(file);
			// if there are more params for minAttributes read and dump them
			for (; j < listCount; ++j)
			{
				CompilerIntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "maxAttributes") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 6, ("Template %s: read array size of %d for array \"maxAttributes\" of size 6, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 6 && j < listCount; ++j)
				m_maxAttributes[j].loadFromIff(file);
			// if there are more params for maxAttributes read and dump them
			for (; j < listCount; ++j)
			{
				CompilerIntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "minDrainModifier") == 0)
			m_minDrainModifier.loadFromIff(file);
		else if (strcmp(paramName, "maxDrainModifier") == 0)
			m_maxDrainModifier.loadFromIff(file);
		else if (strcmp(paramName, "minFaucetModifier") == 0)
			m_minFaucetModifier.loadFromIff(file);
		else if (strcmp(paramName, "maxFaucetModifier") == 0)
			m_maxFaucetModifier.loadFromIff(file);
		else if (strcmp(paramName, "attribMods") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_attribMods.begin(); iter != m_attribMods.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_attribMods.clear();
			m_attribModsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_attribMods.push_back(newData);
			}
			m_attribModsLoaded = true;
		}
		else if (strcmp(paramName, "shockWounds") == 0)
			m_shockWounds.loadFromIff(file);
		else if (strcmp(paramName, "canCreateAvatar") == 0)
			m_canCreateAvatar.loadFromIff(file);
		else if (strcmp(paramName, "nameGeneratorType") == 0)
			m_nameGeneratorType.loadFromIff(file);
		else if (strcmp(paramName, "approachTriggerRange") == 0)
			m_approachTriggerRange.loadFromIff(file);
		else if (strcmp(paramName, "maxMentalStates") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 4, ("Template %s: read array size of %d for array \"maxMentalStates\" of size 4, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 4 && j < listCount; ++j)
				m_maxMentalStates[j].loadFromIff(file);
			// if there are more params for maxMentalStates read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "mentalStatesDecay") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 4, ("Template %s: read array size of %d for array \"mentalStatesDecay\" of size 4, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 4 && j < listCount; ++j)
				m_mentalStatesDecay[j].loadFromIff(file);
			// if there are more params for mentalStatesDecay read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerCreatureObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerCreatureObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerCreatureObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,5));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save defaultWeapon
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("defaultWeapon");
	m_defaultWeapon.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save attributes
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("attributes");
	count = 6;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 6; ++i)
		m_attributes[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save minAttributes
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("minAttributes");
	count = 6;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 6; ++i)
		m_minAttributes[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save maxAttributes
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxAttributes");
	count = 6;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 6; ++i)
		m_maxAttributes[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save minDrainModifier
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("minDrainModifier");
	m_minDrainModifier.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxDrainModifier
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxDrainModifier");
	m_maxDrainModifier.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save minFaucetModifier
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("minFaucetModifier");
	m_minFaucetModifier.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxFaucetModifier
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxFaucetModifier");
	m_maxFaucetModifier.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_attribModsLoaded)
	{
		// mark the list as empty and extending the base list
		m_attribModsAppend = true;
	}
	// save attribMods
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("attribMods");
	file.insertChunkData(&m_attribModsAppend, sizeof(bool));
	count = m_attribMods.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_attribMods[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save shockWounds
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("shockWounds");
	m_shockWounds.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save canCreateAvatar
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("canCreateAvatar");
	m_canCreateAvatar.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save nameGeneratorType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("nameGeneratorType");
	m_nameGeneratorType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save approachTriggerRange
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("approachTriggerRange");
	m_approachTriggerRange.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxMentalStates
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxMentalStates");
	count = 4;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 4; ++i)
		m_maxMentalStates[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save mentalStatesDecay
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("mentalStatesDecay");
	count = 4;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 4; ++i)
		m_mentalStatesDecay[i].saveToIff(file);}
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
}	// ServerCreatureObjectTemplate::save

//@END TFD
