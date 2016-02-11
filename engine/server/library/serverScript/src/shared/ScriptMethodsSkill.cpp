// ScriptMethodsSkill.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/Chat.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/DraftSchematicGroupManager.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"

#include "UnicodeUtils.h"

#include <climits>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsSkillNamespace
// ======================================================================

namespace ScriptMethodsSkillNamespace
{
	bool install();

	jint         JNICALL getModValue(JNIEnv * env, jobject self, jlong target, jstring modName);
	void         JNICALL setModValue(JNIEnv * env, jobject self, jlong target, jstring modName, jint value);
	jint         JNICALL getExperiencePoints(JNIEnv *env, jobject self, jlong target, jstring experienceType);
	jobject      JNICALL getExperiencePointsMap(JNIEnv * env, jobject self, jlong target);
	jint         JNICALL getSkillExperienceLimit(JNIEnv *env, jobject self, jlong target, jstring experienceType);
	jobjectArray JNICALL getSkillCommandsProvided(JNIEnv *env, jobject self, jstring skillName);
	jobjectArray JNICALL getSkillListingForPlayer(JNIEnv * env, jobject self, jlong player);
	jobject      JNICALL getSkillPrerequisiteExperience(JNIEnv *env, jobject self, jstring skillName);
	jobjectArray JNICALL getSkillPrerequisiteSkills(JNIEnv *env, jobject self, jstring skillName);
	jobject      JNICALL getSkillPrerequisiteSpecies(JNIEnv *env, jobject self, jstring skillName);
	jstring      JNICALL getSkillProfession(JNIEnv *env, jobject self, jstring skillName);
	jobject      JNICALL getSkillStatisticModifiers(JNIEnv *env, jobject self, jstring skillName);
	jint         JNICALL getCreatureSkillStatisticModifier(JNIEnv *env, jobject self, jlong creature, jstring skillStatName);
	jintArray    JNICALL getCreatureSkillStatisticModifiers(JNIEnv *env, jobject self, jlong creature, jobjectArray skillStatNames);
	jobject      JNICALL getEnhancedSkillStatisticModifiers(JNIEnv *env, jobject self, jstring skillName);
	jint         JNICALL getCreatureEnhancedSkillStatisticModifier(JNIEnv *env, jobject self, jlong creature, jstring skillStatName);
	jintArray    JNICALL getCreatureEnhancedSkillStatisticModifiers(JNIEnv *env, jobject self, jlong creature, jobjectArray skillStatNames);	
	jint         JNICALL getCreatureEnhancedSkillStatisticModifierUncapped(JNIEnv *env, jobject self, jlong creature, jstring skillStatName);
	jintArray    JNICALL getCreatureEnhancedSkillStatisticModifiersUncapped(JNIEnv *env, jobject self, jlong creature, jobjectArray skillStatNames);
	jint         JNICALL internalGetCreatureEnhancedSkillStatisticModifier(JNIEnv *env, jobject self, jlong creature, jstring skillStatName, jboolean useBonusCap);
	jintArray    JNICALL internalGetCreatureEnhancedSkillStatisticModifiers(JNIEnv *env, jobject self, jlong creature, jobjectArray skillStatNames, jboolean useBonusCap);
	jstring      JNICALL getSkillTitleGranted(JNIEnv *env, jobject self, jstring skillName);
	jboolean     JNICALL grantCommand(JNIEnv *env, jobject self, jlong target, jstring commandName);
	jint         JNICALL grantExperiencePointsByString(JNIEnv *env, jobject self, jlong target, jstring experienceType, jint amount);
	jint         JNICALL grantExperiencePointsByInt(JNIEnv *env, jobject self, jlong target, jint experienceType, jint amount);
	jstring      JNICALL getStringExperienceType(JNIEnv *env, jobject self, jint xpType);
	jboolean     JNICALL grantSkill(JNIEnv *env, jobject self, jlong target, jstring skillName);
	jboolean     JNICALL hasCommand(JNIEnv *env, jobject, jlong target, jstring commandName);
	jboolean     JNICALL hasSkill(JNIEnv *env, jobject self, jlong target, jstring skillName);
	void         JNICALL revokeCommand(JNIEnv *env, jobject self, jlong target, jstring commandName);
	void         JNICALL revokeSkill(JNIEnv *env, jobject self, jlong target, jstring skillName);
	void         JNICALL revokeSkillSilent(JNIEnv *env, jobject self, jlong target, jstring skillName);
	jboolean     JNICALL grantSchematicGroup(JNIEnv *env, jobject self, jlong target, jstring groupName);
	jboolean     JNICALL revokeSchematicGroup(JNIEnv *env, jobject self, jlong target, jstring groupName);
	jboolean     JNICALL grantSchematicName(JNIEnv *env, jobject self, jlong target, jstring schematicName);
	jboolean     JNICALL grantSchematicCrc(JNIEnv *env, jobject self, jlong target, jint schematicCrc);
	jboolean     JNICALL revokeSchematicName(JNIEnv *env, jobject self, jlong target, jstring schematicName);
	jboolean     JNICALL revokeSchematicCrc(JNIEnv *env, jobject self, jlong target, jint schematicCrc);
	jboolean     JNICALL hasSchematicName(JNIEnv *env, jobject self, jlong target, jstring schematicName);
	jboolean     JNICALL hasSchematicCrc(JNIEnv *env, jobject self, jlong target, jint schematicCrc);
	jobjectArray JNICALL getSkillStatModListingForPlayer(JNIEnv *env, jobject self, jlong player);
	jobjectArray JNICALL getCommandListingForPlayer(JNIEnv *env, jobject self, jlong player);
	jintArray    JNICALL getSkillSchematicsGranted(JNIEnv *env, jobject self, jstring skillName);
	jintArray    JNICALL getSchematicListingForPlayer(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL hasCertificationsForItem(JNIEnv *env, jobject self, jlong player, jlong item);
	jobjectArray JNICALL getRequiredCertifications(JNIEnv *env, jobject self, jlong item);
	jint         JNICALL getSkillStatMod(JNIEnv * env, jobject self, jlong player, jstring statModName);
	jboolean     JNICALL applySkillStatisticModifier(JNIEnv *env, jobject self, jlong player, jstring statName, jint value);
	jstring      JNICALL getSkillTemplate(JNIEnv *env, jobject self, jlong player);
	void         JNICALL setSkillTemplate(JNIEnv *env, jobject self, jlong player, jstring skillTemplateName);
	jstring      JNICALL getWorkingSkill(JNIEnv *env, jobject self, jlong player);
	void         JNICALL setWorkingSkill(JNIEnv *env, jobject self, jlong player, jstring skillTemplateName);
	void         JNICALL recomputeCommandSeries(JNIEnv *env, jobject self, jlong player);
	void         JNICALL resetExpertises(JNIEnv *env, jobject self, jlong player);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsSkillNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsSkillNamespace::c)}
	JF("__getModValue", "(JLjava/lang/String;)I", getModValue),
	JF("__setModValue", "(JLjava/lang/String;I)V", setModValue),
	JF("_getExperiencePoints", "(JLjava/lang/String;)I", getExperiencePoints),
	JF("_getExperiencePoints", "(J)Lscript/dictionary;", getExperiencePointsMap),
	JF("_getExperienceCap", "(JLjava/lang/String;)I", getSkillExperienceLimit),
	JF("getSkillCommandsProvided", "(Ljava/lang/String;)[Ljava/lang/String;", getSkillCommandsProvided),
	JF("_getSkillListingForPlayer", "(J)[Ljava/lang/String;", getSkillListingForPlayer),
	JF("getSkillPrerequisiteExperience", "(Ljava/lang/String;)Lscript/dictionary;", getSkillPrerequisiteExperience),
	JF("getSkillPrerequisiteSkills", "(Ljava/lang/String;)[Ljava/lang/String;", getSkillPrerequisiteSkills),
	JF("getSkillPrerequisiteSpecies", "(Ljava/lang/String;)Lscript/dictionary;", getSkillPrerequisiteSpecies),
	JF("getSkillProfession", "(Ljava/lang/String;)Ljava/lang/String;", getSkillProfession),
	JF("getSkillStatisticModifiers", "(Ljava/lang/String;)Lscript/dictionary;", getSkillStatisticModifiers),
	JF("_getSkillStatisticModifier", "(JLjava/lang/String;)I", getCreatureSkillStatisticModifier),
	JF("_getSkillStatisticModifiers", "(J[Ljava/lang/String;)[I", getCreatureSkillStatisticModifiers),
	JF("_getEnhancedSkillStatisticModifier", "(JLjava/lang/String;)I", getCreatureEnhancedSkillStatisticModifier),
	JF("_getEnhancedSkillStatisticModifiers", "(J[Ljava/lang/String;)[I", getCreatureEnhancedSkillStatisticModifiers),	
	JF("_getEnhancedSkillStatisticModifierUncapped", "(JLjava/lang/String;)I", getCreatureEnhancedSkillStatisticModifierUncapped),
	JF("_getEnhancedSkillStatisticModifiersUncapped", "(J[Ljava/lang/String;)[I", getCreatureEnhancedSkillStatisticModifiersUncapped),
	JF("getSkillTitleGranted", "(Ljava/lang/String;)Ljava/lang/String;", getSkillTitleGranted),
	JF("_grantCommand", "(JLjava/lang/String;)Z", grantCommand),
	JF("__grantExperiencePoints", "(JLjava/lang/String;I)I", grantExperiencePointsByString),
	JF("__grantExperiencePoints", "(JII)I", grantExperiencePointsByInt),
	JF("getStringExperienceType", "(I)Ljava/lang/String;", getStringExperienceType),
	JF("_grantSkill", "(JLjava/lang/String;)Z", grantSkill),
	JF("_hasCommand", "(JLjava/lang/String;)Z", hasCommand),
	JF("_hasSkill", "(JLjava/lang/String;)Z", hasSkill),
	JF("_revokeCommand", "(JLjava/lang/String;)V", revokeCommand),
	JF("_revokeSkill", "(JLjava/lang/String;)V", revokeSkill),
	JF("_revokeSkillSilent", "(JLjava/lang/String;)V", revokeSkillSilent),
	JF("_grantSchematicGroup", "(JLjava/lang/String;)Z", grantSchematicGroup),
	JF("_revokeSchematicGroup", "(JLjava/lang/String;)Z", revokeSchematicGroup),
	JF("_grantSchematic", "(JLjava/lang/String;)Z", grantSchematicName),
	JF("_grantSchematic", "(JI)Z", grantSchematicCrc),
	JF("_revokeSchematic", "(JLjava/lang/String;)Z", revokeSchematicName),
	JF("_revokeSchematic", "(JI)Z", revokeSchematicCrc),
	JF("_hasSchematic", "(JLjava/lang/String;)Z", hasSchematicName),
	JF("_hasSchematic", "(JI)Z", hasSchematicCrc),
	JF("_getSkillStatModListingForPlayer", "(J)[Ljava/lang/String;", getSkillStatModListingForPlayer),
	JF("_getCommandListingForPlayer", "(J)[Ljava/lang/String;", getCommandListingForPlayer),
	JF("getSkillSchematicsGranted", "(Ljava/lang/String;)[I", getSkillSchematicsGranted),
	JF("_getSchematicListingForPlayer", "(J)[I", getSchematicListingForPlayer),
	JF("_hasCertificationsForItem", "(JJ)Z", hasCertificationsForItem),
	JF("_getRequiredCertifications", "(J)[Ljava/lang/String;", getRequiredCertifications),
	JF("_getSkillStatMod", "(JLjava/lang/String;)I", getSkillStatMod),
	JF("_applySkillStatisticModifier", "(JLjava/lang/String;I)Z", applySkillStatisticModifier),
	JF("_getSkillTemplate", "(J)Ljava/lang/String;", getSkillTemplate),
	JF("_setSkillTemplate", "(JLjava/lang/String;)V", setSkillTemplate),
	JF("_getWorkingSkill", "(J)Ljava/lang/String;", getWorkingSkill),
	JF("_setWorkingSkill", "(JLjava/lang/String;)V", setWorkingSkill),
	JF("_recomputeCommandSeries", "(J)V", recomputeCommandSeries),
	JF("_resetExpertises", "(J)V", resetExpertises),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

static const SkillObject * getSkill(const JavaStringParam & skillName)
{
	const SkillObject * result = 0;

	std::string name;
	if(!JavaLibrary::convert(skillName, name))
		return 0;

	result = SkillManager::getInstance().getSkill(name);

	return result;
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsSkillNamespace::getModValue(JNIEnv * env, jobject self, jlong target, jstring modName)
{
	jint result = 0;
	CreatureObject * creature = 0;
	if(!JavaLibrary::getObject(target, creature))
		return result;

	JavaStringParam localModName(modName);
	std::string m;
	if(!JavaLibrary::convert(localModName, m))
		return result;

	result = creature->getModValue(m);

	return result;
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsSkillNamespace::getExperiencePoints(JNIEnv *env, jobject self, jlong target, jstring experienceType)
{
	JavaStringParam localExperienceType(experienceType);

	jint result = 0;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return result;

	std::string name;
	if(!JavaLibrary::convert(localExperienceType, name))
		return result;

	result = creature->getExperiencePoints(name);
	return result;
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsSkillNamespace::getExperiencePointsMap(JNIEnv * env, jobject self, jlong target)
{
	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	JavaDictionaryPtr dict;
	if (!JavaLibrary::convert(creature->getExperiencePoints(), dict))
		return 0;
	return dict->getReturnValue();
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsSkillNamespace::getSkillStatMod(JNIEnv * env, jobject self, jlong player, jstring statName)
{
	JavaStringParam localStatName(statName);

	jint result = 0;
	CreatureObject * obj = 0;
	if(JavaLibrary::getObject(player, obj))
	{
		std::string stat;
		if(JavaLibrary::convert(localStatName, stat))
			result = obj->getModValue(stat);
	}
	return result;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsSkillNamespace::applySkillStatisticModifier(JNIEnv * env, jobject self, jlong player, jstring statName, jint value)
{
	JavaStringParam localStatName(statName);

	if (!player)
	{
		return false;
	}

	CreatureObject * obj = 0;
	if(JavaLibrary::getObject(player, obj))
	{
		std::string stat;
		if(JavaLibrary::convert(localStatName, stat))
		{
			obj->addModValue(stat, value, false);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsSkillNamespace::getSkillCommandsProvided(JNIEnv *env, jobject self, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	const SkillObject * skill = getSkill(localSkillName);
	if(! skill)
		return nullptr;

	LocalObjectArrayRefPtr strArray;
	if(! ScriptConversion::convert(skill->getCommandsProvided(), strArray))
		return 0;

	return strArray->getReturnValue();
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsSkillNamespace::getSkillPrerequisiteExperience(JNIEnv *env, jobject self, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	const SkillObject * skill = getSkill(localSkillName);
	if(! skill)
		return nullptr;

	JavaDictionaryPtr dict;
	if (!JavaLibrary::convert(skill->getPrerequisiteExperienceVector(), dict))
		return 0;
	return dict->getReturnValue();
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsSkillNamespace::getSkillExperienceLimit(JNIEnv * env, jobject self, jlong player, jstring skillName)
{
	JavaStringParam localSkillName(skillName);
	jint result = 0;

	CreatureObject * creature = 0;
	if (JavaLibrary::getObject(player, creature))
	{
		const PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
		if (playerObject)
		{
			std::string name;
			if (JavaLibrary::convert(localSkillName, name))
			{
				result = playerObject->getExperienceLimit(name);
				if (result < 1)
					result = 0;
			}
		}
	}
	return result;
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsSkillNamespace::getSkillPrerequisiteSkills(JNIEnv *env, jobject self, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	const SkillObject * skill = getSkill(localSkillName);
	if(! skill)
		return nullptr;

	std::vector<std::string> skillNames;
	std::vector<const SkillObject *>::const_iterator i;
	for(i = skill->getPrerequisiteSkills().begin(); i != skill->getPrerequisiteSkills().end(); ++i)
	{
		skillNames.push_back((*i)->getSkillName());
	}

	LocalObjectArrayRefPtr strArray;
	if(! ScriptConversion::convert(skillNames, strArray))
		return 0;

	return strArray->getReturnValue();;
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsSkillNamespace::getSkillPrerequisiteSpecies(JNIEnv *env, jobject self, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	const SkillObject * skill = getSkill(localSkillName);
	if(! skill)
		return nullptr;

	JavaDictionaryPtr dict;
	if (!JavaLibrary::convert(skill->getPrerequisiteSpecies(), dict))
		return 0;
	return dict->getReturnValue();
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsSkillNamespace::getSkillProfession(JNIEnv *env, jobject self, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	const SkillObject * const skill = getSkill(localSkillName);
	if(!skill)
		return nullptr;
	const SkillObject * const prof = skill->findProfessionForSkill ();
	if(prof)
	{
		JavaString str(prof->getSkillName().c_str());
		return str.getReturnValue();
	}
	return nullptr;
}

//-----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsSkillNamespace::getSkillListingForPlayer(JNIEnv * env, jobject self, jlong jplayerId)
{
	const CreatureObject * player = 0;
	if (!JavaLibrary::getObject(jplayerId, player))
		return 0;

	CreatureObject::SkillList const & skills = player->getSkillList();
	std::vector<std::string> skillList;
	for (CreatureObject::SkillList::const_iterator i = skills.begin(); i != skills.end(); ++i)
		if (*i)
			skillList.push_back((*i)->getSkillName());

	LocalObjectArrayRefPtr strArray;
	if (!ScriptConversion::convert(skillList, strArray))
		return 0;

	return strArray->getReturnValue();
}

//-----------------------------------------------------------------------

/**
 * Returns a dictionary of skill stat modifier values on a creature for a given skill.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param skillName		the skill name
 *
 * @return a dictionary of skillmod name -> mod valus
 */
jobject JNICALL ScriptMethodsSkillNamespace::getSkillStatisticModifiers(JNIEnv *env, jobject self,
	jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	const SkillObject * skill = getSkill(localSkillName);
	if(! skill)
		return nullptr;

	JavaDictionaryPtr dict;
	if (!JavaLibrary::convert(skill->getStatisticModifiers(), dict))
		return 0;
	return dict->getReturnValue();
}

//-----------------------------------------------------------------------

/**
 * Returns the skill stat modifier value on a creature for a given skill stat
 * modifier name.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param creature			the creature whose stat mod we want
 * @param skillStatName		the name of the skill stat mod we want
 *
 * @return the skill stat mod value
 */
jint JNICALL ScriptMethodsSkillNamespace::getCreatureSkillStatisticModifier(JNIEnv *env, jobject self, jlong creature, jstring skillStatName)
{
	UNREF(self);

	JavaStringParam jskillStatName(skillStatName);

	const CreatureObject * object = nullptr;
	if (!JavaLibrary::getObject(creature, object))
		return 0;

	std::string statName;
	if (!JavaLibrary::convert(jskillStatName, statName))
		return 0;

	return object->getModValue(statName);
}	// JavaLibrary::getCreatureSkillStatisticModifier

//-----------------------------------------------------------------------

/**
 * Returns an array of skill stat modifier values on a creature for a list of
 * given skill stat modifier names.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param creature			the creature whose stat mod we want
 * @param skillStatNames 	array of names of the skill stat mods we want
 *
 * @return the skill stat mod values, or nullptr on error
 */
jintArray JNICALL ScriptMethodsSkillNamespace::getCreatureSkillStatisticModifiers(JNIEnv *env, jobject self, jlong creature, jobjectArray skillStatNames)
{
	UNREF(self);

	if (skillStatNames == 0)
		return 0;

	const CreatureObject * object = nullptr;
	if (!JavaLibrary::getObject(creature, object))
		return 0;

	std::string statName;

	int count = env->GetArrayLength(skillStatNames);
	std::vector<jint> stats(count, 0);
	for (int i = 0; i < count; ++i)
	{
		JavaStringPtr jstatName = getStringArrayElement(LocalObjectArrayRefParam(skillStatNames), i);
		if (jstatName != JavaString::cms_nullPtr)
		{
			if (!JavaLibrary::convert(*jstatName, statName))
				return 0;
			stats.at(i) = object->getModValue(statName);
		}
	}

	LocalIntArrayRefPtr jstats = createNewIntArray(count);
	if (jstats == LocalIntArrayRef::cms_nullPtr)
		return 0;

	setIntArrayRegion(*jstats, 0, count, &stats[0]);
	return jstats->getReturnValue();
}	// JavaLibrary::getCreatureSkillStatisticModifiers

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsSkillNamespace::getCreatureEnhancedSkillStatisticModifier(JNIEnv *env, jobject self, jlong creature, jstring skillStatName)
{
	return internalGetCreatureEnhancedSkillStatisticModifier(env, self, creature, skillStatName, true);
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsSkillNamespace::getCreatureEnhancedSkillStatisticModifierUncapped(JNIEnv *env, jobject self, jlong creature, jstring skillStatName)
{
	return internalGetCreatureEnhancedSkillStatisticModifier(env, self, creature, skillStatName, false);
}

//-----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsSkillNamespace::getCreatureEnhancedSkillStatisticModifiers(JNIEnv *env, jobject self, jlong creature, jobjectArray skillStatNames)
{
	return internalGetCreatureEnhancedSkillStatisticModifiers(env, self, creature, skillStatNames, true);
}

//-----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsSkillNamespace::getCreatureEnhancedSkillStatisticModifiersUncapped(JNIEnv *env, jobject self, jlong creature, jobjectArray skillStatNames)
{
	return internalGetCreatureEnhancedSkillStatisticModifiers(env, self, creature, skillStatNames, false);
}

//-----------------------------------------------------------------------

/**
 * Returns the skill stat modifier value, enhanced by equipment, on a creature
 * for a given skill stat modifier name.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param creature			the creature whose stat mod we want
 * @param skillStatName		the name of the skill stat mod we want
 *
 * @return the skill stat mod value
 */
jint JNICALL ScriptMethodsSkillNamespace::internalGetCreatureEnhancedSkillStatisticModifier(JNIEnv *env, jobject self, jlong creature, jstring skillStatName, jboolean useBonusCap)
{
	UNREF(self);

	JavaStringParam jskillStatName(skillStatName);

	const CreatureObject * object = nullptr;
	if (!JavaLibrary::getObject(creature, object))
		return 0;

	std::string statName;
	if (!JavaLibrary::convert(jskillStatName, statName))
		return 0;

	return (useBonusCap ? object->getEnhancedModValue(statName) : object->getEnhancedModValueUncapped(statName));
}	// JavaLibrary::getCreatureEnhancedSkillStatisticModifier

//-----------------------------------------------------------------------

/**
 * Returns an array of skill stat modifier values, enhanced by equipment, on a
 * creature for a list of given skill stat modifier names.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param creature			the creature whose stat mod we want
 * @param skillStatNames 	array of names of the skill stat mods we want
 *
 * @return the skill stat mod values, or nullptr on error
 */
jintArray JNICALL ScriptMethodsSkillNamespace::internalGetCreatureEnhancedSkillStatisticModifiers(JNIEnv *env, jobject self, jlong creature, jobjectArray skillStatNames, jboolean useBonusCap)
{
	UNREF(self);

	if (skillStatNames == 0)
		return 0;

	const CreatureObject * object = nullptr;
	if (!JavaLibrary::getObject(creature, object))
		return 0;

	std::string statName;

	int count = env->GetArrayLength(skillStatNames);
	std::vector<jint> stats(count, 0);
	for (int i = 0; i < count; ++i)
	{
		JavaStringPtr jstatName = getStringArrayElement(LocalObjectArrayRefParam(skillStatNames), i);
		if (jstatName != JavaString::cms_nullPtr)
		{
			if (!JavaLibrary::convert(*jstatName, statName))
				return 0;
			stats.at(i) = (useBonusCap ? object->getEnhancedModValue(statName) : object->getEnhancedModValueUncapped(statName));
		}
	}

	LocalIntArrayRefPtr jstats = createNewIntArray(count);
	if (jstats == LocalIntArrayRef::cms_nullPtr)
		return 0;

	setIntArrayRegion(*jstats, 0, count, &stats[0]);
	return jstats->getReturnValue();
}	// JavaLibrary::getCreatureEnhancedSkillStatisticModifiers

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsSkillNamespace::getSkillTitleGranted(JNIEnv *env, jobject self, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	const SkillObject * const skill = getSkill(localSkillName);
	if(! skill)
		return nullptr;

	if (skill->isTitle ())
		return JavaString(skill->getSkillName ().c_str()).getReturnValue();
	else
		return 0;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsSkillNamespace::grantCommand(JNIEnv *env, jobject self, jlong target, jstring commandName)
{
	JavaStringParam localCommandName(commandName);

	jboolean result = JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return result;

	std::string name;
	if(!JavaLibrary::convert(localCommandName, name))
		return result;

	result = creature->grantCommand(name, false);
	return result;
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsSkillNamespace::grantExperiencePointsByString(JNIEnv *env, jobject self, jlong target, jstring experienceType, jint amount)
{
	if (target == 0 || experienceType == 0)
		return INT_MIN;

	JavaStringParam localExperienceType(experienceType);
	std::string name;
	if(!JavaLibrary::convert(localExperienceType, name))
		return INT_MIN;

	jint result = INT_MIN;
	if(name.empty())
	{
		// throw java exception
		JavaLibrary::throwInternalScriptError("JavaLibrary::grantExperiencePoints() was passed an EMPTY or nullptr experience name. This is probably not what the script intends to do, and the database cannot save unnamed XP. Change your script to ensure that it is calling grantExperiencePoints with the intended parameters.");
	}
	else
	{
		CachedNetworkId targetId(target);
		if (targetId == CachedNetworkId::cms_cachedInvalid)
		{
			WARNING(true, ("JavaLibrary::grantExperiencePointsByString called "
				"with target id = 0"));
		}
		else if (targetId.getObject() == nullptr)
		{
			if (NameManager::getInstance().getPlayerName(targetId).empty())
			{
				WARNING(true, ("JavaLibrary::grantExperiencePointsByString called "
					"with target id = %s, who is not in the player name map",
					targetId.getValueString().c_str()));
			}
//			else if (ServerUniverse::getInstance().getXpManager() != nullptr)
//			{
//				ServerUniverse::getInstance().getXpManager()->grantXp(targetId,
//					name, amount);
//			}
			else
			{
				// send a message to the object for when it comes online
				char buffer[1024];
				sprintf(buffer, "%s %d", name.c_str(), static_cast<int>(amount));
				MessageToQueue::getInstance().sendMessageToC(targetId,
					"C++experience", buffer, 1, true);
				result = amount;
			}
		}
		else
		{
			CreatureObject * creature = dynamic_cast<CreatureObject *>(targetId.getObject());
			if (creature != nullptr)
			{
				result = creature->grantExperiencePoints(name, static_cast<int>(amount));
			}

		}
	}

	return result;
}

//-----------------------------------------------------------------------


/**
 * Grants a specific type of experience to a creature
 *
 * @param env		      Java environment
 * @param self			  class calling this function
 * @param target          creature that will receive the named experience points
 * @param experienceType  the id of the type of experience to grant to the creature
 * @param amount          the amount of experience to grant
 *
 * @return the xp amount granted, or -1 on error
 */
jint JNICALL ScriptMethodsSkillNamespace::grantExperiencePointsByInt(JNIEnv *env, jobject self, jlong target, jint experienceType, jint amount)
{
	UNREF(self);

	if (target == 0)
		return INT_MIN;

	// get the experience type string from ServerObjectTemplate
	const std::string & experienceTypeString = ServerObjectTemplate::getXpString(
		static_cast<ServerObjectTemplate::XpTypes>(experienceType));
	if (experienceTypeString.empty())
		return INT_MIN;

	jint result = INT_MIN;
	CachedNetworkId targetId(target);
	if (targetId.getObject() == nullptr)
	{
		if (targetId == CachedNetworkId::cms_cachedInvalid)
		{
			WARNING(true, ("JavaLibrary::grantExperiencePointsByInt called "
				"with target id = 0"));
		}
		else if (NameManager::getInstance().getPlayerName(targetId).empty())
		{
			WARNING(true, ("JavaLibrary::grantExperiencePointsByInt called "
				"with target id = %s, who is not in the player name map",
				targetId.getValueString().c_str()));
		}
//		else if (ServerUniverse::getInstance().getXpManager() != nullptr)
//		{
//			ServerUniverse::getInstance().getXpManager()->grantXp(targetId,
//				experienceTypeString, amount);
//		}
		else
		{
			// send a message to the object for when it comes online
			char buffer[1024];
			sprintf(buffer, "%s %d", experienceTypeString.c_str(), static_cast<int>(amount));
			MessageToQueue::getInstance().sendMessageToC(targetId,
				"C++experience", buffer, 1, true);
			result = amount;
		}
	}
	else
	{
		CreatureObject * creature = dynamic_cast<CreatureObject *>(targetId.getObject());
		if (creature != nullptr)
		{
			result = creature->grantExperiencePoints(experienceTypeString, static_cast<int>(amount));
		}
	}
	return result;
}	// JavaLibrary::grantExperiencePointsByInt

//-----------------------------------------------------------------------

/**
 * Returns the string experience type for an int xp type
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param xpType            int version of the xp
 *
 * @returns string version of the xp
 */
jstring JNICALL ScriptMethodsSkillNamespace::getStringExperienceType(JNIEnv *env, jobject self, jint xpType)
{
	UNREF(self);

	// get the experience type string from ServerObjectTemplate
	const std::string & experienceTypeString = ServerObjectTemplate::getXpString(static_cast<ServerObjectTemplate::XpTypes>(xpType));

	JavaString str(experienceTypeString.c_str());
	return str.getReturnValue();
}	// JavaLibrary::grantCrafterExperiencePoints

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsSkillNamespace::grantSkill(JNIEnv *env, jobject self, jlong target, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	jboolean result = JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return result;

	std::string name;
	if(!JavaLibrary::convert(localSkillName, name))
		return result;

	const SkillObject * skill = SkillManager::getInstance().getSkill(name);

	if(!skill)
		return result;

	result = creature->grantSkill(*skill);

	return result;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsSkillNamespace::hasCommand(JNIEnv *env, jobject, jlong target, jstring commandName)
{
	JavaStringParam localCommandName(commandName);

	jboolean result = JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return result;

	std::string name;
	if(!JavaLibrary::convert(localCommandName, name))
		return result;

	result = creature->hasCommand(name);

	return result;
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsSkillNamespace::hasSkill(JNIEnv *env, jobject self, jlong target, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	jboolean result = JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return result;

	std::string name;
	if(!JavaLibrary::convert(localSkillName, name))
		return result;

	const SkillObject * skill = SkillManager::getInstance().getSkill(name);

	if(!skill)
		return result;

	result = creature->hasSkill(*skill);

	return result;
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsSkillNamespace::revokeCommand(JNIEnv *env, jobject self, jlong target, jstring commandName)
{
	JavaStringParam localCommandName(commandName);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;

	std::string name;
	if(!JavaLibrary::convert(localCommandName, name))
		return;

	creature->revokeCommand(name, false);
}

//-----------------------------------------------------------------------


void JNICALL ScriptMethodsSkillNamespace::revokeSkill(JNIEnv *env, jobject self, jlong target, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;

	std::string name;
	if(!JavaLibrary::convert(localSkillName, name))
		return;

	const SkillObject * skill = SkillManager::getInstance().getSkill(name);

	if(!skill)
		return;

	LOG("CustomerService", ("Skill: Script has requested the removal of skill %s from character %s.",
		name.c_str(), creature->getNetworkId().getValueString().c_str()));
	creature->revokeSkill(*skill, false);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsSkillNamespace::revokeSkillSilent(JNIEnv *env, jobject self, jlong target, jstring skillName)
{
	JavaStringParam localSkillName(skillName);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;

	std::string name;
	if(!JavaLibrary::convert(localSkillName, name))
		return;

	const SkillObject * skill = SkillManager::getInstance().getSkill(name);

	if(!skill)
		return;

	LOG("CustomerService", ("Skill: Script has requested the removal of skill %s from character %s.",
		name.c_str(), creature->getNetworkId().getValueString().c_str()));
	creature->revokeSkill(*skill, true);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsSkillNamespace::setModValue(JNIEnv * env, jobject self, jlong target, jstring modName, jint value)
{
	CreatureObject * creature = 0;
	if(!JavaLibrary::getObject(target, creature))
		return;

	JavaStringParam lmn(modName);
	std::string m;
	if(!JavaLibrary::convert(lmn, m))
		return;
	creature->setModValue(m, value);
}

//-----------------------------------------------------------------------

/**
 * Adds a schematic group to a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature
 * @param groupName		the schematic group name
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsSkillNamespace::grantSchematicGroup(JNIEnv *env, jobject self, jlong target, jstring groupName)
{
	UNREF(self);

	if (target == 0 || groupName == 0)
		return JNI_FALSE;

	JavaStringParam groupNameParam(groupName);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	std::string group;
	if (!JavaLibrary::convert(groupNameParam, group))
		return JNI_FALSE;

	creature->grantSchematicGroup("+" + group, false);
	return JNI_TRUE;
}	// JavaLibrary::grantSchematicGroup

//-----------------------------------------------------------------------

/**
 * Removes a schematic group from a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature
 * @param groupName		the schematic group name
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsSkillNamespace::revokeSchematicGroup(JNIEnv *env, jobject self, jlong target, jstring groupName)
{
	UNREF(self);

	if (target == 0 || groupName == 0)
		return JNI_FALSE;

	JavaStringParam groupNameParam(groupName);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	std::string group;
	if (!JavaLibrary::convert(groupNameParam, group))
		return JNI_FALSE;

	creature->grantSchematicGroup("-" + group, false);
	return JNI_TRUE;
}	// JavaLibrary::revokeSchematicGroup

//-----------------------------------------------------------------------

/**
 * Adds a schematic to a creature.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param target			the creature
 * @param schematicName		the schematic name
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsSkillNamespace::grantSchematicName(JNIEnv *env, jobject self, jlong target, jstring schematicName)
{
	UNREF(self);

	if (target == 0 || schematicName == 0)
		return JNI_FALSE;

	JavaStringParam schematicNameParam(schematicName);

	std::string schematic;
	if (!JavaLibrary::convert(schematicNameParam, schematic))
		return JNI_FALSE;

	return grantSchematicCrc(env, self, target, static_cast<jint>(Crc::calculate(
		schematic.c_str())));
}	// JavaLibrary::grantSchematicName

//-----------------------------------------------------------------------

/**
 * Adds a schematic to a creature.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param target			the creature
 * @param schematicCrc		the schematic name crc
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsSkillNamespace::grantSchematicCrc(JNIEnv *env, jobject self, jlong target, jint schematicCrc)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->grantSchematic(static_cast<uint32>(schematicCrc), false);
	return JNI_TRUE;
}	// JavaLibrary::grantSchematicCrc

//-----------------------------------------------------------------------

/**
 * Removes a schematic from a creature.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param target			the creature
 * @param schematicName		the schematic name
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsSkillNamespace::revokeSchematicName(JNIEnv *env, jobject self, jlong target, jstring schematicName)
{
	UNREF(self);

	if (target == 0 || schematicName == 0)
		return JNI_FALSE;

	JavaStringParam schematicNameParam(schematicName);

	std::string schematic;
	if (!JavaLibrary::convert(schematicNameParam, schematic))
		return JNI_FALSE;

	return revokeSchematicCrc(env, self, target, static_cast<jint>(Crc::calculate(
		schematic.c_str())));
}	// JavaLibrary::revokeSchematicName

//-----------------------------------------------------------------------

/**
 * Removes a schematic from a creature.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param target			the creature
 * @param schematicCrc		the schematic name crc
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsSkillNamespace::revokeSchematicCrc(JNIEnv *env, jobject self, jlong target, jint schematicCrc)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->revokeSchematic(static_cast<uint32>(schematicCrc), false);
	return JNI_TRUE;
}	// JavaLibrary::revokeSchematicCrc

//-----------------------------------------------------------------------

/**
 * Tests if a creature has a given schematic.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param target			the creature
 * @param schematicName		the schematic name
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsSkillNamespace::hasSchematicName(JNIEnv *env, jobject self, jlong target, jstring schematicName)
{
	UNREF(self);

	if (target == 0 || schematicName == 0)
		return JNI_FALSE;

	JavaStringParam schematicNameParam(schematicName);

	std::string schematic;
	if (!JavaLibrary::convert(schematicNameParam, schematic))
		return JNI_FALSE;

	return hasSchematicCrc(env, self, target, static_cast<jint>(Crc::calculate(
		schematic.c_str())));
}	// JavaLibrary::hasSchematicName

//-----------------------------------------------------------------------

/**
 * Tests if a creature has a given schematic.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param target			the creature
 * @param schematicCrc		the schematic name crc
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsSkillNamespace::hasSchematicCrc(JNIEnv *env, jobject self, jlong target, jint schematicCrc)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (creature->hasSchematic(static_cast<uint32>(schematicCrc)))
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::hasSchematicCrc

//-----------------------------------------------------------------------

/**
 * Finds the skill mods attached to a player.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player
 *
 * @return the skill mod names the player has, or nullptr on error
 */
jobjectArray JNICALL ScriptMethodsSkillNamespace::getSkillStatModListingForPlayer(JNIEnv *env, jobject self, jlong player)
{
	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	typedef std::map<std::string, std::pair<int, int> > ModMap;
	const ModMap & mods = creature->getModMap();
	int count = mods.size();
	LocalObjectArrayRefPtr jmods = createNewObjectArray(count, JavaLibrary::getClsString());
	int i = 0;
	for (ModMap::const_iterator iter = mods.begin(); iter != mods.end(); ++iter, ++i)
	{
		JavaString mod((*iter).first);
		setObjectArrayElement(*jmods, i, mod);
	}
	return jmods->getReturnValue();
}	// JavaLibrary::getSkillStatModListingForPlayer

//-----------------------------------------------------------------------

/**
 * Finds the skill commands attached to a player
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player
 *
 * @return the commands the player has, or nullptr on error
 */
jobjectArray JNICALL ScriptMethodsSkillNamespace::getCommandListingForPlayer(JNIEnv *env, jobject self, jlong player)
{
	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	const std::map<std::string, int> & commands = creature->getCommandList();
	int count = commands.size();
	LocalObjectArrayRefPtr jcommands = createNewObjectArray(count, JavaLibrary::getClsString());
	int index = 0;
	for (std::map<std::string, int>::const_iterator i = commands.begin(); i != commands.end(); ++i)
	{
		JavaString command((*i).first);
		setObjectArrayElement(*jcommands, index++, command);
	}
	return jcommands->getReturnValue();
}	// JavaLibrary::getCommandListingForPlayer

//-----------------------------------------------------------------------

/**
 * Finds the schematics granted by a skill.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param skillName		the skill
 *
 * @return the schematic names the skill grants
 */
jintArray JNICALL ScriptMethodsSkillNamespace::getSkillSchematicsGranted(JNIEnv *env, jobject self,
	jstring skillName)
{
	JavaStringParam jskillName(skillName);
	std::string name;
	if (!JavaLibrary::convert(jskillName, name))
		return 0;

	const SkillObject * skill = SkillManager::getInstance().getSkill(name);
	if (skill == nullptr)
		return 0;

	// get all the granted schematics from the skill groups for the skill
	std::vector<jint> schematics;
	DraftSchematicGroupManager::SchematicVector drafts;
	const SkillObject::StringVector & groups = skill->getSchematicsGranted();
	for (SkillObject::StringVector::const_iterator iter = groups.begin();
		iter != groups.end(); ++iter)
	{
		drafts.clear();
		if (!DraftSchematicGroupManager::getSchematicsForGroup (*iter, drafts))
		{
			WARNING(true, ("JavaLibrary::getSkillSchematicsGranted found bad "
				"schematic group [%s]", (*iter).c_str()));
		}
		else
		{
			for (DraftSchematicGroupManager::SchematicVector::const_iterator jiter =
				drafts.begin (); jiter != drafts.end (); ++jiter)
			{
				schematics.push_back((*jiter).first);
			}
		}
	}

	// xlate the schematic names to Java
	int count = schematics.size();
	LocalIntArrayRefPtr jschematics = createNewIntArray(count);
	if (count > 0)
		setIntArrayRegion(*jschematics, 0, count, &schematics[0]);
	return jschematics->getReturnValue();
}	// JavaLibrary::getSkillSchematicsGranted

//-----------------------------------------------------------------------

/**
 * Finds the draft schematics a player has.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player
 *
 * @return the schematics' crc, or nullptr on error
 */
jintArray JNICALL ScriptMethodsSkillNamespace::getSchematicListingForPlayer(JNIEnv *env, jobject self, jlong player)
{
	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	const std::map<std::pair<uint32, uint32>,int> & schematics = creature->getDraftSchematics();
	int count = schematics.size();
	LocalIntArrayRefPtr jschematics = createNewIntArray(count);
	if (count > 0)
	{
		jint * buffer = new jint[count];
		int index = 0;
		for (std::map<std::pair<uint32, uint32>,int>::const_iterator i = schematics.begin(); i != schematics.end(); ++i)
		{
			buffer[index++] = (*i).first.first;
		}
		setIntArrayRegion(*jschematics, 0, count, buffer);
		delete[] buffer;
	}


	return jschematics->getReturnValue();
}	// JavaLibrary::getSchematicListingForPlayer

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsSkillNamespace::hasCertificationsForItem(JNIEnv *env, jobject self, jlong player, jlong item)
{
	const CreatureObject * creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject) || !creatureObject)
		return JNI_FALSE;

	Client const * const client = creatureObject->getClient();
	if(client)
	{
		if(client->isGod())
		{
			Chat::sendSystemMessage(*creatureObject, Unicode::narrowToWide("(unloclized) Bypassing certification check due to GOD mode."), Unicode::emptyString);
			return JNI_TRUE;
		}
	}

	const TangibleObject * itemObject = nullptr;
	if (!JavaLibrary::getObject(item, itemObject) || !itemObject)
		return JNI_FALSE;

	return creatureObject->hasCertificationsForItem(*itemObject);
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsSkillNamespace::getRequiredCertifications(JNIEnv *env, jobject self, jlong item)
{
	const TangibleObject * itemAsTangible = nullptr;
	if (!JavaLibrary::getObject(item, itemAsTangible) || !itemAsTangible)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true,("getRequiredCertifications called with an object that does not exist"));
		return nullptr;
	}

	std::vector<std::string> certs;
	itemAsTangible->getRequiredCertifications(certs);

	LocalObjectArrayRefPtr results;
	if (!ScriptConversion::convert(certs, results))
		return nullptr;

	return results->getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsSkillNamespace::getSkillTemplate(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject const * creature = nullptr;
	
	if (JavaLibrary::getObject(player, creature))
	{
		PlayerObject const * const player = PlayerCreatureController::getPlayerObject(creature);
		if (player)
		{
			JavaString str(player->getSkillTemplate().c_str());
			return str.getReturnValue();
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSkillNamespace::setSkillTemplate(JNIEnv *env, jobject self, jlong player, jstring skillTemplateName)
{
	CreatureObject * creature = nullptr;
	
	if (JavaLibrary::getObject(player, creature))
	{
		PlayerObject * const player = PlayerCreatureController::getPlayerObject(creature);

		if (player)
		{
			JavaStringParam const javaSkillTemplateName(skillTemplateName);
			std::string skill;
			if (JavaLibrary::convert(javaSkillTemplateName, skill))
			{
				player->setSkillTemplate(skill, false);
			}
		}
	}
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsSkillNamespace::getWorkingSkill(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject const * creature = nullptr;

	if (JavaLibrary::getObject(player, creature))
	{
		PlayerObject const * const player = PlayerCreatureController::getPlayerObject(creature);

		if (player)
		{
			JavaString str(player->getWorkingSkill().c_str());
			return str.getReturnValue();
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSkillNamespace::setWorkingSkill(JNIEnv *env, jobject self, jlong player, jstring workingSkillName)
{
	CreatureObject * creature = nullptr;

	if (JavaLibrary::getObject(player, creature))
	{
		PlayerObject * const player = PlayerCreatureController::getPlayerObject(creature);

		if (player)
		{
			JavaStringParam const javaWorkingSkillName(workingSkillName);
			std::string skill;
			if (JavaLibrary::convert(javaWorkingSkillName, skill))
			{
				player->setWorkingSkill(skill, false);
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSkillNamespace::recomputeCommandSeries(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject * creature = nullptr;

	if (JavaLibrary::getObject(player, creature))
	{
		creature->recomputeCommandSeries();
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSkillNamespace::resetExpertises(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject * creature = nullptr;

	if (JavaLibrary::getObject(player, creature))
	{
		creature->clearAllExpertises();
	}
}
// ======================================================================
