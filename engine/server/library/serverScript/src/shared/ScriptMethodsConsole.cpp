// ScriptMethodsConsole.cpp
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved.
// Author: Roy Fleck

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "UnicodeUtils.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/ServerObject.h"
#include "serverScript/ConfigServerScript.h"


// ======================================================================
// ScriptMethodsConsoleNamespace
// ======================================================================

namespace ScriptMethodsConsoleNamespace
{
	bool install();

	void JNICALL consoleSendMessageObjId(JNIEnv * env, jobject self, jlong _to, jstring _message);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsConsoleNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsConsoleNamespace::c)}
	JF("_sendConsoleMessage", "(JLjava/lang/String;)V", consoleSendMessageObjId),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//----------------------------------------------------------------------

void JNICALL ScriptMethodsConsoleNamespace::consoleSendMessageObjId(JNIEnv * env, jobject self, jlong _to, jstring _message)
{
	UNREF(self);

	if (!_to)
		return;

	const ServerObject* player = 0;
	if(!JavaLibrary::getObject(_to, player))
	{
		DEBUG_WARNING (true, ("ScriptMethodsConsole JavaLibrary::consoleSendMessageObjId failed bad source object (not nullptr)"));
		if (ConfigServerScript::allowDebugConsoleMessages())
		{
			fprintf(stderr, "WARNING: ScriptMethodsConsole JavaLibrary::consoleSendMessageObjId: failed bad source object (not nullptr)\n");
			JavaLibrary::printJavaStack();
		}
		return;
	}

	std::string message;
	if (_message)
	{
		const JavaStringParam jt(_message);
		if (!JavaLibrary::convert(jt, message))
			return;
	}

	if (message.empty())
	{
		DEBUG_WARNING (true, ("consoleSendMessageObjId with empty message"));
	}

	ConsoleMgr::broadcastString(message, player->getNetworkId());
}

//----------------------------------------------------------------------

