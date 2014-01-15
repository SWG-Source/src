//-----------------------------------------------------------------------

// ScriptMethodsRemoteDebug.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"


// ======================================================================
// ScriptMethodsRemoteDebugNamespace
// ======================================================================

namespace ScriptMethodsRemoteDebugNamespace
{
	bool install();

	void JNICALL printChannelMessage (JNIEnv *env, jobject self, jstring channel, jstring message);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsRemoteDebugNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsRemoteDebugNamespace::c)}
	JF("printChannelMessage", "(Ljava/lang/String;Ljava/lang/String;)V", printChannelMessage),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

void JNICALL ScriptMethodsRemoteDebugNamespace::printChannelMessage (JNIEnv *env, jobject self, jstring channel, jstring message)
{
	JavaStringParam localChannel(channel);
	std::string channelText;
	JavaLibrary::convert(localChannel, channelText);
	JavaStringParam localMessage(message);
	std::string messageText;
	JavaLibrary::convert(localMessage, messageText);

	DEBUG_OUTPUT_CHANNEL(channelText.c_str(), ("%s", messageText.c_str()));
}

//-----------------------------------------------------------------------
