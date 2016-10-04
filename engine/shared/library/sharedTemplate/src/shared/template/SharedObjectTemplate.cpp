//========================================================================
//
// SharedObjectTemplate.cpp
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
#include "SharedObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedObjectTemplate::SharedObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: TpfTemplate(filename)
//@END TFD INIT
{
}	// SharedObjectTemplate::SharedObjectTemplate

/**
 * Class destructor.
 */
SharedObjectTemplate::~SharedObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedObjectTemplate::~SharedObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedObjectTemplate_tag, create);
}	// SharedObjectTemplate::registerMe

/**
 * Creates a SharedObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedObjectTemplate::create(const std::string & filename)
{
	return new SharedObjectTemplate(filename);
}	// SharedObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedObjectTemplate::getId(void) const
{
	return SharedObjectTemplate_tag;
}	// SharedObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedObjectTemplate * base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "containerType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_containerType;
		}
		fprintf(stderr, "trying to access single-parameter \"containerType\" as an array\n");
	}
	else if (strcmp(name, "containerVolumeLimit") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_containerVolumeLimit;
		}
		fprintf(stderr, "trying to access single-parameter \"containerVolumeLimit\" as an array\n");
	}
	else if (strcmp(name, "gameObjectType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_gameObjectType;
		}
		fprintf(stderr, "trying to access single-parameter \"gameObjectType\" as an array\n");
	}
	else if (strcmp(name, "surfaceType") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_surfaceType;
		}
		fprintf(stderr, "trying to access single-parameter \"surfaceType\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//SharedObjectTemplate::getCompilerIntegerParam

FloatParam * SharedObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "scale") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_scale;
		}
		fprintf(stderr, "trying to access single-parameter \"scale\" as an array\n");
	}
	else if (strcmp(name, "scaleThresholdBeforeExtentTest") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_scaleThresholdBeforeExtentTest;
		}
		fprintf(stderr, "trying to access single-parameter \"scaleThresholdBeforeExtentTest\" as an array\n");
	}
	else if (strcmp(name, "clearFloraRadius") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_clearFloraRadius;
		}
		fprintf(stderr, "trying to access single-parameter \"clearFloraRadius\" as an array\n");
	}
	else if (strcmp(name, "noBuildRadius") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_noBuildRadius;
		}
		fprintf(stderr, "trying to access single-parameter \"noBuildRadius\" as an array\n");
	}
	else if (strcmp(name, "locationReservationRadius") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_locationReservationRadius;
		}
		fprintf(stderr, "trying to access single-parameter \"locationReservationRadius\" as an array\n");
	}
	else
		return TpfTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//SharedObjectTemplate::getFloatParam

BoolParam * SharedObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "snapToTerrain") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_snapToTerrain;
		}
		fprintf(stderr, "trying to access single-parameter \"snapToTerrain\" as an array\n");
	}
	else if (strcmp(name, "sendToClient") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_sendToClient;
		}
		fprintf(stderr, "trying to access single-parameter \"sendToClient\" as an array\n");
	}
	else if (strcmp(name, "onlyVisibleInTools") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_onlyVisibleInTools;
		}
		fprintf(stderr, "trying to access single-parameter \"onlyVisibleInTools\" as an array\n");
	}
	else if (strcmp(name, "forceNoCollision") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_forceNoCollision;
		}
		fprintf(stderr, "trying to access single-parameter \"forceNoCollision\" as an array\n");
	}
	else
		return TpfTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//SharedObjectTemplate::getBoolParam

StringParam * SharedObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "tintPalette") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_tintPalette;
		}
		fprintf(stderr, "trying to access single-parameter \"tintPalette\" as an array\n");
	}
	else if (strcmp(name, "slotDescriptorFilename") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_slotDescriptorFilename;
		}
		fprintf(stderr, "trying to access single-parameter \"slotDescriptorFilename\" as an array\n");
	}
	else if (strcmp(name, "arrangementDescriptorFilename") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_arrangementDescriptorFilename;
		}
		fprintf(stderr, "trying to access single-parameter \"arrangementDescriptorFilename\" as an array\n");
	}
	else if (strcmp(name, "appearanceFilename") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_appearanceFilename;
		}
		fprintf(stderr, "trying to access single-parameter \"appearanceFilename\" as an array\n");
	}
	else if (strcmp(name, "portalLayoutFilename") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_portalLayoutFilename;
		}
		fprintf(stderr, "trying to access single-parameter \"portalLayoutFilename\" as an array\n");
	}
	else if (strcmp(name, "clientDataFile") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_clientDataFile;
		}
		fprintf(stderr, "trying to access single-parameter \"clientDataFile\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedObjectTemplate::getStringParam

StringIdParam * SharedObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "objectName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_objectName;
		}
		fprintf(stderr, "trying to access single-parameter \"objectName\" as an array\n");
	}
	else if (strcmp(name, "detailedDescription") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_detailedDescription;
		}
		fprintf(stderr, "trying to access single-parameter \"detailedDescription\" as an array\n");
	}
	else if (strcmp(name, "lookAtText") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_lookAtText;
		}
		fprintf(stderr, "trying to access single-parameter \"lookAtText\" as an array\n");
	}
	else
		return TpfTemplate::getStringIdParam(name, deepCheck, index);
	return nullptr;
}	//SharedObjectTemplate::getStringIdParam

VectorParam * SharedObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//SharedObjectTemplate::getVectorParam

DynamicVariableParam * SharedObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedObjectTemplate::getDynamicVariableParam

StructParamOT * SharedObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedObjectTemplate::getTriggerVolumeParam

void SharedObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// SharedObjectTemplate::initStructParamOT

void SharedObjectTemplate::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// SharedObjectTemplate::setAsEmptyList

void SharedObjectTemplate::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// SharedObjectTemplate::setAppend

bool SharedObjectTemplate::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// SharedObjectTemplate::isAppend


int SharedObjectTemplate::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// SharedObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedObjectTemplate_tag)
	{
		TpfTemplate::load(file);
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
	if (getHighestTemplateVersion() != TAG(0,0,1,0))
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
		if (strcmp(paramName, "objectName") == 0)
			m_objectName.loadFromIff(file);
		else if (strcmp(paramName, "detailedDescription") == 0)
			m_detailedDescription.loadFromIff(file);
		else if (strcmp(paramName, "lookAtText") == 0)
			m_lookAtText.loadFromIff(file);
		else if (strcmp(paramName, "snapToTerrain") == 0)
			m_snapToTerrain.loadFromIff(file);
		else if (strcmp(paramName, "containerType") == 0)
			m_containerType.loadFromIff(file);
		else if (strcmp(paramName, "containerVolumeLimit") == 0)
			m_containerVolumeLimit.loadFromIff(file);
		else if (strcmp(paramName, "tintPalette") == 0)
			m_tintPalette.loadFromIff(file);
		else if (strcmp(paramName, "slotDescriptorFilename") == 0)
			m_slotDescriptorFilename.loadFromIff(file);
		else if (strcmp(paramName, "arrangementDescriptorFilename") == 0)
			m_arrangementDescriptorFilename.loadFromIff(file);
		else if (strcmp(paramName, "appearanceFilename") == 0)
			m_appearanceFilename.loadFromIff(file);
		else if (strcmp(paramName, "portalLayoutFilename") == 0)
			m_portalLayoutFilename.loadFromIff(file);
		else if (strcmp(paramName, "clientDataFile") == 0)
			m_clientDataFile.loadFromIff(file);
		else if (strcmp(paramName, "scale") == 0)
			m_scale.loadFromIff(file);
		else if (strcmp(paramName, "gameObjectType") == 0)
			m_gameObjectType.loadFromIff(file);
		else if (strcmp(paramName, "sendToClient") == 0)
			m_sendToClient.loadFromIff(file);
		else if (strcmp(paramName, "scaleThresholdBeforeExtentTest") == 0)
			m_scaleThresholdBeforeExtentTest.loadFromIff(file);
		else if (strcmp(paramName, "clearFloraRadius") == 0)
			m_clearFloraRadius.loadFromIff(file);
		else if (strcmp(paramName, "surfaceType") == 0)
			m_surfaceType.loadFromIff(file);
		else if (strcmp(paramName, "noBuildRadius") == 0)
			m_noBuildRadius.loadFromIff(file);
		else if (strcmp(paramName, "onlyVisibleInTools") == 0)
			m_onlyVisibleInTools.loadFromIff(file);
		else if (strcmp(paramName, "locationReservationRadius") == 0)
			m_locationReservationRadius.loadFromIff(file);
		else if (strcmp(paramName, "forceNoCollision") == 0)
			m_forceNoCollision.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	TpfTemplate::load(file);
	file.exitForm();
	return;
}	// SharedObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,1,0));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save objectName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("objectName");
	m_objectName.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save detailedDescription
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("detailedDescription");
	m_detailedDescription.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save lookAtText
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("lookAtText");
	m_lookAtText.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save snapToTerrain
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("snapToTerrain");
	m_snapToTerrain.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save containerType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("containerType");
	m_containerType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save containerVolumeLimit
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("containerVolumeLimit");
	m_containerVolumeLimit.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save tintPalette
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("tintPalette");
	m_tintPalette.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save slotDescriptorFilename
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("slotDescriptorFilename");
	m_slotDescriptorFilename.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save arrangementDescriptorFilename
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("arrangementDescriptorFilename");
	m_arrangementDescriptorFilename.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save appearanceFilename
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("appearanceFilename");
	m_appearanceFilename.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save portalLayoutFilename
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("portalLayoutFilename");
	m_portalLayoutFilename.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save clientDataFile
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("clientDataFile");
	m_clientDataFile.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save scale
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("scale");
	m_scale.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save gameObjectType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("gameObjectType");
	m_gameObjectType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save sendToClient
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("sendToClient");
	m_sendToClient.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save scaleThresholdBeforeExtentTest
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("scaleThresholdBeforeExtentTest");
	m_scaleThresholdBeforeExtentTest.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save clearFloraRadius
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("clearFloraRadius");
	m_clearFloraRadius.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save surfaceType
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("surfaceType");
	m_surfaceType.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save noBuildRadius
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("noBuildRadius");
	m_noBuildRadius.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save onlyVisibleInTools
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("onlyVisibleInTools");
	m_onlyVisibleInTools.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save locationReservationRadius
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("locationReservationRadius");
	m_locationReservationRadius.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save forceNoCollision
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("forceNoCollision");
	m_forceNoCollision.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	TpfTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// SharedObjectTemplate::save

//@END TFD
