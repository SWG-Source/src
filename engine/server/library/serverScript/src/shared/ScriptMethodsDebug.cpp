//=======================================================================
//
// ScriptMethodsDebug.cpp
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved.
//
//=======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerObject.h"
#include "serverScript/ConfigServerScript.h"
#include "sharedDebug/Profiler.h"
#include "sharedDebug/ProfilerTimer.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"

#ifdef linux
#include <signal.h>
#endif

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsDebugNamespace
// ======================================================================

namespace ScriptMethodsDebugNamespace
{
	std::set<std::string> s_profileSections;

	bool install();

	void JNICALL debugConsoleMsg(JNIEnv *env, jobject self, jlong objId, jstring msg);
	void JNICALL debugSpeakMsg(JNIEnv *env, jobject self, jlong objId, jstring msg);
	void JNICALL debugServerConsoleMsg(JNIEnv *env, jobject self, jlong objId, jstring msg);
	void JNICALL log(JNIEnv *env, jobject self, jstring channel, jstring msg, jlong logger, jlong player1, jlong player2, jboolean alwaysLog);
	void JNICALL logBalance(JNIEnv *env, jobject self, jstring msg);
	void JNICALL profilerStart(JNIEnv *env, jobject self, jstring sectionName);
	void JNICALL profilerStop(JNIEnv *env, jobject self, jstring sectionName);
	void JNICALL profilerStartAccum(JNIEnv *env, jobject self, jstring sectionName);
	void JNICALL profilerStopAccum(JNIEnv *env, jobject self, jstring sectionName);
	void JNICALL disableGameMemoryDump(JNIEnv *env, jobject self);
	void JNICALL debugMemoryReport(JNIEnv *env, jobject self);
	void JNICALL debugMemoryReportMap(JNIEnv *env, jobject self);
	void JNICALL generateJavacore(JNIEnv *env, jobject self);
	jlong JNICALL queryPerformanceCounter( JNIEnv * env, jobject self);
	jlong JNICALL queryPerformanceCounterFrequency(JNIEnv * env, jobject self);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsDebugNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsDebugNamespace::c)}
	JF("_debugConsoleMsg", "(JLjava/lang/String;)V", debugConsoleMsg),
	JF("_debugSpeakMsg", "(JLjava/lang/String;)V", debugSpeakMsg),
	JF("_debugServerConsoleMsg", "(JLjava/lang/String;)V", debugServerConsoleMsg),
	JF("__LOG", "(Ljava/lang/String;Ljava/lang/String;JJJZ)V", log),
	JF("logBalance", "(Ljava/lang/String;)V", logBalance),
	JF("PROFILER_START", "(Ljava/lang/String;)V", profilerStart),
	JF("PROFILER_STOP", "(Ljava/lang/String;)V", profilerStop),
	JF("PROFILER_START_ACCUM", "(Ljava/lang/String;)V", profilerStartAccum),
	JF("PROFILER_STOP_ACCUM", "(Ljava/lang/String;)V", profilerStopAccum),
	JF("disableGameMemoryDump", "()V", disableGameMemoryDump),
	JF("debugMemoryReport", "()V", debugMemoryReport),
	JF("debugMemoryReportMap", "()V", debugMemoryReportMap),
	JF("generateJavacore", "()V", generateJavacore),
	JF("queryPerformanceCounter", "()J", queryPerformanceCounter),
	JF("queryPerformanceCounterFrequency", "()J", queryPerformanceCounterFrequency)
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//----------------------------------------------------------------------

/**
 * Prints a debugging message to a player's console.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId         the player's avatar object id
 * @param msg			the message to print
 */
void JNICALL ScriptMethodsDebugNamespace::debugConsoleMsg(JNIEnv *env, jobject self, jlong objId, jstring msg)
{
	UNREF(self);

	JavaStringParam localMsg(msg);

	if (!ConfigServerScript::allowDebugConsoleMessages())
	{
		return;
	}

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objId, object))
	{
		return;
	}
	std::string msgString;
	JavaLibrary::convert(localMsg, msgString);
	if (ConfigServerGame::getJavaConsoleDebugMessages())
	{
		DEBUG_REPORT_LOG(true, ("debugConsoleMsg (%s): %s\n",
			object->getNetworkId().getValueString().c_str(), msgString.c_str()));
	}
	Client *client = object->getClient();
	if (client)
	{
		ConsoleMgr::broadcastString(msgString, client);
		return;
	}
}

//----------------------------------------------------------------------

/**
 * Prints a debugging message as if a creature spoke it.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId         the id of the object "speaking"
 * @param msg			the message to print
 */
void JNICALL ScriptMethodsDebugNamespace::debugSpeakMsg(JNIEnv *env, jobject self, jlong objId, jstring msg)
{
	UNREF(self);

	if (!ConfigServerScript::allowDebugSpeakMessages())
	{
		return;
	}

	JavaStringParam localMsg(msg);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objId, object))
	{
		DEBUG_REPORT_LOG(true, ("JavaLibrary::debugSpeakMsg invalid object\n"));
		return;
	}

	Unicode::String msgString;
	JavaLibrary::convert(localMsg, msgString);
	if (ConfigServerGame::getJavaConsoleDebugMessages())
	{
		DEBUG_REPORT_LOG(true, ("debugSpeakMsg (%s): %s\n",
			object->getNetworkId().getValueString().c_str(),
			Unicode::wideToNarrow(msgString).c_str()));
	}

	object->speakText(NetworkId::cms_invalid, 0, 0, 0, msgString, 0, Unicode::emptyString);
}	// JavaLibrary::debugSpeakMsg

//----------------------------------------------------------------------

/**
 * Prints a debugging message to the server console.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objId         the player's avatar object id
 * @param msg			the message to print
 */
void JNICALL ScriptMethodsDebugNamespace::debugServerConsoleMsg(JNIEnv *env, jobject self, jlong objId, jstring msg)
{
	if(!ConfigServerGame::getJavaConsoleDebugMessages())
		return;

	UNREF(self);

	JavaStringParam localMsg(msg);
	std::string msgString;
	JavaLibrary::convert(localMsg, msgString);

	ServerObject * object = 0;
	JavaLibrary::getObject(objId, object);

	if (object == nullptr)
	{
		DEBUG_REPORT_LOG(true, ("debugServerConsoleMsg from <unknown>: %s\n",
			msgString.c_str()));
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("debugServerConsoleMsg from %s: %s\n",
			object->getNetworkId().getValueString().c_str(), msgString.c_str()));
	}
}	// JavaLibrary::debugServerConsoleMsg

//----------------------------------------------------------------------

/**
 * Logs a message involving two players. The info for player one will replace the tag %TU in
 * the message, and the info for player two will replace the tag %TT in the message.
 * (The tags should match the prose package tag for user and target).
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param channel	the channel to log to
 * @param msg		the message to log
 * @param logger    id of the object where the log is coming from
 * @param player1	the 1st player for the message (may be nullptr)
 * @param player2	the 2nd player for the message (may be nullptr)
 * @param alwaysLog flag to ignore the disableScriptLogs flag and always log this message
 */
void JNICALL ScriptMethodsDebugNamespace::log(JNIEnv *env, jobject self, jstring channel, jstring msg, jlong logger, jlong player1, jlong player2, jboolean alwaysLog)
{
	UNREF(self);

	if (channel == 0 || msg == 0)
	{
		JavaLibrary::throwInternalScriptError("[designer bug] JavaLibrary::log called with nullptr channel or message");
		return;
	}

	if (alwaysLog || !ConfigServerScript::getDisableScriptLogs())
	{
		JavaStringParam localChan(channel);
		std::string chanStr;
		if (!JavaLibrary::convert(localChan, chanStr))
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::log could not convert channel to C string");
			return;
		}

		JavaStringParam localMsg(msg);
		std::string msgStr;
		if (!JavaLibrary::convert(localMsg, msgStr))
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::log could not convert message to C string");
			return;
		}

		NetworkId loggerId(logger);

		if (player1 != 0)
		{
			const ServerObject * playerObject = nullptr;
			if (JavaLibrary::getObject(player1, playerObject))
			{
				std::string::size_type p = msgStr.find("%TU");
				if (p != std::string::npos)
					msgStr.replace(p, 3, PlayerObject::getAccountDescription(playerObject));
			}
			else
			{
				WARNING(true, ("JavaLibrary::log could not get object for "
					"player1 passed for message %s", msgStr.c_str()));
			}
		}

		if (player2 != 0)
		{
			const ServerObject * playerObject = nullptr;
			if (JavaLibrary::getObject(player2, playerObject))
			{
				std::string::size_type p = msgStr.find("%TT");
				if (p != std::string::npos)
					msgStr.replace(p, 3, PlayerObject::getAccountDescription(playerObject));
			}
			else
			{
				WARNING(true, ("JavaLibrary::log could not get object for "
					"player2 passed for message %s", msgStr.c_str()));
			}
		}

		LOG(chanStr.c_str(), ("%s (from %s)", msgStr.c_str(), loggerId.getValueString().c_str()));
	}
}	// JavaLibrary::log

//----------------------------------------------------------------------

/**
 * Logs a message. To the GameBalance channel
 *
 * @param env      Java environment
 * @param self		 class calling this function
 * @param msg			 the message to log
 */
void JNICALL ScriptMethodsDebugNamespace::logBalance(JNIEnv *env, jobject self, jstring msg)
{
	UNREF(self);

	if (ConfigServerScript::getLogBalance())
	{
		JavaStringParam localMsg(msg);
		std::string msgStr;
		JavaLibrary::convert(localMsg, msgStr);
		LOG("GameBalance", ("balancelog:%d:%s", static_cast<int>(GameServer::getInstance().getProcessId()), msgStr.c_str()));
	}
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsDebugNamespace::profilerStart(JNIEnv *env, jobject self, jstring sectionName)
{
	UNREF(self);
	std::string sectionNameStr;
	JavaLibrary::convert(JavaStringParam(sectionName), sectionNameStr);
	PROFILER_BLOCK_DEFINE(p, (*s_profileSections.insert(sectionNameStr).first).c_str());
	PROFILER_BLOCK_ENTER(p);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsDebugNamespace::profilerStop(JNIEnv *env, jobject self, jstring sectionName)
{
	UNREF(self);
	std::string sectionNameStr;
	JavaLibrary::convert(JavaStringParam(sectionName), sectionNameStr);
	PROFILER_BLOCK_DEFINE(p, (*s_profileSections.insert(sectionNameStr).first).c_str());
	PROFILER_BLOCK_LOST_CHECK(p);
	PROFILER_BLOCK_LEAVE(p);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsDebugNamespace::profilerStartAccum(JNIEnv *env, jobject self, jstring sectionName)
{
	UNREF(self);
	std::string sectionNameStr;
	JavaLibrary::convert(JavaStringParam(sectionName), sectionNameStr);
	PROFILER_BLOCK_DEFINE(p, (*s_profileSections.insert(sectionNameStr).first).c_str());
	PROFILER_BLOCK_ENTER(p);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsDebugNamespace::profilerStopAccum(JNIEnv *env, jobject self, jstring sectionName)
{
	UNREF(self);
	std::string sectionNameStr;
	JavaLibrary::convert(JavaStringParam(sectionName), sectionNameStr);
	PROFILER_BLOCK_DEFINE(p, (*s_profileSections.insert(sectionNameStr).first).c_str());
	PROFILER_BLOCK_LOST_CHECK(p);
	PROFILER_BLOCK_LEAVE(p);
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsDebugNamespace::disableGameMemoryDump(JNIEnv *env, jobject self)
{
	UNREF(self);
	NetworkIdManager::setReportObjectLeaks(false);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsDebugNamespace::debugMemoryReport(JNIEnv *env, jobject self)
{
	UNREF(self);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsDebugNamespace::debugMemoryReportMap(JNIEnv *env, jobject self)
{
	UNREF(self);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsDebugNamespace::generateJavacore(JNIEnv *env, jobject self)
{
	UNREF(self);
	UNREF(env);

#ifdef linux
	if (JavaLibrary::getJavaVmType() == JavaLibrary::JV_ibm)
	{
		::kill(0, SIGQUIT);
	}
#endif
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsDebugNamespace::queryPerformanceCounter(JNIEnv * env, jobject self)
{
	UNREF(env);
	UNREF(self);
	ProfilerTimer::Type time;
	ProfilerTimer::getTime(time);
	
	return static_cast<jlong>(time);
}
// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsDebugNamespace::queryPerformanceCounterFrequency(JNIEnv * env, jobject self)
{
	UNREF(env);
	UNREF(self);
	ProfilerTimer::Type freq;
	ProfilerTimer::getFrequency(freq);

	return static_cast<jlong>(freq);
}

