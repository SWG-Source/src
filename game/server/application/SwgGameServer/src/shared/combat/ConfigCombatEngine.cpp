//========================================================================
//
// ConfigCombatEngine.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "FirstSwgGameServer.h"
#include "ConfigCombatEngine.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "swgSharedUtility/CombatEngineData.h"

#include <stdio.h>
#include "combat.def"


#define KEY_INT(a,b)    (m_data.a = ConfigFile::getKeyInt("CombatEngine", #a, b))


//-------------------------------------------------------------------
// local statics

namespace ConfigCombatEngineNameSpace
{
	typedef std::map<CrcLowerString, int> BoneMap;
	BoneMap ms_boneMap;
}

//-------------------------------------------------------------------

using namespace ConfigCombatEngineNameSpace;


//-------------------------------------------------------------------
// static member variables

ConfigCombatEngine::Data ConfigCombatEngine::m_data;
float                    ConfigCombatEngine::m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorHeavy+1][ServerArmorTemplate::AR_armorHeavy+1];

void ConfigCombatEngine::install(void)
{
	static const std::string CONFIG_FILE("abstract/combat/combat_consts.iff");
	Iff file;

	ms_boneMap.insert(BoneMap::value_type(CrcLowerString("CSB_body"), static_cast<int>(CSB_body)));
	ms_boneMap.insert(BoneMap::value_type(CrcLowerString("CSB_head"), static_cast<int>(CSB_head)));
	ms_boneMap.insert(BoneMap::value_type(CrcLowerString("CSB_rightArm"), static_cast<int>(CSB_rightArm)));
	ms_boneMap.insert(BoneMap::value_type(CrcLowerString("CSB_leftArm"), static_cast<int>(CSB_leftArm)));
	ms_boneMap.insert(BoneMap::value_type(CrcLowerString("CSB_rightLeg"), static_cast<int>(CSB_rightLeg)));
	ms_boneMap.insert(BoneMap::value_type(CrcLowerString("CSB_leftLeg"), static_cast<int>(CSB_leftLeg)));

	installFromConfigFile();

	if (!TreeFile::exists(CONFIG_FILE.c_str()))
		return;
	if (!file.open(CONFIG_FILE.c_str(), true))
		return;

	Tag type = file.getCurrentName();
	if (type != TAG(C,O,M,C))
		return;
	file.enterForm();
	Tag version = file.getCurrentName();

	switch (version)
	{
		case TAG(0,0,0,0):
			parseVersion0(file);
			break;
		default:
			break;
	}

	file.exitForm();
	file.close();
}	// ConfigCombatEngine::install

/**
 * Loads config info from the [CombatEngine] config file section.
 */
void ConfigCombatEngine::installFromConfigFile(void)
{
	KEY_INT(debugQueues, 0);
	KEY_INT(debugTargeting, 0);
	KEY_INT(debugAttack, 0);
	KEY_INT(debugDefense, 0);
	KEY_INT(debugDamage, 0);
	KEY_INT(debugExplosion, 0);
}	// ConfigCombatEngine::installFromConfigFile

void ConfigCombatEngine::parseVersion0(Iff & file)
{
std::string paramName, tempString;
ConfigCombatEngineData::BodyAttackMod bodyAttackMod;
ConfigCombatEngine::SkeletonAttackMod skeletonAttackMod;
//
//	//-- Initialize data structures.
//	memset(&bodyAttackMod, 0, sizeof(bodyAttackMod));
//	memset(&skeletonAttackMod, 0, sizeof(skeletonAttackMod));
//
	file.enterForm();

	for (;;)
	{
		if (file.atEndOfForm())
			break;

		file.enterChunk();

		file.read_string(paramName);
		if (paramName == "maxAims")
		{
			m_data.maxAims = file.read_uint8();
		}
		else if (paramName == "kineticObjectDamage")
		{
			m_data.kineticObjectDamage = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "energyObjectDamage")
		{
			m_data.energyObjectDamage = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "blastObjectDamage")
		{
			m_data.blastObjectDamage = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "stunObjectDamage")
		{
			m_data.stunObjectDamage = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "restraintObjectDamage")
		{
			m_data.restraintObjectDamage = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "elementalObjectDamage")
		{
			m_data.elementalObjectDamage = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "weaponLowerThanArmorRating")
		{
			m_data.weaponLowerThanArmorRating = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "weaponHigherThanArmorRating")
		{
			m_data.weaponHigherThanArmorRating = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "woundChance")
		{
			m_data.woundChance = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "woundPercent")
		{
			m_data.woundPercent = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "shockWoundPercent")
		{
			m_data.shockWoundPercent = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		else if (paramName == "numberSkeletons")
		{
			m_data.numberSkeletons = file.read_uint8() + 1;
			// set up all-body skeleton
			bodyAttackMod.name.clear();
			bodyAttackMod.modelBoneName.clear();
			bodyAttackMod.toHitChance = 100;
			bodyAttackMod.toWoundBonus = 0;
			bodyAttackMod.combatSkeletonBone = 0;
			bodyAttackMod.damageBonus[Attributes::Health] = 0;
			bodyAttackMod.damageBonus[Attributes::Action] = 0;
			bodyAttackMod.damageBonus[Attributes::Mind] = 0;
			skeletonAttackMod.name.clear();
			skeletonAttackMod.numHitLocations = 1;
			skeletonAttackMod.script.clear();
			skeletonAttackMod.attackMods.clear();
			skeletonAttackMod.attackMods.push_back(bodyAttackMod);
			m_data.skeletonAttackMods.push_back(skeletonAttackMod);
		}
		else if (paramName == "skeletonAttackMod")
		{
			skeletonAttackMod.attackMods.clear();
			size_t skeletonIndex = file.read_uint8() + 1;
			while (m_data.skeletonAttackMods.size() <= skeletonIndex)
				m_data.skeletonAttackMods.push_back(skeletonAttackMod);
			ConfigCombatEngine::SkeletonAttackMod & skeleton = m_data.skeletonAttackMods[skeletonIndex];
			file.read_string(tempString);
			skeleton.name = tempString;
			skeleton.numHitLocations = file.read_uint8();
			file.read_string(tempString);
			skeleton.script = tempString;
		}
		else if (paramName == "skeletonAttackLoc")
		{
			size_t skeletonIndex = file.read_uint8() + 1;
			size_t locIndex = file.read_uint8();
			while (m_data.skeletonAttackMods.size() <= skeletonIndex)
				m_data.skeletonAttackMods.push_back(skeletonAttackMod);
			ConfigCombatEngine::SkeletonAttackMod & skeleton = m_data.skeletonAttackMods[skeletonIndex];
			while (skeleton.attackMods.size() <= locIndex)
				skeleton.attackMods.push_back(bodyAttackMod);
			ConfigCombatEngineData::BodyAttackMod & bodyLoc = skeleton.attackMods[locIndex];
			file.read_string(tempString);
			bodyLoc.name = tempString;
			file.read_string(tempString);
			bodyLoc.modelBoneName = tempString;
			bodyLoc.toHitChance = file.read_uint8();
			bodyLoc.toWoundBonus = static_cast<float>(file.read_uint8()) / 100.0f;
			bodyLoc.damageBonus[Attributes::Health] = static_cast<float>(file.read_uint8()) / 100.0f;
			bodyLoc.damageBonus[Attributes::Action] = static_cast<float>(file.read_uint8()) / 100.0f;
			bodyLoc.damageBonus[Attributes::Mind] = static_cast<float>(file.read_uint8()) / 100.0f;
		}
		file.exitChunk();
	}

	file.exitForm();

	setupRatingDifferenceDamageReduction();
}	// ConfigCombatEngine::parseVersion0

//-------------------------------------------------------------------

void ConfigCombatEngine::remove(void)
{
	m_data.skeletonAttackMods.clear();
	ms_boneMap.clear();
}	// ConfigCombatEngine::remove

//-------------------------------------------------------------------

float ConfigCombatEngine::getObjectDamageSpread(
	ServerWeaponObjectTemplate::DamageType damageType)
{
	switch (damageType)
	{
		case ServerWeaponObjectTemplate::DT_kinetic:
			return m_data.kineticObjectDamage;
		case ServerWeaponObjectTemplate::DT_energy:
			return m_data.energyObjectDamage;
		case ServerWeaponObjectTemplate::DT_blast:
			return m_data.blastObjectDamage;
		case ServerWeaponObjectTemplate::DT_stun:
			return m_data.stunObjectDamage;
		case ServerWeaponObjectTemplate::DT_restraint:
			return m_data.restraintObjectDamage;
		default:
			break;
	}//lint !e788 enum constant not used
	return 1.0f;
}	// ConfigCombatEngine::getObjectDamageSpread

//-------------------------------------------------------------------

/**
 * Sets up the RatingDifferenceDamageReduction table.
 */
void ConfigCombatEngine::setupRatingDifferenceDamageReduction(void)
{
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorNone][ServerArmorTemplate::AR_armorNone] = 1.0f;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorNone][ServerArmorTemplate::AR_armorLight] = m_data.weaponLowerThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorNone][ServerArmorTemplate::AR_armorMedium] = m_data.weaponLowerThanArmorRating * m_data.weaponLowerThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorNone][ServerArmorTemplate::AR_armorHeavy] = m_data.weaponLowerThanArmorRating * m_data.weaponLowerThanArmorRating * m_data.weaponLowerThanArmorRating;

	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorLight][ServerArmorTemplate::AR_armorNone] = m_data.weaponHigherThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorLight][ServerArmorTemplate::AR_armorLight] = 1.0f;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorLight][ServerArmorTemplate::AR_armorMedium] = m_data.weaponLowerThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorLight][ServerArmorTemplate::AR_armorHeavy] = m_data.weaponLowerThanArmorRating * m_data.weaponLowerThanArmorRating;

	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorMedium][ServerArmorTemplate::AR_armorNone] = m_data.weaponHigherThanArmorRating * m_data.weaponHigherThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorMedium][ServerArmorTemplate::AR_armorLight] = m_data.weaponHigherThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorMedium][ServerArmorTemplate::AR_armorMedium] = 1.0f;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorMedium][ServerArmorTemplate::AR_armorHeavy] = m_data.weaponLowerThanArmorRating;

	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorHeavy][ServerArmorTemplate::AR_armorNone] = m_data.weaponHigherThanArmorRating * m_data.weaponHigherThanArmorRating * m_data.weaponHigherThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorHeavy][ServerArmorTemplate::AR_armorLight] = m_data.weaponHigherThanArmorRating * m_data.weaponHigherThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorHeavy][ServerArmorTemplate::AR_armorMedium] = m_data.weaponHigherThanArmorRating;
	m_RatingDifferenceDamageReduction[ServerWeaponObjectTemplate::AR_armorHeavy][ServerArmorTemplate::AR_armorHeavy] = 1.0f;
}	// ConfigCombatEngine::setupRatingDifferenceDamageReduction
