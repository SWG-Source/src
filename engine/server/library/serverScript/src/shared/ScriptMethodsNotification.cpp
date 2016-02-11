// ======================================================================
//
// ScriptMethodsNotification.cpp
// Copyright 2003, Sony Online Entertainment, Inc.
// All rights reserved.
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "sharedRandom/Random.h"
#include "sharedNetworkMessages/ClientNotificationBoxMessage.h"

#include "UnicodeUtils.h"

using namespace JNIWrappersNamespace;


// ======================================================================

namespace ScriptMethodsNotificationNamespace
{
	bool install();

	jint JNICALL addNotification(JNIEnv *env, jobject self, jlong player, jstring contents, jboolean useNotificationIcon, jint iconStyle, jfloat timeout, jint channel, jstring sound);
	void JNICALL cancelNotification(JNIEnv *env, jobject self, jlong player, jint notification);
	void JNICALL cancelAllNotifications(JNIEnv *env, jobject self, jlong player);
}

using namespace ScriptMethodsNotificationNamespace;

//========================================================================
// install
//========================================================================

bool ScriptMethodsNotificationNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsNotificationNamespace::c)}
	JF("_addNotification",        "(JLjava/lang/String;ZIFILjava/lang/String;)I", addNotification),
	JF("_cancelNotification",     "(JI)V", cancelNotification),
	JF("_cancelAllNotifications", "(J)V", cancelAllNotifications),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jint JNICALL ScriptMethodsNotificationNamespace::addNotification(JNIEnv *env, jobject self, jlong player, jstring contents, jboolean useNotificationIcon, jint iconStyle, jfloat timeout, jint channel, jstring sound)
{	
	CreatureObject const * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return 0;
	
	JavaStringParam localContents(contents);
	Unicode::String localContentsString;
	JavaLibrary::convert(localContents, localContentsString);
	if (localContentsString.empty())
		return 0;

	JavaStringParam localSoundContents(sound);
	std::string localSoundContentsString;
	JavaLibrary::convert(localSoundContents, localSoundContentsString);
	
	int sequenceId = static_cast<int>(Random::random());
	ClientNotificationBoxMessage cnbm(sequenceId, playerObject->getNetworkId(), localContentsString, useNotificationIcon, iconStyle, timeout, channel, localSoundContentsString);
	
	if(playerObject->getClient())
	{
		playerObject->getClient()->send(cnbm, true);		
		return sequenceId;
	}
	return 0;
}

void JNICALL ScriptMethodsNotificationNamespace::cancelNotification(JNIEnv *env, jobject self, jlong player, jint notification)
{
	CreatureObject const * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return;
	ClientNotificationBoxMessage cnbm(notification, playerObject->getNetworkId(), Unicode::emptyString, FALSE, 0, 0.0f, ClientNotificationBoxMessage::NC_SPECIAL_CANCEL, std::string(""));
	
	if(playerObject->getClient())
	{
		playerObject->getClient()->send(cnbm, true);		
		return;
	}
	return;
}

void JNICALL ScriptMethodsNotificationNamespace::cancelAllNotifications(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject const * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return;
	ClientNotificationBoxMessage cnbm(0, playerObject->getNetworkId(), Unicode::emptyString, FALSE, 0, 0.0f, ClientNotificationBoxMessage::NC_SPECIAL_CANCEL_ALL, std::string(""));
	
	if(playerObject->getClient())
	{
		playerObject->getClient()->send(cnbm, true);		
		return;
	}
	return;
}

