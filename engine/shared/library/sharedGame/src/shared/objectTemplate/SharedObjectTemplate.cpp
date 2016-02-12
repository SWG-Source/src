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

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SharedObjectTemplate.h"

#include "sharedFile/Iff.h"
#include "sharedGame/SharedObjectTemplateClientData.h"
#include "sharedMath/Vector.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/ArrangementDescriptorList.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedObject/PortalPropertyTemplateList.h"
#include "sharedObject/SlotDescriptor.h"
#include "sharedObject/SlotDescriptorList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <algorithm>
#include <cstdio>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool SharedObjectTemplate::ms_allowDefaultTemplateParams = true;

// ======================================================================
// SharedObjectTemplate::PreloadManager
// ======================================================================

class SharedObjectTemplate::PreloadManager
{
public:

	explicit PreloadManager (const SharedObjectTemplate* sharedObjectTemplate);
	~PreloadManager ();

private:
	
	PreloadManager ();
	PreloadManager (const PreloadManager&);
	PreloadManager& operator= (const PreloadManager&);

private:

	const AppearanceTemplate* m_preloadAppearanceTemplate;
	const PortalPropertyTemplate* m_preloadPortalPropertyTemplate;
};

// ----------------------------------------------------------------------

SharedObjectTemplate::PreloadManager::PreloadManager (const SharedObjectTemplate* const sharedObjectTemplate) :
m_preloadAppearanceTemplate (0),
m_preloadPortalPropertyTemplate (0)
{
	NOT_NULL(sharedObjectTemplate);
	const std::string& appearanceFileName = sharedObjectTemplate->getAppearanceFilename ();
	if (!appearanceFileName.empty ())
	{
		bool found = false;
		m_preloadAppearanceTemplate = AppearanceTemplateList::fetch (appearanceFileName.c_str (), found);
		WARNING(!found, ("SharedObjectTemplate [%s] unable to load appearance [%s]", sharedObjectTemplate->getName(), appearanceFileName.c_str()));
		m_preloadAppearanceTemplate->preloadAssets ();
	}

	const std::string& portalLayoutFilename = sharedObjectTemplate->getPortalLayoutFilename ();
	if (!portalLayoutFilename.empty ())
	{
		m_preloadPortalPropertyTemplate = PortalPropertyTemplateList::fetch (TemporaryCrcString (portalLayoutFilename.c_str (), true));
		m_preloadPortalPropertyTemplate->preloadAssets ();
	}

	if (sharedObjectTemplate->m_clientData)
		sharedObjectTemplate->m_clientData->preloadAssets ();
}

// ----------------------------------------------------------------------

SharedObjectTemplate::PreloadManager::~PreloadManager ()
{
	if (m_preloadAppearanceTemplate)
		AppearanceTemplateList::release (m_preloadAppearanceTemplate);

	if (m_preloadPortalPropertyTemplate)
		m_preloadPortalPropertyTemplate->release ();
}

// ======================================================================

/**
 * Class constructor.
 */
SharedObjectTemplate::SharedObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
	, m_slotDescriptor(nullptr)
	, m_arrangementDescriptor(nullptr)
	, m_clientData (0)
	, m_preloadManager (0)	
{
}	// SharedObjectTemplate::SharedObjectTemplate

/**
 * Class destructor.
 */
SharedObjectTemplate::~SharedObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP

	//-- Release ArrangementDescriptor resource.
	if (m_arrangementDescriptor)
	{
		m_arrangementDescriptor->release();
		m_arrangementDescriptor = 0;
	}

	//-- Release SlotDescriptor resource.
	if (m_slotDescriptor)
	{
		m_slotDescriptor->release();
		m_slotDescriptor = 0;
	}

	//-- delete client data
	if (m_clientData)
		m_clientData->releaseReference ();

	//-- delete preloadmanager
	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}	// SharedObjectTemplate::~SharedObjectTemplate

// ----------------------------------------------------------------------

SharedObjectTemplate * SharedObjectTemplate::asSharedObjectTemplate()
{
	return this;
}

// ----------------------------------------------------------------------

SharedObjectTemplate const * SharedObjectTemplate::asSharedObjectTemplate() const
{
	return this;
}

// ----------------------------------------------------------------------

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


void SharedObjectTemplate::preloadAssets () const
{
	ObjectTemplate::preloadAssets ();

	if (!m_preloadManager)
		m_preloadManager = new PreloadManager (this);
}

void SharedObjectTemplate::garbageCollect () const
{
	ObjectTemplate::garbageCollect ();

	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}


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
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedObjectTemplate * base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedObjectTemplate::getHighestTemplateVersion

/**
 * Handle any post-load processing for the instance.
 */
void SharedObjectTemplate::postLoad(void)
{
	//-- fetch the SlotDescriptor if specified
	const std::string &slotFilename = getSlotDescriptorFilename();
	if (!slotFilename.empty())
		m_slotDescriptor = SlotDescriptorList::fetch(slotFilename);

	//-- fetch the ArrangementDescriptor if specified
	const std::string &arrangementFilename = getArrangementDescriptorFilename();
	if (!arrangementFilename.empty())
		m_arrangementDescriptor = ArrangementDescriptorList::fetch(arrangementFilename);

	//-- load the client data file
	if (ms_createClientDataFunction)
	{
		const std::string& clientDataFile = getClientDataFile ();

		if (!clientDataFile.empty ())
			m_clientData = ms_createClientDataFunction (clientDataFile.c_str ());
	}
}	// SharedObjectTemplate::postLoad

//@BEGIN TFD
const StringId SharedObjectTemplate::getObjectName(bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getObjectName(true);
#endif
	}

	if (!m_objectName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter objectName in template %s", DataResource::getName()));
			return DefaultStringId;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter objectName has not been defined in template %s!", DataResource::getName()));
			return base->getObjectName();
		}
	}

	const StringId value = m_objectName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getObjectName

const StringId SharedObjectTemplate::getDetailedDescription(bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDetailedDescription(true);
#endif
	}

	if (!m_detailedDescription.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter detailedDescription in template %s", DataResource::getName()));
			return DefaultStringId;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter detailedDescription has not been defined in template %s!", DataResource::getName()));
			return base->getDetailedDescription();
		}
	}

	const StringId value = m_detailedDescription.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getDetailedDescription

const StringId SharedObjectTemplate::getLookAtText(bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getLookAtText(true);
#endif
	}

	if (!m_lookAtText.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter lookAtText in template %s", DataResource::getName()));
			return DefaultStringId;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter lookAtText has not been defined in template %s!", DataResource::getName()));
			return base->getLookAtText();
		}
	}

	const StringId value = m_lookAtText.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getLookAtText

bool SharedObjectTemplate::getSnapToTerrain(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSnapToTerrain(true);
#endif
	}

	if (!m_snapToTerrain.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter snapToTerrain in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter snapToTerrain has not been defined in template %s!", DataResource::getName()));
			return base->getSnapToTerrain();
		}
	}

	bool value = m_snapToTerrain.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getSnapToTerrain

SharedObjectTemplate::ContainerType SharedObjectTemplate::getContainerType(bool testData) const
{
#ifdef _DEBUG
SharedObjectTemplate::ContainerType testDataValue = static_cast<SharedObjectTemplate::ContainerType>(0);
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getContainerType(true);
#endif
	}

	if (!m_containerType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter containerType in template %s", DataResource::getName()));
			return static_cast<ContainerType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter containerType has not been defined in template %s!", DataResource::getName()));
			return base->getContainerType();
		}
	}

	ContainerType value = static_cast<ContainerType>(m_containerType.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getContainerType

int SharedObjectTemplate::getContainerVolumeLimit(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getContainerVolumeLimit(true);
#endif
	}

	if (!m_containerVolumeLimit.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter containerVolumeLimit in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter containerVolumeLimit has not been defined in template %s!", DataResource::getName()));
			return base->getContainerVolumeLimit();
		}
	}

	int value = m_containerVolumeLimit.getValue();
	char delta = m_containerVolumeLimit.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getContainerVolumeLimit();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getContainerVolumeLimit

int SharedObjectTemplate::getContainerVolumeLimitMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getContainerVolumeLimitMin(true);
#endif
	}

	if (!m_containerVolumeLimit.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter containerVolumeLimit in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter containerVolumeLimit has not been defined in template %s!", DataResource::getName()));
			return base->getContainerVolumeLimitMin();
		}
	}

	int value = m_containerVolumeLimit.getMinValue();
	char delta = m_containerVolumeLimit.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getContainerVolumeLimitMin();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getContainerVolumeLimitMin

int SharedObjectTemplate::getContainerVolumeLimitMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getContainerVolumeLimitMax(true);
#endif
	}

	if (!m_containerVolumeLimit.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter containerVolumeLimit in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter containerVolumeLimit has not been defined in template %s!", DataResource::getName()));
			return base->getContainerVolumeLimitMax();
		}
	}

	int value = m_containerVolumeLimit.getMaxValue();
	char delta = m_containerVolumeLimit.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getContainerVolumeLimitMax();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getContainerVolumeLimitMax

const std::string & SharedObjectTemplate::getTintPalette(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTintPalette(true);
#endif
	}

	if (!m_tintPalette.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter tintPalette in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter tintPalette has not been defined in template %s!", DataResource::getName()));
			return base->getTintPalette();
		}
	}

	const std::string & value = m_tintPalette.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getTintPalette

const std::string & SharedObjectTemplate::getSlotDescriptorFilename(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSlotDescriptorFilename(true);
#endif
	}

	if (!m_slotDescriptorFilename.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slotDescriptorFilename in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slotDescriptorFilename has not been defined in template %s!", DataResource::getName()));
			return base->getSlotDescriptorFilename();
		}
	}

	const std::string & value = m_slotDescriptorFilename.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getSlotDescriptorFilename

const std::string & SharedObjectTemplate::getArrangementDescriptorFilename(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getArrangementDescriptorFilename(true);
#endif
	}

	if (!m_arrangementDescriptorFilename.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter arrangementDescriptorFilename in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter arrangementDescriptorFilename has not been defined in template %s!", DataResource::getName()));
			return base->getArrangementDescriptorFilename();
		}
	}

	const std::string & value = m_arrangementDescriptorFilename.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getArrangementDescriptorFilename

const std::string & SharedObjectTemplate::getAppearanceFilename(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getAppearanceFilename(true);
#endif
	}

	if (!m_appearanceFilename.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter appearanceFilename in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter appearanceFilename has not been defined in template %s!", DataResource::getName()));
			return base->getAppearanceFilename();
		}
	}

	const std::string & value = m_appearanceFilename.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getAppearanceFilename

const std::string & SharedObjectTemplate::getPortalLayoutFilename(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPortalLayoutFilename(true);
#endif
	}

	if (!m_portalLayoutFilename.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter portalLayoutFilename in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter portalLayoutFilename has not been defined in template %s!", DataResource::getName()));
			return base->getPortalLayoutFilename();
		}
	}

	const std::string & value = m_portalLayoutFilename.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getPortalLayoutFilename

const std::string & SharedObjectTemplate::getClientDataFile(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getClientDataFile(true);
#endif
	}

	if (!m_clientDataFile.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter clientDataFile in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter clientDataFile has not been defined in template %s!", DataResource::getName()));
			return base->getClientDataFile();
		}
	}

	const std::string & value = m_clientDataFile.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getClientDataFile

float SharedObjectTemplate::getScale(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getScale(true);
#endif
	}

	if (!m_scale.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter scale in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter scale has not been defined in template %s!", DataResource::getName()));
			return base->getScale();
		}
	}

	float value = m_scale.getValue();
	char delta = m_scale.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getScale();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getScale

float SharedObjectTemplate::getScaleMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getScaleMin(true);
#endif
	}

	if (!m_scale.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter scale in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter scale has not been defined in template %s!", DataResource::getName()));
			return base->getScaleMin();
		}
	}

	float value = m_scale.getMinValue();
	char delta = m_scale.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getScaleMin();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getScaleMin

float SharedObjectTemplate::getScaleMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getScaleMax(true);
#endif
	}

	if (!m_scale.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter scale in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter scale has not been defined in template %s!", DataResource::getName()));
			return base->getScaleMax();
		}
	}

	float value = m_scale.getMaxValue();
	char delta = m_scale.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getScaleMax();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getScaleMax

SharedObjectTemplate::GameObjectType SharedObjectTemplate::getGameObjectType(bool testData) const
{
#ifdef _DEBUG
SharedObjectTemplate::GameObjectType testDataValue = static_cast<SharedObjectTemplate::GameObjectType>(0);
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getGameObjectType(true);
#endif
	}

	if (!m_gameObjectType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter gameObjectType in template %s", DataResource::getName()));
			return static_cast<GameObjectType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter gameObjectType has not been defined in template %s!", DataResource::getName()));
			return base->getGameObjectType();
		}
	}

	GameObjectType value = static_cast<GameObjectType>(m_gameObjectType.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getGameObjectType

bool SharedObjectTemplate::getSendToClient(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSendToClient(true);
#endif
	}

	if (!m_sendToClient.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter sendToClient in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter sendToClient has not been defined in template %s!", DataResource::getName()));
			return base->getSendToClient();
		}
	}

	bool value = m_sendToClient.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getSendToClient

float SharedObjectTemplate::getScaleThresholdBeforeExtentTest(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getScaleThresholdBeforeExtentTest(true);
#endif
	}

	if (!m_scaleThresholdBeforeExtentTest.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter scaleThresholdBeforeExtentTest in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter scaleThresholdBeforeExtentTest has not been defined in template %s!", DataResource::getName()));
			return base->getScaleThresholdBeforeExtentTest();
		}
	}

	float value = m_scaleThresholdBeforeExtentTest.getValue();
	char delta = m_scaleThresholdBeforeExtentTest.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getScaleThresholdBeforeExtentTest();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getScaleThresholdBeforeExtentTest

float SharedObjectTemplate::getScaleThresholdBeforeExtentTestMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getScaleThresholdBeforeExtentTestMin(true);
#endif
	}

	if (!m_scaleThresholdBeforeExtentTest.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter scaleThresholdBeforeExtentTest in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter scaleThresholdBeforeExtentTest has not been defined in template %s!", DataResource::getName()));
			return base->getScaleThresholdBeforeExtentTestMin();
		}
	}

	float value = m_scaleThresholdBeforeExtentTest.getMinValue();
	char delta = m_scaleThresholdBeforeExtentTest.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getScaleThresholdBeforeExtentTestMin();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getScaleThresholdBeforeExtentTestMin

float SharedObjectTemplate::getScaleThresholdBeforeExtentTestMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getScaleThresholdBeforeExtentTestMax(true);
#endif
	}

	if (!m_scaleThresholdBeforeExtentTest.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter scaleThresholdBeforeExtentTest in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter scaleThresholdBeforeExtentTest has not been defined in template %s!", DataResource::getName()));
			return base->getScaleThresholdBeforeExtentTestMax();
		}
	}

	float value = m_scaleThresholdBeforeExtentTest.getMaxValue();
	char delta = m_scaleThresholdBeforeExtentTest.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getScaleThresholdBeforeExtentTestMax();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getScaleThresholdBeforeExtentTestMax

float SharedObjectTemplate::getClearFloraRadius(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getClearFloraRadius(true);
#endif
	}

	if (!m_clearFloraRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter clearFloraRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter clearFloraRadius has not been defined in template %s!", DataResource::getName()));
			return base->getClearFloraRadius();
		}
	}

	float value = m_clearFloraRadius.getValue();
	char delta = m_clearFloraRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getClearFloraRadius();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getClearFloraRadius

float SharedObjectTemplate::getClearFloraRadiusMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getClearFloraRadiusMin(true);
#endif
	}

	if (!m_clearFloraRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter clearFloraRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter clearFloraRadius has not been defined in template %s!", DataResource::getName()));
			return base->getClearFloraRadiusMin();
		}
	}

	float value = m_clearFloraRadius.getMinValue();
	char delta = m_clearFloraRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getClearFloraRadiusMin();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getClearFloraRadiusMin

float SharedObjectTemplate::getClearFloraRadiusMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getClearFloraRadiusMax(true);
#endif
	}

	if (!m_clearFloraRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter clearFloraRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter clearFloraRadius has not been defined in template %s!", DataResource::getName()));
			return base->getClearFloraRadiusMax();
		}
	}

	float value = m_clearFloraRadius.getMaxValue();
	char delta = m_clearFloraRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getClearFloraRadiusMax();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getClearFloraRadiusMax

SharedObjectTemplate::SurfaceType SharedObjectTemplate::getSurfaceType(bool testData) const
{
#ifdef _DEBUG
SharedObjectTemplate::SurfaceType testDataValue = static_cast<SharedObjectTemplate::SurfaceType>(0);
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSurfaceType(true);
#endif
	}

	if (!m_surfaceType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter surfaceType in template %s", DataResource::getName()));
			return static_cast<SurfaceType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter surfaceType has not been defined in template %s!", DataResource::getName()));
			return base->getSurfaceType();
		}
	}

	SurfaceType value = static_cast<SurfaceType>(m_surfaceType.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getSurfaceType

float SharedObjectTemplate::getNoBuildRadius(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getNoBuildRadius(true);
#endif
	}

	if (!m_noBuildRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter noBuildRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter noBuildRadius has not been defined in template %s!", DataResource::getName()));
			return base->getNoBuildRadius();
		}
	}

	float value = m_noBuildRadius.getValue();
	char delta = m_noBuildRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getNoBuildRadius();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getNoBuildRadius

float SharedObjectTemplate::getNoBuildRadiusMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getNoBuildRadiusMin(true);
#endif
	}

	if (!m_noBuildRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter noBuildRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter noBuildRadius has not been defined in template %s!", DataResource::getName()));
			return base->getNoBuildRadiusMin();
		}
	}

	float value = m_noBuildRadius.getMinValue();
	char delta = m_noBuildRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getNoBuildRadiusMin();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getNoBuildRadiusMin

float SharedObjectTemplate::getNoBuildRadiusMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getNoBuildRadiusMax(true);
#endif
	}

	if (!m_noBuildRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter noBuildRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter noBuildRadius has not been defined in template %s!", DataResource::getName()));
			return base->getNoBuildRadiusMax();
		}
	}

	float value = m_noBuildRadius.getMaxValue();
	char delta = m_noBuildRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getNoBuildRadiusMax();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getNoBuildRadiusMax

bool SharedObjectTemplate::getOnlyVisibleInTools(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getOnlyVisibleInTools(true);
#endif
	}

	if (!m_onlyVisibleInTools.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter onlyVisibleInTools in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter onlyVisibleInTools has not been defined in template %s!", DataResource::getName()));
			return base->getOnlyVisibleInTools();
		}
	}

	bool value = m_onlyVisibleInTools.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getOnlyVisibleInTools

float SharedObjectTemplate::getLocationReservationRadius(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getLocationReservationRadius(true);
#endif
	}

	if (!m_locationReservationRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter locationReservationRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter locationReservationRadius has not been defined in template %s!", DataResource::getName()));
			return base->getLocationReservationRadius();
		}
	}

	float value = m_locationReservationRadius.getValue();
	char delta = m_locationReservationRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getLocationReservationRadius();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getLocationReservationRadius

float SharedObjectTemplate::getLocationReservationRadiusMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getLocationReservationRadiusMin(true);
#endif
	}

	if (!m_locationReservationRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter locationReservationRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter locationReservationRadius has not been defined in template %s!", DataResource::getName()));
			return base->getLocationReservationRadiusMin();
		}
	}

	float value = m_locationReservationRadius.getMinValue();
	char delta = m_locationReservationRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getLocationReservationRadiusMin();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getLocationReservationRadiusMin

float SharedObjectTemplate::getLocationReservationRadiusMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getLocationReservationRadiusMax(true);
#endif
	}

	if (!m_locationReservationRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter locationReservationRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter locationReservationRadius has not been defined in template %s!", DataResource::getName()));
			return base->getLocationReservationRadiusMax();
		}
	}

	float value = m_locationReservationRadius.getMaxValue();
	char delta = m_locationReservationRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getLocationReservationRadiusMax();
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
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getLocationReservationRadiusMax

bool SharedObjectTemplate::getForceNoCollision(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const SharedObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getForceNoCollision(true);
#endif
	}

	if (!m_forceNoCollision.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter forceNoCollision in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter forceNoCollision has not been defined in template %s!", DataResource::getName()));
			return base->getForceNoCollision();
		}
	}

	bool value = m_forceNoCollision.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedObjectTemplate::getForceNoCollision

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getObjectName(true));
	IGNORE_RETURN(getDetailedDescription(true));
	IGNORE_RETURN(getLookAtText(true));
	IGNORE_RETURN(getSnapToTerrain(true));
	IGNORE_RETURN(getContainerType(true));
	IGNORE_RETURN(getContainerVolumeLimitMin(true));
	IGNORE_RETURN(getContainerVolumeLimitMax(true));
	IGNORE_RETURN(getTintPalette(true));
	IGNORE_RETURN(getSlotDescriptorFilename(true));
	IGNORE_RETURN(getArrangementDescriptorFilename(true));
	IGNORE_RETURN(getAppearanceFilename(true));
	IGNORE_RETURN(getPortalLayoutFilename(true));
	IGNORE_RETURN(getClientDataFile(true));
	IGNORE_RETURN(getScaleMin(true));
	IGNORE_RETURN(getScaleMax(true));
	IGNORE_RETURN(getGameObjectType(true));
	IGNORE_RETURN(getSendToClient(true));
	IGNORE_RETURN(getScaleThresholdBeforeExtentTestMin(true));
	IGNORE_RETURN(getScaleThresholdBeforeExtentTestMax(true));
	IGNORE_RETURN(getClearFloraRadiusMin(true));
	IGNORE_RETURN(getClearFloraRadiusMax(true));
	IGNORE_RETURN(getSurfaceType(true));
	IGNORE_RETURN(getNoBuildRadiusMin(true));
	IGNORE_RETURN(getNoBuildRadiusMax(true));
	IGNORE_RETURN(getOnlyVisibleInTools(true));
	IGNORE_RETURN(getLocationReservationRadiusMin(true));
	IGNORE_RETURN(getLocationReservationRadiusMax(true));
	IGNORE_RETURN(getForceNoCollision(true));
}	// SharedObjectTemplate::testValues
#endif

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
	return;
}	// SharedObjectTemplate::load

//@END TFD

//===================================================================
// PUBLIC STATIC SharedObjectTemplate
//===================================================================

void SharedObjectTemplate::setCreateClientDataFunction (SharedObjectTemplate::CreateClientDataFunction createClientDataFunction)
{
	ms_createClientDataFunction = createClientDataFunction;
}

//===================================================================
// PUBLIC SharedObjectTemplate
//===================================================================

const SharedObjectTemplateClientData* SharedObjectTemplate::getClientData () const
{
	return m_clientData;
}

//===================================================================
// PRIVATE STATIC SharedObjectTemplate
//===================================================================

SharedObjectTemplate::CreateClientDataFunction SharedObjectTemplate::ms_createClientDataFunction;

//===================================================================
