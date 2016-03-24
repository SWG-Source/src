//========================================================================
//
// SharedTerrainSurfaceObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedTerrainSurfaceObjectTemplate.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Vector.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <algorithm>
#include <cstdio>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool SharedTerrainSurfaceObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedTerrainSurfaceObjectTemplate::SharedTerrainSurfaceObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedTerrainSurfaceObjectTemplate::SharedTerrainSurfaceObjectTemplate

/**
 * Class destructor.
 */
SharedTerrainSurfaceObjectTemplate::~SharedTerrainSurfaceObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedTerrainSurfaceObjectTemplate::~SharedTerrainSurfaceObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedTerrainSurfaceObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedTerrainSurfaceObjectTemplate_tag, create);
}	// SharedTerrainSurfaceObjectTemplate::registerMe

/**
 * Creates a SharedTerrainSurfaceObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedTerrainSurfaceObjectTemplate::create(const std::string & filename)
{
	return new SharedTerrainSurfaceObjectTemplate(filename);
}	// SharedTerrainSurfaceObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedTerrainSurfaceObjectTemplate::getId(void) const
{
	return SharedTerrainSurfaceObjectTemplate_tag;
}	// SharedTerrainSurfaceObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedTerrainSurfaceObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedTerrainSurfaceObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedTerrainSurfaceObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedTerrainSurfaceObjectTemplate * base = dynamic_cast<const SharedTerrainSurfaceObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedTerrainSurfaceObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
float SharedTerrainSurfaceObjectTemplate::getCover() const
{


	const SharedTerrainSurfaceObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTerrainSurfaceObjectTemplate *>(m_baseData);
	}

	if (!m_cover.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter cover in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter cover has not been defined in template %s!", DataResource::getName()));
			return base->getCover();
		}
	}

	float value = m_cover.getValue();
	char delta = m_cover.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCover();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}

	return value;
}	// SharedTerrainSurfaceObjectTemplate::getCover

float SharedTerrainSurfaceObjectTemplate::getCoverMin() const
{


	const SharedTerrainSurfaceObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTerrainSurfaceObjectTemplate *>(m_baseData);
	}

	if (!m_cover.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter cover in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter cover has not been defined in template %s!", DataResource::getName()));
			return base->getCoverMin();
		}
	}

	float value = m_cover.getMinValue();
	char delta = m_cover.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCoverMin();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}

	return value;
}	// SharedTerrainSurfaceObjectTemplate::getCoverMin

float SharedTerrainSurfaceObjectTemplate::getCoverMax() const
{


	const SharedTerrainSurfaceObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTerrainSurfaceObjectTemplate *>(m_baseData);
	}

	if (!m_cover.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter cover in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter cover has not been defined in template %s!", DataResource::getName()));
			return base->getCoverMax();
		}
	}

	float value = m_cover.getMaxValue();
	char delta = m_cover.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCoverMax();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}

	return value;
}	// SharedTerrainSurfaceObjectTemplate::getCoverMax

const std::string & SharedTerrainSurfaceObjectTemplate::getSurfaceType() const
{


	const SharedTerrainSurfaceObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTerrainSurfaceObjectTemplate *>(m_baseData);
	}

	if (!m_surfaceType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter surfaceType in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter surfaceType has not been defined in template %s!", DataResource::getName()));
			return base->getSurfaceType();
		}
	}

	const std::string & value = m_surfaceType.getValue();

	return value;
}	// SharedTerrainSurfaceObjectTemplate::getSurfaceType


/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedTerrainSurfaceObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedTerrainSurfaceObjectTemplate_tag)
	{
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
		
			
		m_versionOk = false;
	}

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "cover") == 0)
			m_cover.loadFromIff(file);
		else if (strcmp(paramName, "surfaceType") == 0)
			m_surfaceType.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	return;
}	// SharedTerrainSurfaceObjectTemplate::load

//@END TFD
