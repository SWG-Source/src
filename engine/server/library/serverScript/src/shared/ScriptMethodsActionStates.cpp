//========================================================================
//
// ScriptMethodsActionStatesNamespace.cpp - implements script methods dealing with action states
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/CellObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"

using namespace JNIWrappersNamespace;


//========================================================================
// ScriptMethodsActionStatesNamespace
//========================================================================

namespace ScriptMethodsActionStatesNamespace
{
	bool install();

	jboolean     JNICALL wanderAngleDelay(JNIEnv *env, jobject self, jlong mob, jfloat minDist, jfloat maxDist, jfloat minAngle, jfloat maxAngle, jfloat minDelay, jfloat maxDelay);
	void         JNICALL setCreatureStatic(JNIEnv *env, jobject self, jlong creature, jboolean value);
	void         JNICALL stop(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL follow(JNIEnv *env, jobject self, jlong mob, jlong target, jfloat minDistance, jfloat maxDistance);
	jboolean     JNICALL followOffset(JNIEnv *env, jobject self, jlong mob, jlong target, jobject offset);
	jboolean     JNICALL swarm(JNIEnv *env, jobject self, jlong mob, jlong target);
	jboolean     JNICALL swarm2(JNIEnv *env, jobject self, jlong mob, jlong target, jfloat offset);
	jboolean     JNICALL flee(JNIEnv *env, jobject self, jlong mob, jlong target, jfloat minDistance, jfloat maxDistance);
	jboolean     JNICALL suspendMovement(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL resumeMovement(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL hasSuspendedMovement(JNIEnv *env, jobject self, jlong mob);
	jfloat       JNICALL getAcceleration(JNIEnv *env, jobject self, jlong mob, jint movementType);
	jfloat       JNICALL getTurnRate(JNIEnv *env, jobject self, jlong mob, jint movementType);
	jfloat       JNICALL getStepHeight(JNIEnv *env, jobject self, jlong mob);
	jfloat       JNICALL getApproachTriggerRange(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL isCreatureStatic(JNIEnv *env, jobject self, jlong creature);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsActionStatesNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsActionStatesNamespace::c)}
	JF("_wanderAngleDelay", "(JFFFFFF)Z", wanderAngleDelay),
	JF("_setCreatureStatic", "(JZ)V", setCreatureStatic),
	JF("_stop", "(J)V", stop),
	JF("_follow", "(JJFF)Z", follow),
	JF("_follow", "(JJLscript/location;)Z", followOffset),
	JF("_swarm", "(JJ)Z", swarm),
	JF("_swarm", "(JJF)Z", swarm2),
	JF("_flee", "(JJFF)Z", flee),
	JF("_suspendMovement", "(J)Z", suspendMovement),
	JF("_resumeMovement", "(J)Z", resumeMovement),
	JF("_hasSuspendedMovement", "(J)Z", hasSuspendedMovement),
	JF("_getAcceleration", "(JI)F",getAcceleration),
	JF("_getTurnRate", "(JI)F",getTurnRate),
	JF("_getStepHeight", "(J)F",getStepHeight),
	JF("_getApproachTriggerRange", "(J)F",getApproachTriggerRange),
	JF("_isCreatureStatic", "(J)Z", isCreatureStatic),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI action state callback methods
//========================================================================

/**
 * Sets a creature into the "Stop" state where he doesn't do anything.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 *
 * @return JNI_TRUE if the creature stopped, or false if there was an error
 */
void JNICALL ScriptMethodsActionStatesNamespace::stop(JNIEnv *env, jobject , jlong ai)
{
	CreatureObject * aiCreatureObject = 0;
	AICreatureController * aiCreatureController = 0;

	if (!JavaLibrary::getObjectController(ai, aiCreatureObject, aiCreatureController))
	{
		return;
	}

	if (aiCreatureController->isRetreating())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(ConfigServerGame::getMovementWhileRetreatingThrowsException(),
		                            ("ScriptMethodsActionStatesNamespace::stop() ai(%s) Calling a movement command on a retreating AI is not allowed.", aiCreatureObject->getDebugInformation().c_str()));
		return;
	}

	aiCreatureController->stop();
}

/**
 * Sets a creature into the "Follow" state where he follows someone else around.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param mob	    id of creature to access
 * @param target        target id of creature to follow
 * @param minDist    closest to get to creature
 * @param maxDist   farthest to get from target
 *
 * @return JNI_TRUE if the creature stopped, or false if there was an error
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::follow(JNIEnv *env, jobject self, jlong mob, jlong target, jfloat minDist, jfloat maxDist)
{
	UNREF(self);

	if (mob == 0 || target == 0)
		return JNI_FALSE;

	CreatureObject * object = 0;
	AICreatureController * controller = 0;
	jboolean result=JNI_FALSE;

	if (!JavaLibrary::getObjectController(mob, object, controller))
		return result;

	NetworkId mobId = object->getNetworkId();
	NetworkId targetId(target);

	if(mobId == targetId)
	{
		JavaLibrary::throwInternalScriptError("ScriptMethodsActionStatesNamespace::follow - Creature is trying to follow itself\n");
		return JNI_FALSE;
	}

	if (controller->follow(NetworkId(target), minDist, maxDist))
		result = JNI_TRUE;

	return result;
}

/**
 * Sets a creature into the "Follow" state where he follows someone else around.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param mob	    id of creature to access
 * @param target        target id of creature to follow
 * @param minDist    closest to get to creature
 * @param maxDist   farthest to get from target
 *
 * @return JNI_TRUE if the creature stopped, or false if there was an error
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::followOffset(JNIEnv *env, jobject self, jlong mob, jlong target, jobject offset)
{
	UNREF(self);

	if (mob == 0 || target == 0)
		return JNI_FALSE;

	CreatureObject * object = 0;
	AICreatureController * controller = 0;
	jboolean result=JNI_FALSE;

	if (!JavaLibrary::getObjectController(mob, object, controller))
		return result;

	Vector location;
	NetworkId cell;
	ScriptConversion::convert(offset, location, cell, object->getPosition_w());

	NetworkId mobId = object->getNetworkId();
	NetworkId targetId(target);

	if(mobId == targetId)
	{
		JavaLibrary::throwInternalScriptError("ScriptMethodsActionStatesNamespace::followOffset - Creature is trying to follow itself\n");
		return JNI_FALSE;
	}

	if (controller->follow(NetworkId(target), location))
		result = JNI_TRUE;

	return result;
}

/**
 * Sets a creature into the "Swarm" state where he and his comrades try to follow and surround the target.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param mob	    id of creature to access
 * @param target        target id of creature to follow
 * @param minDist    closest to get to creature
 * @param maxDist   farthest to get from target
 *
 * @return JNI_TRUE if the creature stopped, or false if there was an error
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::swarm(JNIEnv *env, jobject self, jlong mob, jlong target)
{
	UNREF(self);

	if (mob == 0 || target == 0)
		return JNI_FALSE;

	CreatureObject * object = 0;
	AICreatureController * controller = 0;
	jboolean result=JNI_FALSE;

	if (!JavaLibrary::getObjectController(mob, object, controller))
		return result;

	NetworkId mobId = object->getNetworkId();
	NetworkId targetId(target);

	if(mobId == targetId)
	{
		JavaLibrary::throwInternalScriptError("ScriptMethodsActionStatesNamespace::swarm - Creature is trying to swarm itself\n");
		return JNI_FALSE;
	}

	if (controller->swarm(NetworkId(target)))
		result = JNI_TRUE;

	return result;
}

/**
 * Sets a creature into the "Swarm" state where he and his comrades try to follow and surround the target.
 *
 * @param env	    Java environment
 * @param self	    class calling this function
 * @param mob	    id of creature to access
 * @param target    target id of creature to follow
 * @param offset    how far away to stay from the target
 *
 * @return JNI_TRUE if the creature stopped, or false if there was an error
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::swarm2(JNIEnv *env, jobject self, jlong mob, jlong target, jfloat offset)
{
	UNREF(self);

	if (mob == 0 || target == 0)
		return JNI_FALSE;

	CreatureObject * object = 0;
	AICreatureController * controller = 0;
	jboolean result=JNI_FALSE;

	if (!JavaLibrary::getObjectController(mob, object, controller))
		return result;

	NetworkId mobId = object->getNetworkId();
	NetworkId targetId(target);

	if(mobId == targetId)
	{
		JavaLibrary::throwInternalScriptError("ScriptMethodsActionStatesNamespace::swarm - Creature is trying to swarm itself\n");
		return JNI_FALSE;
	}

	if (controller->swarm(NetworkId(target), offset))
		result = JNI_TRUE;

	return result;
}

/**
 * Sets a creature into the "Flee" state where he runs away from another object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param mob	    id of creature to access
 * @param target        target id of creature to flee
 *
 * @return JNI_TRUE if the creature stopped, or false if there was an error
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::flee(JNIEnv *env, jobject self, jlong mob, jlong target, jfloat minDistance, jfloat maxDistance)
{
	UNREF(self);

	if (mob == 0 || target == 0)
		return JNI_FALSE;

	CreatureObject * object = 0;
	AICreatureController * controller = 0;
	jboolean result=JNI_FALSE;

	if (!JavaLibrary::getObjectController(mob, object, controller))
		return result;

	NetworkId mobId = object->getNetworkId();
	NetworkId targetId(target);

	if(mobId == targetId)
	{
		JavaLibrary::throwInternalScriptError("ScriptMethodsActionStatesNamespace::flee - Creature is trying to flee from itself\n");
		return JNI_FALSE;
	}

	if (controller->flee(NetworkId(target), minDistance, maxDistance))
		result = JNI_TRUE;

	return result;
}

/**
 * Sets a creature into the "Wander" state where he wanders around somewhat aimlessly.  The wandering is broken into legs,
 * each of which is between minDist and maxDist distance.  After each leg he will turn either left or right at an angle between
 * minAngle and maxAngle degrees.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param mob	    id of creature to access
 * @param minDist   minimum length of a leg
 * @param maxDist   maximum length of a leg
 * @param minAngle  minimum angle to turn after a leg
 * @param maxAngle  maximum angle to turn after a leg
 * @param minDelay   minimum time to wait between movements
 * @param maxDelay  maximum time to wait between movements
 *
 * @return JNI_TRUE if the creature stopped, or false if there was an error
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::wanderAngleDelay(JNIEnv *env, jobject self, jlong mob, jfloat minDist, jfloat maxDist, jfloat minAngle, jfloat maxAngle, jfloat minDelay, jfloat maxDelay)
{
	UNREF(self);
	CreatureObject * object = 0;
	AICreatureController * controller = 0;

	if (!JavaLibrary::getObjectController(mob, object, controller))
		return JNI_FALSE;

	minAngle *= PI_OVER_180;
	maxAngle *= PI_OVER_180;

	controller->wander(minDist, maxDist, minAngle, maxAngle, minDelay, maxDelay);

	return JNI_TRUE;
}	// ScriptMethodsActionStatesNamespace::stop()

//------------------------------------------------------------------------------------------------

/**
 * Stops the current movement type of the creature, allowing it to be resumed at a
 * later time.
 *
 * NOTE: Only one movement can be suspended at a time. If this function
 * is called while a previous movement is suspended, it will fail.
 *
 * @return true if the movement has been suspended, false if not.
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::suspendMovement(JNIEnv *env, jobject self, jlong mob)
{
	UNREF(self);
	CreatureObject * object = 0;
	AICreatureController * controller = 0;
	if (!JavaLibrary::getObjectController(mob, object, controller))
		return JNI_FALSE;

	return static_cast<jboolean>(controller->suspendMovement() ? JNI_TRUE : JNI_FALSE);
}

//------------------------------------------------------------------------------------------------

/**
 * Resumes a previously suspended movement.
 *
 * @return true if the movement was resumed, false if there was no previous movement.
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::resumeMovement(JNIEnv *env, jobject self, jlong mob)
{
	UNREF(self);
	CreatureObject * object = 0;
	AICreatureController * controller = 0;
	if (!JavaLibrary::getObjectController(mob, object, controller))
		return JNI_FALSE;

	return static_cast<jboolean>(controller->resumeMovement() ? JNI_TRUE : JNI_FALSE);
}

//------------------------------------------------------------------------------------------------

/**
 * Returns if a creature has movement suspended or not.
 *
 * @return true there is suspended movement, false if not
 */
jboolean JNICALL ScriptMethodsActionStatesNamespace::hasSuspendedMovement(JNIEnv *env, jobject self, jlong mob)
{
	UNREF(self);
	CreatureObject * object = 0;
	AICreatureController * controller = 0;
	if (!JavaLibrary::getObjectController(mob, object, controller))
		return JNI_FALSE;

	return static_cast<jboolean>(controller->hasSuspendedMovement() ? JNI_TRUE : JNI_FALSE);
}

//------------------------------------------------------------------------------------------------

jfloat JNICALL ScriptMethodsActionStatesNamespace::getAcceleration(JNIEnv *env, jobject self, jlong mob, jint movementType)
{
	CreatureObject * object = 0;
	float result = -1.0f;
	if (!JavaLibrary::getObject(mob, object))
		return result;

	return object->getAcceleration(static_cast<SharedCreatureObjectTemplate::MovementTypes>(movementType));

}
//------------------------------------------------------------------------------------------------

jfloat JNICALL ScriptMethodsActionStatesNamespace::getTurnRate(JNIEnv *env, jobject self, jlong mob, jint movementType)
{
	CreatureObject * object = 0;
	float result = -1.0f;
	if (!JavaLibrary::getObject(mob, object))
		return result;

	return object->getTurnRate(static_cast<SharedCreatureObjectTemplate::MovementTypes>(movementType));

}

//------------------------------------------------------------------------------------------------

jfloat JNICALL ScriptMethodsActionStatesNamespace::getStepHeight(JNIEnv *env, jobject self, jlong mob)
{
	CreatureObject * object = 0;
	float result = -1.0f;
	if (!JavaLibrary::getObject(mob, object))
		return result;

	return object->getStepHeight();

}

//------------------------------------------------------------------------------------------------

jfloat JNICALL ScriptMethodsActionStatesNamespace::getApproachTriggerRange(JNIEnv *env, jobject self, jlong mob)
{
	CreatureObject * object = 0;
	float result = -1.0f;
	if (!JavaLibrary::getObject(mob, object))
		return result;

	return object->getApproachTriggerRange();

}

//------------------------------------------------------------------------------------------------

void JNICALL ScriptMethodsActionStatesNamespace::setCreatureStatic(JNIEnv *env, jobject self, jlong creature, jboolean value)
{
	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(creature, object))
		return;

	DEBUG_REPORT_LOG(!value, ("Script is setting a creature to not be static.  This is ok, but questionable.\n"));
	object->setIsStatic(value);

}

//------------------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsActionStatesNamespace::isCreatureStatic(JNIEnv *env, jobject self, jlong creature)
{
	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(creature, object))
		return JNI_FALSE;

	if(!object)
		return JNI_FALSE;

	return object->getIsStatic() ? JNI_TRUE : JNI_FALSE;
}

//------------------------------------------------------------------------------------------------
