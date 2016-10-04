//========================================================================
//
// ScriptMethodsJedi.cpp - implements script methods dealing with Jedi.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "SwgGameServer/JediManagerObject.h"
#include "SwgGameServer/SwgCreatureObject.h"
#include "SwgGameServer/SwgPlayerObject.h"
#include "SwgGameServer/SwgServerUniverse.h"

using namespace JNIWrappersNamespace;


// @todo: move these functions into SwgGameServer project

// ======================================================================
// ScriptMethodsJediNamespace
// ======================================================================

namespace ScriptMethodsJediNamespace
{
	bool install();

	jint         JNICALL getMaxForcePower(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setMaxForcePower(JNIEnv *env, jobject self, jlong target, jint value);
	jboolean     JNICALL alterMaxForcePower(JNIEnv *env, jobject self, jlong target, jint delta);
	jint         JNICALL getForcePower(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setForcePower(JNIEnv *env, jobject self, jlong target, jint value);
	jboolean     JNICALL alterForcePower(JNIEnv *env, jobject self, jlong target, jint delta);
	jfloat       JNICALL getForcePowerRegenRate(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setForcePowerRegenRate(JNIEnv *env, jobject self, jlong target, jfloat rate);
	jint         JNICALL getJediState(JNIEnv *env, jobject self, jlong jedi);
	jboolean     JNICALL setJediState(JNIEnv *env, jobject self, jlong jedi, jint state);
	jboolean     JNICALL hasJediSlot(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL addJediSlot(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL isJedi(JNIEnv * env, jobject self, jlong target);
	jint         JNICALL getJediVisibility(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL setJediVisibility(JNIEnv * env, jobject self, jlong target, jint visibility);
	jboolean     JNICALL changeJediVisibility(JNIEnv * env, jobject self, jlong target, jint delta);
	jboolean     JNICALL setJediBountyValue(JNIEnv * env, jobject self, jlong target, jint bountyValue);
	jobject      JNICALL requestJedi(JNIEnv * env, jobject self, jint visibility, jint bountyValue, jint minLevel, jint maxLevel, jint hoursAlive, jint state, jint bounties);
	jobject      JNICALL requestOneJedi(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL requestJediBounty(JNIEnv * env, jobject self, jlong target, jlong hunter, jstring successCallback, jstring failCallback, jlong callbackobj);
	jboolean     JNICALL removeJediBounty(JNIEnv * env, jobject self, jlong target, jlong hunter);
	jboolean     JNICALL removeAllJediBounties(JNIEnv * env, jobject self, jlong target);
	jlongArray   JNICALL getBountyHunterBounties(JNIEnv * env, jobject self, jlong hunter);
	jlongArray   JNICALL getJediBounties(JNIEnv * env, jobject self, jlong target);
	jboolean     JNICALL isBeingHuntedByBountyHunter(JNIEnv * env, jobject self, jlong target, jlong hunter);
	void         JNICALL updateJediScriptData(JNIEnv * env, jobject self, jlong id, jstring name, jint value);
	void         JNICALL removeJediScriptData(JNIEnv * env, jobject self, jlong id, jstring name);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsJediNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsJediNamespace::c)}
	JF("_getMaxForcePower", "(J)I", getMaxForcePower),
	JF("_setMaxForcePower", "(JI)Z", setMaxForcePower),
	JF("_alterMaxForcePower", "(JI)Z", alterMaxForcePower),
	JF("_getForcePower", "(J)I", getForcePower),
	JF("_setForcePower", "(JI)Z", setForcePower),
	JF("_alterForcePower", "(JI)Z", alterForcePower),
	JF("_getForcePowerRegenRate", "(J)F", getForcePowerRegenRate),
	JF("_setForcePowerRegenRate", "(JF)Z", setForcePowerRegenRate),
	JF("_getJediState", "(J)I", getJediState),
	JF("_setJediState", "(JI)Z", setJediState),
	JF("_hasJediSlot", "(J)Z", hasJediSlot),
	JF("_addJediSlot", "(J)Z", addJediSlot),
	JF("_isJedi", "(J)Z", isJedi),
	JF("_getJediVisibility", "(J)I", getJediVisibility),
	JF("_setJediVisibility", "(JI)Z", setJediVisibility),
	JF("_changeJediVisibility", "(JI)Z", changeJediVisibility),
	JF("_setJediBountyValue", "(JI)Z", setJediBountyValue),
	JF("_requestJedi", "(IIIIIII)Lscript/dictionary;", requestJedi),
	JF("__requestJedi", "(J)Lscript/dictionary;", requestOneJedi),
	JF("_requestJediBounty", "(JJLjava/lang/String;Ljava/lang/String;J)Z", requestJediBounty),
	JF("_removeJediBounty", "(JJ)Z", removeJediBounty),
	JF("_removeAllJediBounties", "(J)Z", removeAllJediBounties),
	JF("_getJediBounties", "(J)[J", getJediBounties),
	JF("_getBountyHunterBounties", "(J)[J", getBountyHunterBounties),
	JF("_isBeingHuntedByBountyHunter", "(JJ)Z", isBeingHuntedByBountyHunter),
	JF("_updateJediScriptData", "(JLjava/lang/String;I)V", updateJediScriptData),
	JF("_removeJediScriptData", "(JLjava/lang/String;)V", removeJediScriptData),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI script callback methods
//========================================================================

/**
 * Returns the maximum force power of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature who's power to get
 *
 * @return the max force power
 */
jint JNICALL ScriptMethodsJediNamespace::getMaxForcePower(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	if (!creature->isPlayerControlled())
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return 0;

	return playerObject->getMaxForcePower();
}	// JavaLibrary::getMaxForcePower

/**
 * Sets the maximum force power of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature who's power to set
 * @param value			the new maximum force power value
 *
 * @return true on success, false if the target isn't a creature
 */
jboolean JNICALL ScriptMethodsJediNamespace::setMaxForcePower(JNIEnv *env, jobject self, jlong target, jint value)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (!creature->isPlayerControlled())
	{
		// we want to have scripts think non-player creatures can have force
		return JNI_TRUE;
	}

	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return JNI_FALSE;

	playerObject->setMaxForcePower(value);
	return JNI_TRUE;
}	// JavaLibrary::setMaxForcePower

/**
 * Changes the maximum force power of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature who's power to change
 * @param value			the value to add to the creature's maximum force power
 *
 * @return true on success, false if the target isn't a creature
 */
jboolean JNICALL ScriptMethodsJediNamespace::alterMaxForcePower(JNIEnv *env, jobject self, jlong target, jint delta)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (!creature->isPlayerControlled())
	{
		// we want to have scripts think non-player creatures can have force
		return JNI_TRUE;
	}

	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return JNI_FALSE;

	playerObject->setMaxForcePower(playerObject->getMaxForcePower() + delta);
	return JNI_TRUE;
}	// JavaLibrary::alterMaxForcePower

/**
 * Returns the force power of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature who's power to get
 *
 * @return the force power
 */
jint JNICALL ScriptMethodsJediNamespace::getForcePower(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	if (!creature->isPlayerControlled())
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return 0;

	return playerObject->getForcePower();
}	// JavaLibrary::getForcePower

/**
 * Sets the force power of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature who's power to set
 * @param value			the new force power value
 *
 * @return true on success, false if the target isn't a creature
 */
jboolean JNICALL ScriptMethodsJediNamespace::setForcePower(JNIEnv *env, jobject self, jlong target, jint value)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (!creature->isPlayerControlled())
	{
		// we want to have scripts think non-player creatures can have force
		return JNI_TRUE;
	}

	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return JNI_FALSE;

	playerObject->setForcePower(value);
	return JNI_TRUE;
}	// JavaLibrary::setForcePower

/**
 * Changes the force power of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature who's power to change
 * @param value			the value to add to the creature's force power
 *
 * @return true on success, false if the target isn't a creature
 */
jboolean JNICALL ScriptMethodsJediNamespace::alterForcePower(JNIEnv *env, jobject self, jlong target, jint delta)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (!creature->isPlayerControlled())
	{
		// we want to have scripts think non-player creatures can have force
		return JNI_TRUE;
	}

	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return JNI_FALSE;

	playerObject->setForcePower(playerObject->getForcePower() + delta);
	return JNI_TRUE;
}	// JavaLibrary::alterForcePower

/**
 * Returns the regeneration rate for the force power of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature who's regen rate to get
 *
 * @return the regen rate, in units/sec
 */
jfloat JNICALL ScriptMethodsJediNamespace::getForcePowerRegenRate(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	if (!creature->isPlayerControlled())
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return 0;

	return playerObject->getForcePowerRegenRate();
}	// JavaLibrary::getForcePowerRegenRate

/**
 * Sets the regeneration rate for the force power of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature who's regen rate to set
 * @param rate          the regen rate, in units/sec
 *
 * @return true on success, false if the target wasn't a creature
 */
jboolean JNICALL ScriptMethodsJediNamespace::setForcePowerRegenRate(JNIEnv *env, jobject self, jlong target, jfloat rate)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (!creature->isPlayerControlled())
	{
		// we want to have scripts think non-player creatures can have force
		return JNI_TRUE;
	}

	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (playerObject == nullptr)
		return JNI_FALSE;

	playerObject->setForcePowerRegenRate(rate);
	return JNI_TRUE;
}	// JavaLibrary::setForcePowerRegenRate

/**
 * Returns the state of a Jedi.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param jedi		the Jedi to get
 *
 * @return the state, or -1 on error
 */
jint JNICALL ScriptMethodsJediNamespace::getJediState(JNIEnv *env, jobject self, jlong jedi)
{
	const CreatureObject * object = nullptr;
	if (!JavaLibrary::getObject(jedi, object))
	{
		WARNING(true, ("JavaLibrary::getJediState did not find creature for id passed in"));
		return -1;
	}

	const SwgPlayerObject * player = safe_cast<const SwgPlayerObject *>(PlayerCreatureController::getPlayerObject(object));
	if (player == nullptr)
	{
		WARNING(true, ("JavaLibrary::getJediState did not find player for creature "
			"%s", object->getNetworkId().getValueString().c_str()));
		return -1;
	}

	return player->getJediState();
}	// JavaLibrary::getJediState

/**
 * Sets the state of a Jedi.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param jedi		the Jedi to set
 * @param state		the state to set
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsJediNamespace::setJediState(JNIEnv *env, jobject self, jlong jedi, jint state)
{
	CreatureObject * object = nullptr;
	if (!JavaLibrary::getObject(jedi, object))
		return JNI_FALSE;

	SwgPlayerObject * const player = safe_cast<SwgPlayerObject *>(PlayerCreatureController::getPlayerObject(object));
	if (player == nullptr)
		return JNI_FALSE;

	if (state == JS_none ||
		state == JS_forceSensitive ||
		state == JS_jedi ||
		state == JS_forceRankedLight ||
		state == JS_forceRankedDark
		)
	{
		player->setJediState(static_cast<JediState>(state));
		return JNI_TRUE;
	}

	WARNING(true, ("JavaLibrary::setJediState called for player %s with invalid "
		"state %d", object->getNetworkId().getValueString().c_str(), state));
	return JNI_FALSE;
}	// JavaLibrary::setJediState

/**
 * Returns if a player has a Jedi slot available to them.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player to test
 *
 * @return true if the player has a Jedi slot, false if not
 */
jboolean JNICALL ScriptMethodsJediNamespace::hasJediSlot(JNIEnv * env, jobject self, jlong target)
{
	const ServerObject * player = nullptr;
	if (!JavaLibrary::getObject(target, player))
		return JNI_FALSE;

	// @todo: implement function

	return JNI_FALSE;
}	// JavaLibrary::hasJediSlot

/**
 * Adds a Jedi slot to a player's available character slots.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player to test
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsJediNamespace::addJediSlot(JNIEnv * env, jobject self, jlong target)
{
	const CreatureObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	PlayerObject const * const player = PlayerCreatureController::getPlayerObject(object);
	if (player == nullptr)
		return JNI_FALSE;

	player->addJediToAccount();
	// since we won't know if this function succeeds util we get the callback,
	// set up a timed callback to re-call this function if addJediToAccountAck()
	// never gets called
	MessageToQueue::getInstance().sendMessageToC(object->getNetworkId(),
		"C++AddJediSlot", "", 5 * 60, true);

	return JNI_FALSE;
}	// JavaLibrary::addJediSlot

/**
 * Returns if a creature is a Jedi or not.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the creature to test
 *
 * @return the true if the creature is a jedi, false if not
 */
jboolean JNICALL ScriptMethodsJediNamespace::isJedi(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	const PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
		return JNI_FALSE;
	const SwgPlayerObject * jedi = safe_cast<const SwgPlayerObject *>(player);

	if (jedi->isJedi())
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::isJedi

/**
 * Returns the visibility of a Jedi.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the Jedi to get
 *
 * @return the visibility, or -1 on error
 */
jint JNICALL ScriptMethodsJediNamespace::getJediVisibility(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	const PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
		return JNI_FALSE;
	const SwgPlayerObject * jedi = safe_cast<const SwgPlayerObject *>(player);

	if (!jedi->isJedi())
		return -1;

	return jedi->getJediVisibility();
}	// JavaLibrary::getJediVisibility

/**
 * Sets the visibility of a Jedi.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the Jedi to set
 * @param visibility	the visibility to set
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsJediNamespace::setJediVisibility(JNIEnv * env, jobject self, jlong target, jint visibility)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
		return JNI_FALSE;
	SwgPlayerObject * jedi = safe_cast<SwgPlayerObject *>(player);

	if (!jedi->isJedi())
		return JNI_FALSE;

	jedi->setJediVisibility(visibility);
	return JNI_TRUE;
}	// JavaLibrary::setJediVisibility

/**
 * Changes the current visibility of a Jedi.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the Jedi to change
 * @param delta			the amount to add to the Jedi's current visibility
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsJediNamespace::changeJediVisibility(JNIEnv * env, jobject self, jlong target, jint delta)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
		return JNI_FALSE;
	SwgPlayerObject * jedi = safe_cast<SwgPlayerObject *>(player);

	if (!jedi->isJedi())
		return JNI_FALSE;

	jedi->setJediVisibility(jedi->getJediVisibility() + delta);
	return JNI_TRUE;
}	// JavaLibrary::changeJediVisibility

/**
 * Sets the bounty value of a Jedi.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the Jedi to set
 * @param bountyValue	the bounty value to set
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsJediNamespace::setJediBountyValue(JNIEnv * env, jobject self, jlong target, jint bountyValue)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;
	SwgCreatureObject const * jediCreature = safe_cast<SwgCreatureObject const *>(creature);

	PlayerObject const * player = PlayerCreatureController::getPlayerObject(creature);
	if (player == nullptr)
		return JNI_FALSE;
	SwgPlayerObject const * jediPlayer = safe_cast<SwgPlayerObject const *>(player);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return JNI_FALSE;

	jediManager->addJedi(creature->getNetworkId(), creature->getObjectName(),
		creature->getPosition_w(), creature->getSceneId(), jediPlayer->getJediVisibility(),
		bountyValue, creature->getLevel(),
		0, jediPlayer->getJediState(), jediCreature->getSpentJediSkillPoints(), creature->getPvpFaction());

	return JNI_TRUE;
}	// JavaLibrary::setJediVisibility

/**
 * Sends a request to find Jedi characters. Limits are passed in for statistics
 * that the caller is interested in; the Jedi returned will have the requested
 * value or greater for the statistic if the value given is >= 0, or less than
 * the statistic if the value given is < 0. If the statistic should be ignored,
 * IGNORE_JEDI_STAT should be passed in as the limit value.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param visibility	limit for visibility statistic
 * @param bountyValue   limit for bounty value statistic
 * @param minLevel      minimum Jedi level (inclusive)
 * @param maxLevel      maximum Jedi level (inclusive)
 * @param hoursAlive	limit for hours alive statistic
 * @param bounties		limit for number of bounties on the Jedi statistic
 * @param state         what state(s) the Jedi should have
 *
 * @return a dictionary with the Jedi data on success, nullptr on error
 */
jobject JNICALL ScriptMethodsJediNamespace::requestJedi(JNIEnv * env, jobject self, jint visibility,
	jint bountyValue, jint minLevel, jint maxLevel, jint hoursAlive, jint bounties, jint state)
{
	UNREF(self);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return 0;

	ScriptParams params;
	jediManager->getJedi(visibility, bountyValue, minLevel, maxLevel, hoursAlive, bounties, state, params);

	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(params, dictionary);
	return dictionary->getReturnValue();
}	// JavaLibrary::requestJedi

/**
 * Sends a request to find a specific Jedi.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the id of the Jedi
 *
 * @return a dictionary with the Jedi data on success, nullptr on error
 */
jobject JNICALL ScriptMethodsJediNamespace::requestOneJedi(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return 0;

	const NetworkId id(target);
	if (id == NetworkId::cms_invalid)
		return 0;

	ScriptParams params;
	jediManager->getJedi(id, params);
	if (params.getParams().size() == 0)
		return 0;

	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(params, dictionary);
	return dictionary->getReturnValue();
}	// JavaLibrary::requestJedi

/**
 * Requests to assign a bounty hunter to track down a Jedi.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param target			the Jedi to hunt
 * @param hunter			the bounty hunter
 * @param successCallback	name of a messageHandler than will be called if the
 *							bounty was assigned
 * @param failCallback		name of a messageHandler than will be called if the
 *							bounty was not assigned
 * @param callbackobj		the object that will get the callback
 *
 * @return true if the request was sent, false if there was an error
 */
jboolean JNICALL ScriptMethodsJediNamespace::requestJediBounty(JNIEnv * env, jobject self, jlong target, jlong hunter, jstring successCallback, jstring failCallback, jlong callbackobj)
{
	UNREF(self);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return JNI_FALSE;

	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return JNI_FALSE;

	const NetworkId hunterId(hunter);
	if (hunterId == NetworkId::cms_invalid)
		return JNI_FALSE;

	JavaStringParam jsuccessCallback(successCallback);
	std::string successCallbackString;
	if (!JavaLibrary::convert(jsuccessCallback, successCallbackString))
		return JNI_FALSE;

	JavaStringParam jfailCallback(failCallback);
	std::string failCallbackString;
	if (!JavaLibrary::convert(jfailCallback, failCallbackString))
		return JNI_FALSE;

	const NetworkId callbackId(callbackobj);
	if (callbackId == NetworkId::cms_invalid)
		return JNI_FALSE;

	jediManager->requestJediBounty(targetId, hunterId, successCallbackString,
		failCallbackString, callbackId);

	return JNI_TRUE;
}	// JavaLibrary::requestJediBounty

/**
 * Removes a bounty hunter from a Jedi.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the Jedi
 * @param hunter	the hunter's id
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsJediNamespace::removeJediBounty(JNIEnv * env, jobject self, jlong target, jlong hunter)
{
	UNREF(self);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return JNI_FALSE;

	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return JNI_FALSE;

	const NetworkId hunterId(hunter);
	if (hunterId == NetworkId::cms_invalid)
		return JNI_FALSE;

	jediManager->removeJediBounty(targetId, hunterId);
	return JNI_TRUE;
}	// JavaLibrary::removeJediBounty

/**
 * Removes all the bounties on a Jedi.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the Jedi
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsJediNamespace::removeAllJediBounties(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return JNI_FALSE;

	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return JNI_FALSE;

	jediManager->removeAllJediBounties(targetId);
	return JNI_TRUE;
}	// JavaLibrary::removeAllJediBounties

/**
 * Returns a list of players who are hunting a Jedi.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the Jedi
 *
 * @return an array of hunter ids, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsJediNamespace::getJediBounties(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return 0;

	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return 0;

	const std::vector<NetworkId> & hunters = jediManager->getJediBounties(targetId);

	LocalLongArrayRefPtr returnArray;
	if (!ScriptConversion::convert(hunters, returnArray))
		return 0;

	return returnArray->getReturnValue();
}	// JavaLibrary::getJediBounties

/**
* Checks to see if a bounty hunter has a bounty on a target
*
* @param target            the Jedi to hunt
* @param hunter            the bounty hunter
*
* @return true or false
*/
jboolean JNICALL ScriptMethodsJediNamespace::isBeingHuntedByBountyHunter(JNIEnv * env, jobject self, jlong target, jlong hunter)
{
	UNREF(self);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
		ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return JNI_FALSE;

	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return JNI_FALSE;

	const NetworkId hunterId(hunter);
	if (hunterId == NetworkId::cms_invalid)
		return JNI_FALSE;

	if (jediManager->hasBountyOnJedi(targetId, hunterId))
		return JNI_TRUE;

	return JNI_FALSE;
}	// JavaLibrary::isBeingHuntedByBountyHunter

/**
 * Returns a list of Jedi being hunted by the bounty hunter.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param hunter	the bounty hunter
 *
 * @return an array of jedi ids, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsJediNamespace::getBountyHunterBounties(JNIEnv * env, jobject self, jlong hunter)
{
	UNREF(self);

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return 0;

	const NetworkId hunterId(hunter);
	if (hunterId == NetworkId::cms_invalid)
		return 0;

	std::vector<NetworkId> jedis;
	jediManager->getBountyHunterBounties(hunterId, jedis);

	LocalLongArrayRefPtr returnArray;
	if (!ScriptConversion::convert(jedis, returnArray))
		return 0;

	return returnArray->getReturnValue();
}	// JavaLibrary::getBountyHunterBounties

/**
 * Adds data about a Jedi to the Jedi manager.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the Jedi
 * @param name		the data name
 * @param value		the data value
 */
void JNICALL ScriptMethodsJediNamespace::updateJediScriptData(JNIEnv * env, jobject self, jlong target, jstring name, jint value)
{
	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return;

	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return;

	std::string dataName;
	if (!JavaLibrary::convert(JavaStringParam(name), dataName))
		return;

	jediManager->updateJediScriptData(targetId, dataName, value);
}	// JavaLibrary::updateJediScriptData

/**
 * Removes data about a Jedi from the Jedi manager.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the Jedi
 * @param name		the data name
 * @param value		the data value
 */
void JNICALL ScriptMethodsJediNamespace::removeJediScriptData(JNIEnv * env, jobject self, jlong target, jstring name)
{
	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
	if (jediManager == nullptr)
		return;

	const NetworkId targetId(target);
	if (targetId == NetworkId::cms_invalid)
		return;

	std::string dataName;
	if (!JavaLibrary::convert(JavaStringParam(name), dataName))
		return;

	jediManager->removeJediScriptData(targetId, dataName);
}	// JavaLibrary::removeJediScriptData


