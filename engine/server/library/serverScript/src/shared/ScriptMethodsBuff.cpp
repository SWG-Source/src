//========================================================================
//
// ScriptMethodsBuff.cpp - implements script methods dealing armor.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"
#include "serverGame/BuffManager.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/ServerArmorTemplate.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/TangibleObject.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedObject/ObjectTemplateList.h"
#include "swgSharedUtility/Attributes.def"
#include "serverUtility/ServerClock.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsBuffNamespace
// ======================================================================

namespace ScriptMethodsBuffNamespace
{
	bool install();

	jboolean     JNICALL addBuff(JNIEnv *env, jobject self, jlong target, jint nameCrc);
	jboolean     JNICALL addBuffCaster(JNIEnv *env, jobject self, jlong target, jlong caster, jint nameCrc);
	jboolean     JNICALL addBuffWithDuration(JNIEnv *env, jobject self, jlong target, jint nameCrc, jfloat duration, jfloat value);
	jboolean     JNICALL addBuffWithDurationCaster(JNIEnv *env, jobject self, jlong target, jlong caster, jint nameCrc, jfloat duration, jfloat value);
    jboolean     JNICALL addBuffDurationCasterStack(JNIEnv *env, jobject self, jlong target, jlong caster, jint nameCrc, jfloat duration, jfloat value, jint count);
	jboolean     JNICALL removeBuff(JNIEnv *env, jobject self, jlong target, jint nameCrc);
	jboolean     JNICALL hasBuff(JNIEnv *env, jobject self, jlong target, jint nameCrc);
	jintArray    JNICALL getAllBuffs(JNIEnv *env, jobject self, jlong target);
	jfloat       JNICALL getBuffTimeRemaining(JNIEnv *env, jobject self, jlong target, jint nameCrc);
	jfloat       JNICALL getBuffCustomValue(JNIEnv *env, jobject self, jlong target, jint nameCrc);
	jlong        JNICALL getBuffStackCount(JNIEnv *env, jobject self, jlong target, jint nameCrc);
	jboolean     JNICALL decrementBuffStack(JNIEnv *env, jobject self, jlong target, jint nameCrc, jint stacksToRemove);
	jlong        JNICALL getBuffCaster(JNIEnv *env, jobject self, jlong target, jint nameCrc);
	jboolean     JNICALL decayBuff(JNIEnv *env, jobject self, jlong target, jint nameCrc, jfloat decayPercent);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsBuffNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsBuffNamespace::c)}
	JF("__addBuff", "(JI)Z", addBuff),
	JF("__addBuff", "(JJI)Z", addBuffCaster),
	JF("__addBuff", "(JIFF)Z", addBuffWithDuration),
	JF("__addBuff", "(JJIFF)Z", addBuffWithDurationCaster),
	JF("__addBuff", "(JJIFFI)Z", addBuffDurationCasterStack),
	JF("__removeBuff", "(JI)Z", removeBuff),
	JF("__hasBuff", "(JI)Z", hasBuff),
	JF("__getAllBuffs", "(J)[I", getAllBuffs),
	JF("__getBuffTimeRemaining", "(JI)F", getBuffTimeRemaining),
	JF("__getBuffCustomValue", "(JI)F", getBuffCustomValue),
	JF("__getBuffStackCount", "(JI)J", getBuffStackCount),
	JF("__decrementBuffStack", "(JII)Z", decrementBuffStack),
	JF("__getBuffCaster", "(JI)J", getBuffCaster),
	JF("__decayBuff", "(JIF)Z", decayBuff),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI buff callback methods
//========================================================================

/**
 * Adds a buff to a creature.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the creature to add the buff to
 * @param nameCrc	crc of the name of the buff to add
 *
 * @return false if an error occurred
 */
jboolean JNICALL ScriptMethodsBuffNamespace::addBuff(JNIEnv *env, jobject self, jlong target, jint nameCrc)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->addBuff(nameCrc);
	return JNI_TRUE;
}	// JavaLibrary::addBuff

//--------------------------------------------------------

/**
* Adds a buff to a creature.
*
* @param env		Java environment
* @param self		class calling this function
* @param target	the creature to add the buff to
* @param nameCrc	crc of the name of the buff to add
*
* @return false if an error occurred
*/
jboolean JNICALL ScriptMethodsBuffNamespace::addBuffCaster(JNIEnv *env, jobject self, jlong target, jlong caster, jint nameCrc)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;


	creature->addBuff(nameCrc, 0.0f, 0.0f, NetworkId(caster));
	return JNI_TRUE;
}	// JavaLibrary::addBuff

//--------------------------------------------------------

/**
* Adds a buff to a creature.
*
* @param env		Java environment
* @param self		class calling this function
* @param target	the creature to add the buff to
* @param nameCrc	crc of the name of the buff to add
* @param duration  float in seconds of the buff duration, or 0 as default
* @param value     a float value that replaces the first effect's strength
*
* @return false if an error occurred
*/
jboolean JNICALL ScriptMethodsBuffNamespace::addBuffWithDurationCaster(JNIEnv *env, jobject self, jlong target, jlong caster, jint nameCrc, jfloat duration, jfloat value)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->addBuff( nameCrc, duration, value, NetworkId(caster));
	return JNI_TRUE;
}

//--------------------------------------------------------

/**
* Adds a buff to a creature.
*
* @param env		Java environment
* @param self		class calling this function
* @param target	the creature to add the buff to
* @param nameCrc	crc of the name of the buff to add
* @param duration  float in seconds of the buff duration, or 0 as default
* @param value     a float value that replaces the first effect's strength
* @param count     the stack count to apply
*
* @return false if an error occurred
*/
jboolean JNICALL ScriptMethodsBuffNamespace::addBuffDurationCasterStack(JNIEnv *env, jobject self, jlong target, jlong caster, jint nameCrc, jfloat duration, jfloat value, jint count)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->addBuff( nameCrc, duration, value, NetworkId(caster), count);
	return JNI_TRUE;
}

//--------------------------------------------------------

/**
 * Adds a buff to a creature.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the creature to add the buff to
 * @param nameCrc	crc of the name of the buff to add
 * @param duration  float in seconds of the buff duration, or 0 as default
 * @param value     a float value that replaces the first effect's strength
 *
 * @return false if an error occurred
 */
jboolean JNICALL ScriptMethodsBuffNamespace::addBuffWithDuration(JNIEnv *env, jobject self, jlong target, jint nameCrc, jfloat duration, jfloat value)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->addBuff(nameCrc, duration, value);
	return JNI_TRUE;
}	// JavaLibrary::addBuffWithDuration

//--------------------------------------------------------

/**
 * Removes a buff from a creature.  Remember buffs expire naturally.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the creature to remove the buff from
 * @param nameCrc	crc of the name of the buff to remove
 *
 * @return false if an error occurred
 */
jboolean JNICALL ScriptMethodsBuffNamespace::removeBuff(JNIEnv *env, jobject self, jlong target, jint nameCrc)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->removeBuff(nameCrc);
	return JNI_TRUE;
}	// JavaLibrary::removeBuff

//--------------------------------------------------------

/**
 * Does this creature have this buff?
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the creature to check
 * @param nameCrc	crc of the name of the buff to check
 *
 * @return false if an error occurred
 */
jboolean JNICALL ScriptMethodsBuffNamespace::hasBuff(JNIEnv *env, jobject self, jlong target, jint nameCrc)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	return static_cast<jboolean>(creature->hasBuff(nameCrc) ? JNI_TRUE : JNI_FALSE);
}	// JavaLibrary::hasBuff

//--------------------------------------------------------

/**
 * Return the crcs of all the buffs the creature has
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the creature to get buffs for
 *
 * @return false if an error occurred
 */
jintArray JNICALL ScriptMethodsBuffNamespace::getAllBuffs(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	std::vector<uint32> buffCrcs;
	creature->getAllBuffs(buffCrcs);

	LocalIntArrayRefPtr valueArray = createNewIntArray(buffCrcs.size());
	if (valueArray == LocalIntArrayRef::cms_nullPtr)
		return 0;

	setIntArrayRegion(*valueArray, 0, buffCrcs.size(), reinterpret_cast<jint *>(&buffCrcs[0]) );
	return valueArray->getReturnValue();


}	// JavaLibrary::getAllBuffs

//--------------------------------------------------------

/**
 * Return the time remaining on this buff for this character
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the creature to get buff for
 * @param nameCrc   the buff crc to get info for
 *
 * @return -1 / 0 for the 2 different "timeless" buffs, otherwise return the time left in seconds.
 */
jfloat JNICALL ScriptMethodsBuffNamespace::getBuffTimeRemaining(JNIEnv *env, jobject self, jlong target, jint nameCrc)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0.0f;

	Buff b;
	if(!creature->getBuff(nameCrc, b))
		return 0.0f;
	if (creature->isPlayerControlled())
	{
		PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
		if (playerObject && playerObject->isInitialized())
		{
			uint32 currentTime = playerObject->getPlayedTime();

			if (BuffManager::getBuffIsCelestial(nameCrc))
			{
				currentTime = ServerClock::getInstance().getGameTimeSeconds();
			}

			jfloat timeLeft = 0.0f;

			if (b.m_timestamp == 0xFFFFFFFF)
			{
				timeLeft = -1.0f;
			}
			else if (b.m_timestamp == 0)
			{
				timeLeft = 0.0f;
			}
			else
			{
				timeLeft = static_cast<jfloat>( static_cast<int32>(b.m_timestamp) - static_cast<int32>(currentTime));
			}

			return timeLeft;
		}
	}
	return static_cast<jfloat>(b.m_timestamp - creature->getPseudoPlayedTime());
}	// JavaLibrary::getBuffTimeRemaining

//--------------------------------------------------------

/**
 * Return the time remaining on this buff for this character
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param target	the creature to get buff for
 * @param nameCrc   the buff crc to get info for
 *
 * @return 0 if the buff doesn't exist - time is in seconds
 */
jfloat JNICALL ScriptMethodsBuffNamespace::getBuffCustomValue(JNIEnv *env, jobject self, jlong target, jint nameCrc)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0.0f;

	Buff b;
	if(!creature->getBuff(nameCrc, b))
		return 0.0f;
	return static_cast<jfloat>(b.m_value);
}	// JavaLibrary::getBuffCustomValue

//--------------------------------------------------------

/**
* Return the current stack count of the buff
*
* @param env		Java environment
* @param self		class calling this function
* @param target	the creature to get buff for
* @param nameCrc   the buff crc to get info for
*
* @return 0 if the buff doesn't exist - otherwise return the stackcount
*/
jlong JNICALL ScriptMethodsBuffNamespace::getBuffStackCount(JNIEnv *env, jobject self, jlong target, jint nameCrc)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	Buff b;
	if(!creature->getBuff(nameCrc, b))
		return 0;

	return static_cast<jlong>(b.m_stackCount);
}	// JavaLibrary::getBuffStackCount

//--------------------------------------------------------

/**
* Return true if the buff stack was decremented
*
* @param env		Java environment
* @param self		class calling this function
* @param target	the creature to decrement
* @param nameCrc   the buff crc to get info for
*
* @return false if the buff doesn't exist - true if it succeeded
*/
jboolean JNICALL ScriptMethodsBuffNamespace::decrementBuffStack(JNIEnv *env, jobject self, jlong target, jint nameCrc, jint stacksToRemove)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	Buff b;
	if(!creature->getBuff(nameCrc, b))
		return JNI_FALSE;

	creature->decrementBuff(nameCrc, stacksToRemove);

	return JNI_TRUE;
}	// JavaLibrary::decrementBuffStack

//--------------------------------------------------------

/**
* Return the OID of the buff caster
*
* @param env		Java environment
* @param self		class calling this function
* @param target	the creature to get buff for
* @param nameCrc   the buff crc to get info for
*
* @return -1 if the buff or creature cant be found, 0 will return if no OID was supplied originally
*/
jlong JNICALL ScriptMethodsBuffNamespace::getBuffCaster(JNIEnv *env, jobject self, jlong target, jint nameCrc)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	Buff b;
	if(!creature->getBuff(nameCrc, b))
		return -1;

	return static_cast<jlong>(b.m_caster.getValue());
}	// JavaLibrary::getBuffCaster

//--------------------------------------------------------

jboolean JNICALL ScriptMethodsBuffNamespace::decayBuff(JNIEnv *env, jobject self, jlong target, jint nameCrc, jfloat decayPercent)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if(!creature->hasBuff(nameCrc))
		return JNI_FALSE;

	creature->decayBuff(nameCrc, decayPercent);

	return JNI_TRUE;
}

//--------------------------------------------------------
