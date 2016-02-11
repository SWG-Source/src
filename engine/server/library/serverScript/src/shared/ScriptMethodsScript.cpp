//========================================================================
//
// ScriptMethodsScript.cpp - implements script methods dealing with scripts.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "boost/smart_ptr.hpp"
#include "UnicodeUtils.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptMessage.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Crc.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsScriptNamespace
// ======================================================================

namespace ScriptMethodsScriptNamespace
{
	bool install();

	jint         JNICALL attachScript(JNIEnv *env, jobject self, jlong objId, jstring scriptName);
	jboolean     JNICALL detachScript(JNIEnv *env, jobject self, jlong objId, jstring scriptName);
	jboolean     JNICALL detachAllScripts(JNIEnv *env, jobject self, jlong objId);
	jboolean     JNICALL hasScript(JNIEnv *env, jobject self, jobject objId, jstring scriptName);
	jboolean     JNICALL reloadScriptFromScript(JNIEnv * env, jobject self, jstring scriptName);
	jint         JNICALL triggerScript(JNIEnv *env, jobject self, jlong object, jstring script, jint triggerId, jobjectArray params);
	jboolean     JNICALL localMessageTo(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jobject params);
	jboolean     JNICALL remoteMessageTo(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jbyteArray params, jfloat delay, jboolean guaranteed, jlong undeliveredCallbackObject, jstring undeliveredCallbackMethodName);
	void         JNICALL messageToPlayersOnPlanet(JNIEnv *env, jobject self, jstring methodName, jbyteArray params, jfloat delay, jobject loc, jfloat radius, jboolean includeDisconnectedPlayers);
	jint         JNICALL remoteMessageToCalendarTimeDayOfWeek(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jbyteArray params, jint dayOfWeek, jint hour, jint minute, jint second);
	jint         JNICALL remoteMessageToCalendarTimeDayOfMonth(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jbyteArray params, jint month, jint dayOfMonth, jint hour, jint minute, jint second);
	void         JNICALL recurringMessageTo(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jbyteArray params, jfloat delay);
	void         JNICALL cancelRecurringMessageTo(JNIEnv *env, jobject self, jlong receiver, jstring methodName);
	jint         JNICALL timeUntilMessageTo(JNIEnv *env, jobject self, jlong object, jstring methodName);
	jint         JNICALL getServerFrame(JNIEnv * env, jobject self);
	jint         JNICALL getGameTimeSeconds(JNIEnv * env, jobject self);
	jint         JNICALL getCalendarTimeSeconds(JNIEnv * env, jobject self);
	jint         JNICALL getCalendarTimeSeconds2(JNIEnv * env, jobject self, jint year, jint month, jint day, jint hour, jint minute, jint second);
	jstring      JNICALL getCalendarTimeStringGMT(JNIEnv *env, jobject self, jint calendarTimeSeconds);
	jstring      JNICALL getCalendarTimeStringGMT_YYYYMMDDHHMMSS(JNIEnv *env, jobject self, jint calendarTimeSeconds);
	jstring      JNICALL getCalendarTimeStringLocal(JNIEnv *env, jobject self, jint calendarTimeSeconds);
	jstring      JNICALL getCalendarTimeStringLocal_YYYYMMDDHHMMSS(JNIEnv *env, jobject self, jint calendarTimeSeconds);
	jint         JNICALL secondsUntilCalendarTimeDayOfWeek(JNIEnv *env, jobject self, jint dayOfWeek, jint hour, jint minute, jint second);
	jint         JNICALL secondsUntilCalendarTimeDayOfMonth(JNIEnv *env, jobject self, jint month, jint dayOfMonth, jint hour, jint minute, jint second);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsScriptNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsScriptNamespace::c)}
	JF("_attachScript", "(JLjava/lang/String;)I", attachScript),
	JF("_detachScript", "(JLjava/lang/String;)Z", detachScript),
	JF("_detachAllScripts", "(J)Z", detachAllScripts),
	JF("reloadScript", "(Ljava/lang/String;)Z", reloadScriptFromScript),
	JF("_triggerScript", "(JLjava/lang/String;I[Ljava/lang/Object;)I", triggerScript),
	JF("_localMessageTo", "(JLjava/lang/String;Lscript/dictionary;)Z", localMessageTo),
	JF("_remoteMessageTo", "(JLjava/lang/String;[BFZJLjava/lang/String;)Z", remoteMessageTo),
	JF("_messageToPlayersOnPlanet", "(Ljava/lang/String;[BFLscript/location;FZ)V", messageToPlayersOnPlanet),
	JF("_remoteMessageTo", "(JLjava/lang/String;[BIIII)I", remoteMessageToCalendarTimeDayOfWeek),
	JF("_remoteMessageTo", "(JLjava/lang/String;[BIIIII)I", remoteMessageToCalendarTimeDayOfMonth),
	JF("_internalRecurringMessageTo", "(JLjava/lang/String;[BF)V", recurringMessageTo),
	JF("_cancelRecurringMessageTo", "(JLjava/lang/String;)V", cancelRecurringMessageTo),
	JF("_timeUntilMessageTo", "(JLjava/lang/String;)I", timeUntilMessageTo),
	JF("getServerFrame", "()I", getServerFrame),
	JF("getGameTime", "()I", getGameTimeSeconds),
	JF("getCalendarTime", "()I", getCalendarTimeSeconds),
	JF("getCalendarTime", "(IIIIII)I", getCalendarTimeSeconds2),
	JF("getCalendarTimeStringGMT", "(I)Ljava/lang/String;", getCalendarTimeStringGMT),
	JF("getCalendarTimeStringGMT_YYYYMMDDHHMMSS", "(I)Ljava/lang/String;", getCalendarTimeStringGMT_YYYYMMDDHHMMSS),
	JF("getCalendarTimeStringLocal", "(I)Ljava/lang/String;", getCalendarTimeStringLocal),
	JF("getCalendarTimeStringLocal_YYYYMMDDHHMMSS", "(I)Ljava/lang/String;", getCalendarTimeStringLocal_YYYYMMDDHHMMSS),
	JF("secondsUntilCalendarTimeDayOfWeek", "(IIII)I", secondsUntilCalendarTimeDayOfWeek),
	JF("secondsUntilCalendarTimeDayOfMonth", "(IIIII)I", secondsUntilCalendarTimeDayOfMonth),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI script callback methods
//========================================================================

/**
 * Adds a script to an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class to attach the script to
 * @param scriptName    name of the script
 *
 * @return SCRIPT_CONTINUE on success, SCRIPT_OVERRIDE on error
 */
jint JNICALL ScriptMethodsScriptNamespace::attachScript(JNIEnv *env, jobject self, jlong objId, jstring scriptName)
{
	UNREF(self);

	JavaStringParam localScriptName(scriptName);
	
	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objId, object))
		return SCRIPT_OVERRIDE;

	GameScriptObject* scripts = object->getScriptObject();
	if (scripts == nullptr)
		return SCRIPT_OVERRIDE;

	char buffer[MAX_SCRIPT_NAME_LEN];
	localScriptName.fillBuffer(buffer, sizeof(buffer));

	if (!*buffer)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("attachScript attempt to attach empty script name to [%s]", object->getNetworkId().getValueString().c_str()));
		return SCRIPT_CONTINUE;
	}

	if (scripts->hasScript(buffer))
	{
		return SCRIPT_OVERRIDE;
	}

	return scripts->attachScript(buffer, true);
}	// JavaLibrary::attachScript

/**
 * Removes a script from an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class to detach the script from
 * @param scriptName    name of the script
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsScriptNamespace::detachScript(JNIEnv *env, jobject self, jlong objId, jstring scriptName)
{
	UNREF(self);

	JavaStringParam localScriptName(scriptName);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objId, object))
		return JNI_FALSE;

	GameScriptObject* scripts = object->getScriptObject();
	if (scripts == nullptr)
		return JNI_FALSE;

	char buffer[MAX_SCRIPT_NAME_LEN];
	localScriptName.fillBuffer(buffer, sizeof(buffer));

	if (scripts->detachScript(buffer))
		return JNI_TRUE;
	else
		return JNI_FALSE;
}	// JavaLibrary::detachScript

/**
 * Removes all scripts from an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class to detach the script from
 *
 * @return true if all the scripts were detached, false if not
 */
jboolean JNICALL ScriptMethodsScriptNamespace::detachAllScripts(JNIEnv *env, jobject self, jlong objId)
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objId, object))
		return JNI_FALSE;

	GameScriptObject* scriptObject = object->getScriptObject();
	if (scriptObject == nullptr)
		return JNI_FALSE;

	std::vector<std::string> scriptNames;
	scriptObject->enumerateScripts(scriptNames);
	
	// go through all the scripts and detach them
	jboolean result = JNI_TRUE;
	for (std::vector<std::string>::const_iterator i = scriptNames.begin(); i != scriptNames.end(); ++i)
	{
		if (!scriptObject->detachScript(*i))
			result = JNI_FALSE;
	}
	return result;
}	// JavaLibrary::detachAllScripts

/**
 * Checks if a class has a script.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class to check
 * @param scriptName    name of the script
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsScriptNamespace::hasScript(JNIEnv *env, jobject self, jobject objId,
	jstring scriptName)
{
	UNREF(self);

	JavaStringParam localScriptName(scriptName);

	ServerObject* object = 0;
	if (!JavaLibrary::getObject(objId, object))
		return JNI_FALSE;

	GameScriptObject* scripts = object->getScriptObject();
	if (scripts == nullptr)
		return JNI_FALSE;

	char buffer[MAX_SCRIPT_NAME_LEN];
	localScriptName.fillBuffer(buffer, sizeof(buffer));

	if (scripts->hasScript(buffer))
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::hasScript

/**
 * Executes a script trigger.
 * 
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId		    id of class to run the script from
 * @param scriptName    name of the script
 * @param triggerId 	id of trigger to run
 * @param params		paramaters to send to the trigger
 *
 * @return results from calling script, or SCRIPT_OVERRIDE on error
 */
jint JNICALL ScriptMethodsScriptNamespace::triggerScript(JNIEnv *env, jobject self, jlong objId, jstring scriptName, jint triggerId, jobjectArray params)
{
	UNREF(self);

	JavaStringParam localScriptName(scriptName);

	// get the trigger info
	if (triggerId < 0 || triggerId >= Scripting::TRIG_LAST_TRIGGER)
		return SCRIPT_OVERRIDE;
	Scripting::_ScriptFuncHashMap::const_iterator funcIter = Scripting::ScriptFuncHashMap->find(triggerId);
	if (funcIter == Scripting::ScriptFuncHashMap->end())
		return SCRIPT_OVERRIDE;
	const Scripting::ScriptFuncTable *func = (*funcIter).second;

	// get the object to run the script on
	ServerObject* object = 0;
	if (!JavaLibrary::getObject(objId, object))
		return SCRIPT_OVERRIDE;

	// convert the script name to ansii
	char buffer[MAX_SCRIPT_NAME_LEN];
	localScriptName.fillBuffer(buffer, sizeof(buffer));

	// run the script; note that we are not checking if the paramaters match the
	// types given in ScriptFuncTable
	return JavaLibrary::instance()->runScript(object->getNetworkId(), buffer, func->name, params);
}	// JavaLibrary::triggerScript


//========================================================================
// class JavaLibrary JNI messaging callback methods
//========================================================================

/**
 * Sets up a timed message to be called on an object's script.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param receiver		object id of object the message is attached to
 * @param methodName	script method to be called when the message is signalled
 * @param params		parameters to be sent to the methodName
 * @param delay			time (in secs) before the message is signalled
 * @param guaranteed	flag to guarantee the message will be signaled (even if the object is unloaded)
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsScriptNamespace::localMessageTo(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jobject params)
{
	UNREF(self);

	JavaStringParam localMethodName(methodName);

	if (receiver == 0 || methodName == 0 || params == 0)
		return JNI_FALSE;

	if (!env->IsInstanceOf(params, JavaLibrary::getClsDictionary()))
		return JNI_FALSE;

	jboolean result = JNI_FALSE;
	std::string methodNameStdString;
	if (JavaLibrary::convert(localMethodName, methodNameStdString))
	{
		// get the receiver object
		ServerObject * object = 0;
		if (!(JavaLibrary::getObject(receiver, object) && object && object->isAuthoritative()))
		{
			DEBUG_WARNING(true,("localMessageTo called for an object which was not authoritative on this server.\n"));
			result = JNI_FALSE;
		}
		else
		{
			GameScriptObject* scripts = object->getScriptObject();
			if (scripts == nullptr)
				return JNI_FALSE;
			
			ScriptDictionaryPtr dictionary(new JavaDictionary(params));
			scripts->handleMessage(methodNameStdString, dictionary); // was addMessage, changed name because it now fires right away

			result = JNI_TRUE;
		}
	}
	return result;
}

// ----------------------------------------------------------------------

/**
 * Sets up a timed message to be called on an object's script.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param receiver		object id of object the message is attached to
 * @param methodName	script method to be called when the message is signalled
 * @param params		parameters to be sent to the methodName
 * @param delay			time (in secs) before the message is signalled
 * @param guaranteed	flag to guarantee the message will be signaled (even if the object is unloaded)
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsScriptNamespace::remoteMessageTo(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jbyteArray params, jfloat delay, jboolean guaranteed, jlong undeliveredCallbackObject, jstring undeliveredCallbackMethodName)
{
	UNREF(self);
	
	JavaStringParam localMethodName(methodName);
	std::string methodNameStdString;
	if (!JavaLibrary::convert(localMethodName, methodNameStdString))
		return JNI_FALSE;

	std::string undeliveredCallbackMethodNameStdString;
	if (undeliveredCallbackMethodName)
	{
		JavaStringParam temp(undeliveredCallbackMethodName);
		if (!JavaLibrary::convert(temp, undeliveredCallbackMethodNameStdString))
			return JNI_FALSE;
	}

	if (guaranteed==JNI_TRUE && !undeliveredCallbackMethodNameStdString.empty())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("Sending message \"%s\" to object %s, the message was guaranteed but also had an undelivered callback specified.", methodNameStdString.c_str(), NetworkId(receiver).getValueString().c_str()));
		return false;
	}
	
	if (params != 0)
	{
		std::vector<int8> localParams;
		if (!ScriptConversion::convert(params, localParams))
			return JNI_FALSE;

		// add crc info to the string so we can verify it when we get the message
		if (!localParams.empty())
		{
			MessageToQueue::getInstance().sendMessageToJava(NetworkId(receiver), methodNameStdString, localParams, static_cast<int>(delay), guaranteed == JNI_TRUE, NetworkId(undeliveredCallbackObject), undeliveredCallbackMethodNameStdString);
		}
	}
	else
	{
		MessageToQueue::getInstance().sendMessageToJava(NetworkId(receiver), methodNameStdString, std::vector<int8>(), static_cast<int>(delay), guaranteed == JNI_TRUE, NetworkId(undeliveredCallbackObject), undeliveredCallbackMethodNameStdString);
	}
	
	return JNI_TRUE;
}

// ----------------------------------------------------------------------
/**
* "messageTo" for players on the current planet
*
* If you want everyone on the planet to receive the message,
* specify nullptr for loc and -1.0f for radius; otherwise, specify
* a loc and a radius and only players on the planet within the
* specified area will receive the message
*/
void JNICALL ScriptMethodsScriptNamespace::messageToPlayersOnPlanet(JNIEnv *env, jobject self, jstring methodName, jbyteArray params, jfloat delay, jobject loc, jfloat radius, jboolean includeDisconnectedPlayers)
{
	UNREF(self);

	JavaStringParam localMethodName(methodName);
	std::string methodNameStdString;
	if (!JavaLibrary::convert(localMethodName, methodNameStdString))
		return;

	std::vector<int8> localParams;
	if (params)
	{
		if (!ScriptConversion::convert(params, localParams))
			return;
	}

	Vector vectorLocation;
	if (loc)
	{
		if (!ScriptConversion::convertWorld(loc, vectorLocation))
			return;
	}

	// send to planet server for distribution to all game servers
	// running this planet; each game server will then check for
	// all players within the range and send the player the messageTo;
	// we need to do this because the range could cover multiple game servers
	GenericValueTypeMessage<std::pair<std::pair<std::pair<std::string, std::vector<int8> >, std::pair<float, bool> >, std::pair<Vector, float> > > const m("MessageToPlayersOnPlanet", std::make_pair(std::make_pair(std::make_pair(methodNameStdString, localParams), std::make_pair(delay, (includeDisconnectedPlayers != JNI_FALSE))), std::make_pair(vectorLocation, (loc ? radius : -1.0f))));
	GameServer::getInstance().sendToPlanetServer(m);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsScriptNamespace::remoteMessageToCalendarTimeDayOfWeek(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jbyteArray params, jint dayOfWeek, jint hour, jint minute, jint second)
{
	UNREF(self);

	JavaStringParam localMethodName(methodName);
	std::string methodNameStdString;
	if (!JavaLibrary::convert(localMethodName, methodNameStdString))
		return -1;

	// calculate the absolute time when the messageTo should go off
	time_t const timeNow = ::time(nullptr);
	time_t const timeTarget = CalendarTime::getNextGMTTimeOcurrence(timeNow, dayOfWeek, hour, minute, second);

	if ((timeTarget <= 0) || (timeTarget <= timeNow))
		return -1;

	int const timeUntil = static_cast<int>(timeTarget - timeNow);

	if (params != 0)
	{
		std::vector<int8> localParams;
		if (!ScriptConversion::convert(params, localParams))
			return -1;

		// add crc info to the string so we can verify it when we get the message
		if (!localParams.empty())
		{
			MessageToQueue::getInstance().sendMessageToJava(NetworkId(receiver), methodNameStdString, localParams, std::max(static_cast<int>(1), timeUntil + 2), false);
		}
	}
	else
	{
		MessageToQueue::getInstance().sendMessageToJava(NetworkId(receiver), methodNameStdString, std::vector<int8>(), std::max(static_cast<int>(1), timeUntil + 2), false);
	}

	return timeUntil;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsScriptNamespace::remoteMessageToCalendarTimeDayOfMonth(JNIEnv *env, jobject self, jlong receiver, jstring methodName, jbyteArray params, jint month, jint dayOfMonth, jint hour, jint minute, jint second)
{
	UNREF(self);

	JavaStringParam localMethodName(methodName);
	std::string methodNameStdString;
	if (!JavaLibrary::convert(localMethodName, methodNameStdString))
		return -1;

	// calculate the absolute time when the messageTo should go off
	time_t const timeNow = ::time(nullptr);
	time_t const timeTarget = CalendarTime::getNextGMTTimeOcurrence(timeNow, month, dayOfMonth, hour, minute, second);

	if ((timeTarget <= 0) || (timeTarget <= timeNow))
		return -1;

	int const timeUntil = static_cast<int>(timeTarget - timeNow);

	if (params != 0)
	{
		std::vector<int8> localParams;
		if (!ScriptConversion::convert(params, localParams))
			return -1;

		// add crc info to the string so we can verify it when we get the message
		if (!localParams.empty())
		{
			MessageToQueue::getInstance().sendMessageToJava(NetworkId(receiver), methodNameStdString, localParams, std::max(static_cast<int>(1), timeUntil + 2), false);
		}
	}
	else
	{
		MessageToQueue::getInstance().sendMessageToJava(NetworkId(receiver), methodNameStdString, std::vector<int8>(), std::max(static_cast<int>(1), timeUntil + 2), false);
	}

	return timeUntil;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsScriptNamespace::recurringMessageTo(JNIEnv * /*env*/, jobject /*self*/, jlong receiver, jstring methodName, jbyteArray params, jfloat delay)
{
	JavaStringParam localMethodName(methodName);
	std::string methodNameStdString;
	if (!JavaLibrary::convert(localMethodName, methodNameStdString))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::recurringMessageTo was not able to convert methodName into a string"));
		return;
	}
	
	if (params != 0)
	{
		std::vector<int8> localParams;
		if (!ScriptConversion::convert(params, localParams) || localParams.empty())
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::recurringMessageTo was not able to convert params into a std::vector<int8>"));
			return;
		}

		MessageToQueue::getInstance().sendRecurringMessageToJava(NetworkId(receiver), methodNameStdString, localParams, static_cast<int>(delay));
	}
	else
		MessageToQueue::getInstance().sendRecurringMessageToJava(NetworkId(receiver), methodNameStdString, std::vector<int8>(), static_cast<int>(delay));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsScriptNamespace::cancelRecurringMessageTo(JNIEnv *env, jobject self, jlong receiver, jstring methodName)
{
	JavaStringParam localMethodName(methodName);
	std::string methodNameStdString;
	if (!JavaLibrary::convert(localMethodName, methodNameStdString))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::cancelRecurringMessageTo was not able to convert methodName into a string"));
		return;
	}
	
	MessageToQueue::getInstance().cancelRecurringMessageTo(NetworkId(receiver), methodNameStdString);
}

// ----------------------------------------------------------------------
// returns the number of seconds until the messageTo fires, which *CAN* be 0
// returns -1 if object doesn't have the messageTo
jint JNICALL ScriptMethodsScriptNamespace::timeUntilMessageTo(JNIEnv *env, jobject self, jlong object, jstring methodName)
{
	ServerObject const * so = nullptr;
	if (!JavaLibrary::getObject(object, so))
		return -1;

	JavaStringParam localMethodName(methodName);
	std::string methodNameStdString;
	if (!JavaLibrary::convert(localMethodName, methodNameStdString))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::timeUntilMessageTo was not able to convert methodName into a string"));
		return -1;
	}

	return so->timeUntilMessageTo(methodNameStdString);
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsScriptNamespace::getServerFrame(JNIEnv * env, jobject self)
{
	if(env == 0)
		return 0;

	unsigned long t = ServerClock::getInstance().getServerFrame();
	return t;
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsScriptNamespace::getGameTimeSeconds(JNIEnv * env, jobject self)
{
	if(env == 0)
		return 0;

	unsigned long t = ServerClock::getInstance().getGameTimeSeconds();
	return t;
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsScriptNamespace::getCalendarTimeSeconds(JNIEnv * env, jobject self)
{
	if(env == 0)
		return 0;

	return ::time(nullptr);
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsScriptNamespace::getCalendarTimeSeconds2(JNIEnv * env, jobject self, jint year, jint month, jint day, jint hour, jint minute, jint second)
{
	if(env == 0)
		return -1;

	time_t const rawtime = ::time(nullptr);
	struct tm * timeinfo = ::localtime(&rawtime);
	if (!timeinfo)
		return -1;

	timeinfo->tm_year = year - 1900;
	timeinfo->tm_mon = month - 1;
	timeinfo->tm_mday = day;
	timeinfo->tm_hour = hour;
	timeinfo->tm_min = minute;
	timeinfo->tm_sec = second;

	return ::mktime(timeinfo);
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsScriptNamespace::getCalendarTimeStringGMT(JNIEnv *env, jobject self, jint calendarTimeSeconds)
{
	if(env == 0)
		return 0;

	return JavaString(CalendarTime::convertEpochToTimeStringGMT(calendarTimeSeconds)).getReturnValue();
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsScriptNamespace::getCalendarTimeStringGMT_YYYYMMDDHHMMSS(JNIEnv *env, jobject self, jint calendarTimeSeconds)
{
	if(env == 0)
		return 0;

	return JavaString(CalendarTime::convertEpochToTimeStringGMT_YYYYMMDDHHMMSS(calendarTimeSeconds)).getReturnValue();
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsScriptNamespace::getCalendarTimeStringLocal(JNIEnv *env, jobject self, jint calendarTimeSeconds)
{
	if(env == 0)
		return 0;

	return JavaString(CalendarTime::convertEpochToTimeStringLocal(calendarTimeSeconds)).getReturnValue();
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsScriptNamespace::getCalendarTimeStringLocal_YYYYMMDDHHMMSS(JNIEnv *env, jobject self, jint calendarTimeSeconds)
{
	if(env == 0)
		return 0;

	return JavaString(CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(calendarTimeSeconds)).getReturnValue();
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsScriptNamespace::secondsUntilCalendarTimeDayOfWeek(JNIEnv *env, jobject self, jint dayOfWeek, jint hour, jint minute, jint second)
{
	UNREF(self);

	// calculate the time between now and the specified time
	time_t const timeNow = ::time(nullptr);
	time_t const timeTarget = CalendarTime::getNextGMTTimeOcurrence(timeNow, dayOfWeek, hour, minute, second);

	if ((timeTarget <= 0) || (timeTarget <= timeNow))
		return -1;

	return static_cast<int>(timeTarget - timeNow);
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsScriptNamespace::secondsUntilCalendarTimeDayOfMonth(JNIEnv *env, jobject self, jint month, jint dayOfMonth, jint hour, jint minute, jint second)
{
	UNREF(self);

	// calculate the time between now and the specified time
	time_t const timeNow = ::time(nullptr);
	time_t const timeTarget = CalendarTime::getNextGMTTimeOcurrence(timeNow, month, dayOfMonth, hour, minute, second);

	if ((timeTarget <= 0) || (timeTarget <= timeNow))
		return -1;

	return static_cast<int>(timeTarget - timeNow);
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsScriptNamespace::reloadScriptFromScript(JNIEnv * env, jobject self, jstring scriptName)
{
	jboolean result = JNI_FALSE;
	
	JavaStringParam localScriptName(scriptName);
	std::string scriptNameStdString;
	if (!JavaLibrary::convert(localScriptName, scriptNameStdString))
		return JNI_FALSE;

	if(GameScriptObject::reloadScript(scriptNameStdString))
	{
		result = JNI_TRUE;
	}
	return result;
}

//-----------------------------------------------------------------------
