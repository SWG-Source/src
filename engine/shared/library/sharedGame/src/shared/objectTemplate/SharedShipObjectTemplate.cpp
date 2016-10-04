//========================================================================
//
// SharedShipObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SharedShipObjectTemplate.h"

#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/Object.h"
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

bool SharedShipObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedShipObjectTemplate::SharedShipObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedShipObjectTemplate::SharedShipObjectTemplate

/**
 * Class destructor.
 */
SharedShipObjectTemplate::~SharedShipObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedShipObjectTemplate::~SharedShipObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedShipObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedShipObjectTemplate_tag, create);
}	// SharedShipObjectTemplate::registerMe

/**
 * Creates a SharedShipObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedShipObjectTemplate::create(const std::string & filename)
{
	return new SharedShipObjectTemplate(filename);
}	// SharedShipObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedShipObjectTemplate::getId(void) const
{
	return SharedShipObjectTemplate_tag;
}	// SharedShipObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedShipObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedShipObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedShipObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedShipObjectTemplate * base = dynamic_cast<const SharedShipObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedShipObjectTemplate::getHighestTemplateVersion

//----------------------------------------------------------------------

/**
* createCustomizationDataPropertyAsNeeded
*/

void SharedShipObjectTemplate::createCustomizationDataPropertyAsNeeded(Object &object, bool /* forceCreation */) const
{
	//-- Create the CustomizationDataProperty, add to Object property collection
	CustomizationDataProperty *const cdProperty = new CustomizationDataProperty(object);
	object.addProperty(*cdProperty);

	//-- Fetch the CustomizationData instance
	CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
	if (customizationData)
	{
		//-- Create customization variables associated with this appearance.  Don't skip /shared_owner/
		//   variables since we are a creature.  The SharedTangibleObjectTemplate version of
		//   this function sets it to true.
		bool const skipSharedOwnerVariables = false;
		const std::string & appearanceFilename =  getAppearanceFilename();
		if(!appearanceFilename.empty())
		{	
			AssetCustomizationManager::addCustomizationVariablesForAsset(TemporaryCrcString(appearanceFilename.c_str(), true), *customizationData, skipSharedOwnerVariables);
		}
		else
		{
			//Perhaps it's a POB ship, check the portalLayoutFilename
			const std::string & portalLayoutFilename = getPortalLayoutFilename();
			AssetCustomizationManager::addCustomizationVariablesForAsset(TemporaryCrcString(portalLayoutFilename.c_str(), true), *customizationData, skipSharedOwnerVariables);		
		}

		//-- release local reference to the CustomizationData instance
		customizationData->release();
	}
}

//@BEGIN TFD
const std::string & SharedShipObjectTemplate::getCockpitFilename(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedShipObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedShipObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCockpitFilename(true);
#endif
	}

	if (!m_cockpitFilename.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter cockpitFilename in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter cockpitFilename has not been defined in template %s!", DataResource::getName()));
			return base->getCockpitFilename();
		}
	}

	const std::string & value = m_cockpitFilename.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedShipObjectTemplate::getCockpitFilename

bool SharedShipObjectTemplate::getHasWings(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const SharedShipObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedShipObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getHasWings(true);
#endif
	}

	if (!m_hasWings.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter hasWings in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter hasWings has not been defined in template %s!", DataResource::getName()));
			return base->getHasWings();
		}
	}

	bool value = m_hasWings.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedShipObjectTemplate::getHasWings

bool SharedShipObjectTemplate::getPlayerControlled(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const SharedShipObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedShipObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPlayerControlled(true);
#endif
	}

	if (!m_playerControlled.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter playerControlled in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter playerControlled has not been defined in template %s!", DataResource::getName()));
			return base->getPlayerControlled();
		}
	}

	bool value = m_playerControlled.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedShipObjectTemplate::getPlayerControlled

const std::string & SharedShipObjectTemplate::getInteriorLayoutFileName(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedShipObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedShipObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getInteriorLayoutFileName(true);
#endif
	}

	if (!m_interiorLayoutFileName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter interiorLayoutFileName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter interiorLayoutFileName has not been defined in template %s!", DataResource::getName()));
			return base->getInteriorLayoutFileName();
		}
	}

	const std::string & value = m_interiorLayoutFileName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedShipObjectTemplate::getInteriorLayoutFileName

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedShipObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getCockpitFilename(true));
	IGNORE_RETURN(getHasWings(true));
	IGNORE_RETURN(getPlayerControlled(true));
	IGNORE_RETURN(getInteriorLayoutFileName(true));
	SharedTangibleObjectTemplate::testValues();
}	// SharedShipObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedShipObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedShipObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,4))
	{
		if (DataLint::isEnabled())
			DEBUG_WARNING(true, ("template %s version out of date", file.getFileName()));
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
		if (strcmp(paramName, "cockpitFilename") == 0)
			m_cockpitFilename.loadFromIff(file);
		else if (strcmp(paramName, "hasWings") == 0)
			m_hasWings.loadFromIff(file);
		else if (strcmp(paramName, "playerControlled") == 0)
			m_playerControlled.loadFromIff(file);
		else if (strcmp(paramName, "interiorLayoutFileName") == 0)
			m_interiorLayoutFileName.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedShipObjectTemplate::load

//@END TFD
