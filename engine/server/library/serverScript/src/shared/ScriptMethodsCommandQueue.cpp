// ======================================================================
//
// ScriptMethodsCommandQueue.cpp - implements script methods dealing with
// creature command queues.
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CommandQueue.h"
#include "serverGame/CommandQueueEntry.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverUtility/ServerClock.h"
#include "sharedDebug/Profiler.h"
#include "sharedGame/CommandTable.h"
#include "sharedNetworkMessages/MessageQueueCommandTimer.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsCommandQueueNamespace
// ======================================================================

namespace ScriptMethodsCommandQueueNamespace
{
	bool install();

	jboolean     JNICALL queueCommand(JNIEnv *env, jobject self, jlong actor, jint commandHash, jlong target, jstring params, jint priority);
	jboolean     JNICALL queueClear(JNIEnv *env, jobject self, jlong actor);
	jboolean     JNICALL queueHasCommandFromGroup(JNIEnv *env, jobject self, jlong actor, jint group);
	jboolean     JNICALL queueClearCommandsFromGroup(JNIEnv *env, jobject self, jlong actor, jint group);
	jfloat       JNICALL getCommandMaxRange(JNIEnv *env, jobject self, jint commandHash);
	jboolean     JNICALL setCommandTimerValue( JNIEnv *env, jobject self, jlong actor, jint timerClass, jfloat newTime );
	jint         JNICALL getCommandQueueSize( JNIEnv *env, jobject self, jobject actor, jboolean combatOnly );
	jfloat       JNICALL getCooldownTimeLeft( JNIEnv *env, jobject self, jlong actor, jint cooldownTimer );
	jfloat       JNICALL getCooldownTimeLeftString( JNIEnv *env, jobject self, jlong actor, jstring command );
	jint         JNICALL getCurrentCommand( JNIEnv *env, jobject self, jlong actor );
	static void  JNICALL sendCooldownGroupTimingOnly(JNIEnv * env, jobject self, jlong actor, jint cooldownGroupCrc, jfloat time);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsCommandQueueNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsCommandQueueNamespace::c)}
	JF("_queueCommand", "(JIJLjava/lang/String;I)Z", queueCommand),
	JF("_queueClear", "(J)Z", queueClear),
	JF("_queueHasCommandFromGroup", "(JI)Z", queueHasCommandFromGroup),
	JF("_queueClearCommandsFromGroup", "(JI)Z", queueClearCommandsFromGroup),
	JF("getCommandMaxRange", "(I)F", getCommandMaxRange),
	JF("_setCommandTimerValue",    "(JIF)Z",     setCommandTimerValue),
	JF("_getCooldownTimeLeft",     "(JI)F",      getCooldownTimeLeft),
	JF("_getCooldownTimeLeft",     "(JLjava/lang/String;)F", getCooldownTimeLeftString ),
	JF("_getCurrentCommand",       "(J)I",       getCurrentCommand),
	JF("_sendCooldownGroupTimingOnly", "(JIF)V", sendCooldownGroupTimingOnly),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================
// class JavaLibrary JNI command table methods
// ======================================================================

/**
 * Enqueues a command on a creature's command queue.
 *
 * @param env      Java Environment
 * @param self     class calling this function
 * @param actor    creature we wish to enqueue the command for
 * @param command  command to enqueue
 * @param target   object on which the command is targeted
 * @param params   string of any additional parameters to pass to the command
 * @param priority priority at which to execute the command (may be default)
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCommandQueueNamespace::queueCommand(JNIEnv *env, jobject self, jlong actor, jint commandHash, jlong target, jstring params, jint priority)
{
	PROFILER_AUTO_BLOCK_DEFINE("queueCommand");
	JavaStringParam localParamsStr(params);

	if (priority < 0 || priority >= Command::CP_NumberOfPriorities)
		return JNI_FALSE;

	NetworkId actorId(actor);
	TangibleObject * const actorTangibleObject = TangibleObject::getTangibleObject(actorId);

	if (actorTangibleObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::queueCommand() Unable to resolve actor(%s) to a TangibleObject", actorId.getValueString().c_str()));
		return JNI_FALSE;
	}

	NetworkId targetId(target);
	if (targetId != NetworkId::cms_invalid)
	{
		ServerObject const * const targetServerObject = ServerObject::getServerObject(targetId);

		//If script specified a target id, make sure it can be resolved.
		if (!targetServerObject)
			return JNI_FALSE;

	}

	const Command &command = CommandTable::getCommand(static_cast<uint32>(commandHash));
	if (command.isNull())
		return JNI_FALSE;

	Unicode::String strParams;
	if (!JavaLibrary::convert(localParamsStr, strParams))
		return JNI_FALSE;

	actorTangibleObject->commandQueueEnqueue(command, targetId, strParams, 0, false, static_cast<Command::Priority>(priority), true);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

/**
 * Clears a creature's command queue (only the clearable commands).
 *
 * @param env     Java Environment
 * @param self    class calling this function
 * @param actor   creature we wish to clear the queue on
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCommandQueueNamespace::queueClear(JNIEnv *env, jobject self, jlong actor)
{
	NetworkId actorId(actor);
	TangibleObject * const actorTangibleObject = TangibleObject::getTangibleObject(actorId);

	if (actorTangibleObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::queueClear() Unable to resolve actor(%s) to a TangibleObject", actorId.getValueString().c_str()));
		return JNI_FALSE;
	}

	actorTangibleObject->commandQueueClear();

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

/**
 * Check if a creature has commands from a command group in its queue.
 *
 * @param env     Java Environment
 * @param self    class calling this function
 * @param actor   creature we wish to check the queue on
 * @param group   hash of the group we are checking for
 *
 * @return true if commands are present, false if not or on error
 */
jboolean JNICALL ScriptMethodsCommandQueueNamespace::queueHasCommandFromGroup(JNIEnv *env, jobject self, jlong actor, jint group)
{
	NetworkId actorId(actor);
	TangibleObject * const actorTangibleObject = TangibleObject::getTangibleObject(actorId);

	if (actorTangibleObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::queueHasCommandFromGroup() Unable to resolve actor(%s) to a TangibleObject", actorId.getValueString().c_str()));
		return JNI_FALSE;
	}

	return actorTangibleObject->commandQueueHasCommandFromGroup(static_cast<uint32>(group));
}

// ----------------------------------------------------------------------

/**
 * Clear commands of a specified group from a creature's command queue.
 *
 * @param env     Java Environment
 * @param self    class calling this function
 * @param actor   creature we wish to clear commands from the queue on
 * @param group   hash of the group we are checking for
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCommandQueueNamespace::queueClearCommandsFromGroup(JNIEnv *env, jobject self, jlong actor, jint group)
{
	NetworkId actorId(actor);
	TangibleObject * const actorTangibleObject = TangibleObject::getTangibleObject(actorId);

	if (actorTangibleObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::queueClearCommandsFromGroup() Unable to resolve actor(%s) to a TangibleObject", actorId.getValueString().c_str()));
		return JNI_FALSE;
	}

	actorTangibleObject->commandQueueClearCommandsFromGroup(group, true);

	return JNI_TRUE;
}

/**
 * Returns the max range a player can be from a target to execute a given command.
 *
 * @param env			Java Environment
 * @param self			class calling this function
 * @param commandHash	the command to get the range for
 *
 * @return the range, or -1 if an unknown command
 */
jfloat JNICALL ScriptMethodsCommandQueueNamespace::getCommandMaxRange(JNIEnv * /*env*/, jobject /*self*/, jint commandHash)
{
	const Command & command = CommandTable::getCommand(commandHash);
	if (&command == &CommandTable::getNullCommand())
		return -1;

	return command.m_maxRangeToTarget;
}	// JavaLibrary::getCommandMaxRange

/**
 * Sets the specified duration timer for the currently executing command
 *
 * @param env        Java Environment
 * @param self       call calling this function
 * @param actor      creature who's command queue we're going to access
 * @param timerClass the class of the timer we're going to change
 * @param newTime    the new time for this command.
 */
jboolean JNICALL ScriptMethodsCommandQueueNamespace::setCommandTimerValue( JNIEnv *env, jobject, jlong actor, jint timerClass, jfloat newTime )
{
	NetworkId actorId(actor);
	TangibleObject * const actorTangibleObject = TangibleObject::getTangibleObject(actorId);

	if (actorTangibleObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::setCommandTimerValue() Unable to resolve actor(%s) to a TangibleObject", actorId.getValueString().c_str()));
		return JNI_FALSE;
	}
	CommandQueue * const queue = actorTangibleObject->getCommandQueue();

	if (queue == nullptr)
	{
		WARNING(true, ("JavaLibrary::setCommandTimerValue() object(%s) does not have a CommandQueue", actorTangibleObject->getDebugInformation().c_str()));
		return JNI_FALSE;
	}

	return queue->setCommandTimerValue(static_cast<CommandQueue::TimerClass>(timerClass), newTime);
}

/**
 * Returns the crc32 of the current command
 *
 * @param env        Java Environment
 * @param self       call calling this function
 * @param actor      creature who's command queue we're going to access, or 0 on error
 */
jint JNICALL ScriptMethodsCommandQueueNamespace::getCurrentCommand( JNIEnv *env, jobject, jlong actor )
{
	NetworkId actorId(actor);
	TangibleObject * const actorTangibleObject = TangibleObject::getTangibleObject(actorId);

	if (actorTangibleObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::getCurrentCommand() Unable to resolve actor(%s) to a TangibleObject", actorId.getValueString().c_str()));
		return 0;
	}

	CommandQueue const * const queue = actorTangibleObject->getCommandQueue();

	if (queue == nullptr)
	{
		WARNING(true, ("JavaLibrary::getCurrentCommand() object(%s) does not have a CommandQueue", actorTangibleObject->getDebugInformation().c_str()));
		return 0;
	}

	return queue->getCurrentCommand();
}

/**
 * Returns the time left in a specified cooldown timer group
 *
 * @param env        Java Environment
 * @param self       call calling this function
 * @param actor      creature who's command queue we're going to access
 * @param group      the cooldown timer group
 * @return           the time left in the cooldown group.
 */
jfloat JNICALL ScriptMethodsCommandQueueNamespace::getCooldownTimeLeft( JNIEnv *env, jobject, jlong actor, jint group )
{
	NetworkId actorId(actor);
	TangibleObject * const actorTangibleObject = TangibleObject::getTangibleObject(actorId);

	if (actorTangibleObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::getCooldownTimeLeft() Unable to resolve actor(%s) to a TangibleObject", actorId.getValueString().c_str()));
		return 0.0f;
	}

	CommandQueue const * const queue = actorTangibleObject->getCommandQueue();

	if (queue == nullptr)
	{
		WARNING(true, ("JavaLibrary::getCooldownTimeLeft() object(%s) does not have a CommandQueue", actorTangibleObject->getDebugInformation().c_str()));
		return 0.0f;
	}

	return queue->getCooldownTimeLeft(group);
}

/**
 * Returns the time left in a specified cooldown timer group
 *
 * @param env        Java Environment
 * @param self       call calling this function
 * @param actor      creature who's command queue we're going to access
 * @param command    the cooldown timer group
 * @return           the time left in the cooldown group.
 */
jfloat JNICALL ScriptMethodsCommandQueueNamespace::getCooldownTimeLeftString( JNIEnv *env, jobject, jlong actor, jstring command )
{
	NetworkId actorId(actor);
	TangibleObject * const actorTangibleObject = TangibleObject::getTangibleObject(actorId);

	if (actorTangibleObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::getCooldownTimeLeftString() Unable to resolve actor(%s) to a TangibleObject", actorId.getValueString().c_str()));
		return 0.0f;
	}
	
	JavaStringParam s( command );
	Unicode::NarrowString out;

	if ( !JavaLibrary::convert( s, out ) )
	{
		WARNING( true, ( "unable to convert string.\n" ) );
		return 0.f;
	}

	CommandQueue const * const queue = actorTangibleObject->getCommandQueue();
	float value = 0.0f;
	
	if (queue == nullptr)
	{
		WARNING(true, ("JavaLibrary::getCooldownTimeLeftString() object(%s) does not have a CommandQueue", actorTangibleObject->getDebugInformation().c_str()));
		return 0.0f;
	}

	if (queue != nullptr)
	{
		value = queue->getCooldownTimeLeft( out );
	}
	return value;
}

/**
 * send the client information about a cooldown group, without associated command info.
 * we use this for things that cooldown but have no command, e.g. stimpacks
 */
void JNICALL ScriptMethodsCommandQueueNamespace::sendCooldownGroupTimingOnly(JNIEnv *env, jobject, jlong actor, jint cooldownGroupCrc, jfloat time)
{
	NetworkId actorId(actor);
	CreatureObject * const actorCreatureObject = CreatureObject::getCreatureObject(actorId);

	if (actorCreatureObject == nullptr)
	{
		WARNING(true, ("JavaLibrary::sendCooldownGroupTimingOnly() Unable to resolve actor(%s) to a CreatureObject", actorId.getValueString().c_str()));
		return;
	}

	CreatureController *controller = actorCreatureObject->getCreatureController();
	if(!controller)
		return;

	MessageQueueCommandTimer *msg = new MessageQueueCommandTimer(
			0,
			cooldownGroupCrc,
			-1,
			0 );
	msg->setCurrentTime( MessageQueueCommandTimer::F_cooldown, 0.0f );
	msg->setMaxTime    ( MessageQueueCommandTimer::F_cooldown, time );

	// if the cool down value is large enough, we want to cache/persist it so
	// that if the client is restarted, we can provide an accurate representation
	// of where the cool down is at; if the cool down value is small, don't bother
	// because by the time the client is back up and running, the cool down probably
	// has already finished
	if (static_cast<int>(time) >= ConfigServerGame::getCoolDownPersistThresholdSeconds())
	{
		int const currentGameTime = static_cast<int>(ServerClock::getInstance().getGameTimeSeconds());

		char buffer[256];

		snprintf(buffer, sizeof(buffer)-1, "visualCooldown.beginTime.%d", static_cast<int>(cooldownGroupCrc));
		buffer[sizeof(buffer)-1] = '\0';
		std::string const coolDownBeginTime = buffer;

		snprintf(buffer, sizeof(buffer)-1, "visualCooldown.endTime.%d", static_cast<int>(cooldownGroupCrc));
		buffer[sizeof(buffer)-1] = '\0';
		std::string const coolDownEndTime = buffer;

		IGNORE_RETURN(actorCreatureObject->setObjVarItem(coolDownBeginTime, currentGameTime));
		IGNORE_RETURN(actorCreatureObject->setObjVarItem(coolDownEndTime, (currentGameTime + static_cast<int>(time))));
	}

	controller->appendMessage(
		static_cast< int >( CM_commandTimer ),
		0.0f,
		msg,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_CLIENT );	
}

// ======================================================================
