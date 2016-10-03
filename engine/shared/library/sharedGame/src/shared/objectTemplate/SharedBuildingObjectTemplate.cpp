//========================================================================
//
// SharedBuildingObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedBuildingObjectTemplate.h"
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

bool SharedBuildingObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedBuildingObjectTemplate::SharedBuildingObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedBuildingObjectTemplate::SharedBuildingObjectTemplate

/**
 * Class destructor.
 */
SharedBuildingObjectTemplate::~SharedBuildingObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedBuildingObjectTemplate::~SharedBuildingObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedBuildingObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedBuildingObjectTemplate_tag, create);
}	// SharedBuildingObjectTemplate::registerMe

/**
 * Creates a SharedBuildingObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedBuildingObjectTemplate::create(const std::string & filename)
{
	return new SharedBuildingObjectTemplate(filename);
}	// SharedBuildingObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedBuildingObjectTemplate::getId(void) const
{
	return SharedBuildingObjectTemplate_tag;
}	// SharedBuildingObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedBuildingObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedBuildingObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedBuildingObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedBuildingObjectTemplate * base = dynamic_cast<const SharedBuildingObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedBuildingObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
const std::string & SharedBuildingObjectTemplate::getTerrainModificationFileName(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedBuildingObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedBuildingObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTerrainModificationFileName(true);
#endif
	}

	if (!m_terrainModificationFileName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter terrainModificationFileName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter terrainModificationFileName has not been defined in template %s!", DataResource::getName()));
			return base->getTerrainModificationFileName();
		}
	}

	const std::string & value = m_terrainModificationFileName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedBuildingObjectTemplate::getTerrainModificationFileName

const std::string & SharedBuildingObjectTemplate::getInteriorLayoutFileName(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedBuildingObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedBuildingObjectTemplate *>(m_baseData);
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
}	// SharedBuildingObjectTemplate::getInteriorLayoutFileName

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedBuildingObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getTerrainModificationFileName(true));
	IGNORE_RETURN(getInteriorLayoutFileName(true));
	SharedTangibleObjectTemplate::testValues();
}	// SharedBuildingObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedBuildingObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedBuildingObjectTemplate_tag)
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
		if (strcmp(paramName, "terrainModificationFileName") == 0)
			m_terrainModificationFileName.loadFromIff(file);
		else if (strcmp(paramName, "interiorLayoutFileName") == 0)
			m_interiorLayoutFileName.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedBuildingObjectTemplate::load

//@END TFD
