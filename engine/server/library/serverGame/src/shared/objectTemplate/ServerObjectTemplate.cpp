//========================================================================
//
// ServerObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerObjectTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Vector.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
#include "ServerObjectTemplate.h"
//@END TFD TEMPLATE REFS
#include <stdio.h>

#include <unordered_map>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerObjectTemplate::ms_allowDefaultTemplateParams = true;

typedef std::unordered_map<ServerObjectTemplate::XpTypes, std::string, std::hash<int> > XP_MAP; 
static XP_MAP * XpMap = NULL;


/**
 * Class constructor.
 */
ServerObjectTemplate::ServerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ObjectTemplate(filename)
	,m_scriptsLoaded(false)
	,m_scriptsAppend(false)
	,m_visibleFlagsLoaded(false)
	,m_visibleFlagsAppend(false)
	,m_deleteFlagsLoaded(false)
	,m_deleteFlagsAppend(false)
	,m_moveFlagsLoaded(false)
	,m_moveFlagsAppend(false)
	,m_contentsLoaded(false)
	,m_contentsAppend(false)
	,m_xpPointsLoaded(false)
	,m_xpPointsAppend(false)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerObjectTemplate::ServerObjectTemplate

/**
 * Class destructor.
 */
ServerObjectTemplate::~ServerObjectTemplate()
{
	if (m_baseData)
	{
		m_baseData->releaseReference();
		m_baseData=0;
	}
//@BEGIN TFD CLEANUP
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_scripts.begin(); iter != m_scripts.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_scripts.clear();
	}
	{
		std::vector<IntegerParam *>::iterator iter;
		for (iter = m_visibleFlags.begin(); iter != m_visibleFlags.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_visibleFlags.clear();
	}
	{
		std::vector<IntegerParam *>::iterator iter;
		for (iter = m_deleteFlags.begin(); iter != m_deleteFlags.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_deleteFlags.clear();
	}
	{
		std::vector<IntegerParam *>::iterator iter;
		for (iter = m_moveFlags.begin(); iter != m_moveFlags.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_moveFlags.clear();
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_contents.begin(); iter != m_contents.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_contents.clear();
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_xpPoints.begin(); iter != m_xpPoints.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_xpPoints.clear();
	}
//@END TFD CLEANUP
}	// ServerObjectTemplate::~ServerObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerObjectTemplate_tag, create);

	if (XpMap == NULL)
	{
		XpMap = new XP_MAP();
		NOT_NULL(XpMap);
		ExitChain::add(exit, "ServerObjectTemplate");

		// @todo: fill in the rest of this map
		(*XpMap)[XP_rangedCombat] = "";
		(*XpMap)[XP_meleeCombat] = "";
		(*XpMap)[XP_tamingNoTime] = "";
		(*XpMap)[XP_tamingShortTime] = "";
		(*XpMap)[XP_tamingMediumTime] = "";
		(*XpMap)[XP_tamingLargeTime] = "";
		(*XpMap)[XP_architect] = "";
		(*XpMap)[XP_botany] = "";
		(*XpMap)[XP_bountyHunter] = "";
		(*XpMap)[XP_chemist] = "";
		(*XpMap)[XP_dancing] = "";
		(*XpMap)[XP_facilityManagementSavings] = "";
		(*XpMap)[XP_facilityManagementWearAndTear] = "";
		(*XpMap)[XP_squadLeaderNoTime] = "";
		(*XpMap)[XP_squadLeaderSmallTime] = "";
		(*XpMap)[XP_squadLeaderMediumTime] = "";
		(*XpMap)[XP_squadLeaderLargeTime] = "";
		(*XpMap)[XP_hairdressing] = "";
		(*XpMap)[XP_holonetSlicingPopularity] = "";
		(*XpMap)[XP_holonetSlicingQuantity] = "";
		(*XpMap)[XP_investigation] = "";
		(*XpMap)[XP_medic] = "";
		(*XpMap)[XP_military] = "";
		(*XpMap)[XP_mining] = "";
		(*XpMap)[XP_miningSavings] = "";
		(*XpMap)[XP_musician] = "music";
		(*XpMap)[XP_musicanPopularity] = "";
		(*XpMap)[XP_politics] = "";
		(*XpMap)[XP_slicing] = "";
		(*XpMap)[XP_smuggling] = "";
		(*XpMap)[XP_surveyingNoMinimum] = "";
		(*XpMap)[XP_surveyingLowMinimum] = "";
		(*XpMap)[XP_surveyingMediumMinimum] = "";
		(*XpMap)[XP_surveyingHighMinimum] = "";
		(*XpMap)[XP_survivalHealing] = "";
		(*XpMap)[XP_survivalTracking] = "";
		(*XpMap)[XP_vehicularControl] = "";
		(*XpMap)[XP_vehicularWearAndTear] = "";
		(*XpMap)[XP_resourceHarvestingOrganic] = "resource_harvesting_organic";
		(*XpMap)[XP_resourceHarvestingInorganic] = "resource_harvesting_inorganic";
		(*XpMap)[XP_crafting] = "crafting_general";
		(*XpMap)[XP_craftingClothing] = "crafting_clothing_general";
		(*XpMap)[XP_craftingClothingArmor] = "crafting_clothing_armor";
		(*XpMap)[XP_craftingDroid] = "crafting_droid_general";
		(*XpMap)[XP_craftingFood] = "crafting_food_general";
		(*XpMap)[XP_craftingMedicine] = "crafting_medicine_general";
		(*XpMap)[XP_craftingStructure] = "crafting_structure_general";
		(*XpMap)[XP_craftingWeapons] = "crafting_weapons_general";
		(*XpMap)[XP_craftingWeaponsMelee] = "crafting_weapons_melee";
		(*XpMap)[XP_craftingWeaponsRanged] = "crafting_weapons_ranged";
		(*XpMap)[XP_craftingWeaponsMunition] = "crafting_weapons_munition";
		(*XpMap)[XP_craftingScout] = "scout";
		(*XpMap)[XP_craftingCreature] = "crafting_bio_engineer_creature";
		(*XpMap)[XP_craftingTissue] = "crafting_bio_engineer_tissue";
		(*XpMap)[XP_craftingCamp] = "camp";
		(*XpMap)[XP_craftingTrapping] = "trapping";
		(*XpMap)[XP_craftingSpice] = "crafting_spice";
		(*XpMap)[XP_industrialist] = "industrialist";
		(*XpMap)[XP_jediGeneral] = "jedi_general";
		(*XpMap)[XP_shipwright] = "shipwright";
		(*XpMap)[XP_reverseEngineering] = "reverse_engineering";
	}
}	// ServerObjectTemplate::registerMe

/**
 * Called on program exit to clean up class-level memory.
 */
void ServerObjectTemplate::exit()
{
	if (XpMap != NULL)
	{
		delete XpMap;
		XpMap = NULL;
	}
}	// ServerObjectTemplate::exit

/**
 * Creates a ServerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::create(const std::string & filename)
{
	return new ServerObjectTemplate(filename);
}	// ServerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::getId(void) const
{
	return ServerObjectTemplate_tag;
}	// ServerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerObjectTemplate::getHighestTemplateVersion

// ----------------------------------------------------------------------

ServerObjectTemplate * ServerObjectTemplate::asServerObjectTemplate()
{
	return this;
}

// ----------------------------------------------------------------------

ServerObjectTemplate const * ServerObjectTemplate::asServerObjectTemplate() const
{
	return this;
}

// ----------------------------------------------------------------------

/**
 * Returns the xp string associated with an xp enum value
 *
 * @param type		the xp type
 *
 * @return the string associated with the xp, or an empty string for a bad value
 */
const std::string & ServerObjectTemplate::getXpString(XpTypes type)
{
	static const std::string emptyString;

	if (XpMap != NULL)
	{
		XP_MAP::const_iterator result = XpMap->find(type);
		if (result != XpMap->end())
			return (*result).second;
	}

	return emptyString;
}	// ServerObjectTemplate::getXpString

//----------------------------------------------------------------------

const std::string & ServerObjectTemplate::getDamageTypeString  (DamageType type)
{
	static const std::string emptyString;

	static const std::string DAMAGE_TYPE_NAMES[] = 
	{
		"kinetic",
		"energy",
		"blast",
		"stun",
		"restraint",
		"elemental_heat",
		"elemental_cold",
		"elemental_acid",
		"elemental_electrical",
		"environmental_heat",
		"environmental_cold",
		"environmental_acid",
		"environmental_electrical"
	};

	const int DAMAGE_TYPE_NAMES_SIZE = sizeof(DAMAGE_TYPE_NAMES) / sizeof(DAMAGE_TYPE_NAMES[0]);

	const int i_damage = static_cast<int>(type);

	if (i_damage != 0)
	{
		for (int index = 0; index < DAMAGE_TYPE_NAMES_SIZE; ++index)
		{
			if ((i_damage & (1 << index)) != 0)
				return DAMAGE_TYPE_NAMES [index];
		}
	}

	return emptyString;
}

//----------------------------------------------------------------------

const std::string & ServerObjectTemplate::getArmorRatingString (ArmorRating type)
{
	static const std::string emptyString;

	static const std::string ARMOR_RATING_NAMES[] = 
	{
		"none",
		"light",
		"medium",
		"heavy"
	};

	const int ARMOR_RATING_NAMES_SIZE = sizeof(ARMOR_RATING_NAMES) / sizeof(ARMOR_RATING_NAMES[0]);

	const int i_type = static_cast<int>(type);

	if (i_type < 0 || i_type >= ARMOR_RATING_NAMES_SIZE)
		return emptyString;

	return ARMOR_RATING_NAMES [i_type];
}

//----------------------------------------------------------------------

//@BEGIN TFD
const std::string & ServerObjectTemplate::getSharedTemplate(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSharedTemplate(true);
#endif
	}

	if (!m_sharedTemplate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter sharedTemplate in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter sharedTemplate has not been defined in template %s!", DataResource::getName()));
			return base->getSharedTemplate();
		}
	}

	const std::string & value = m_sharedTemplate.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::getSharedTemplate

const std::string & ServerObjectTemplate::getScripts(int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_scriptsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter scripts in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter scripts has not been defined in template %s!", DataResource::getName()));
			return base->getScripts(index);
		}
	}

	if (m_scriptsAppend && base != nullptr)
	{
		int baseCount = base->getScriptsCount();
		if (index < baseCount)
			return base->getScripts(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_scripts.size(), ("template param index out of range"));
	const std::string & value = m_scripts[index]->getValue();
	return value;
}	// ServerObjectTemplate::getScripts

size_t ServerObjectTemplate::getScriptsCount(void) const
{
	if (!m_scriptsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getScriptsCount();
	}

	size_t count = m_scripts.size();

	// if we are extending our base template, add it's count
	if (m_scriptsAppend && m_baseData != nullptr)
	{
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getScriptsCount();
	}

	return count;
}	// ServerObjectTemplate::getScriptsCount

void ServerObjectTemplate::getObjvars(DynamicVariableList &list) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_objvars.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter objvars in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter objvars has not been defined in template %s!", DataResource::getName()));
			base->getObjvars(list);
			return;
		}
	}

	if (m_objvars.isExtendingBaseList() && base != nullptr)
		base->getObjvars(list);
	m_objvars.getDynamicVariableList(list);
}	// ServerObjectTemplate::getObjvars

int ServerObjectTemplate::getVolume(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVolume(true);
#endif
	}

	if (!m_volume.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter volume in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter volume has not been defined in template %s!", DataResource::getName()));
			return base->getVolume();
		}
	}

	int value = m_volume.getValue();
	char delta = m_volume.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getVolume();
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
}	// ServerObjectTemplate::getVolume

int ServerObjectTemplate::getVolumeMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVolumeMin(true);
#endif
	}

	if (!m_volume.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter volume in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter volume has not been defined in template %s!", DataResource::getName()));
			return base->getVolumeMin();
		}
	}

	int value = m_volume.getMinValue();
	char delta = m_volume.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getVolumeMin();
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
}	// ServerObjectTemplate::getVolumeMin

int ServerObjectTemplate::getVolumeMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVolumeMax(true);
#endif
	}

	if (!m_volume.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter volume in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter volume has not been defined in template %s!", DataResource::getName()));
			return base->getVolumeMax();
		}
	}

	int value = m_volume.getMaxValue();
	char delta = m_volume.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getVolumeMax();
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
}	// ServerObjectTemplate::getVolumeMax

ServerObjectTemplate::VisibleFlags ServerObjectTemplate::getVisibleFlags(int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_visibleFlagsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter visibleFlags in template %s", DataResource::getName()));
			return static_cast<VisibleFlags>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter visibleFlags has not been defined in template %s!", DataResource::getName()));
			return base->getVisibleFlags(index);
		}
	}

	if (m_visibleFlagsAppend && base != nullptr)
	{
		int baseCount = base->getVisibleFlagsCount();
		if (index < baseCount)
			return base->getVisibleFlags(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_visibleFlags.size(), ("template param index out of range"));
	return static_cast<VisibleFlags>(m_visibleFlags[index]->getValue());
}	// ServerObjectTemplate::getVisibleFlags

size_t ServerObjectTemplate::getVisibleFlagsCount(void) const
{
	if (!m_visibleFlagsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getVisibleFlagsCount();
	}

	size_t count = m_visibleFlags.size();

	// if we are extending our base template, add it's count
	if (m_visibleFlagsAppend && m_baseData != nullptr)
	{
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getVisibleFlagsCount();
	}

	return count;
}	// ServerObjectTemplate::getVisibleFlagsCount

ServerObjectTemplate::DeleteFlags ServerObjectTemplate::getDeleteFlags(int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_deleteFlagsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter deleteFlags in template %s", DataResource::getName()));
			return static_cast<DeleteFlags>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter deleteFlags has not been defined in template %s!", DataResource::getName()));
			return base->getDeleteFlags(index);
		}
	}

	if (m_deleteFlagsAppend && base != nullptr)
	{
		int baseCount = base->getDeleteFlagsCount();
		if (index < baseCount)
			return base->getDeleteFlags(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_deleteFlags.size(), ("template param index out of range"));
	return static_cast<DeleteFlags>(m_deleteFlags[index]->getValue());
}	// ServerObjectTemplate::getDeleteFlags

size_t ServerObjectTemplate::getDeleteFlagsCount(void) const
{
	if (!m_deleteFlagsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getDeleteFlagsCount();
	}

	size_t count = m_deleteFlags.size();

	// if we are extending our base template, add it's count
	if (m_deleteFlagsAppend && m_baseData != nullptr)
	{
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getDeleteFlagsCount();
	}

	return count;
}	// ServerObjectTemplate::getDeleteFlagsCount

ServerObjectTemplate::MoveFlags ServerObjectTemplate::getMoveFlags(int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_moveFlagsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter moveFlags in template %s", DataResource::getName()));
			return static_cast<MoveFlags>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter moveFlags has not been defined in template %s!", DataResource::getName()));
			return base->getMoveFlags(index);
		}
	}

	if (m_moveFlagsAppend && base != nullptr)
	{
		int baseCount = base->getMoveFlagsCount();
		if (index < baseCount)
			return base->getMoveFlags(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_moveFlags.size(), ("template param index out of range"));
	return static_cast<MoveFlags>(m_moveFlags[index]->getValue());
}	// ServerObjectTemplate::getMoveFlags

size_t ServerObjectTemplate::getMoveFlagsCount(void) const
{
	if (!m_moveFlagsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getMoveFlagsCount();
	}

	size_t count = m_moveFlags.size();

	// if we are extending our base template, add it's count
	if (m_moveFlagsAppend && m_baseData != nullptr)
	{
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getMoveFlagsCount();
	}

	return count;
}	// ServerObjectTemplate::getMoveFlagsCount

bool ServerObjectTemplate::getInvulnerable(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getInvulnerable(true);
#endif
	}

	if (!m_invulnerable.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter invulnerable in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter invulnerable has not been defined in template %s!", DataResource::getName()));
			return base->getInvulnerable();
		}
	}

	bool value = m_invulnerable.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::getInvulnerable

float ServerObjectTemplate::getComplexity(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexity(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexity();
		}
	}

	float value = m_complexity.getValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexity();
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
}	// ServerObjectTemplate::getComplexity

float ServerObjectTemplate::getComplexityMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexityMin(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexityMin();
		}
	}

	float value = m_complexity.getMinValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexityMin();
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
}	// ServerObjectTemplate::getComplexityMin

float ServerObjectTemplate::getComplexityMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexityMax(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexityMax();
		}
	}

	float value = m_complexity.getMaxValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexityMax();
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
}	// ServerObjectTemplate::getComplexityMax

int ServerObjectTemplate::getTintIndex(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTintIndex(true);
#endif
	}

	if (!m_tintIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter tintIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter tintIndex has not been defined in template %s!", DataResource::getName()));
			return base->getTintIndex();
		}
	}

	int value = m_tintIndex.getValue();
	char delta = m_tintIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTintIndex();
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
}	// ServerObjectTemplate::getTintIndex

int ServerObjectTemplate::getTintIndexMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTintIndexMin(true);
#endif
	}

	if (!m_tintIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter tintIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter tintIndex has not been defined in template %s!", DataResource::getName()));
			return base->getTintIndexMin();
		}
	}

	int value = m_tintIndex.getMinValue();
	char delta = m_tintIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTintIndexMin();
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
}	// ServerObjectTemplate::getTintIndexMin

int ServerObjectTemplate::getTintIndexMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTintIndexMax(true);
#endif
	}

	if (!m_tintIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter tintIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter tintIndex has not been defined in template %s!", DataResource::getName()));
			return base->getTintIndexMax();
		}
	}

	int value = m_tintIndex.getMaxValue();
	char delta = m_tintIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTintIndexMax();
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
}	// ServerObjectTemplate::getTintIndexMax

float ServerObjectTemplate::getUpdateRanges(UpdateRanges index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 3, ("template param index out of range"));
	if (!m_updateRanges[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter updateRanges in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter updateRanges has not been defined in template %s!", DataResource::getName()));
			return base->getUpdateRanges(index);
		}
	}

	float value = m_updateRanges[index].getValue();
	char delta = m_updateRanges[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getUpdateRanges(index);
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
}	// ServerObjectTemplate::getUpdateRanges

float ServerObjectTemplate::getUpdateRangesMin(UpdateRanges index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 3, ("template param index out of range"));
	if (!m_updateRanges[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter updateRanges in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter updateRanges has not been defined in template %s!", DataResource::getName()));
			return base->getUpdateRangesMin(index);
		}
	}

	float value = m_updateRanges[index].getMinValue();
	char delta = m_updateRanges[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getUpdateRangesMin(index);
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
}	// ServerObjectTemplate::getUpdateRangesMin

float ServerObjectTemplate::getUpdateRangesMax(UpdateRanges index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 3, ("template param index out of range"));
	if (!m_updateRanges[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter updateRanges in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter updateRanges has not been defined in template %s!", DataResource::getName()));
			return base->getUpdateRangesMax(index);
		}
	}

	float value = m_updateRanges[index].getMaxValue();
	char delta = m_updateRanges[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getUpdateRangesMax(index);
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
}	// ServerObjectTemplate::getUpdateRangesMax

void ServerObjectTemplate::getContents(Contents &data, int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_contentsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter contents in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter contents has not been defined in template %s!", DataResource::getName()));
			base->getContents(data, index);
			return;
		}
	}

	if (m_contentsAppend && base != nullptr)
	{
		int baseCount = base->getContentsCount();
		if (index < baseCount)
			{
				base->getContents(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_contents.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_contents[index]).getValue();
	NOT_NULL(structTemplate);
	const _Contents *param = dynamic_cast<const _Contents *>(structTemplate);
	NOT_NULL(param);
	data.slotName = param->getSlotName(m_versionOk);
	data.equipObject = param->getEquipObject(m_versionOk);
	data.content = param->getContent(m_versionOk);
}	// ServerObjectTemplate::getContents

void ServerObjectTemplate::getContentsMin(Contents &data, int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_contentsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter contents in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter contents has not been defined in template %s!", DataResource::getName()));
			base->getContentsMin(data, index);
			return;
		}
	}

	if (m_contentsAppend && base != nullptr)
	{
		int baseCount = base->getContentsCount();
		if (index < baseCount)
			{
				base->getContentsMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_contents.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_contents[index]).getValue();
	NOT_NULL(structTemplate);
	const _Contents *param = dynamic_cast<const _Contents *>(structTemplate);
	NOT_NULL(param);
	data.slotName = param->getSlotName(m_versionOk);
	data.equipObject = param->getEquipObject(m_versionOk);
	data.content = param->getContent(m_versionOk);
}	// ServerObjectTemplate::getContentsMin

void ServerObjectTemplate::getContentsMax(Contents &data, int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_contentsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter contents in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter contents has not been defined in template %s!", DataResource::getName()));
			base->getContentsMax(data, index);
			return;
		}
	}

	if (m_contentsAppend && base != nullptr)
	{
		int baseCount = base->getContentsCount();
		if (index < baseCount)
			{
				base->getContentsMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_contents.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_contents[index]).getValue();
	NOT_NULL(structTemplate);
	const _Contents *param = dynamic_cast<const _Contents *>(structTemplate);
	NOT_NULL(param);
	data.slotName = param->getSlotName(m_versionOk);
	data.equipObject = param->getEquipObject(m_versionOk);
	data.content = param->getContent(m_versionOk);
}	// ServerObjectTemplate::getContentsMax

size_t ServerObjectTemplate::getContentsCount(void) const
{
	if (!m_contentsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getContentsCount();
	}

	size_t count = m_contents.size();

	// if we are extending our base template, add it's count
	if (m_contentsAppend && m_baseData != nullptr)
	{
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getContentsCount();
	}

	return count;
}	// ServerObjectTemplate::getContentsCount

void ServerObjectTemplate::getXpPoints(Xp &data, int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_xpPointsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter xpPoints in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter xpPoints has not been defined in template %s!", DataResource::getName()));
			base->getXpPoints(data, index);
			return;
		}
	}

	if (m_xpPointsAppend && base != nullptr)
	{
		int baseCount = base->getXpPointsCount();
		if (index < baseCount)
			{
				base->getXpPoints(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_xpPoints.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_xpPoints[index]).getValue();
	NOT_NULL(structTemplate);
	const _Xp *param = dynamic_cast<const _Xp *>(structTemplate);
	NOT_NULL(param);
	data.type = param->getType(m_versionOk);
	data.level = param->getLevel(m_versionOk);
	data.value = param->getValue(m_versionOk);
}	// ServerObjectTemplate::getXpPoints

void ServerObjectTemplate::getXpPointsMin(Xp &data, int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_xpPointsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter xpPoints in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter xpPoints has not been defined in template %s!", DataResource::getName()));
			base->getXpPointsMin(data, index);
			return;
		}
	}

	if (m_xpPointsAppend && base != nullptr)
	{
		int baseCount = base->getXpPointsCount();
		if (index < baseCount)
			{
				base->getXpPointsMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_xpPoints.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_xpPoints[index]).getValue();
	NOT_NULL(structTemplate);
	const _Xp *param = dynamic_cast<const _Xp *>(structTemplate);
	NOT_NULL(param);
	data.type = param->getType(m_versionOk);
	data.level = param->getLevelMin(m_versionOk);
	data.value = param->getValueMin(m_versionOk);
}	// ServerObjectTemplate::getXpPointsMin

void ServerObjectTemplate::getXpPointsMax(Xp &data, int index) const
{
	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	}

	if (!m_xpPointsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter xpPoints in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter xpPoints has not been defined in template %s!", DataResource::getName()));
			base->getXpPointsMax(data, index);
			return;
		}
	}

	if (m_xpPointsAppend && base != nullptr)
	{
		int baseCount = base->getXpPointsCount();
		if (index < baseCount)
			{
				base->getXpPointsMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_xpPoints.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_xpPoints[index]).getValue();
	NOT_NULL(structTemplate);
	const _Xp *param = dynamic_cast<const _Xp *>(structTemplate);
	NOT_NULL(param);
	data.type = param->getType(m_versionOk);
	data.level = param->getLevelMax(m_versionOk);
	data.value = param->getValueMax(m_versionOk);
}	// ServerObjectTemplate::getXpPointsMax

size_t ServerObjectTemplate::getXpPointsCount(void) const
{
	if (!m_xpPointsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getXpPointsCount();
	}

	size_t count = m_xpPoints.size();

	// if we are extending our base template, add it's count
	if (m_xpPointsAppend && m_baseData != nullptr)
	{
		const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getXpPointsCount();
	}

	return count;
}	// ServerObjectTemplate::getXpPointsCount

bool ServerObjectTemplate::getPersistByDefault(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPersistByDefault(true);
#endif
	}

	if (!m_persistByDefault.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter persistByDefault in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter persistByDefault has not been defined in template %s!", DataResource::getName()));
			return base->getPersistByDefault();
		}
	}

	bool value = m_persistByDefault.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::getPersistByDefault

bool ServerObjectTemplate::getPersistContents(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPersistContents(true);
#endif
	}

	if (!m_persistContents.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter persistContents in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter persistContents has not been defined in template %s!", DataResource::getName()));
			return base->getPersistContents();
		}
	}

	bool value = m_persistContents.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::getPersistContents

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getSharedTemplate(true));
	IGNORE_RETURN(getVolumeMin(true));
	IGNORE_RETURN(getVolumeMax(true));
	IGNORE_RETURN(getInvulnerable(true));
	IGNORE_RETURN(getComplexityMin(true));
	IGNORE_RETURN(getComplexityMax(true));
	IGNORE_RETURN(getTintIndexMin(true));
	IGNORE_RETURN(getTintIndexMax(true));
	IGNORE_RETURN(getPersistByDefault(true));
	IGNORE_RETURN(getPersistContents(true));
}	// ServerObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,1,1))
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
		if (strcmp(paramName, "sharedTemplate") == 0)
			m_sharedTemplate.loadFromIff(file);
		else if (strcmp(paramName, "scripts") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_scripts.begin(); iter != m_scripts.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_scripts.clear();
			m_scriptsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_scripts.push_back(newData);
			}
			m_scriptsLoaded = true;
		}
		else if (strcmp(paramName, "objvars") == 0)
			m_objvars.loadFromIff(file);
		else if (strcmp(paramName, "volume") == 0)
			m_volume.loadFromIff(file);
		else if (strcmp(paramName, "visibleFlags") == 0)
		{
			std::vector<IntegerParam *>::iterator iter;
			for (iter = m_visibleFlags.begin(); iter != m_visibleFlags.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_visibleFlags.clear();
			m_visibleFlagsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				IntegerParam * newData = new IntegerParam;
				newData->loadFromIff(file);
				m_visibleFlags.push_back(newData);
			}
			m_visibleFlagsLoaded = true;
		}
		else if (strcmp(paramName, "deleteFlags") == 0)
		{
			std::vector<IntegerParam *>::iterator iter;
			for (iter = m_deleteFlags.begin(); iter != m_deleteFlags.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_deleteFlags.clear();
			m_deleteFlagsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				IntegerParam * newData = new IntegerParam;
				newData->loadFromIff(file);
				m_deleteFlags.push_back(newData);
			}
			m_deleteFlagsLoaded = true;
		}
		else if (strcmp(paramName, "moveFlags") == 0)
		{
			std::vector<IntegerParam *>::iterator iter;
			for (iter = m_moveFlags.begin(); iter != m_moveFlags.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_moveFlags.clear();
			m_moveFlagsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				IntegerParam * newData = new IntegerParam;
				newData->loadFromIff(file);
				m_moveFlags.push_back(newData);
			}
			m_moveFlagsLoaded = true;
		}
		else if (strcmp(paramName, "invulnerable") == 0)
			m_invulnerable.loadFromIff(file);
		else if (strcmp(paramName, "complexity") == 0)
			m_complexity.loadFromIff(file);
		else if (strcmp(paramName, "tintIndex") == 0)
			m_tintIndex.loadFromIff(file);
		else if (strcmp(paramName, "updateRanges") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 3, ("Template %s: read array size of %d for array \"updateRanges\" of size 3, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 3 && j < listCount; ++j)
				m_updateRanges[j].loadFromIff(file);
			// if there are more params for updateRanges read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "contents") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_contents.begin(); iter != m_contents.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_contents.clear();
			m_contentsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_contents.push_back(newData);
			}
			m_contentsLoaded = true;
		}
		else if (strcmp(paramName, "xpPoints") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_xpPoints.begin(); iter != m_xpPoints.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_xpPoints.clear();
			m_xpPointsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_xpPoints.push_back(newData);
			}
			m_xpPointsLoaded = true;
		}
		else if (strcmp(paramName, "persistByDefault") == 0)
			m_persistByDefault.loadFromIff(file);
		else if (strcmp(paramName, "persistContents") == 0)
			m_persistContents.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	return;
}	// ServerObjectTemplate::load


//=============================================================================
// class ServerObjectTemplate::_AttribMod

/**
 * Class constructor.
 */
ServerObjectTemplate::_AttribMod::_AttribMod(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// ServerObjectTemplate::_AttribMod::_AttribMod

/**
 * Class destructor.
 */
ServerObjectTemplate::_AttribMod::~_AttribMod()
{
}	// ServerObjectTemplate::_AttribMod::~_AttribMod

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::_AttribMod::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_AttribMod_tag, create);
}	// ServerObjectTemplate::_AttribMod::registerMe

/**
 * Creates a ServerObjectTemplate::_AttribMod template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::_AttribMod::create(const std::string & filename)
{
	return new ServerObjectTemplate::_AttribMod(filename);
}	// ServerObjectTemplate::_AttribMod::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::_AttribMod::getId(void) const
{
	return _AttribMod_tag;
}	// ServerObjectTemplate::_AttribMod::getId

ServerObjectTemplate::Attributes ServerObjectTemplate::_AttribMod::getTarget(bool versionOk, bool testData) const
{
#ifdef _DEBUG
ServerObjectTemplate::Attributes testDataValue = static_cast<ServerObjectTemplate::Attributes>(0);
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTarget(true);
#endif
	}

	if (!m_target.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter target in template %s", DataResource::getName()));
			return static_cast<Attributes>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter target has not been defined in template %s!", DataResource::getName()));
			return base->getTarget(versionOk);
		}
	}

	Attributes value = static_cast<Attributes>(m_target.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::_AttribMod::getTarget

int ServerObjectTemplate::_AttribMod::getValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValue(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValue(versionOk);
		}
	}

	int value = m_value.getValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValue(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getValue

int ServerObjectTemplate::_AttribMod::getValueMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMin(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMin(versionOk);
		}
	}

	int value = m_value.getMinValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMin(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getValueMin

int ServerObjectTemplate::_AttribMod::getValueMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMax(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMax(versionOk);
		}
	}

	int value = m_value.getMaxValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMax(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getValueMax

float ServerObjectTemplate::_AttribMod::getTime(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTime(true);
#endif
	}

	if (!m_time.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter time in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter time has not been defined in template %s!", DataResource::getName()));
			return base->getTime(versionOk);
		}
	}

	float value = m_time.getValue();
	char delta = m_time.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTime(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getTime

float ServerObjectTemplate::_AttribMod::getTimeMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeMin(true);
#endif
	}

	if (!m_time.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter time in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter time has not been defined in template %s!", DataResource::getName()));
			return base->getTimeMin(versionOk);
		}
	}

	float value = m_time.getMinValue();
	char delta = m_time.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeMin(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getTimeMin

float ServerObjectTemplate::_AttribMod::getTimeMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeMax(true);
#endif
	}

	if (!m_time.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter time in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter time has not been defined in template %s!", DataResource::getName()));
			return base->getTimeMax(versionOk);
		}
	}

	float value = m_time.getMaxValue();
	char delta = m_time.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeMax(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getTimeMax

float ServerObjectTemplate::_AttribMod::getTimeAtValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeAtValue(true);
#endif
	}

	if (!m_timeAtValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter timeAtValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter timeAtValue has not been defined in template %s!", DataResource::getName()));
			return base->getTimeAtValue(versionOk);
		}
	}

	float value = m_timeAtValue.getValue();
	char delta = m_timeAtValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeAtValue(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getTimeAtValue

float ServerObjectTemplate::_AttribMod::getTimeAtValueMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeAtValueMin(true);
#endif
	}

	if (!m_timeAtValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter timeAtValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter timeAtValue has not been defined in template %s!", DataResource::getName()));
			return base->getTimeAtValueMin(versionOk);
		}
	}

	float value = m_timeAtValue.getMinValue();
	char delta = m_timeAtValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeAtValueMin(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getTimeAtValueMin

float ServerObjectTemplate::_AttribMod::getTimeAtValueMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeAtValueMax(true);
#endif
	}

	if (!m_timeAtValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter timeAtValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter timeAtValue has not been defined in template %s!", DataResource::getName()));
			return base->getTimeAtValueMax(versionOk);
		}
	}

	float value = m_timeAtValue.getMaxValue();
	char delta = m_timeAtValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeAtValueMax(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getTimeAtValueMax

float ServerObjectTemplate::_AttribMod::getDecay(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDecay(true);
#endif
	}

	if (!m_decay.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter decay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter decay has not been defined in template %s!", DataResource::getName()));
			return base->getDecay(versionOk);
		}
	}

	float value = m_decay.getValue();
	char delta = m_decay.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDecay(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getDecay

float ServerObjectTemplate::_AttribMod::getDecayMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDecayMin(true);
#endif
	}

	if (!m_decay.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter decay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter decay has not been defined in template %s!", DataResource::getName()));
			return base->getDecayMin(versionOk);
		}
	}

	float value = m_decay.getMinValue();
	char delta = m_decay.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDecayMin(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getDecayMin

float ServerObjectTemplate::_AttribMod::getDecayMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_AttribMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_AttribMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDecayMax(true);
#endif
	}

	if (!m_decay.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter decay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter decay has not been defined in template %s!", DataResource::getName()));
			return base->getDecayMax(versionOk);
		}
	}

	float value = m_decay.getMaxValue();
	char delta = m_decay.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDecayMax(versionOk);
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
}	// ServerObjectTemplate::_AttribMod::getDecayMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerObjectTemplate::_AttribMod::testValues(void) const
{
	IGNORE_RETURN(getTarget(true));
	IGNORE_RETURN(getValueMin(true));
	IGNORE_RETURN(getValueMax(true));
	IGNORE_RETURN(getTimeMin(true));
	IGNORE_RETURN(getTimeMax(true));
	IGNORE_RETURN(getTimeAtValueMin(true));
	IGNORE_RETURN(getTimeAtValueMax(true));
	IGNORE_RETURN(getDecayMin(true));
	IGNORE_RETURN(getDecayMax(true));
}	// ServerObjectTemplate::_AttribMod::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::_AttribMod::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "target") == 0)
			m_target.loadFromIff(file);
		else if (strcmp(paramName, "value") == 0)
			m_value.loadFromIff(file);
		else if (strcmp(paramName, "time") == 0)
			m_time.loadFromIff(file);
		else if (strcmp(paramName, "timeAtValue") == 0)
			m_timeAtValue.loadFromIff(file);
		else if (strcmp(paramName, "decay") == 0)
			m_decay.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerObjectTemplate::_AttribMod::load


//=============================================================================
// struct ServerObjectTemplate::Contents

/**
 * Struct constructor.
 */
ServerObjectTemplate::Contents::Contents(void)
{
	content = nullptr;
}	// ServerObjectTemplate::Contents::Contents()

/**
 * Struct copy constructor.
 */
ServerObjectTemplate::Contents::Contents(const ServerObjectTemplate::Contents & source) :
	slotName(source.slotName),
	equipObject(source.equipObject),
	content(source.content)
{
	if (content != nullptr)
		const_cast<ObjectTemplate *>(content)->addReference();
}	// ServerObjectTemplate::Contents::Contents(const ServerObjectTemplate::Contents &)

/**
 * Struct destructor.
 */
ServerObjectTemplate::Contents::~Contents()
{
	if (content != nullptr)
	{
		content->releaseReference();
		content = nullptr;
	}
}	// ServerObjectTemplate::Contents::~Contents

#ifdef WIN32
/**
 * Struct destructor.
 */
ServerObjectTemplate::Contents & ServerObjectTemplate::Contents::operator =(const ServerObjectTemplate::Contents &)
{
	DEBUG_FATAL(true, ("operator = should not be called for template structures!"));
	return *this;
} // ServerObjectTemplate::Contents::operator =
#endif


//=============================================================================
// class ServerObjectTemplate::_Contents

/**
 * Class constructor.
 */
ServerObjectTemplate::_Contents::_Contents(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// ServerObjectTemplate::_Contents::_Contents

/**
 * Class destructor.
 */
ServerObjectTemplate::_Contents::~_Contents()
{
}	// ServerObjectTemplate::_Contents::~_Contents

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::_Contents::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_Contents_tag, create);
}	// ServerObjectTemplate::_Contents::registerMe

/**
 * Creates a ServerObjectTemplate::_Contents template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::_Contents::create(const std::string & filename)
{
	return new ServerObjectTemplate::_Contents(filename);
}	// ServerObjectTemplate::_Contents::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::_Contents::getId(void) const
{
	return _Contents_tag;
}	// ServerObjectTemplate::_Contents::getId

const std::string & ServerObjectTemplate::_Contents::getSlotName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Contents * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Contents *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSlotName(true);
#endif
	}

	if (!m_slotName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slotName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slotName has not been defined in template %s!", DataResource::getName()));
			return base->getSlotName(versionOk);
		}
	}

	const std::string & value = m_slotName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::_Contents::getSlotName

bool ServerObjectTemplate::_Contents::getEquipObject(bool versionOk, bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Contents * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Contents *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getEquipObject(true);
#endif
	}

	if (!m_equipObject.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter equipObject in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter equipObject has not been defined in template %s!", DataResource::getName()));
			return base->getEquipObject(versionOk);
		}
	}

	bool value = m_equipObject.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::_Contents::getEquipObject

const ServerObjectTemplate * ServerObjectTemplate::_Contents::getContent(bool versionOk) const
{
	const ServerObjectTemplate::_Contents * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Contents *>(m_baseData);
	}

	if (!m_content.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter content in template %s", DataResource::getName()));
			return nullptr;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter content has not been defined in template %s!", DataResource::getName()));
			return base->getContent(versionOk);
		}
	}

	const ServerObjectTemplate * returnValue = nullptr;
	const std::string & templateName = m_content.getValue();
	if (!templateName.empty())
	{
		returnValue = dynamic_cast<const ServerObjectTemplate *>(ObjectTemplateList::fetch(templateName));
		if (returnValue == nullptr)
			WARNING_STRICT_FATAL(true, ("Error loading template %s",templateName.c_str()));
	}
	return returnValue;
}	// ServerObjectTemplate::_Contents::getContent

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerObjectTemplate::_Contents::testValues(void) const
{
	IGNORE_RETURN(getSlotName(true));
	IGNORE_RETURN(getEquipObject(true));
}	// ServerObjectTemplate::_Contents::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::_Contents::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "slotName") == 0)
			m_slotName.loadFromIff(file);
		else if (strcmp(paramName, "equipObject") == 0)
			m_equipObject.loadFromIff(file);
		else if (strcmp(paramName, "content") == 0)
			m_content.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerObjectTemplate::_Contents::load


//=============================================================================
// class ServerObjectTemplate::_MentalStateMod

/**
 * Class constructor.
 */
ServerObjectTemplate::_MentalStateMod::_MentalStateMod(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// ServerObjectTemplate::_MentalStateMod::_MentalStateMod

/**
 * Class destructor.
 */
ServerObjectTemplate::_MentalStateMod::~_MentalStateMod()
{
}	// ServerObjectTemplate::_MentalStateMod::~_MentalStateMod

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::_MentalStateMod::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_MentalStateMod_tag, create);
}	// ServerObjectTemplate::_MentalStateMod::registerMe

/**
 * Creates a ServerObjectTemplate::_MentalStateMod template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::_MentalStateMod::create(const std::string & filename)
{
	return new ServerObjectTemplate::_MentalStateMod(filename);
}	// ServerObjectTemplate::_MentalStateMod::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::_MentalStateMod::getId(void) const
{
	return _MentalStateMod_tag;
}	// ServerObjectTemplate::_MentalStateMod::getId

ServerObjectTemplate::MentalStates ServerObjectTemplate::_MentalStateMod::getTarget(bool versionOk, bool testData) const
{
#ifdef _DEBUG
ServerObjectTemplate::MentalStates testDataValue = static_cast<ServerObjectTemplate::MentalStates>(0);
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTarget(true);
#endif
	}

	if (!m_target.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter target in template %s", DataResource::getName()));
			return static_cast<MentalStates>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter target has not been defined in template %s!", DataResource::getName()));
			return base->getTarget(versionOk);
		}
	}

	MentalStates value = static_cast<MentalStates>(m_target.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::_MentalStateMod::getTarget

float ServerObjectTemplate::_MentalStateMod::getValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValue(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValue(versionOk);
		}
	}

	float value = m_value.getValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValue(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getValue

float ServerObjectTemplate::_MentalStateMod::getValueMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMin(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMin(versionOk);
		}
	}

	float value = m_value.getMinValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMin(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getValueMin

float ServerObjectTemplate::_MentalStateMod::getValueMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMax(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMax(versionOk);
		}
	}

	float value = m_value.getMaxValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMax(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getValueMax

float ServerObjectTemplate::_MentalStateMod::getTime(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTime(true);
#endif
	}

	if (!m_time.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter time in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter time has not been defined in template %s!", DataResource::getName()));
			return base->getTime(versionOk);
		}
	}

	float value = m_time.getValue();
	char delta = m_time.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTime(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getTime

float ServerObjectTemplate::_MentalStateMod::getTimeMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeMin(true);
#endif
	}

	if (!m_time.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter time in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter time has not been defined in template %s!", DataResource::getName()));
			return base->getTimeMin(versionOk);
		}
	}

	float value = m_time.getMinValue();
	char delta = m_time.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeMin(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getTimeMin

float ServerObjectTemplate::_MentalStateMod::getTimeMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeMax(true);
#endif
	}

	if (!m_time.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter time in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter time has not been defined in template %s!", DataResource::getName()));
			return base->getTimeMax(versionOk);
		}
	}

	float value = m_time.getMaxValue();
	char delta = m_time.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeMax(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getTimeMax

float ServerObjectTemplate::_MentalStateMod::getTimeAtValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeAtValue(true);
#endif
	}

	if (!m_timeAtValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter timeAtValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter timeAtValue has not been defined in template %s!", DataResource::getName()));
			return base->getTimeAtValue(versionOk);
		}
	}

	float value = m_timeAtValue.getValue();
	char delta = m_timeAtValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeAtValue(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getTimeAtValue

float ServerObjectTemplate::_MentalStateMod::getTimeAtValueMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeAtValueMin(true);
#endif
	}

	if (!m_timeAtValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter timeAtValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter timeAtValue has not been defined in template %s!", DataResource::getName()));
			return base->getTimeAtValueMin(versionOk);
		}
	}

	float value = m_timeAtValue.getMinValue();
	char delta = m_timeAtValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeAtValueMin(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getTimeAtValueMin

float ServerObjectTemplate::_MentalStateMod::getTimeAtValueMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTimeAtValueMax(true);
#endif
	}

	if (!m_timeAtValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter timeAtValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter timeAtValue has not been defined in template %s!", DataResource::getName()));
			return base->getTimeAtValueMax(versionOk);
		}
	}

	float value = m_timeAtValue.getMaxValue();
	char delta = m_timeAtValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTimeAtValueMax(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getTimeAtValueMax

float ServerObjectTemplate::_MentalStateMod::getDecay(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDecay(true);
#endif
	}

	if (!m_decay.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter decay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter decay has not been defined in template %s!", DataResource::getName()));
			return base->getDecay(versionOk);
		}
	}

	float value = m_decay.getValue();
	char delta = m_decay.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDecay(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getDecay

float ServerObjectTemplate::_MentalStateMod::getDecayMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDecayMin(true);
#endif
	}

	if (!m_decay.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter decay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter decay has not been defined in template %s!", DataResource::getName()));
			return base->getDecayMin(versionOk);
		}
	}

	float value = m_decay.getMinValue();
	char delta = m_decay.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDecayMin(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getDecayMin

float ServerObjectTemplate::_MentalStateMod::getDecayMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_MentalStateMod * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_MentalStateMod *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDecayMax(true);
#endif
	}

	if (!m_decay.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter decay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter decay has not been defined in template %s!", DataResource::getName()));
			return base->getDecayMax(versionOk);
		}
	}

	float value = m_decay.getMaxValue();
	char delta = m_decay.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDecayMax(versionOk);
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
}	// ServerObjectTemplate::_MentalStateMod::getDecayMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerObjectTemplate::_MentalStateMod::testValues(void) const
{
	IGNORE_RETURN(getTarget(true));
	IGNORE_RETURN(getValueMin(true));
	IGNORE_RETURN(getValueMax(true));
	IGNORE_RETURN(getTimeMin(true));
	IGNORE_RETURN(getTimeMax(true));
	IGNORE_RETURN(getTimeAtValueMin(true));
	IGNORE_RETURN(getTimeAtValueMax(true));
	IGNORE_RETURN(getDecayMin(true));
	IGNORE_RETURN(getDecayMax(true));
}	// ServerObjectTemplate::_MentalStateMod::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::_MentalStateMod::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "target") == 0)
			m_target.loadFromIff(file);
		else if (strcmp(paramName, "value") == 0)
			m_value.loadFromIff(file);
		else if (strcmp(paramName, "time") == 0)
			m_time.loadFromIff(file);
		else if (strcmp(paramName, "timeAtValue") == 0)
			m_timeAtValue.loadFromIff(file);
		else if (strcmp(paramName, "decay") == 0)
			m_decay.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerObjectTemplate::_MentalStateMod::load


//=============================================================================
// class ServerObjectTemplate::_Xp

/**
 * Class constructor.
 */
ServerObjectTemplate::_Xp::_Xp(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// ServerObjectTemplate::_Xp::_Xp

/**
 * Class destructor.
 */
ServerObjectTemplate::_Xp::~_Xp()
{
}	// ServerObjectTemplate::_Xp::~_Xp

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::_Xp::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_Xp_tag, create);
}	// ServerObjectTemplate::_Xp::registerMe

/**
 * Creates a ServerObjectTemplate::_Xp template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::_Xp::create(const std::string & filename)
{
	return new ServerObjectTemplate::_Xp(filename);
}	// ServerObjectTemplate::_Xp::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::_Xp::getId(void) const
{
	return _Xp_tag;
}	// ServerObjectTemplate::_Xp::getId

ServerObjectTemplate::XpTypes ServerObjectTemplate::_Xp::getType(bool versionOk, bool testData) const
{
#ifdef _DEBUG
ServerObjectTemplate::XpTypes testDataValue = static_cast<ServerObjectTemplate::XpTypes>(0);
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Xp * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Xp *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getType(true);
#endif
	}

	if (!m_type.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter type in template %s", DataResource::getName()));
			return static_cast<XpTypes>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter type has not been defined in template %s!", DataResource::getName()));
			return base->getType(versionOk);
		}
	}

	XpTypes value = static_cast<XpTypes>(m_type.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerObjectTemplate::_Xp::getType

int ServerObjectTemplate::_Xp::getLevel(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Xp * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Xp *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getLevel(true);
#endif
	}

	if (!m_level.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter level in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter level has not been defined in template %s!", DataResource::getName()));
			return base->getLevel(versionOk);
		}
	}

	int value = m_level.getValue();
	char delta = m_level.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getLevel(versionOk);
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
}	// ServerObjectTemplate::_Xp::getLevel

int ServerObjectTemplate::_Xp::getLevelMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Xp * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Xp *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getLevelMin(true);
#endif
	}

	if (!m_level.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter level in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter level has not been defined in template %s!", DataResource::getName()));
			return base->getLevelMin(versionOk);
		}
	}

	int value = m_level.getMinValue();
	char delta = m_level.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getLevelMin(versionOk);
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
}	// ServerObjectTemplate::_Xp::getLevelMin

int ServerObjectTemplate::_Xp::getLevelMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Xp * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Xp *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getLevelMax(true);
#endif
	}

	if (!m_level.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter level in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter level has not been defined in template %s!", DataResource::getName()));
			return base->getLevelMax(versionOk);
		}
	}

	int value = m_level.getMaxValue();
	char delta = m_level.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getLevelMax(versionOk);
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
}	// ServerObjectTemplate::_Xp::getLevelMax

int ServerObjectTemplate::_Xp::getValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Xp * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Xp *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValue(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValue(versionOk);
		}
	}

	int value = m_value.getValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValue(versionOk);
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
}	// ServerObjectTemplate::_Xp::getValue

int ServerObjectTemplate::_Xp::getValueMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Xp * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Xp *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMin(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMin(versionOk);
		}
	}

	int value = m_value.getMinValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMin(versionOk);
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
}	// ServerObjectTemplate::_Xp::getValueMin

int ServerObjectTemplate::_Xp::getValueMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerObjectTemplate::_Xp * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerObjectTemplate::_Xp *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMax(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMax(versionOk);
		}
	}

	int value = m_value.getMaxValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMax(versionOk);
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
}	// ServerObjectTemplate::_Xp::getValueMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerObjectTemplate::_Xp::testValues(void) const
{
	IGNORE_RETURN(getType(true));
	IGNORE_RETURN(getLevelMin(true));
	IGNORE_RETURN(getLevelMax(true));
	IGNORE_RETURN(getValueMin(true));
	IGNORE_RETURN(getValueMax(true));
}	// ServerObjectTemplate::_Xp::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::_Xp::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "type") == 0)
			m_type.loadFromIff(file);
		else if (strcmp(paramName, "level") == 0)
			m_level.loadFromIff(file);
		else if (strcmp(paramName, "value") == 0)
			m_value.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerObjectTemplate::_Xp::load

//@END TFD
