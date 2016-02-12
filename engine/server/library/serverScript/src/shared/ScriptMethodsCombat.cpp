//========================================================================
//
// ScriptMethodsCombat.cpp - implements script methods dealing with general
// object info.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AiLogManager.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/WeaponObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/ClientCombatManagerSupport.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "SwgGameServer/CombatEngine.h"
#include "SwgGameServer/ConfigCombatEngine.h"
#include "swgSharedUtility/CombatEngineData.h"
#include "swgSharedNetworkMessages/MessageQueueCombatAction.h"
#include "UnicodeUtils.h"

using namespace JNIWrappersNamespace;

static const std::string UNSKILLED_COMMAND("unskilled");


// ======================================================================
// ScriptMethodsCombatNamespace
// ======================================================================

namespace ScriptMethodsCombatNamespace
{
	bool install();

	jint         JNICALL getCombatDuration(JNIEnv * env, jobject self, jlong object);
	jlong        JNICALL getCurrentWeapon(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setCurrentWeapon(JNIEnv *env, jobject self, jlong target, jlong targetWeapon);
	jlong        JNICALL getHeldWeapon(JNIEnv *env, jobject self, jlong target);
	jlong        JNICALL getDefaultWeapon(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL swapDefaultWeapons(JNIEnv *env, jobject self, jlong jcreature, jlong jweapon, jlong jcontainer);
	jboolean     JNICALL isWeapon(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL isDefaultWeapon(JNIEnv *env, jobject self, jlong target);
	jfloat       JNICALL getMinRange(JNIEnv *env, jobject self, jlong weaponId);
	jfloat       JNICALL getMaxRange(JNIEnv *env, jobject self, jlong weaponId);
	jfloat       JNICALL getAverageDamage(JNIEnv *env, jobject self, jlong weaponId);
	jint         JNICALL getWeaponType(JNIEnv *env, jobject self, jlong weaponId);
	jint         JNICALL getWeaponAttackType(JNIEnv *env, jobject self, jlong weaponId);
	jint         JNICALL getWeaponDamageType(JNIEnv *env, jobject self, jlong weapon);
  	jboolean     JNICALL setWeaponDamageType(JNIEnv *env, jobject self, jlong weaponId, jint type);
	jint         JNICALL getWeaponMinDamage(JNIEnv *env, jobject self, jlong weapon);
	jboolean     JNICALL setWeaponMinDamage(JNIEnv *env, jobject self, jlong weapon, jint damage);
	jint         JNICALL getWeaponMaxDamage(JNIEnv *env, jobject self, jlong weapon);
	jboolean     JNICALL setWeaponMaxDamage(JNIEnv *env, jobject self, jlong weapon, jint damage);
	jfloat       JNICALL getWeaponAttackSpeed(JNIEnv *env, jobject self, jlong weapon);
	jboolean     JNICALL setWeaponAttackSpeed(JNIEnv *env, jobject self, jlong weapon, jfloat speed);
	jobject      JNICALL getWeaponRangeInfo(JNIEnv *env, jobject self, jlong weapon);
	jboolean     JNICALL setWeaponRangeInfo(JNIEnv *env, jobject self, jlong weapon, jobject info);
	jfloat       JNICALL getWeaponWoundChance(JNIEnv *env, jobject self, jlong weapon);
	jboolean     JNICALL setWeaponWoundChance(JNIEnv *env, jobject self, jlong weapon, jfloat chance);
	jfloat       JNICALL getWeaponDamageRadius(JNIEnv *env, jobject self, jlong weapon);
	jboolean     JNICALL setWeaponDamageRadius(JNIEnv *env, jobject self, jlong weapon, jfloat radius);
	jfloat       JNICALL getAudibleRange(JNIEnv *env, jobject self, jlong mob);
  	jint         JNICALL getWeaponAttackCost(JNIEnv *env, jobject self, jlong weaponId);
  	jboolean     JNICALL setWeaponAttackCost(JNIEnv *env, jobject self, jlong weaponId, jint cost);
  	jint         JNICALL getWeaponAccuracy(JNIEnv *env, jobject self, jlong weaponId);
  	jboolean     JNICALL setWeaponAccuracy(JNIEnv *env, jobject self, jlong weaponId, jint accuracy);
  	jint         JNICALL getWeaponElementalType(JNIEnv *env, jobject self, jlong weaponId);
  	jboolean     JNICALL setWeaponElementalType(JNIEnv *env, jobject self, jlong weaponId, jint type);
  	jint         JNICALL getWeaponElementalValue(JNIEnv *env, jobject self, jlong weaponId);
  	jboolean     JNICALL setWeaponElementalValue(JNIEnv *env, jobject self, jlong weaponId, jint value);
	jboolean     JNICALL isCombatEnabled(JNIEnv *env, jobject self);
	jboolean     JNICALL directDamage(JNIEnv *env, jobject self, jlong targetId, jint type, jint location, jint amount);
	jboolean     JNICALL directWeaponDamage(JNIEnv *env, jobject self, jlong targetId, jlong weaponId, jint amount, jint location);
	jboolean     JNICALL directAreaDamageObject(JNIEnv *env, jobject self, jlong targetId, jfloat radius, jint type, jint amount);
	jboolean     JNICALL directAreaDamageLocation(JNIEnv *env, jobject self, jobject center, jfloat radius, jint type, jint amount);
	void         JNICALL startCombat(JNIEnv *env, jobject self, jlong attacker, jlong defender);
	void         JNICALL startCombatWithAssist(JNIEnv *env, jobject self, jlong attacker, jlong defender);
	void         JNICALL stopCombat(JNIEnv *env, jobject self, jlong object);
	jboolean     JNICALL getCombatData(JNIEnv *env, jobject self, jlongArray attackers, jlongArray defenders, jobjectArray attackersData, jobjectArray defendersData, jobjectArray weaponsData);
	jboolean     JNICALL getWeaponData(JNIEnv *env, jobject self, jlong weapon, jobject weaponData);
	jboolean     JNICALL doDamage(JNIEnv *env, jobject self, jlong attacker, jlong defender, jlong weapon, jint damage, jint hitLocation);
	jboolean     JNICALL doDamageNoWeapon(JNIEnv *env, jobject self, jlong attacker, jlong defender, jint damage, jint hitLocation);
	jboolean     JNICALL doCombatResults(JNIEnv *env, jobject self, jstring animationId, jobject attackerResult, jobjectArray defenderResult);
	jboolean     JNICALL callDefenderCombatAction(JNIEnv *env, jobject self, jlongArray defenders, jintArray results, jlong attacker, jlong weapon);
	void         JNICALL setWantSawAttackTriggers(JNIEnv *env, jobject self, jlong obj, jboolean enable);
	jboolean     JNICALL addSlowDownEffect(JNIEnv *env, jobject self, jlong attacker, jlong defender, jfloat coneLength, jfloat coneAngle, jfloat slopeAngle, jfloat effectTime);
	jboolean     JNICALL removeSlowDownEffect(JNIEnv *env, jobject self, jlong attacker);
	void         JNICALL sendCombatSpam(JNIEnv *env, jobject self, jlong attacker, jlong defender, jlong weapon, jobject result, jobject attackName, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jint spamType);
	void         JNICALL sendCombatSpamWeaponString(JNIEnv *env, jobject self, jlong attacker, jlong defender, jobject weaponName, jobject result, jobject attackName, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jint spamType);
	void         JNICALL sendCombatSpamMessage(JNIEnv *env, jobject self, jlong attacker, jlong defender, jobject message, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jint spamType);
	void         JNICALL sendCombatSpamMessageCGP(JNIEnv *env, jobject self, jlong attacker, jlong defender, jobject message, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jboolean critical, jboolean glancing, jboolean proc, jint spamType);
	void         JNICALL sendCombatSpamMessageOob(JNIEnv *env, jobject self, jlong attacker, jlong defender, jstring oob, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jint spamType);
	void         JNICALL incrementKillMeter(JNIEnv *env, jobject self, jlong player, jint amount);
	jint         JNICALL getKillMeter(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL overrideDefaultAttack(JNIEnv *env, jobject self, jlong player, jstring attack);
	jboolean     JNICALL removeDefaultAttackOverride(JNIEnv *env, jobject self, jlong player);
	jstring      JNICALL getDefaultAttackOverrideActionName(JNIEnv *env, jobject self, jlong player);

	int const totalWildcards = 2;
	std::string const s_wildcardReplacements[totalWildcards] = { "medium", "light" };
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsCombatNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsCombatNamespace::c)}
	JF("_getCombatDuration", "(J)I", getCombatDuration),
	JF("_getCurrentWeapon", "(J)J", getCurrentWeapon),
	JF("_setCurrentWeapon", "(JJ)Z", setCurrentWeapon),
	JF("_getHeldWeapon", "(J)J", getHeldWeapon),
	JF("_getDefaultWeapon", "(J)J", getDefaultWeapon),
	JF("_swapDefaultWeapons", "(JJJ)Z", swapDefaultWeapons),
	JF("_isWeapon", "(J)Z", isWeapon),
	JF("_isDefaultWeapon", "(J)Z", isDefaultWeapon),
	JF("_getMinRange", "(J)F", getMinRange),
	JF("_getMaxRange", "(J)F", getMaxRange),
	JF("_getAverageDamage", "(J)F", getAverageDamage),
	JF("_getWeaponType", "(J)I", getWeaponType),
	JF("_getWeaponAttackType", "(J)I", getWeaponAttackType),
	JF("_getWeaponDamageType", "(J)I", getWeaponDamageType),
  	JF("_setWeaponDamageType", "(JI)Z", setWeaponDamageType),
	JF("_getWeaponMinDamage", "(J)I", getWeaponMinDamage),
	JF("_setWeaponMinDamage", "(JI)Z", setWeaponMinDamage),
	JF("_getWeaponMaxDamage", "(J)I", getWeaponMaxDamage),
	JF("_setWeaponMaxDamage", "(JI)Z", setWeaponMaxDamage),
	JF("_getWeaponAttackSpeed", "(J)F", getWeaponAttackSpeed),
	JF("_setWeaponAttackSpeed", "(JF)Z", setWeaponAttackSpeed),
	JF("_getWeaponRangeInfo", "(J)Lscript/base_class$range_info;", getWeaponRangeInfo),
	JF("_setWeaponRangeInfo", "(JLscript/base_class$range_info;)Z", setWeaponRangeInfo),
	JF("_getWeaponWoundChance", "(J)F", getWeaponWoundChance),
	JF("_setWeaponWoundChance", "(JF)Z", setWeaponWoundChance),
	JF("_getWeaponDamageRadius", "(J)F", getWeaponDamageRadius),
	JF("_setWeaponDamageRadius", "(JF)Z", setWeaponDamageRadius),
	JF("_getWeaponAudibleRange", "(J)F",getAudibleRange),
  	JF("_getWeaponAttackCost", "(J)I", getWeaponAttackCost),
  	JF("_setWeaponAttackCost", "(JI)Z", setWeaponAttackCost),
  	JF("_getWeaponAccuracy", "(J)I", getWeaponAccuracy),
  	JF("_setWeaponAccuracy", "(JI)Z", setWeaponAccuracy),
  	JF("_getWeaponElementalType", "(J)I", getWeaponElementalType),
  	JF("_setWeaponElementalType", "(JI)Z", setWeaponElementalType),
  	JF("_getWeaponElementalValue", "(J)I", getWeaponElementalValue),
  	JF("_setWeaponElementalValue", "(JI)Z", setWeaponElementalValue),
	JF("isCombatEnabled", "()Z", isCombatEnabled),
	JF("_damage", "(JIII)Z", directDamage),
	JF("_damage", "(JJII)Z", directWeaponDamage),
	JF("_damage", "(JFII)Z", directAreaDamageObject),
	JF("damage", "(Lscript/location;FII)Z", directAreaDamageLocation),
	JF("_startCombat", "(JJ)V", startCombat),
	JF("_startCombatWithAssist", "(JJ)V", startCombatWithAssist),
	JF("_stopCombat", "(J)V", stopCombat),
	JF("__getCombatData", "([J[J[Lscript/combat_engine$attacker_data;[Lscript/combat_engine$defender_data;[Lscript/combat_engine$weapon_data;)Z", getCombatData),
	JF("__getWeaponData", "(JLscript/combat_engine$weapon_data;)Z", getWeaponData),
	JF("__doDamage", "(JJJII)Z", doDamage),
	JF("__doDamageNoWeapon", "(JJII)Z", doDamageNoWeapon),
	JF("doCombatResults", "(Ljava/lang/String;Lscript/base_class$attacker_results;[Lscript/base_class$defender_results;)Z", doCombatResults),
	JF("_callDefenderCombatAction", "([J[IJJ)Z", callDefenderCombatAction),
	JF("_setWantSawAttackTriggers", "(JZ)V", setWantSawAttackTriggers),
	JF("_addSlowDownEffect", "(JJFFFF)Z", addSlowDownEffect),
	JF("_removeSlowDownEffect", "(J)Z", removeSlowDownEffect),
	JF("_sendCombatSpam", "(JJJLscript/combat_engine$hit_result;Lscript/string_id;ZZZI)V", sendCombatSpam),
	JF("_sendCombatSpam", "(JJLscript/string_id;Lscript/combat_engine$hit_result;Lscript/string_id;ZZZI)V", sendCombatSpamWeaponString),
	JF("_sendCombatSpamMessage", "(JJLscript/string_id;ZZZI)V", sendCombatSpamMessage),
	JF("_sendCombatSpamMessageCGP", "(JJLscript/string_id;ZZZZZZI)V", sendCombatSpamMessageCGP),
	JF("_sendCombatSpamMessageOob", "(JJLjava/lang/String;ZZZI)V", sendCombatSpamMessageOob),
	JF("_incrementKillMeter", "(JI)V", incrementKillMeter),
	JF("_getKillMeter", "(J)I", getKillMeter),
	JF("_overrideDefaultAttack", "(JLjava/lang/String;)Z", overrideDefaultAttack),
	JF("_removeDefaultAttackOverride", "(J)Z", removeDefaultAttackOverride),
	JF("_getDefaultAttackOverrideActionName", "(J)Ljava/lang/String;", getDefaultAttackOverrideActionName)
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// local utility functions
//========================================================================

/**
 * Fills in a Java combat_engine.weapon_data structure with data from a weapon.
 *
 * @param env		    Java environment
 * @param weapon		weapon to get the data from
 * @param weaponData	data to fill in
 */
void JavaLibrary::setupWeaponCombatData(JNIEnv *env, const WeaponObject * weapon,
	jobject weaponData)
{
	if (env == nullptr || weaponData == nullptr)
		return;

	if (weapon == nullptr)
	{
		env->SetObjectField(weaponData, ms_fidCombatEngineWeaponDataId, nullptr);
		return;
	}

	LocalRefParam jweaponData(weaponData);
	LocalRefPtr weaponId = getObjId(weapon->getNetworkId());

	// set up base weapon data
	setObjectField(jweaponData, ms_fidCombatEngineWeaponDataId, *weaponId);
	setIntField(jweaponData, ms_fidCombatEngineWeaponDataMinDamage, weapon->getMinDamage());
	setIntField(jweaponData, ms_fidCombatEngineWeaponDataMaxDamage, weapon->getMaxDamage());
	setIntField(jweaponData, ms_fidCombatEngineWeaponDataWeaponType, weapon->getWeaponType());
	setIntField(jweaponData, ms_fidCombatEngineWeaponDataAttackType, weapon->getAttackType());
	setIntField(jweaponData, ms_fidCombatEngineWeaponDataDamageType, weapon->getDamageType());
	setIntField(jweaponData, ms_fidCombatEngineWeaponDataElementalType, weapon->getElementalType());
	setIntField(jweaponData, ms_fidCombatEngineWeaponDataElementalValue, weapon->getElementalValue());
	setFloatField(jweaponData, ms_fidCombatEngineWeaponDataAttackSpeed, weapon->getAttackTime());
    setFloatField(jweaponData, ms_fidCombatEngineWeaponDataWoundChance, weapon->getWoundChance());
    setIntField(jweaponData, ms_fidCombatEngineWeaponDataAccuracy, weapon->getAccuracy());
	setFloatField(jweaponData, ms_fidCombatEngineWeaponDataMinRange, weapon->getMinRange());
	setFloatField(jweaponData, ms_fidCombatEngineWeaponDataMaxRange, weapon->getMaxRange());
	setFloatField(jweaponData, ms_fidCombatEngineWeaponDataDamageRadius, weapon->getDamageRadius());
	setIntField(jweaponData, ms_fidCombatEngineWeaponDataAttackCost, weapon->getAttackCost());
	setBooleanField(jweaponData, ms_fidCombatEngineWeaponDataIsDisabled, weapon->isDisabled());
}	// setupWeaponCombatData


//========================================================================
// class JavaLibrary JNI weapon callback methods
//========================================================================

jint JNICALL ScriptMethodsCombatNamespace::getCombatDuration(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	TangibleObject const * const tangibleObject = TangibleObject::getTangibleObject(networkId);
	
	if (tangibleObject == nullptr)
	{
		return 0;
	}

	return tangibleObject->getCombatDuration();
}

/**
 * Returns the current weapon a creature is using for combat.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of the creature
 *
 * @return the object id of the weapon, or nullptr on error
 */
jlong JNICALL ScriptMethodsCombatNamespace::getCurrentWeapon(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	const WeaponObject * weapon = creature->getCurrentWeapon();
	if (weapon == nullptr)
		return 0;

	return (weapon->getNetworkId()).getValue();
}	// JavaLibrary::getCurrentWeapon

/**
 * Sets the weapon a creature will use for combat.
 * @param target	    id of the object
 * @param weapon        id of the weapon
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setCurrentWeapon(JNIEnv *env, jobject self, jlong target, jlong targetWeapon)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	WeaponObject * weapon = 0;
	if (!JavaLibrary::getObject(targetWeapon, weapon))
		return JNI_FALSE;

	creature->setCurrentWeapon(*weapon);
	return JNI_TRUE;
}	// JavaLibrary::setCurrentWeapon

/**
 * Returns the current weapon a creature has equipped.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of the creature
 *
 * @return the object id of the weapon, or nullptr on error
 */
jlong JNICALL ScriptMethodsCombatNamespace::getHeldWeapon(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	const WeaponObject * weapon = creature->getReadiedWeapon();
	if (weapon == nullptr)
		return 0;

	return (weapon->getNetworkId()).getValue();
}	// JavaLibrary::getHeldWeapon

/**
 * Returns the default weapon of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of the creature
 *
 * @return the object id of the default weapon, or nullptr on error
 */
jlong JNICALL ScriptMethodsCombatNamespace::getDefaultWeapon(JNIEnv *env, jobject self, jlong target)
{
UNREF(self);

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	const WeaponObject * weapon = creature->getDefaultWeapon();
	if (weapon == nullptr)
		return 0;

	return (weapon->getNetworkId()).getValue();
}	// JavaLibrary::getDefaultWeapon

/**
 * Replaces the default weapon of a creature.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param creature    id of the creature
 * @param weapon			id of the new default weapon
 * @param container			id of the container to put the old weapon in
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::swapDefaultWeapons(JNIEnv *env, jobject, jlong jcreature, jlong jweapon, jlong jcontainer)
{
	CreatureObject *creature = 0;
	if (!JavaLibrary::getObject(jcreature, creature))
		return JNI_FALSE;

	WeaponObject *newDefaultWeapon = 0;
	if (!JavaLibrary::getObject(jweapon, newDefaultWeapon))
		return JNI_FALSE;

	ServerObject *weaponContainer = 0;
	if (!JavaLibrary::getObject(jcontainer, weaponContainer))
		return JNI_FALSE;

	if (!creature->swapDefaultWeapons(*newDefaultWeapon, *weaponContainer))
		return JNI_FALSE;

	return JNI_TRUE;
}

/**
 * Tests to see if an object is a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object we want to check
 *
 * @return true if the object is a weapon, false if not
 */
jboolean JNICALL ScriptMethodsCombatNamespace::isWeapon(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(target, weapon))
		return JNI_FALSE;
	return JNI_TRUE;
}	// JavaLibrary::isWeapon

/**
 * Tests to see if an object is a default weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object we want to check
 *
 * @return true if the object is a default weapon, false if not
 */
jboolean JNICALL ScriptMethodsCombatNamespace::isDefaultWeapon(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(target, weapon))
		return JNI_FALSE;

	const CreatureObject * owner = dynamic_cast<const CreatureObject *>(ContainerInterface::getContainedByObject(*weapon));
	if (owner == nullptr || owner->getDefaultWeapon() != weapon)
		return JNI_FALSE;

	return JNI_TRUE;
}	// JavaLibrary::isDefaultWeapon

/**
 * Returns the minimum attack range of a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId	    id of weapon
 *
 * @return the range, or -1 on error
 */
jfloat JNICALL ScriptMethodsCombatNamespace::getMinRange(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1.0f;

	jfloat range = weapon->getMinRange();
	return range;
}	// JavaLibrary::getMinRange

/**
 * Returns the maximum attack range of a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId	    id of weapon
 *
 * @return the range, or -1 on error
 */
jfloat JNICALL ScriptMethodsCombatNamespace::getMaxRange(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1.0f;

	jfloat range = weapon->getMaxRange();
	return range;
}	// JavaLibrary::getMaxRange

/**
 * Returns the average damage a weapon will do
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId	    id of weapon
 *
 * @return the damage, or 0 on error
 */
jfloat JNICALL ScriptMethodsCombatNamespace::getAverageDamage(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return 0;

	jfloat damage = weapon->getMinDamage() + (weapon->getMaxDamage() -
		weapon->getMinDamage()) / 2.0f;

	return damage;
}	// JavaLibrary::getAverageDamage

/**
 * Returns a weapon's type.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId	    id of weapon
 *
 * @return the type, or -1 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponType(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1;

	jint type = weapon->getWeaponType();
	return type;
}	// JavaLibrary::getWeaponType

/**
 * Returns a weapon's attack type.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId	    id of weapon
 *
 * @return the type, or -1 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponAttackType(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1;

	jint type = weapon->getAttackType();
	return type;
}	// JavaLibrary::getWeaponAttackType

/**
 * Gets the damage type of a weapon.
 *
 * @param weapon		the id of the weapon
 *
 * @return the damage type, or -1 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponDamageType(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1;

	jint type = weapon->getDamageType();
	return type;
}	// JavaLibrary::getWeaponDamageType

/**
 * Sets the ddamage type for a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param cost			damage type for the weapon
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponDamageType(JNIEnv *env, jobject self, jlong weaponId, jint type)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
  		return JNI_FALSE;

	weapon->setDamageType(type);

	return JNI_TRUE;
}	// JavaLibrary::setWeaponDamageType

	/**
 * Gets the minimum damage a weapon does.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the damage, or -1 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponMinDamage(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1;

	jint damage = weapon->getMinDamage();
	return damage;
}	// JavaLibrary::getWeaponMinDamage

/**
 * Sets the minimum damage a weapon does.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param damage		the minimum damage
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponMinDamage(JNIEnv *env, jobject self, jlong weaponId, jint damage)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return JNI_FALSE;

	if (damage < 0)
		return JNI_FALSE;

	weapon->setMinDamage(damage);
	return JNI_TRUE;
}	// JavaLibrary::setWeaponMinDamage

/**
 * Gets the maximum damage a weapon does.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the damage, or -1 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponMaxDamage(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1;

	jint damage = weapon->getMaxDamage();
	return damage;
}	// JavaLibrary::getWeaponMaxDamage

/**
 * Sets the maximum damage a weapon does.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param damage		the maximum damage
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponMaxDamage(JNIEnv *env, jobject self, jlong weaponId, jint damage)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return JNI_FALSE;

	if (damage < 0)
		return JNI_FALSE;

	weapon->setMaxDamage(damage);
	return JNI_TRUE;
}	// JavaLibrary::setWeaponMaxDamage

/**
 * Gets the attack speed of a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the attack speed, or -1 on error
 */
jfloat JNICALL ScriptMethodsCombatNamespace::getWeaponAttackSpeed(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1.0f;

	jfloat speed = weapon->getAttackTime();
	return speed;
}	// JavaLibrary::getWeaponAttackSpeed

/**
 * Sets the attack speed of a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param speed			the attack speed
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponAttackSpeed(JNIEnv *env, jobject self, jlong weaponId, jfloat speed)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return JNI_FALSE;

	if (speed < 0)
		return JNI_FALSE;

	weapon->setAttackTime(speed);
	return JNI_TRUE;
}	// JavaLibrary::setWeaponAttackSpeed

/**
 * Gets the % change that a weapon will cause a wound.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the wound chance, or -1 on error
 */
jfloat JNICALL ScriptMethodsCombatNamespace::getWeaponWoundChance(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1.0f;

	jfloat chance = weapon->getWoundChance();
	return chance;
}	// JavaLibrary::getWeaponWoundChance

/**
 * Gets the range info of a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the range info, or nullptr on error
 */
jobject JNICALL ScriptMethodsCombatNamespace::getWeaponRangeInfo(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return 0;

	LocalRefPtr range = allocObject(JavaLibrary::getClsBaseClassRangeInfo());
	if (range == LocalRef::cms_nullPtr)
		return 0;

	setFloatField(*range, JavaLibrary::getFidBaseClassRangeInfoMinRange(), weapon->getMinRange());
	setFloatField(*range, JavaLibrary::getFidBaseClassRangeInfoMaxRange(), weapon->getMaxRange());
	return range->getReturnValue();
}	// JavaLibrary::getWeaponRangeInfo

/**
 * Sets all the range info of a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param info			the new range info
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponRangeInfo(JNIEnv *env, jobject self, jlong weaponId, jobject info)
{
	UNREF(self);

	if (info == 0)
		return JNI_FALSE;

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return JNI_FALSE;

	weapon->setMinRange(env->GetFloatField(info, JavaLibrary::getFidBaseClassRangeInfoMinRange()));
	weapon->setMaxRange(env->GetFloatField(info, JavaLibrary::getFidBaseClassRangeInfoMaxRange()));

	return JNI_TRUE;
}	// JavaLibrary::setWeaponRangeInfo

/**
 * Sets the % change that a weapon will cause a wound.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param chance		the wound % chance
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponWoundChance(JNIEnv *env, jobject self, jlong weaponId, jfloat chance)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return JNI_FALSE;

	if (chance < 0)
		return JNI_FALSE;

	weapon->setWoundChance(chance);
	return JNI_TRUE;
}	// JavaLibrary::setWeaponWoundChance

/**
 * Gets the damage radius for area-effect weapons.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the damage radius, or -1 on error
 */
jfloat JNICALL ScriptMethodsCombatNamespace::getWeaponDamageRadius(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1.0f;

	jfloat radius = weapon->getDamageRadius();
	return radius;
}	// JavaLibrary::getWeaponDamageRadius

/**
 * Sets the damage radius for area-effect weapons.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param radius		the damage radius
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponDamageRadius(JNIEnv *env, jobject self, jlong weaponId, jfloat radius)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return JNI_FALSE;

	if (radius < 0)
		return JNI_FALSE;

	weapon->setDamageRadius(radius);
	return JNI_TRUE;
}	// JavaLibrary::setWeaponDamageRadius

/**
 *
 */
jfloat JNICALL ScriptMethodsCombatNamespace::getAudibleRange(JNIEnv *env, jobject self, jlong weaponId)
{
	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return -1;

	jfloat range = weapon->getAudibleRange();

	return range;
}

/**
 * Gets the attack cost for a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the attack costs, or 0 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponAttackCost(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return 0;

	jint cost = weapon->getAttackCost();
	return cost;

}	// JavaLibrary::getWeaponAttackCost

/**
 * Sets the attack cost for a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param cost		attack cost for the weapon
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponAttackCost(JNIEnv *env, jobject self, jlong weaponId, jint cost)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
  		return JNI_FALSE;

	weapon->setAttackCost(cost);

	return JNI_TRUE;
}	// JavaLibrary::setWeaponAttackCost

/**
 * Gets the accuracy for a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the accuracy, or 0 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponAccuracy(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return 0;

	jint accuracy = weapon->getAccuracy();
	return accuracy;

}	// JavaLibrary::getWeaponAccuracy

/**
 * Sets the accuracy for a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param accuracy		accuracy for the weapon
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponAccuracy(JNIEnv *env, jobject self, jlong weaponId, jint accuracy)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
  		return JNI_FALSE;

	weapon->setAccuracy(accuracy);

	return JNI_TRUE;
}	// JavaLibrary::setWeaponAccuracy

/**
 * Gets the elemantal damage type of a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the elemantal damage type, or 0 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponElementalType(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return 0;

	jint eleType = weapon->getElementalType();
	return eleType;

}	// JavaLibrary::getWeaponElementalType

/**
 * Sets the elemental damage type for a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param cost			elemental damage type for the weapon
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponElementalType(JNIEnv *env, jobject self, jlong weaponId, jint type)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
  		return JNI_FALSE;

	weapon->setElementalType(type);

	return JNI_TRUE;
}	// JavaLibrary::setWeaponElementalType

/**
 * Gets the elemantal damage value of a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 *
 * @return the elemantal damage value, or 0 on error
 */
jint JNICALL ScriptMethodsCombatNamespace::getWeaponElementalValue(JNIEnv *env, jobject self, jlong weaponId)
{
	UNREF(self);

	const WeaponObject *weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return 0;

	jint value = weapon->getElementalValue();
	return value;

}	// JavaLibrary::getWeaponElementalValue

/**
 * Sets the elemental damage value for a weapon.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param weaponId		the id of the weapon
 * @param cost			elemental damage value for the weapon
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::setWeaponElementalValue(JNIEnv *env, jobject self, jlong weaponId, jint value)
{
	UNREF(self);

	WeaponObject *weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
  		return JNI_FALSE;

	weapon->setElementalValue(value);

	return JNI_TRUE;
}	// JavaLibrary::setWeaponElementalValue


//========================================================================
// class JavaLibrary JNI combat callback methods
//========================================================================

/**
 * Returns if the combat engine is enabled.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return JNI_TRUE if combat is enabled, JNI_FALSE if not
 */
jboolean JNICALL ScriptMethodsCombatNamespace::isCombatEnabled(JNIEnv *env, jobject self)
{
	UNREF(env);
	UNREF(self);

	if (ConfigServerGame::getDisableCombat())
		return JNI_FALSE;
	return JNI_TRUE;
}	// JavaLibrary::isCombatEnabled


/**
 * Applies damage to an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param targetId		id of the object to damage
 * @param type			damage type being done
 * @param location		the hit location of the damage
 * @param amount		amount of damage to do
 *
 * @return the true on success, false on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::directDamage(JNIEnv *env, jobject self, jlong targetId, jint type, jint location, jint amount)
{
	UNREF(self);

	TangibleObject *target = 0;
	if (!JavaLibrary::getObject(targetId, target))
		return JNI_FALSE;

	// make sure the parameters are valid
	if (type < ServerObjectTemplate::DT_kinetic ||
		type > ServerObjectTemplate::DamageType_Last)
	{
		return JNI_FALSE;
	}

	ServerWeaponObjectTemplate::DamageType damageType = static_cast<ServerObjectTemplate::DamageType>(type);
	CombatEngine::damage(*target, damageType, static_cast<uint16>(location), amount);
	return JNI_TRUE;
}	// JavaLibrary::directDamage

/**
 * Applies weapon-based damage to an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param targetId		id of the object to damage
 * @param weaponId		id of the weapon doing the damage
 * @param amount		amount of damage to do
 * @param location		where to do the damage
 *
 * @return the true on success, false on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::directWeaponDamage(JNIEnv *env, jobject self, jlong targetId, jlong weaponId, jint amount, jint location)
{
	UNREF(self);

	TangibleObject *target = 0;
	if (!JavaLibrary::getObject(targetId, target))
		return JNI_FALSE;

	WeaponObject * weapon = 0;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return JNI_FALSE;

	CombatEngine::damage(*target, *weapon, amount, location);
	return JNI_TRUE;
}	// JavaLibrary::directWeaponDamage

/**
 * Applies damage to all objects in a given area.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param targetId		id of the object to use as the center of the damage area
 * @param radius        radius of the damage area
 * @param type			damage type being done
 * @param amount		amount of damage to do
 *
 * @return the true on success, false on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::directAreaDamageObject(JNIEnv *env, jobject self, jlong targetId, jfloat radius, jint type, jint amount)
{
	UNREF(self);

	TangibleObject *target = 0;
	if (!JavaLibrary::getObject(targetId, target))
		return JNI_FALSE;

	// make sure the damage type and subtype are valid
	if (type < ServerObjectTemplate::DT_kinetic ||
		type > ServerObjectTemplate::DamageType_Last)
	{
		return JNI_FALSE;
	}

	ServerWeaponObjectTemplate::DamageType damageType = static_cast<
		ServerWeaponObjectTemplate::DamageType>(type);
	CombatEngine::damage(target->getPosition_w(), radius, damageType, amount);

	return JNI_TRUE;
}	// JavaLibrary::directAreaDamageObject

/**
 * Applies damage to all objects in a given area.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param center		the center of the damage area
 * @param radius        radius of the damage area
 * @param type			damage type being done
 * @param amount		amount of damage to do
 *
 * @return the true on success, false on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::directAreaDamageLocation(JNIEnv *env, jobject self,
	jobject center, jfloat radius, jint type, jint amount)
{
	UNREF(self);

	if (center == 0)
		return JNI_FALSE;

	// @todo: can center.area be a different planet that the one this gameserver
	// handles?

	Vector centerPos;
	if (!ScriptConversion::convertWorld(center, centerPos))
		return JNI_FALSE;

	// make sure the damage type and subtype are valid
	if (type < ServerObjectTemplate::DT_kinetic ||
		type > ServerObjectTemplate::DamageType_Last)
	{
		return JNI_FALSE;
	}

	ServerWeaponObjectTemplate::DamageType damageType = static_cast<
		ServerWeaponObjectTemplate::DamageType>(type);
	CombatEngine::damage(centerPos, radius, damageType, amount);

	return JNI_TRUE;
}	// JavaLibrary::directAreaDamageLocation

void _startCombat(char const * const functionName, jlong attacker, jlong defender, bool const assist)
{
	NetworkId const attackerId(attacker);
	TangibleObject * const attackerTangibleObject = TangibleObject::getTangibleObject(attackerId);

	if (attackerTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsCombat::%s() Unable to resolve the attacker(%s) to a TangibleObject.", functionName, attackerId.getValueString().c_str()));
		return;
	}

	NetworkId const defenderId(defender);
	TangibleObject * const defenderTangibleObject = TangibleObject::getTangibleObject(defenderId);

	if (defenderTangibleObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsCombat::%s() attacker(%s) Unable to resolve the defender(%s) to a TangibleObject.", functionName, attackerId.getValueString().c_str(), defenderId.getValueString().c_str()));
		return;
	}

	if (attackerId == defenderId)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsCombat::%s() The attacker(%s) is trying to start combat with himself.", functionName, attackerTangibleObject->getDebugInformation().c_str()));
		return;
	}

	// Do not reset the attacker's hate list timer if the defender is already in the hate list

	if (!attackerTangibleObject->isOnHateList(defenderId))
	{
		float const hate = assist ? 1.0f : 0.0f;
		attackerTangibleObject->addHate(defenderId, hate);
	}

	// If the defender is a player, immediatelly let them know they are being attacked. This check is only for players
	// because sending an AI on an AI (pets) instantly causes both parties to enter combat. That behavior is not
	// desired because a pet could pull a AI from any distance without the pet having to attack the AI or get within
	// the AI's aggro range.

	if (   defenderTangibleObject->isPlayerControlled()
	    && !defenderTangibleObject->isOnHateList(attackerId))
	{
		float const hate = assist ? 1.0f : 0.0f;
		defenderTangibleObject->addHate(attackerId, hate);
	}
}

/**
 * Puts the attacker in combat towards the defender
 *
 * @param env               Java environment
 * @param self              class calling this function
 * @param attacker          the object you want to start combat
 * @param defender          the object you want the attacker to be in combat towards
 *
 */
void JNICALL ScriptMethodsCombatNamespace::startCombat(JNIEnv * /*env*/, jobject /*self*/, jlong attacker, jlong defender)
{
	_startCombat("startCombat", attacker, defender, false);
}

/**
 * Puts the attacker in combat towards the defender while notifying the nearby attacker's
 * friends of the defender.
 *
 * @param env               Java environment
 * @param self              class calling this function
 * @param attacker          the object you want to start combat
 * @param target            the object you want the attacker to be in combat towards
 *
 */
void JNICALL ScriptMethodsCombatNamespace::startCombatWithAssist(JNIEnv * /*env*/, jobject /*self*/, jlong attacker, jlong defender)
{
	_startCombat("startCombatWithAssist", attacker, defender, true);
}

/**
 * Removes the object from combat
 *
 * @param env               Java environment
 * @param self              class calling this function
 * @param object            the object you want to stop combat
 *
 */
void JNICALL ScriptMethodsCombatNamespace::stopCombat(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const objectId(object);
	TangibleObject * const tangibleObject = TangibleObject::getTangibleObject(objectId);

	if (tangibleObject == nullptr)
	{
		return;
	}

	tangibleObject->clearHateList();
}

// ------------------------------------------------------------------------------------

/**
 * Gets data needed for combat for given attackers and defenders.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param attackers			ids of the attackers we want data for
 * @param defenders			ids of the defenders we want data for
 * @param attackerData      data for each attacker
 * @param defenderData		data for each defender
 * @param weaponData		data for each attackers' weapon
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::getCombatData(JNIEnv *env, jobject self, jlongArray attackers, jlongArray defenders, jobjectArray attackersData, jobjectArray defendersData, jobjectArray weaponsData)
{
int i;

	UNREF(self);

	int attackerCount = 0;
	int defenderCount = 0;

	if (attackers != nullptr)
		attackerCount = env->GetArrayLength(attackers);
	if (defenders != nullptr)
		defenderCount = env->GetArrayLength(defenders);

	// get the attacker and weapon data
	for (i = 0; i < attackerCount; ++i)
	{
		jlong attackerId;
		env->GetLongArrayRegion(attackers, i, 1, &attackerId);
		LocalRefPtr attackerData = getObjectArrayElement(LocalObjectArrayRefParam(attackersData), i);
		LocalRefPtr weaponData = getObjectArrayElement(LocalObjectArrayRefParam(weaponsData), i);
		if (!attackerId)
		{
			WARNING(true, ("JavaLibrary::getCombatData got nullptr attacker id"));
			return JNI_FALSE;
		}
		if (attackerData == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("JavaLibrary::getCombatData got nullptr attacker data"));
			return JNI_FALSE;
		}
		if (weaponData == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("JavaLibrary::getCombatData got nullptr weapon data"));
			return JNI_FALSE;
		}

		// set up the attacker data
		const TangibleObject * attacker = nullptr;
		if (!JavaLibrary::getObject(attackerId, attacker))
		{
			WARNING(true, ("JavaLibrary::getCombatData cannot get attacker object"));
			return JNI_FALSE;
		}

		// get the attacker's position in local and world coordinates
		LocalRefPtr location;
		LocalRefPtr worldLocation;
		const Object * attackerCell = ContainerInterface::getContainedByObject(*attacker);
		ScriptConversion::convert(attacker->getPosition_p(), attacker->getSceneId(),
			attackerCell ? attackerCell->getNetworkId() : NetworkId::cms_invalid,
			location);
		if (location.get() == nullptr || location == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("JavaLibrary::getCombatData cannot convert local position "
				"of attacker %s to Java", attacker->getNetworkId().getValueString().c_str()));
			return JNI_FALSE;
		}
		if (attacker->isInWorldCell())
			worldLocation = location;
		else
		{
			ScriptConversion::convert(attacker->getPosition_w(), attacker->getSceneId(),
				CellProperty::getWorldCellProperty()->getOwner().getNetworkId(),
				worldLocation);
			if (worldLocation.get() == nullptr || worldLocation == LocalRef::cms_nullPtr)
			{
				WARNING(true, ("JavaLibrary::getCombatData cannot convert world position "
					"of attacker %s to Java", attacker->getNetworkId().getValueString().c_str()));
				return JNI_FALSE;
			}
		}

		const WeaponObject * weapon = nullptr;
		bool isCreature = false;
		int posture = 0;
		int locomotion = 0;
		int weaponSkill = 0;
		int aims = 0;
		const CreatureObject * creature = dynamic_cast<const CreatureObject *>(attacker);
		if (creature != nullptr)
		{
			isCreature = true;
			posture = creature->getPosture();
			locomotion = creature->getLocomotion();
			weapon = (const_cast<CreatureObject *>(creature))->getCurrentWeapon();
		}
		else
		{
			weapon = dynamic_cast<const WeaponObject *>(attacker);
		}

		CombatEngineData::CombatData const * combatData = attacker->getCombatData();

		if (combatData != nullptr)
		{
			aims = combatData->attackData.aims;
		}

		setObjectField(*attackerData, JavaLibrary::getFidCombatEngineCombatantDataPos(), *location);
		setObjectField(*attackerData, JavaLibrary::getFidCombatEngineCombatantDataWorldPos(), *worldLocation);
		setFloatField(*attackerData, JavaLibrary::getFidCombatEngineCombatantDataRadius(), attacker->getRadius());
		setBooleanField(*attackerData, JavaLibrary::getFidCombatEngineCombatantDataIsCreature(), isCreature);
		setIntField(*attackerData, JavaLibrary::getFidCombatEngineCombatantDataPosture(), posture);
		setIntField(*attackerData, JavaLibrary::getFidCombatEngineCombatantDataLocomotion(), locomotion);
		setIntField(*attackerData, JavaLibrary::getFidCombatEngineCombatantDataScriptMod(), 0);
		setIntField(*attackerData, JavaLibrary::getFidCombatEngineAttackerDataWeaponSkill(), weaponSkill);
		setIntField(*attackerData, JavaLibrary::getFidCombatEngineAttackerDataAims(), aims);

		// set up the weapon data
		JavaLibrary::setupWeaponCombatData(env, weapon, weaponData->getValue());
	}

	// get the defender data
	for (i = 0; i < defenderCount; ++i)
	{
		jlong defenderId;
		env->GetLongArrayRegion(defenders, i, 1, &defenderId);
		LocalRefPtr defenderData = getObjectArrayElement(LocalObjectArrayRefParam(defendersData), i);
		if (!defenderId)
		{
			WARNING(true, ("JavaLibrary::getCombatData got nullptr defender id"));
			return JNI_FALSE;
		}
		if (defenderData == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("JavaLibrary::getCombatData got nullptr defender data"));
			return JNI_FALSE;
		}

		// set up the defender data
		const TangibleObject * defender = nullptr;
		if (!JavaLibrary::getObject(defenderId, defender))
		{
			WARNING(true, ("JavaLibrary::getCombatData cannot get defender object"));
			return JNI_FALSE;
		}

		// get the defender's position in local and world coordinates
		LocalRefPtr location;
		LocalRefPtr worldLocation;
		const Object * defenderCell = ContainerInterface::getContainedByObject(*defender);
		ScriptConversion::convert(defender->getPosition_p(), defender->getSceneId(),
			defenderCell ? defenderCell->getNetworkId() : NetworkId::cms_invalid,
			location);
		if (location.get() == nullptr || location == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("JavaLibrary::getCombatData cannot convert local position "
				"of defender %s to Java", defender->getNetworkId().getValueString().c_str()));
			return JNI_FALSE;
		}
		if (defender->isInWorldCell())
			worldLocation = location;
		else
		{
			ScriptConversion::convert(defender->getPosition_w(), defender->getSceneId(),
				CellProperty::getWorldCellProperty()->getOwner().getNetworkId(),
				worldLocation);
			if (worldLocation.get() == nullptr || worldLocation == LocalRef::cms_nullPtr)
			{
				WARNING(true, ("JavaLibrary::getCombatData cannot convert world position "
					"of defender %s to Java", defender->getNetworkId().getValueString().c_str()));
				return JNI_FALSE;
			}
		}

		bool isCreature = false;
		int posture = 0;
		int locomotion = 0;
		int combatSkeleton = defender->getCombatSkeleton();
		int cover = 0;
		const CreatureObject * creature = dynamic_cast<const CreatureObject *>(defender);
		if (creature != nullptr)
		{
			isCreature = true;
			posture = creature->getPosture();
			locomotion = creature->getLocomotion();
			cover = creature->getCover();
		}

		// set the defender's hit location chances
		const ConfigCombatEngine::SkeletonAttackMod & attackMods = ConfigCombatEngine::getSkeletonAttackMod(static_cast<ServerTangibleObjectTemplate::CombatSkeleton>(defender->getCombatSkeleton()));
		int numBones = attackMods.numHitLocations;
		if (numBones == 0)
		{
			WARNING_STRICT_FATAL(true, ("JavaLibrary::getCombatData, defender "
				"combat skeleton %d returned 0 bones!", combatSkeleton));
			return JNI_FALSE;
		}
		LocalIntArrayRefPtr chances = createNewIntArray(numBones);
		if (chances == LocalIntArrayRef::cms_nullPtr)
			return JNI_FALSE;
		std::vector<jint> hitChances;
		for (int j = 0; j < numBones; ++j)
			hitChances.push_back(attackMods.attackMods[j].toHitChance);
		setIntArrayRegion(*chances, 0, numBones, &hitChances[0]);
		setObjectField(*defenderData, JavaLibrary::getFidCombatEngineDefenderDataHitLocationChances(), *chances);

		// set the rest of the defender data
		setObjectField(*defenderData, JavaLibrary::getFidCombatEngineCombatantDataPos(), *location);
		setObjectField(*defenderData, JavaLibrary::getFidCombatEngineCombatantDataWorldPos(), *worldLocation);
		setFloatField(*defenderData, JavaLibrary::getFidCombatEngineCombatantDataRadius(), defender->getRadius());
		setBooleanField(*defenderData, JavaLibrary::getFidCombatEngineCombatantDataIsCreature(), isCreature);
		setIntField(*defenderData, JavaLibrary::getFidCombatEngineCombatantDataPosture(), posture);
		setIntField(*defenderData, JavaLibrary::getFidCombatEngineCombatantDataLocomotion(), locomotion);
		setIntField(*defenderData, JavaLibrary::getFidCombatEngineCombatantDataScriptMod(), 0);
		setIntField(*defenderData, JavaLibrary::getFidCombatEngineDefenderDataCombatSkeleton(), combatSkeleton);
		setIntField(*defenderData, JavaLibrary::getFidCombatEngineDefenderDataCover(), cover);
	}

	return JNI_TRUE;
}	// JavaLibrary::getCombatData

// ------------------------------------------------------------------------------------

/**
 * Gets data needed for combat for a given weapon.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param weapon			the weapon to get data for
 * @param weaponData		data for the weapon
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::getWeaponData(JNIEnv *env, jobject self, jlong weapon, jobject weaponData)
{
	UNREF(self);

	if (weapon == 0 || weaponData == 0)
		return JNI_FALSE;

	const WeaponObject * localWeapon = nullptr;
	if (!JavaLibrary::getObject(weapon, localWeapon))
		return JNI_FALSE;

	JavaLibrary::setupWeaponCombatData(env, localWeapon, weaponData);
	return JNI_TRUE;
}	// JavaLibrary::getWeaponData

// ------------------------------------------------------------------------------------

/**
 * Applies damage to an object being attacked.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param attackerId	the attacker
 * @param defenderId	the defender
 * @param weaponId		the attacker's weapon
 * @param damage		amount of damage done
 * @param hitLocation	where the defender was hit
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::doDamage(JNIEnv *env, jobject self, jlong attackerId, jlong defenderId, jlong weaponId, jint damage, jint hitLocation)
{
	UNREF(self);

	if (damage <= 0)
		return JNI_TRUE;

	TangibleObject * attacker = nullptr;
	if (!JavaLibrary::getObject(attackerId, attacker))
		return JNI_FALSE;

	TangibleObject * defender = nullptr;
	if (!JavaLibrary::getObject(defenderId, defender))
		return JNI_FALSE;

	WeaponObject * weapon = nullptr;
	if (!JavaLibrary::getObject(weaponId, weapon))
		return JNI_FALSE;

	bool result = CombatEngine::onSuccessfulAttack(*attacker, *defender, *weapon,
		damage, hitLocation);
	if (result)
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::doDamage

// ------------------------------------------------------------------------------------

/**
 * Applies damage to an object being attacked.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param attackerId	the attacker
 * @param defenderId	the defender
 * @param damage		amount of damage done
 * @param hitLocation	where the defender was hit
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::doDamageNoWeapon(JNIEnv *env, jobject self, jlong attackerId, jlong defenderId, jint damage, jint hitLocation)
{
	UNREF(self);

	if (damage <= 0)
		return JNI_TRUE;

	TangibleObject * attacker = nullptr;
	if (!JavaLibrary::getObject(attackerId, attacker))
		return JNI_FALSE;

	TangibleObject * defender = nullptr;
	if (!JavaLibrary::getObject(defenderId, defender))
		return JNI_FALSE;

	bool result = CombatEngine::onSuccessfulAttack(*attacker, *defender, damage, hitLocation);
	if (result)
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::doDamage

// ------------------------------------------------------------------------------------

/**
 * Creates a package to be sent to the client for displaying combat results.
 *
 * @param env					Java environment
 * @param self					class calling this function
 * @param animationId			name of animation script to play
 * @param attackerResult		info about the attacker
 * @param defenderResult		info about the defender(s)
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::doCombatResults(JNIEnv *env, jobject self,
	jstring animationId, jobject attackerResult, jobjectArray defenderResult)
{
	UNREF(self);

	if (animationId == 0 || attackerResult == 0)
		return JNI_FALSE;

	JavaStringParam animationIdString(animationId);
	std::string animationString;
	if (!JavaLibrary::convert(animationIdString, animationString))
		return JNI_FALSE;

	unsigned int replaceWildcardIndex = animationString.find('^');

	if( replaceWildcardIndex != std::string::npos)
	{
		std::string newString = animationString.substr(0, replaceWildcardIndex);
	
		newString += s_wildcardReplacements[Random::random(0, totalWildcards-1)];

		animationString = newString;
	}
	
	if (!ClientCombatManagerSupport::doesCombatActionNameExist(TemporaryCrcString(animationString.c_str(), true)))
	{
		WARNING(true, ("doCombatResults(): caller passed in unsupported combat action name [%s]!  Continuing to send combat action.  Call stack follows in stdout.", animationString.c_str()));
		// @todo make this go to the log rather than stdout; otherwise, we're going to have to do a lot more work to see this info.
		JavaLibrary::printJavaStack();
	}

	// create a CombatAction message and fill it in
	MessageQueueCombatAction * message = new MessageQueueCombatAction();
	message->setActionId(animationString);
	// @todo: change set postures to set locomotions

	// set attacker data
	LocalRefPtr attackerId = getObjectField(LocalRefParam(attackerResult), JavaLibrary::getFidBaseClassAttackerResultsId());
	CachedNetworkId attacker(JavaLibrary::getNetworkId(*attackerId));
	LocalRefPtr weaponId = getObjectField(LocalRefParam(attackerResult), JavaLibrary::getFidBaseClassAttackerResultsWeapon());
	CachedNetworkId weapon(JavaLibrary::getNetworkId(*weaponId));
	if (attacker != CachedNetworkId::cms_invalid && attacker.getObject())
	{
		message->setAttacker(attacker, weapon);
		CreatureObject * creature = dynamic_cast<CreatureObject *>(attacker.getObject());
		if (creature != nullptr)
		{
			Postures::Enumerator posture = static_cast<Postures::Enumerator>(
				env->GetIntField(attackerResult, JavaLibrary::getFidBaseClassAttackerResultsPosture()));
			if (posture >= 0 && posture < Postures::NumberOfPostures)
			{
				if (!creature->isIncapacitated())
					creature->setPosture(posture);
				else
					posture = creature->getPosture();
			}
			else
			{
				WARNING(true, ("JavaLibrary::doCombatResults called with an invalid "
					"posture %d for attacker %s", posture, attacker.getValueString().c_str()));
			}
		}
		// Set the client trails special effects bitfield.
		message->setAttackerTrailBits(env->GetIntField(attackerResult,
			JavaLibrary::getFidBaseClassAttackerResultsTrailBits()));
		// Set the attacker client effect id.
		message->setAttackerClientEffectId(env->GetIntField(attackerResult,
			JavaLibrary::getFidBaseClassAttackerResultsClientEffectId()));

	    // Set the attacker action name
		message->setAttackerActionNameCrc(env->GetIntField(attackerResult,
			JavaLibrary::getFidBaseClassAttackerResultsActionName()));

		// Get the attacker useLocation field
		bool useLocation = env->GetBooleanField(attackerResult, 
			JavaLibrary::getFidBaseClassAttackerResultsUseLocation());
		Vector targetLocation;
		if(useLocation)
		{		
			LocalRefPtr targetLocationRef = getObjectField(LocalRefParam(attackerResult), JavaLibrary::getFidBaseClassAttackerResultsTargetLocation());
			if (!ScriptConversion::convert((*targetLocationRef).getValue(), targetLocation))
				JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::doCombatResults bad location target"));
			LocalRefPtr targetCellRef = getObjectField(LocalRefParam(attackerResult), JavaLibrary::getFidBaseClassAttackerResultsTargetCell());
			CachedNetworkId targetCell(JavaLibrary::getNetworkId(*targetCellRef));	
			message->setAttackerTargetLocation(targetLocation, targetCell, useLocation);		
		}
		else
		{
			message->setAttackerTargetLocation(targetLocation, NetworkId::cms_invalid, useLocation);
		}
	}
	else
	{
		JavaLibrary::throwInternalScriptError("JavaLibrary::doCombatResult() was passed an invalid attacker (not found on server)");
		WARNING(true, ("JavaLibrary::doCombatResources called with invalid attacker %s",attacker.getValueString().c_str()));
		delete message;
		return JNI_FALSE;
	}

	// set defender(s) data
	jsize defenderCount = 0;
	if (defenderResult != 0)
		defenderCount = env->GetArrayLength(defenderResult);
	for (jsize i = 0; i < defenderCount; ++i)
	{
		LocalRefPtr defenderData = getObjectArrayElement(LocalObjectArrayRefParam(defenderResult), i);
		if (defenderData != LocalRef::cms_nullPtr)
		{
			LocalRefPtr defenderId = getObjectField(*defenderData, JavaLibrary::getFidBaseClassDefenderResultsId());
			CachedNetworkId defender(JavaLibrary::getNetworkId(*defenderId));

			CombatEngineData::CombatDefense combatResult =
				static_cast<CombatEngineData::CombatDefense>(
					getIntField(*defenderData,
					JavaLibrary::getFidBaseClassDefenderResultsResult()));

			if (combatResult >= CombatEngineData::CD_numCombatDefense)
			{
				WARNING(true, ("doCombatResults(): caller passed in an invalid defender result [%d]!  Setting result to COMBAT_RESULT_MISS.  Call stack follows in stdout.", combatResult));
				combatResult = CombatEngineData::CD_miss;
				// @todo make this go to the log rather than stdout; otherwise, we're going to have to do a lot more work to see this info.
				JavaLibrary::printJavaStack();
			}

			const int defenderClientEffectId = getIntField(*defenderData,
				JavaLibrary::getFidBaseClassDefenderResultsClientEffectId());
			Postures::Enumerator endPosture = static_cast<Postures::Enumerator>(
				getIntField(*defenderData, JavaLibrary::getFidBaseClassDefenderResultsPosture()));

			const int defenderHitLocation = getIntField(*defenderData,
				JavaLibrary::getFidBaseClassDefenderResultsHitLocation());

			CreatureObject * creature = dynamic_cast<CreatureObject *>(
				defender.getObject());
			if (creature)
			{
				if (endPosture >= 0 && endPosture < Postures::NumberOfPostures)
				{
					if (!creature->isIncapacitated())
						creature->setPosture(endPosture);
					else
						endPosture = creature->getPosture();
				}
				else
				{
					WARNING(true, ("JavaLibrary::doCombatResults called with an "
						"invalid posture %d for defender %s", endPosture,
						defender.getValueString().c_str()));
				}
			}

			const int defenderDamageAmount = getIntField(*defenderData,
			JavaLibrary::getFidBaseClassDefenderDamageAmount());

			message->addDefender(defender, combatResult, endPosture,
				defenderClientEffectId, defenderHitLocation, defenderDamageAmount);
		}
	}

	// send the message

	Controller *const controller = attacker.getObject()->getController();
	if (controller != nullptr)
	{
		float f_hold_ms = ConfigServerGame::getCombatDamageDelaySeconds() * 1000.0f;
		if ( f_hold_ms < 1.0 )  // 0 hold time (allowing for rounding error) 
		{
			controller->appendMessage(
				CM_combatAction,
				0.0f,
				message,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_ALL_CLIENT
				);
		}
		else
		{
			MessageQueueCombatAction *held = (MessageQueueCombatAction*)controller->peekHeldMessage( CM_combatAction );
			bool b_merge = true;
			if ( held == nullptr )
				b_merge = false;
			else if ( held->getComparisonChecksum() != message->getComparisonChecksum() )
				b_merge = false;

			if ( b_merge == false )  // can't merge, so hold for later sending
			{
				controller->holdMessage( int(f_hold_ms),  CM_combatAction, 0.0f, message, 
					GameControllerMessageFlags::SEND |
       		                 	GameControllerMessageFlags::RELIABLE |
       		                 	GameControllerMessageFlags::DEST_ALL_CLIENT
					);
			}
			else  // do the merge
			{
				held->mergePacket( message );
				delete( message );
			}
		}
	}
	else
	{
		delete message;
		return JNI_FALSE;
	}

	return JNI_TRUE;
}	// JavaLibrary::doCombatResults

// ------------------------------------------------------------------------------------

/**
 * Triggers OnDefenderCombatAction for a list of defenders.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param defenders		list of defenders to be triggered
 * @param results		list of combat results for each defender
 * @param attacker		the attacker
 * @param weapon		the attacker's weapon
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsCombatNamespace::callDefenderCombatAction(JNIEnv *env, jobject self, jlongArray defenders, jintArray results, jlong attacker, jlong weapon)
{
	UNREF(self);

	if (defenders == 0 || results == 0 || attacker == 0)
		return JNI_FALSE;

	jsize defenderCount = env->GetArrayLength(defenders);
	jsize resultCount = env->GetArrayLength(results);
	if (defenderCount != resultCount)
		return JNI_FALSE;

	// get the attacker info
	CachedNetworkId attackerId(attacker);
	NetworkId weaponId;
	if (weapon != 0)
	{
		weaponId = NetworkId(weapon);
	}
	else
	{
		// use the attacker's current weapon
		CreatureObject * creatureAttacker = dynamic_cast<CreatureObject *>(attackerId.getObject());
		if (creatureAttacker != nullptr)
		{
			const WeaponObject * weaponObject = creatureAttacker->getCurrentWeapon();
			if (weaponObject != nullptr)
				weaponId = weaponObject->getNetworkId();
		}
		else
		{
			const WeaponObject * weaponAttacker = dynamic_cast<const WeaponObject *>(
				attackerId.getObject());
			if (weaponAttacker != nullptr)
				weaponId = weaponAttacker->getNetworkId();
		}
	}

	// call the trigger for each defender
	jint * resultsArray = env->GetIntArrayElements(results, nullptr);
	if (resultsArray == nullptr)
		return JNI_FALSE;

	for (jsize i = 0; i < defenderCount; ++i)
	{
		jlong defender;
		env->GetLongArrayRegion(defenders, i, 1, &defender);
		if (defender)
		{
			CachedNetworkId defenderId(defender);
			if (defenderId.getObject() != nullptr)
			{
				ServerObject * defenderObject = safe_cast<ServerObject *>(
					defenderId.getObject());
				if (defenderObject->getScriptObject() != nullptr)
				{
					ScriptParams params;
					params.addParam(attackerId);
					params.addParam(weaponId);
					params.addParam(static_cast<int>(resultsArray[i]));
					defenderObject->getScriptObject()->trigAllScripts(
						Scripting::TRIG_DEFENDER_COMBAT_ACTION, params);
				}
			}
		}
	}

	env->ReleaseIntArrayElements(results, resultsArray, JNI_ABORT);

	return JNI_TRUE;
}	// JavaLibrary::callDefenderCombatAction

// ------------------------------------------------------------------------------------

/**
 * sets whether an object is interested in OnSawAttack triggers
 *
 * @param env     Java environment
 * @param self    class calling this function
 * @param obj     the object in question
 * @param enable  whether OnSawAttack triggers should be enabled for the object
 */
void JNICALL ScriptMethodsCombatNamespace::setWantSawAttackTriggers(JNIEnv *env, jobject self, jlong obj, jboolean enable)
{
	TangibleObject * tangible = nullptr;

	if (!JavaLibrary::getObject(obj, tangible))
	{
		JavaLibrary::throwInternalScriptError("JavaLibrary::setWantSawAttackTriggers - invalid object");
	}
	else
	{
		tangible->setWantSawAttackTriggers(enable == JNI_TRUE);
		LOGC(AiLogManager::isLogging(tangible->getNetworkId()), "debug_ai", ("ScriptMethodsCombat::setWantSawAttackTriggers() object(%s) enable(%s)", tangible->getDebugInformation().c_str(), enable ? "yes" : "no"));
	}
}

// ------------------------------------------------------------------------------------

/**
 * Creates a "slow down" effect between an attacker and defender. The effect acts as if
 * there was a hill between the combatants, with the attacker at the top and the defender
 * at the bottom. The area of the effect is a cone between the attacker and defender.
 * The "hill" will be maintained between the combatants even if they move.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param attacker		who is at the top of the hill
 * @param defender		who is at the bottom of the hill
 * @param coneLength	the length of the effect cone
 * @param coneAngle		the angle of the effect cone, in degrees; 0 < angle <= 180
 * @param slopeAngle	the effective angle of the slope in degrees; 1 <= slope < 90
 * @param effectTime	how long the effect will last, in secs
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsCombatNamespace::addSlowDownEffect(JNIEnv *env, jobject self, jlong attacker, jlong defender, jfloat coneLength, jfloat coneAngle, jfloat slopeAngle, jfloat effectTime)
{
	if (coneLength <= 0)
	{
		WARNING(true, ("[script bug] JavaLibrary::addSlowDownEffect called "
			"with invalid cone length %f", coneLength));
		return JNI_FALSE;
	}
	if (coneAngle <= 0 || coneAngle > 180)
	{
		WARNING(true, ("[script bug] JavaLibrary::addSlowDownEffect called "
			"with invalid cone angle %f", coneAngle));
		return JNI_FALSE;
	}
	if (slopeAngle < 1 || slopeAngle >= 90)
	{
		WARNING(true, ("[script bug] JavaLibrary::addSlowDownEffect called "
			"with invalid slope angle %f", slopeAngle));
		return JNI_FALSE;
	}

	if (effectTime <= 0)
	{
		WARNING(true, ("[script bug] JavaLibrary::addSlowDownEffect called "
			"with invalid time %f", effectTime));
		return JNI_FALSE;
	}

	CreatureObject * attackerObject = nullptr;
	if (!JavaLibrary::getObject(attacker, attackerObject))
	{
		WARNING(true, ("[script bug] JavaLibrary::addSlowDownEffect called "
			"with invalid attacker"));
		return JNI_FALSE;
	}

	const TangibleObject * defenderObject = nullptr;
	if (!JavaLibrary::getObject(defender, defenderObject))
	{
		WARNING(true, ("[script bug] JavaLibrary::addSlowDownEffect called "
			"with invalid defender"));
		return JNI_FALSE;
	}

	unsigned long expireTime = static_cast<unsigned long>(effectTime) + ServerClock::getInstance().getGameTimeSeconds();
	attackerObject->addSlowDownEffect(*defenderObject, coneLength,
		convertDegreesToRadians(coneAngle), convertDegreesToRadians(slopeAngle),
		expireTime);
	return JNI_TRUE;
}	// JavaLibrary::addSlowDownEffect

// ------------------------------------------------------------------------------------

/**
 * Causes a slow down effect being used by an attacker to be canceled early.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param attacker		the attacker who is using the effect
 *
 * @return true on success, false on error (bad attacker)
 */
jboolean JNICALL ScriptMethodsCombatNamespace::removeSlowDownEffect(JNIEnv *env, jobject self, jlong attacker)
{
	CreatureObject * attackerObject = nullptr;
	if (!JavaLibrary::getObject(attacker, attackerObject))
	{
		WARNING(true, ("[script bug] JavaLibrary::removeSlowDownEffect called "
			"with invalid attacker"));
		return JNI_FALSE;
	}

	attackerObject->removeSlowDownEffect();
	return JNI_TRUE;
}	// JavaLibrary::removeSlowDownEffect

// ------------------------------------------------------------------------------------

/**
 * Sends standard combat spam info to players.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param attacker			who the attacker was
 * @param defender			who the defender was
 * @param weapon			what weapon was used
 * @param result			structure with the numerical data for the attack
 * @param attackName		the name of the attack
 * @param sendToAttacker	flag to send the spam to the attacker
 * @param sendToDefender	flag to send the spam to the defender
 * @param sendToBystanders	flag to send the spam to other players near the combat
 * @param spamType			flag that sets what color the spam will be show as
 */
void JNICALL ScriptMethodsCombatNamespace::sendCombatSpam(JNIEnv *env, jobject self, jlong attacker, jlong defender, jlong weapon, jobject result, jobject attackName, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jint spamType)
{
	NetworkId attackerId(attacker);
	ServerObject * attackerObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(attackerId));
	if (attackerObj == nullptr)
	{
		WARNING(true, ("WARNING: JavaLibrary::sendCombatSpam could not find attacker for id %s", attackerId.getValueString().c_str()));
		return;
	}

	NetworkId defenderId(defender);
	Object const * const defenderObj = (defenderId.isValid() ? NetworkIdManager::getObjectById(defenderId) : nullptr);

	NetworkId weaponId(weapon);
	StringId attackNameSid;
	if (!ScriptConversion::convert(attackName, attackNameSid))
		return;

	int rawDamage = 0;
	int damageType = 0;
	int elementalDamage = 0;
	int elementalDamageType = 0;
	int bleedDamage = 0;
	int critDamage = 0;
	int blockedDamage = 0;
	int finalDamage = 0;
	int hitLocation = 0;
	jboolean success = false;
	jboolean critical = false;
	jboolean glancing = false;
	jboolean crushing = false;
	jboolean strikethrough = false;
	float strikethroughAmmount = 0.0f;
	jboolean evadeResult = false;
	float evadeAmmount = 0.0f;
	jboolean blockResult = false;
	int block = 0;
	jboolean dodge = false;
	jboolean parry = false;
	jboolean proc = false;
	NetworkId armorId;

	if (result != 0)
	{
		LocalRefParam jresult(result);
		success = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultSuccess());
		critical = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultCritical());
		glancing = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultGlancing());
		proc = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultProc());

		if (success)
		{
			LocalRefPtr armor = getObjectField(jresult, JavaLibrary::getFidCombatEngineHitResultBlockingArmor());
			armorId = JavaLibrary::getNetworkId(*armor);

			rawDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultRawDamage());
			damageType = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultDamageType());
			elementalDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultElementalDamage());
			elementalDamageType = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultElementalDamageType());
			bleedDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultBleedDamage());
			critDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultCritDamage());
			blockedDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultBlockedDamage());
			finalDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultDamage());
			hitLocation = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultHitLocation());
			crushing = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultCrushing());
			strikethrough = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultStrikethrough());
			strikethroughAmmount = getFloatField(jresult, JavaLibrary::getFidCombatEngineHitResultStrikethroughAmount());
			evadeResult = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultEvadeResult());
			evadeAmmount = getFloatField(jresult, JavaLibrary::getFidCombatEngineHitResultEvadeAmount());
			blockResult = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultBlockResult());
			block = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultBlock());
		}
		else
		{
			dodge = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultDodge());
			parry = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultParry());
		}
	}

	MessageQueueCombatSpam spam(attackerId,	attackerObj->getPosition_w(), defenderId, (defenderObj ? defenderObj->getPosition_w() : Vector::zero), weaponId, armorId, attackNameSid, rawDamage, damageType, elementalDamage, elementalDamageType, bleedDamage, critDamage, blockedDamage, finalDamage, hitLocation, success, critical, glancing, crushing, strikethrough, strikethroughAmmount, evadeResult, evadeAmmount, blockResult, block, dodge, parry, proc, spamType);
	attackerObj->performCombatSpam(spam, sendToAttacker, sendToDefender, sendToBystanders);
}	// JavaLibrary::sendCombatSpam

/**
 * Sends standard combat spam info to players. The weapon is a string id instead
 * of an object.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param attacker			who the attacker was
 * @param defender			who the defender was
 * @param weaponName		the name of the weapon used
 * @param result			structure with the numerical data for the attack
 * @param attackName		the name of the attack
 * @param sendToAttacker	flag to send the spam to the attacker
 * @param sendToDefender	flag to send the spam to the defender
 * @param sendToBystanders	flag to send the spam to other players near the combat
 * @param spamType			flag that sets what color the spam will be show as
 */
void JNICALL ScriptMethodsCombatNamespace::sendCombatSpamWeaponString(JNIEnv *env, jobject self, jlong attacker, jlong defender, jobject weaponName, jobject result, jobject attackName, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jint spamType)
{
	NetworkId attackerId(attacker);
	ServerObject * attackerObj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(attackerId));
	if (attackerObj == nullptr)
	{
		WARNING(true, ("WARNING: JavaLibrary::sendCombatSpam could not find attacker for id %s", attackerId.getValueString().c_str()));
		return;
	}

	NetworkId defenderId(defender);
	Object const * const defenderObj = (defenderId.isValid() ? NetworkIdManager::getObjectById(defenderId) : nullptr);

	StringId weaponNameSid;
	if (!ScriptConversion::convert(weaponName, weaponNameSid))
		return;
	StringId attackNameSid;
	if (!ScriptConversion::convert(attackName, attackNameSid))
		return;

	int rawDamage = 0;
	int damageType = 0;
	int elementalDamage = 0;
	int elementalDamageType = 0;
	int bleedDamage = 0;
	int critDamage = 0;
	int blockedDamage = 0;
	int finalDamage = 0;
	int hitLocation = 0;
	jboolean success = false;
	jboolean critical = false;
	jboolean glancing = false;
	jboolean crushing = false;
	jboolean strikethrough = false;
	float strikethroughAmmount = 0.0f;
	jboolean evadeResult = false;
	float evadeAmmount = 0.0f;
	jboolean blockResult = false;
	int block = 0;
	jboolean dodge = false;
	jboolean parry = false;
	jboolean proc = false;
	NetworkId armorId;

	if (result != 0)
	{
		LocalRefParam jresult(result);
		success = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultSuccess());
		critical = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultCritical());
		glancing = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultGlancing());
		proc = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultProc());

		if (success)
		{
			LocalRefPtr armor = getObjectField(jresult, JavaLibrary::getFidCombatEngineHitResultBlockingArmor());
			armorId = JavaLibrary::getNetworkId(*armor);

			rawDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultRawDamage());
			damageType = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultDamageType());
			elementalDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultElementalDamage());
			elementalDamageType = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultElementalDamageType());
			bleedDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultBleedDamage());
			critDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultCritDamage());
			blockedDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultBlockedDamage());
			finalDamage = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultDamage());
			hitLocation = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultHitLocation());
			crushing = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultCrushing());
			strikethrough = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultStrikethrough());
			strikethroughAmmount = getFloatField(jresult, JavaLibrary::getFidCombatEngineHitResultStrikethroughAmount());
			evadeResult = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultEvadeResult());
			evadeAmmount = getFloatField(jresult, JavaLibrary::getFidCombatEngineHitResultEvadeAmount());
			blockResult = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultBlockResult());
			block = getIntField(jresult, JavaLibrary::getFidCombatEngineHitResultBlock());
		}
		else
		{
			dodge = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultDodge());
			parry = getBooleanField(jresult, JavaLibrary::getFidCombatEngineHitResultParry());
		}
	}

	MessageQueueCombatSpam spam(attackerId,	attackerObj->getPosition_w(), defenderId, (defenderObj ? defenderObj->getPosition_w() : Vector::zero), weaponNameSid, armorId, attackNameSid, rawDamage, damageType, elementalDamage, elementalDamageType, bleedDamage, critDamage, blockedDamage, finalDamage, hitLocation, success, critical, glancing, crushing, strikethrough, strikethroughAmmount, evadeResult, evadeAmmount, blockResult, block, dodge, parry, proc, spamType);
	attackerObj->performCombatSpam(spam, sendToAttacker, sendToDefender, sendToBystanders);
}	// JavaLibrary::sendCombatSpamWeaponString

// ------------------------------------------------------------------------------------

/**
 * Send a string id to a player in the combat spam window.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param attacker			the main player associated with the spam
 * @param defender			who the attacker is interacting with
 * @param message			the message
 * @param sendToAttacker	flag to send the message to the attacker
 * @param sendToDefender	flag to send the message to the defender
 * @param sendToBystanders	flag to send the message to anyone else in the area
 * @param spamType			flag that sets what color the spam will be show as
 */
void JNICALL ScriptMethodsCombatNamespace::sendCombatSpamMessage(JNIEnv *env, jobject self, jlong attacker, jlong defender, jobject message, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jint spamType)
{
	ServerObject * attackerObj = nullptr;
	if (!JavaLibrary::getObject(attacker, attackerObj))
		return;

	NetworkId defenderId(defender);
	Object const * const defenderObj = (defenderId.isValid() ? NetworkIdManager::getObjectById(defenderId) : nullptr);

	StringId messageSid;
	if (!ScriptConversion::convert(message, messageSid))
		return;

	MessageQueueCombatSpam spam(attackerObj->getNetworkId(), attackerObj->getPosition_w(), defenderId, (defenderObj ? defenderObj->getPosition_w() : Vector::zero), Unicode::narrowToWide("@" + messageSid.getCanonicalRepresentation()), false, false, false, spamType);
	attackerObj->performCombatSpam(spam, sendToAttacker, sendToDefender, sendToBystanders);
}	// JavaLibrary::sendCombatSpamMessage

// ------------------------------------------------------------------------------------

/**
* Send a string id to a player in the combat spam window.
*
* @param env				Java environment
* @param self				class calling this function
* @param attacker			the main player associated with the spam
* @param defender			who the attacker is interacting with
* @param message			the message
* @param sendToAttacker	flag to send the message to the attacker
* @param sendToDefender	flag to send the message to the defender
* @param sendToBystanders	flag to send the message to anyone else in the area
* @param critical			changes spam on client
* @param glancing			changes spam on client
* @param proc				changes spam on client
* @param spamType			flag that sets what color the spam will be show as
*/
void JNICALL ScriptMethodsCombatNamespace::sendCombatSpamMessageCGP(JNIEnv *env, jobject self, jlong attacker, jlong defender, jobject message, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jboolean critical, jboolean glancing, jboolean proc, jint spamType)
{
	ServerObject * attackerObj = nullptr;
	if (!JavaLibrary::getObject(attacker, attackerObj))
		return;

	NetworkId defenderId(defender);
	Object const * const defenderObj = (defenderId.isValid() ? NetworkIdManager::getObjectById(defenderId) : nullptr);

	StringId messageSid;
	if (!ScriptConversion::convert(message, messageSid))
		return;

	MessageQueueCombatSpam spam(attackerObj->getNetworkId(), attackerObj->getPosition_w(), defenderId, (defenderObj ? defenderObj->getPosition_w() : Vector::zero), Unicode::narrowToWide("@" + messageSid.getCanonicalRepresentation()), critical, glancing, proc, spamType);
	attackerObj->performCombatSpam(spam, sendToAttacker, sendToDefender, sendToBystanders);
}	// JavaLibrary::sendCombatSpamMessage

// ------------------------------------------------------------------------------------
/**
 * Send an out-of-band package to a player in the combat spam window.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param attacker			the main player associated with the spam
 * @param defender			who the attacker is interacting with
 * @param message			the message
 * @param sendToAttacker	flag to send the message to the attacker
 * @param sendToDefender	flag to send the message to the defender
 * @param sendToBystanders	flag to send the message to anyone else in the area
 * @param spamType			flag that sets what color the spam will be show as
 */
void JNICALL ScriptMethodsCombatNamespace::sendCombatSpamMessageOob(JNIEnv *env, jobject self, jlong attacker, jlong defender, jstring oob, jboolean sendToAttacker, jboolean sendToDefender, jboolean sendToBystanders, jint spamType)
{
	ServerObject * attackerObj = nullptr;
	if (!JavaLibrary::getObject(attacker, attackerObj))
		return;

	NetworkId defenderId(defender);
	Object const * const defenderObj = (defenderId.isValid() ? NetworkIdManager::getObjectById(defenderId) : nullptr);

	Unicode::String oobString;
	if (!JavaLibrary::convert(JavaStringParam(oob), oobString))
		return;

	MessageQueueCombatSpam spam(attackerObj->getNetworkId(), attackerObj->getPosition_w(), defenderId, (defenderObj ? defenderObj->getPosition_w() : Vector::zero), oobString, false, false, false, spamType);
	attackerObj->performCombatSpam(spam, sendToAttacker, sendToDefender, sendToBystanders);
}	// JavaLibrary::sendCombatSpamMessageOob


// ------------------------------------------------------------------------------------
void JNICALL ScriptMethodsCombatNamespace::incrementKillMeter(JNIEnv *env, jobject self, jlong player, jint amount )
{
	CreatureObject* playerCreature = 0;
	if (!JavaLibrary::getObject(player, playerCreature))
		return;
	if (!playerCreature->isPlayerControlled())
		return;
	
	PlayerObject* playerObject = PlayerCreatureController::getPlayerObject(playerCreature);
	if (!playerObject)
		return;

	playerObject->incrementKillMeter(amount);
}

// ------------------------------------------------------------------------------------

jint JNICALL ScriptMethodsCombatNamespace::getKillMeter(JNIEnv *env, jobject self, jlong player )
{
	CreatureObject* playerCreature = 0;
	if (!JavaLibrary::getObject(player, playerCreature))
		return 0;
	if (!playerCreature->isPlayerControlled())
		return 0;

	PlayerObject* playerObject = PlayerCreatureController::getPlayerObject(playerCreature);
	if (!playerObject)
		return 0;

	return playerObject->getKillMeter();
}

// ------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCombatNamespace::overrideDefaultAttack(JNIEnv *env, jobject self, jlong player, jstring attack)
{
	CreatureObject* playerCreature = 0;
	if (!JavaLibrary::getObject(player, playerCreature))
		return JNI_FALSE;
	if (!playerCreature->isPlayerControlled())
		return JNI_FALSE;

	PlayerObject* playerObject = PlayerCreatureController::getPlayerObject(playerCreature);
	if (!playerObject)
		return JNI_FALSE;

	JavaStringParam javaAttack(attack);
	std::string convertAttack;
	if (!JavaLibrary::convert(javaAttack, convertAttack))
		return JNI_FALSE;

	playerObject->overrideDefaultAttack(convertAttack);

	return JNI_TRUE;
}

// ------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCombatNamespace::removeDefaultAttackOverride(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject* playerCreature = 0;
	if (!JavaLibrary::getObject(player, playerCreature))
		return JNI_FALSE;
	if (!playerCreature->isPlayerControlled())
		return JNI_FALSE;

	PlayerObject* playerObject = PlayerCreatureController::getPlayerObject(playerCreature);
	if (!playerObject)
		return JNI_FALSE;

	// Setting the attack override to an empty string removes the effect on the client.
	playerObject->overrideDefaultAttack(std::string());

	return JNI_TRUE;
}

// ------------------------------------------------------------------------------------

jstring JNICALL ScriptMethodsCombatNamespace::getDefaultAttackOverrideActionName(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject* playerCreature = 0;
	if (!JavaLibrary::getObject(player, playerCreature))
		return JNI_FALSE;
	if (!playerCreature->isPlayerControlled())
		return JNI_FALSE;

	PlayerObject* playerObject = PlayerCreatureController::getPlayerObject(playerCreature);
	if (!playerObject)
		return JNI_FALSE;

	return JavaString(playerObject->getDefaultAttackOverride()).getValue();
}
// ======================================================================

