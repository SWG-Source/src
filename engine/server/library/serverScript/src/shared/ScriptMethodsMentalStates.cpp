//========================================================================
//
// ScriptMethodsMentalStates.cpp - implements script methods dealing with mental states.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AiCombatPulseQueue.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedDebug/Profiler.h"
#include "sharedLog/Log.h"

using namespace JNIWrappersNamespace;


//========================================================================
// constants
//========================================================================

static const float MENTAL_STATE_ERROR = -34359738368.0f;

// ======================================================================
// ScriptMethodsMentalStatesNamespace
// ======================================================================

namespace ScriptMethodsMentalStatesNamespace
{
	bool install();

	jfloat       JNICALL getMentalState(JNIEnv *env, jobject self, jlong mob, jint attrib);
	jfloat       JNICALL getMaxMentalState(JNIEnv *env, jobject self, jlong mob, jint attrib);
	jfloat       JNICALL getMentalStateDecay(JNIEnv *env, jobject self, jlong mob, jint attrib);
	jboolean     JNICALL setMentalState(JNIEnv *env, jobject self, jlong mob, jint attrib, jfloat value);
	jboolean     JNICALL setMentalStateDecay(JNIEnv *env, jobject self, jlong mob, jint attrib, jfloat value);
	jboolean     JNICALL addToMentalState(JNIEnv *env, jobject self, jlong mob, jint attrib, jfloat value);
	jboolean     JNICALL addMentalStateModifier(JNIEnv *env, jobject self, jlong mob, jint attrib, jfloat value, jfloat duration, jfloat attackRate, jfloat decayRate);
	jobjectArray JNICALL getMentalStateModifiers(JNIEnv *env, jobject self, jlong mob, jint attrib);
	jobjectArray JNICALL getMentalStates(JNIEnv *env, jobject self, jlong mob);
	jobjectArray JNICALL getMaxMentalStates(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL setMentalStates(JNIEnv *env, jobject self, jlong mob, jobjectArray values);
	jboolean     JNICALL addToMentalStates(JNIEnv *env, jobject self, jlong mob, jobjectArray values);
	jint         JNICALL getBehavior(JNIEnv *env, jobject self, jlong mob);
	jint         JNICALL getBehaviorToward(JNIEnv *env, jobject self, jlong mob, jlong target);
	jfloat       JNICALL getMentalStateToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint attrib);
	jboolean     JNICALL setMentalStateToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint attrib, jfloat value);
	jboolean     JNICALL setMentalStateTowardClampBehavior(JNIEnv *env, jobject self, jlong mob, jlong target, jint attrib, jfloat value, jint behavior);
	jboolean     JNICALL addToMentalStateToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint attrib, jfloat value);
	jboolean     JNICALL addToMentalStateTowardClampBehavior(JNIEnv *env, jobject self, jlong mob, jlong target, jint attrib, jfloat value, jint behavior);
	jboolean     JNICALL addMentalStateModifierToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint attrib, jfloat value, jfloat duration, jfloat attackRate, jfloat decayRate);
	jobjectArray JNICALL getMentalStateModifiersToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint attrib);
	jobjectArray JNICALL getMentalStatesToward(JNIEnv *env, jobject self, jlong mob, jlong target);
	jboolean     JNICALL setMentalStatesToward(JNIEnv *env, jobject self, jlong mob, jlong target, jobjectArray values);
	jboolean     JNICALL addToMentalStatesToward(JNIEnv *env, jobject self, jlong mob, jlong target, jobjectArray values);
	jboolean     JNICALL setMentalStateTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jint attrib, jfloat value);
	jboolean     JNICALL addToMentalStateTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jint attrib, jfloat value);
	jboolean     JNICALL addMentalStateModifierTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jint attrib, jfloat value, jfloat duration, jfloat attackRate, jfloat decayRate);
	jboolean     JNICALL setMentalStatesTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jobjectArray values);
	jboolean     JNICALL addToMentalStatesTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jobjectArray values);
	void         JNICALL scheduleAiCombatPulse(JNIEnv *env, jobject self, jlong mob);
	jlongArray   JNICALL getBehaviorTargets(JNIEnv *env, jobject self, jlong mob, jint behavior);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsMentalStatesNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsMentalStatesNamespace::c)}
	JF("_getMentalState", "(JI)F", getMentalState),
	JF("_getMaxMentalState", "(JI)F", getMaxMentalState),
	JF("_getMentalStateDecay", "(JI)F", getMentalStateDecay),
	JF("_setMentalState", "(JIF)Z", setMentalState),
	JF("_setMentalStateDecay", "(JIF)Z", setMentalStateDecay),
	JF("_addToMentalState", "(JIF)Z", addToMentalState),
	JF("_addMentalStateModifier", "(JIFFFF)Z", addMentalStateModifier),
	JF("_getMentalStateModifiers", "(JI)[Lscript/mental_state_mod;", getMentalStateModifiers),
	JF("_getMentalStates", "(J)[Lscript/mental_state;", getMentalStates),
	JF("_getMaxMentalStates", "(J)[Lscript/mental_state;", getMaxMentalStates),
	JF("_setMentalStates", "(J[Lscript/mental_state;)Z", setMentalStates),
	JF("_addToMentalStates", "(J[Lscript/mental_state;)Z", addToMentalStates),
	JF("_getMentalStateToward", "(JJI)F", getMentalStateToward),
	JF("_setMentalStateToward", "(JJIF)Z", setMentalStateToward),
	JF("_setMentalStateToward", "(JJIFI)Z", setMentalStateTowardClampBehavior),
	JF("_addToMentalStateToward", "(JJIF)Z", addToMentalStateToward),
	JF("_addToMentalStateToward", "(JJIFI)Z", addToMentalStateTowardClampBehavior),
	JF("_addMentalStateModifierToward", "(JJIFFFF)Z", addMentalStateModifierToward),
	JF("_getMentalStateModifiersToward", "(JJI)[Lscript/mental_state_mod;", getMentalStateModifiersToward),
	JF("_getMentalStatesToward", "(JJ)[Lscript/mental_state;", getMentalStatesToward),
	JF("_setMentalStatesToward", "(JJ[Lscript/mental_state;)Z", setMentalStatesToward),
	JF("_addToMentalStatesToward", "(JJ[Lscript/mental_state;)Z", addToMentalStatesToward),
	JF("_setMentalStateToward", "(J[JIF)Z", setMentalStateTowardArray),
	JF("_addToMentalStateToward", "(J[JIF)Z", addToMentalStateTowardArray),
	JF("_addMentalStateModifierToward", "(J[JIFFFF)Z", addMentalStateModifierTowardArray),
	JF("_setMentalStatesToward", "(J[J[Lscript/mental_state;)Z", setMentalStatesTowardArray),
	JF("_addToMentalStatesToward", "(J[J[Lscript/mental_state;)Z", addToMentalStatesTowardArray),
	JF("_getBehaviorTargets", "(JI)[J", getBehaviorTargets),
	JF("_getBehavior", "(J)I", getBehavior),
	JF("_getBehaviorToward", "(JJ)I", getBehaviorToward),
	JF("_scheduleAiCombatPulse", "(J)V", scheduleAiCombatPulse),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI mental state callback methods
//========================================================================

/**
* Returns a creature's mental state.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param mob	    id of creature to access
* @param mental state		mental state we are interested in
*
* @return the mental state value, or MENTAL_STATE_ERROR on error
*/
jfloat JNICALL ScriptMethodsMentalStatesNamespace::getMentalState(JNIEnv *env, jobject self, jlong mob, jint mentalState)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return MENTAL_STATE_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return MENTAL_STATE_ERROR;

	return creature->getMentalState(static_cast<MentalStates::Enumerator>(mentalState));
}	// JavaLibrary::getMentalState

	/**
	* Returns a creature's max mentalState.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mentalState we are interested in
	*
	* @return the max mental state value, or MENTAL_STATE_ERROR on error
*/
jfloat JNICALL ScriptMethodsMentalStatesNamespace::getMaxMentalState(JNIEnv *env, jobject self, jlong mob, jint mentalState)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return MENTAL_STATE_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return MENTAL_STATE_ERROR;

	return creature->getMaxMentalState(static_cast<MentalStates::Enumerator>(mentalState));
}	// JavaLibrary::getMaxMentalStates

	/**
	* Returns a creature's mental state decay time.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mentalState we are interested in
	*
	* @return the max mental state value, or MENTAL_STATE_ERROR on error
*/
jfloat JNICALL ScriptMethodsMentalStatesNamespace::getMentalStateDecay(JNIEnv *env, jobject self, jlong mob, jint mentalState)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return MENTAL_STATE_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return MENTAL_STATE_ERROR;

	return creature->getMentalStateDecay(static_cast<MentalStates::Enumerator>(mentalState));
}	// JavaLibrary::getMaxMentalStates

	/**
	* Sets a creatue's mental state.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param target	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value to set the mental state to
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::setMentalState(JNIEnv *env, jobject self, jlong mob, jint mentalState, jfloat value)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	creature->setMentalState(static_cast<MentalStates::Enumerator>(mentalState),
		static_cast<MentalStates::Value>(value));
	return JNI_TRUE;
}	// JavaLibrary::setAttrib

	/**
	* Sets a creatue's mental state decay time.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value to set the mental state to
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::setMentalStateDecay(JNIEnv *env, jobject self, jlong mob, jint mentalState, jfloat value)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	creature->setMentalStateDecay(static_cast<MentalStates::Enumerator>(mentalState), value);
	return JNI_TRUE;
}	// JavaLibrary::setMentalStateDecay

	/**
	* Adds a value to a creature's mental state.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value to add to the mental state
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addToMentalState(JNIEnv *env, jobject self, jlong mob, jint mentalState, jfloat value)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	creature->setMentalState(static_cast<MentalStates::Enumerator>(mentalState),
		static_cast<MentalStates::Value>((creature->getMentalState(
		static_cast<MentalStates::Enumerator>(mentalState)) + value)));
	return JNI_TRUE;
}	// JavaLibrary::addToMentalState

	/**
	* Adds an mental state modifier to a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value of the modifier
	* @param duration		how long the modifier lasts (in sec)
	* @param attackRate	how long before the modifier reaches it's max value (in sec)
	* @param decayRate		how long before the modifier reaches 0 (in sec)
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addMentalStateModifier(JNIEnv *env, jobject self, jlong mob, jint mentalState, jfloat value, jfloat duration, jfloat attackRate, jfloat decayRate)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	// @todo: wait for implementation on CreatureObject
	UNREF(mob);
	UNREF(mentalState);
	UNREF(value);
	UNREF(duration);
	UNREF(attackRate);
	UNREF(decayRate);

	return JNI_FALSE;
}	// JavaLibrary::addMentalStateModifier

	/**
	* Returns the mental state modifiers for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	*
	* @return the mental state modifiers for the creature, or nullptr if it has none
*/
jobjectArray JNICALL ScriptMethodsMentalStatesNamespace::getMentalStateModifiers(JNIEnv *env, jobject self, jlong mob, jint mentalState)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return 0;

	UNREF(mob);
	UNREF(mentalState);
	// @todo: wait for implementation on CreatureObject
	return 0;
}	// JavaLibrary::getMentalStateModifiers

	/**
	* Returns all the mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	*
	* @return the mental states for the creature
*/
jobjectArray JNICALL ScriptMethodsMentalStatesNamespace::getMentalStates(JNIEnv *env, jobject self, jlong mob)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return 0;

	// create the array of mental states
	LocalObjectArrayRefPtr attribs = createNewObjectArray(
		MentalStates::NumberOfMentalStates, JavaLibrary::getClsMentalState());
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	// set the array elements
	for (int i = 0; i < MentalStates::NumberOfMentalStates; ++i)
	{
		LocalRefPtr mentalState = createNewObject(JavaLibrary::getClsMentalState(), JavaLibrary::getMidMentalState(),
			i, creature->getMentalState(static_cast<MentalStates::Enumerator>(i)));
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*attribs, i, *mentalState);
	}
	return attribs->getReturnValue();
}	// JavaLibrary::getMentalStates

	/**
	* Returns all the max mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	*
	* @return the max mental states for the creature
*/
jobjectArray JNICALL ScriptMethodsMentalStatesNamespace::getMaxMentalStates(JNIEnv *env, jobject self, jlong mob)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return 0;

	// create the array of mental states
	LocalObjectArrayRefPtr attribs = createNewObjectArray(
		MentalStates::NumberOfMentalStates, JavaLibrary::getClsMentalState());
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	// set the array elements
	for (int i = 0; i < MentalStates::NumberOfMentalStates; ++i)
	{
		LocalRefPtr mentalState = createNewObject(JavaLibrary::getClsMentalState(), JavaLibrary::getMidMentalState(),
			i, creature->getMaxMentalState(static_cast<MentalStates::Enumerator>(i)));
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*attribs, i, *mentalState);
	}
	return attribs->getReturnValue();
}	// JavaLibrary::getMaxMentalStates

	/**
	* Sets the mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param values		mental state/value pairs to set
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::setMentalStates(JNIEnv *env, jobject self, jlong mob, jobjectArray values)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr mentalState = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (mentalState == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		int type = getIntField(*mentalState, JavaLibrary::getFidMentalStateType());
		if (type < 0 || type >= MentalStates::NumberOfMentalStates)
			return JNI_FALSE;
		creature->setMentalState(static_cast<MentalStates::Enumerator>(type),
			static_cast<MentalStates::Value>(getFloatField(*mentalState,
			JavaLibrary::getFidMentalStateValue())));
	}
	return JNI_TRUE;
}	// JavaLibrary::setMentalStates

	/**
	* Adds values to the mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param values		mental state/value pairs to add
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addToMentalStates(JNIEnv *env, jobject self, jlong mob, jobjectArray values)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr mentalState = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (mentalState == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		int type = getIntField(*mentalState, JavaLibrary::getFidMentalStateType());
		if (type < 0 || type >= MentalStates::NumberOfMentalStates)
			return JNI_FALSE;
		MentalStates::Value value = creature->getMentalState(static_cast<MentalStates::Enumerator>(type));
		value += getFloatField(*mentalState, JavaLibrary::getFidMentalStateValue());
		creature->setMentalState(static_cast<MentalStates::Enumerator>(type),
			static_cast<MentalStates::Value>(value));
	}
	return JNI_TRUE;
}	// JavaLibrary::addToMentalStates

//---------

/**
* Returns a creature's mental state.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param mob	    id of creature to access
* @param mental state		mental state we are interested in
*
* @return the mental state value, or MENTAL_STATE_ERROR on error
*/
jfloat JNICALL ScriptMethodsMentalStatesNamespace::getMentalStateToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint mentalState)
{
	UNREF(self);

	if (target == 0 || mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return MENTAL_STATE_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return MENTAL_STATE_ERROR;

	NetworkId targetId(target);
	if (!targetId)
		return MENTAL_STATE_ERROR;

	return creature->getMentalStateToward(targetId, static_cast<MentalStates::Enumerator>(mentalState));
}	// JavaLibrary::getMentalState

	/**
	* Sets a creatue's mental state.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param target	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value to set the mental state to
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::setMentalStateToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint mentalState, jfloat value)
{
	PROFILER_AUTO_BLOCK_DEFINE("setMentalStateToward");
	UNREF(self);

	if (target == 0 || mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	NetworkId targetId(target);
	if (!targetId)
		return JNI_FALSE;

	creature->setMentalStateToward(targetId,
		static_cast<MentalStates::Enumerator>(mentalState),
		static_cast<MentalStates::Value>(value));

	return JNI_TRUE;
}	// JavaLibrary::setAttrib

	/**
	* Sets a creatue's mental state.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param target	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value to set the mental state to
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::setMentalStateTowardClampBehavior(JNIEnv *env, jobject self, jlong mob, jlong target, jint mentalState, jfloat value, jint behavior)
{
	UNREF(self);

	if (target == 0 || mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	NetworkId targetId(target);
	if (!targetId)
		return JNI_FALSE;

	creature->setMentalStateTowardClampBehavior(targetId,
		static_cast<MentalStates::Enumerator>(mentalState),
		static_cast<MentalStates::Value>(value),
		static_cast<Behaviors::Enumerator>(behavior));

	return JNI_TRUE;
}	// JavaLibrary::setAttrib

	/**
	* Adds a value to a creature's mental state.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value to add to the mental state
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addToMentalStateToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint mentalState, jfloat value)
{
	UNREF(self);

	if (target == 0 || mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
	{
		LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsMentalStates::addToMentalStateToward() error 1"));
		return JNI_FALSE;
	}

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
	{
		LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsMentalStates::addToMentalStateToward() error 2"));
		return JNI_FALSE;
	}

	NetworkId targetId(target);
	if (!targetId)
	{
		LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsMentalStates::addToMentalStateToward() error 3"));
		return JNI_FALSE;
	}

	MentalStates::Value current = creature->getMentalStateToward(targetId, static_cast<MentalStates::Enumerator>(mentalState));

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsMentalStates::addToMentalStateToward() %s->%s state(%s) value(%.0f+%.0f)", creature->getNetworkId().getValueString().c_str(), targetId.getValueString().c_str(), CreatureObject::getMentalStateString(mentalState), current, value));

	creature->setMentalStateToward(targetId, static_cast<MentalStates::Enumerator>(mentalState), static_cast<MentalStates::Value>(current + value));

	return JNI_TRUE;
}	// JavaLibrary::addToMentalState

	/**
	* Adds a value to a creature's mental state.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value to add to the mental state
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addToMentalStateTowardClampBehavior(JNIEnv *env, jobject self, jlong mob, jlong target, jint mentalState, jfloat value, jint behavior)
{
	UNREF(self);

	if (target == 0 || mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	NetworkId targetId(target);
	if (!targetId)
		return JNI_FALSE;

	MentalStates::Value current = creature->getMentalStateToward(targetId, static_cast<MentalStates::Enumerator>(mentalState));

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsMentalStates::addToMentalStateTowardClampBehavior() %s->%s state(%s) value(%.0f+%.0f) behavior(%s)", creature->getNetworkId().getValueString().c_str(), targetId.getValueString().c_str(), CreatureObject::getMentalStateString(mentalState), current, value, CreatureObject::getBehaviorString(behavior)));

	creature->setMentalStateTowardClampBehavior(targetId,
		static_cast<MentalStates::Enumerator>(mentalState),
		static_cast<MentalStates::Value>(current + value),
		static_cast<Behaviors::Enumerator>(behavior));

	return JNI_TRUE;
}	// JavaLibrary::addToMentalState

	/**
	* Adds an mental state modifier to a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value of the modifier
	* @param duration		how long the modifier lasts (in sec)
	* @param attackRate	how long before the modifier reaches it's max value (in sec)
	* @param decayRate		how long before the modifier reaches 0 (in sec)
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addMentalStateModifierToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint mentalState, jfloat value, jfloat duration, jfloat attackRate, jfloat decayRate)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	// @todo: wait for implementation on CreatureObject
	UNREF(mob);
	UNREF(mentalState);
	UNREF(value);
	UNREF(duration);
	UNREF(attackRate);
	UNREF(decayRate);
	UNREF(target);

	return JNI_FALSE;
}	// JavaLibrary::addMentalStateModifier

	/**
	* Returns the mental state modifiers for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	*
	* @return the mental state modifiers for the creature, or nullptr if it has none
*/
jobjectArray JNICALL ScriptMethodsMentalStatesNamespace::getMentalStateModifiersToward(JNIEnv *env, jobject self, jlong mob, jlong target, jint mentalState)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return 0;

	UNREF(mob);
	UNREF(mentalState);
	UNREF(target);
	// @todo: wait for implementation on CreatureObject
	return 0;
}	// JavaLibrary::getMentalStateModifiers

	/**
	* Returns all the mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	*
	* @return the mental states for the creature
*/
jobjectArray JNICALL ScriptMethodsMentalStatesNamespace::getMentalStatesToward(JNIEnv *env, jobject self, jlong mob, jlong target)
{
	UNREF(self);

	if (target == 0)
		return 0;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return 0;

	NetworkId targetId(target);
	if (!targetId)
		return 0;

	// create the array of mental states
	LocalObjectArrayRefPtr attribs = createNewObjectArray(
		MentalStates::NumberOfMentalStates, JavaLibrary::getClsMentalState());
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	// set the array elements
	for (int i = 0; i < MentalStates::NumberOfMentalStates; ++i)
	{
		LocalRefPtr mentalState = createNewObject(JavaLibrary::getClsMentalState(), JavaLibrary::getMidMentalState(),
			i, creature->getMentalStateToward(targetId, static_cast<MentalStates::Enumerator>(i)));
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*attribs, i, *mentalState);
	}
	return attribs->getReturnValue();
}	// JavaLibrary::getMentalStates

	/**
	* Sets the mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param values		mental state/value pairs to set
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::setMentalStatesToward(JNIEnv *env, jobject self, jlong mob, jlong target, jobjectArray values)
{
	UNREF(self);

	if (target == 0)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	NetworkId targetId(target);
	if (!targetId)
		return JNI_FALSE;

	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr mentalState = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (mentalState == LocalRef::cms_nullPtr)
			return JNI_FALSE;

		int type = getIntField(*mentalState, JavaLibrary::getFidMentalStateType());
		float value = getFloatField(*mentalState, JavaLibrary::getFidMentalStateValue());
		if (type < 0 || type >= MentalStates::NumberOfMentalStates)
			return JNI_FALSE;

		creature->setMentalStateToward(targetId, static_cast<MentalStates::Enumerator>(type),
			static_cast<MentalStates::Value>(value));
	}
	return JNI_TRUE;
}	// JavaLibrary::setMentalStates

	/**
	* Adds values to the mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param values		mental state/value pairs to add
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addToMentalStatesToward(JNIEnv *env, jobject self, jlong mob, jlong target, jobjectArray values)
{
	UNREF(self);

	if (target == 0)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	NetworkId targetId(target);
	if (!targetId)
		return JNI_FALSE;

	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr mentalState = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (mentalState == LocalRef::cms_nullPtr)
			return JNI_FALSE;

		int type = getIntField(*mentalState, JavaLibrary::getFidMentalStateType());
		float delta = getFloatField(*mentalState, JavaLibrary::getFidMentalStateValue());
		if (type < 0 || type >= MentalStates::NumberOfMentalStates)
			return JNI_FALSE;

		MentalStates::Value value = creature->getMentalStateToward(targetId, static_cast<MentalStates::Enumerator>(type));
		creature->setMentalStateToward(targetId, static_cast<MentalStates::Enumerator>(type),
			static_cast<MentalStates::Value>(value+delta));
	}
	return JNI_TRUE;
}	// JavaLibrary::addToMentalStates

//-----------------------

/**
* Sets a creatue's mental state.
*
* @param env		    Java environment
* @param self		    class calling this function
* @param target	    id of creature to access
* @param mentalState		mental state we are interested in
* @param value			value to set the mental state to
*
* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::setMentalStateTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jint mentalState, jfloat value)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	std::vector<NetworkId> targetIds;
	if (!ScriptConversion::convert(target, targetIds))
		return JNI_FALSE;

	std::vector<NetworkId>::iterator i;
	for (i=targetIds.begin(); i!=targetIds.end(); ++i)
	{
		creature->setMentalStateToward(*i, static_cast<MentalStates::Enumerator>(mentalState),
			static_cast<MentalStates::Value>(value));
	}
	return JNI_TRUE;
}	// JavaLibrary::setAttrib

	/**
	* Adds a value to a creature's mental state.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value to add to the mental state
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addToMentalStateTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jint mentalState, jfloat value)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	std::vector<NetworkId> targetIds;
	if (!ScriptConversion::convert(target, targetIds))
		return JNI_FALSE;

	std::vector<NetworkId>::iterator i;
	for (i=targetIds.begin(); i!=targetIds.end(); ++i)
	{
		MentalStates::Value current = creature->getMentalStateToward(*i, static_cast<MentalStates::Enumerator>(mentalState));
		creature->setMentalStateToward(*i, static_cast<MentalStates::Enumerator>(mentalState), static_cast<MentalStates::Value>(current + value));
	}
	return JNI_TRUE;
}	// JavaLibrary::addToMentalState

	/**
	* Adds an mental state modifier to a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param mentalState		mental state we are interested in
	* @param value			value of the modifier
	* @param duration		how long the modifier lasts (in sec)
	* @param attackRate	how long before the modifier reaches it's max value (in sec)
	* @param decayRate		how long before the modifier reaches 0 (in sec)
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addMentalStateModifierTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jint mentalState, jfloat value, jfloat duration, jfloat attackRate, jfloat decayRate)
{
	UNREF(self);

	if (mentalState < 0 || mentalState >= MentalStates::NumberOfMentalStates)
		return JNI_FALSE;

	// @todo: wait for implementation on CreatureObject
	UNREF(mob);
	UNREF(mentalState);
	UNREF(value);
	UNREF(duration);
	UNREF(attackRate);
	UNREF(decayRate);
	UNREF(target);

	return JNI_FALSE;
}	// JavaLibrary::addMentalStateModifier

	/**
	* Sets the mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param values		mental state/value pairs to set
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::setMentalStatesTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jobjectArray values)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	std::vector<NetworkId> targetIds;
	if (!ScriptConversion::convert(target, targetIds))
		return JNI_FALSE;

	jboolean result = JNI_TRUE;
	std::vector<NetworkId>::iterator iTarget;
	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr mentalState = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (mentalState != LocalRef::cms_nullPtr)
		{
			int type = getIntField(*mentalState, JavaLibrary::getFidMentalStateType());
			float value = getFloatField(*mentalState, JavaLibrary::getFidMentalStateValue());
			if (type >= 0 && type < MentalStates::NumberOfMentalStates)
			{
				for (iTarget=targetIds.begin(); iTarget!=targetIds.end(); ++iTarget)
				{
					creature->setMentalStateToward(*iTarget,
						static_cast<MentalStates::Enumerator>(type),
						static_cast<MentalStates::Value>(value));
				}
			}
			else
				result = JNI_FALSE;
		}
		else
			result = JNI_FALSE;
	}
	return result;
}	// JavaLibrary::setMentalStates

	/**
	* Adds values to the mental states for a creature.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param values		mental state/value pairs to add
	*
	* @return true on success, false on fail
*/
jboolean JNICALL ScriptMethodsMentalStatesNamespace::addToMentalStatesTowardArray(JNIEnv *env, jobject self, jlong mob, jlongArray target, jobjectArray values)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return JNI_FALSE;

	std::vector<NetworkId> targetIds;
	if (!ScriptConversion::convert(target, targetIds))
		return JNI_FALSE;

	jboolean result = JNI_TRUE;
	std::vector<NetworkId>::iterator iTarget;
	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr mentalState = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (mentalState != LocalRef::cms_nullPtr)
		{
			int type = getIntField(*mentalState, JavaLibrary::getFidMentalStateType());
			float delta = getFloatField(*mentalState, JavaLibrary::getFidMentalStateValue());
			if (type >= 0 && type < MentalStates::NumberOfMentalStates)
			{
				for (iTarget=targetIds.begin(); iTarget!=targetIds.end(); ++iTarget)
				{
					MentalStates::Value value = creature->getMentalStateToward(*iTarget, static_cast<MentalStates::Enumerator>(type));
					value += delta;
					creature->setMentalStateToward(*iTarget, static_cast<MentalStates::Enumerator>(type),
						static_cast<MentalStates::Value>(value));
				}
			}
			else
				result = JNI_FALSE;
		}
		else
			result = JNI_FALSE;
	}
	return result;
}	// JavaLibrary::addToMentalStates

	/**
	* Get the mental states for a behavior.
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param behavior	  behavior to check
	*
	* @return an obj_id[] of the targets
*/
jlongArray JNICALL ScriptMethodsMentalStatesNamespace::getBehaviorTargets(JNIEnv *env, jobject self, jlong mob, jint behavior)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return 0;

	std::vector<NetworkId> targetIds;
	creature->getBehaviorTargets(behavior, targetIds);

	LocalLongArrayRefPtr result;
	if (!ScriptConversion::convert(targetIds, result))
		return 0;
	return result->getReturnValue();
}

/**
* Returns a creature's current behavior
*
* @param env		    Java environment
* @param self		    class calling this function
* @param mob	    id of creature to access
*
* @return the hehavior, or 0 on error
*/
jint JNICALL ScriptMethodsMentalStatesNamespace::getBehavior(JNIEnv *env, jobject self, jlong mob)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return 0;

	return creature->getBehavior();
}	// JavaLibrary::getBehavior

	/**
	* Returns a creature's current behavior toward another
	*
	* @param env		    Java environment
	* @param self		    class calling this function
	* @param mob	    id of creature to access
	* @param target   id of creature to get mental state towrad
	*
	* @return the hehavior, or 0 on error
*/
jint JNICALL ScriptMethodsMentalStatesNamespace::getBehaviorToward(JNIEnv *env, jobject self, jlong mob, jlong target)
{
	UNREF(self);

	if (target == 0)
		return 0;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return 0;

	return creature->getBehaviorToward(NetworkId(target));
}	// JavaLibrary::getBehavior

void JNICALL ScriptMethodsMentalStatesNamespace::scheduleAiCombatPulse(JNIEnv *env, jobject self, jlong mob)
{
	UNREF(self);

	if (mob == 0)
		return;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(mob, creature))
		return;

	//AiCombatPulseQueue::scheduleAiCombatPulse(creature->getNetworkId());
}
