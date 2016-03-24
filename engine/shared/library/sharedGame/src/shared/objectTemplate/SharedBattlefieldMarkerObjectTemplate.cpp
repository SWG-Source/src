//========================================================================
//
// SharedBattlefieldMarkerObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedBattlefieldMarkerObjectTemplate.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Vector.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <algorithm>
#include <cstdio>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool SharedBattlefieldMarkerObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedBattlefieldMarkerObjectTemplate::SharedBattlefieldMarkerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedBattlefieldMarkerObjectTemplate::SharedBattlefieldMarkerObjectTemplate

/**
 * Class destructor.
 */
SharedBattlefieldMarkerObjectTemplate::~SharedBattlefieldMarkerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedBattlefieldMarkerObjectTemplate::~SharedBattlefieldMarkerObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedBattlefieldMarkerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedBattlefieldMarkerObjectTemplate_tag, create);
}	// SharedBattlefieldMarkerObjectTemplate::registerMe

/**
 * Creates a SharedBattlefieldMarkerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedBattlefieldMarkerObjectTemplate::create(const std::string & filename)
{
	return new SharedBattlefieldMarkerObjectTemplate(filename);
}	// SharedBattlefieldMarkerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedBattlefieldMarkerObjectTemplate::getId(void) const
{
	return SharedBattlefieldMarkerObjectTemplate_tag;
}	// SharedBattlefieldMarkerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedBattlefieldMarkerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedBattlefieldMarkerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedBattlefieldMarkerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedBattlefieldMarkerObjectTemplate * base = dynamic_cast<const SharedBattlefieldMarkerObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedBattlefieldMarkerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
int SharedBattlefieldMarkerObjectTemplate::getNumberOfPoles() const
{


	const SharedBattlefieldMarkerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedBattlefieldMarkerObjectTemplate *>(m_baseData);
	}

	if (!m_numberOfPoles.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter numberOfPoles in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter numberOfPoles has not been defined in template %s!", DataResource::getName()));
			return base->getNumberOfPoles();
		}
	}

	int value = m_numberOfPoles.getValue();
	char delta = m_numberOfPoles.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getNumberOfPoles();
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
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}

	return value;
}	// SharedBattlefieldMarkerObjectTemplate::getNumberOfPoles

int SharedBattlefieldMarkerObjectTemplate::getNumberOfPolesMin() const
{


	const SharedBattlefieldMarkerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedBattlefieldMarkerObjectTemplate *>(m_baseData);
	}

	if (!m_numberOfPoles.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter numberOfPoles in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter numberOfPoles has not been defined in template %s!", DataResource::getName()));
			return base->getNumberOfPolesMin();
		}
	}

	int value = m_numberOfPoles.getMinValue();
	char delta = m_numberOfPoles.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getNumberOfPolesMin();
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
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}

	return value;
}	// SharedBattlefieldMarkerObjectTemplate::getNumberOfPolesMin

int SharedBattlefieldMarkerObjectTemplate::getNumberOfPolesMax() const
{


	const SharedBattlefieldMarkerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedBattlefieldMarkerObjectTemplate *>(m_baseData);
	}

	if (!m_numberOfPoles.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter numberOfPoles in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter numberOfPoles has not been defined in template %s!", DataResource::getName()));
			return base->getNumberOfPolesMax();
		}
	}

	int value = m_numberOfPoles.getMaxValue();
	char delta = m_numberOfPoles.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getNumberOfPolesMax();
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
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}

	return value;
}	// SharedBattlefieldMarkerObjectTemplate::getNumberOfPolesMax

float SharedBattlefieldMarkerObjectTemplate::getRadius() const
{


	const SharedBattlefieldMarkerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedBattlefieldMarkerObjectTemplate *>(m_baseData);
	}

	if (!m_radius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter radius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter radius has not been defined in template %s!", DataResource::getName()));
			return base->getRadius();
		}
	}

	float value = m_radius.getValue();
	char delta = m_radius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getRadius();
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
}	// SharedBattlefieldMarkerObjectTemplate::getRadius

float SharedBattlefieldMarkerObjectTemplate::getRadiusMin() const
{


	const SharedBattlefieldMarkerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedBattlefieldMarkerObjectTemplate *>(m_baseData);
	}

	if (!m_radius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter radius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter radius has not been defined in template %s!", DataResource::getName()));
			return base->getRadiusMin();
		}
	}

	float value = m_radius.getMinValue();
	char delta = m_radius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getRadiusMin();
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
}	// SharedBattlefieldMarkerObjectTemplate::getRadiusMin

float SharedBattlefieldMarkerObjectTemplate::getRadiusMax() const
{


	const SharedBattlefieldMarkerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedBattlefieldMarkerObjectTemplate *>(m_baseData);
	}

	if (!m_radius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter radius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter radius has not been defined in template %s!", DataResource::getName()));
			return base->getRadiusMax();
		}
	}

	float value = m_radius.getMaxValue();
	char delta = m_radius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getRadiusMax();
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
}	// SharedBattlefieldMarkerObjectTemplate::getRadiusMax


/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedBattlefieldMarkerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedBattlefieldMarkerObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,1))
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
		if (strcmp(paramName, "numberOfPoles") == 0)
			m_numberOfPoles.loadFromIff(file);
		else if (strcmp(paramName, "radius") == 0)
			m_radius.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedBattlefieldMarkerObjectTemplate::load

//@END TFD
