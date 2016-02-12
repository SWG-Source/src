//========================================================================
//
// ScriptMethodsBuffBuilder.cpp - script methods dealing with the buff builder.
//
// copyright 2006 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/Chat.h"
#include "serverGame/ServerBuffBuilderManager.h"
#include "serverGame/ServerObject.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/SharedBuffBuilderManager.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedNetworkMessages/BuffBuilderStartMessage.h"
#include "sharedObject/Controller.h"


// ======================================================================
// ScriptMethodsBuffBuilderNamespace
// ======================================================================

namespace ScriptMethodsBuffBuilderNamespace
{
	bool install();

	void         JNICALL buffBuilderStart(JNIEnv *env, jobject self, jlong jbufferId, jlong jrecipientId );
	jboolean     JNICALL buffBuilderValidated(JNIEnv *env, jobject self, 
		jlong jbufferId, 
		jlong jrecipientId, 
		
		jint jstartingTime, 
		jint jbufferRequiredCredits, 
		jint jrecipientPaidCredits, 
		
		jboolean jaccepted, 
		jobjectArray jbuffComponentKeys, 
		jintArray jbuffComponentValues);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsBuffBuilderNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsBuffBuilderNamespace::c)}
	JF("_buffBuilderStart", "(JJ)V", buffBuilderStart),
	JF("_buffBuilderValidated", "(JJIIIZ[Ljava/lang/String;[I)Z", buffBuilderValidated)
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI buff builder callback methods
//========================================================================

void JNICALL ScriptMethodsBuffBuilderNamespace::buffBuilderStart(JNIEnv *env, jobject self, jlong jbufferId, jlong jrecipientId)
{
	UNREF(env);
	UNREF(self);
	ServerObject * bufferObj = 0;
	if (!JavaLibrary::getObject(jbufferId, bufferObj))
	{
		WARNING(true, ("bad buffer passed to JavaLibrary::buffBuilderStart"));
		return;
	}

	ServerObject * recipientObj = 0;
	if (!JavaLibrary::getObject(jrecipientId, recipientObj))
	{
		WARNING(true, ("bad recipient passed to JavaLibrary::buffBuilderStart"));
		return;
	}

	SharedBuffBuilderManager::Session newSession;
	newSession.bufferId = bufferObj->getNetworkId();
	newSession.recipientId = recipientObj->getNetworkId();
	newSession.startingTime = Os::getRealSystemTime();

	SharedBuffBuilderManager::startSession(newSession);

	Controller * const bufferController = bufferObj->getController();
	if(bufferController)
	{
		BuffBuilderStartMessage * const msg = new BuffBuilderStartMessage(bufferObj->getNetworkId(), recipientObj->getNetworkId());
		bufferController->appendMessage(CM_buffBuilderStart, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	Controller * const recipientController = recipientObj->getController();
	if(recipientController && recipientController != bufferController)
	{
		BuffBuilderStartMessage * const msg = new BuffBuilderStartMessage(bufferObj->getNetworkId(), recipientObj->getNetworkId());
		recipientController->appendMessage(CM_buffBuilderStart, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

//------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsBuffBuilderNamespace::buffBuilderValidated(JNIEnv *env, jobject self, jlong jbufferId, jlong jrecipientId, jint jstartingTime, jint jbufferRequiredCredits, jint jrecipientPaidCredits, jboolean jaccepted, jobjectArray jbuffComponentKeys, jintArray jbuffComponentValues)
{
	UNREF(self);
	UNREF(jrecipientPaidCredits);
	
	SharedBuffBuilderManager::Session session;
	ServerObject * bufferObj = 0;
	if (!JavaLibrary::getObject(jbufferId, bufferObj) || !bufferObj)
	{
		WARNING(true, ("bad buffer passed to JavaLibrary::buffBuilderValidated"));
		return JNI_FALSE;
	}

	ServerObject * recipientObj = 0;
	if (!JavaLibrary::getObject(jrecipientId, recipientObj) || !recipientObj)
	{
		WARNING(true, ("bad recipient passed to JavaLibrary::buffBuilderValidated"));
		return JNI_FALSE;
	}

	session.bufferId = bufferObj->getNetworkId();
	session.recipientId = recipientObj->getNetworkId();
	session.startingTime = jstartingTime;
	session.bufferRequiredCredits = jbufferRequiredCredits;
	session.accepted = jaccepted;

	SharedBuffBuilderManager::Session currentSession;
	bool const gotCurrentSession = SharedBuffBuilderManager::getSession(session.bufferId, currentSession);
	if(!gotCurrentSession)
	{
		WARNING(true,("ScriptMethodsBuffBuilderNamespace::buffBuilderValidated - could not get current session\n"));
		return JNI_FALSE;
	}

	//if the session isn't valid, cancel it and don't apply changes
	if(!session.accepted)
	{
		Chat::sendSystemMessage(*bufferObj, SharedStringIds::buffbuilder_session_not_valid, Unicode::emptyString);
		
		if(bufferObj != recipientObj)
		{
			Chat::sendSystemMessage(*recipientObj, SharedStringIds::buffbuilder_session_not_valid, Unicode::emptyString);
		}
		ServerBuffBuilderManager::cancelSession(session.bufferId, session.recipientId);
		return JNI_TRUE;
	}

	std::string tempString;

	//get buffComponent values
	std::map<std::string, std::pair<int,int> > buffComponents;
	int countKeys = env->GetArrayLength(jbuffComponentKeys);
	int countValues = env->GetArrayLength(jbuffComponentValues);
	if(countKeys != countValues)
	{
		WARNING(true, ("index keys and values arrays are of diffent sizes"));
		return JNI_FALSE;
	}
	jint * const buffComponentsValuesArray = env->GetIntArrayElements(jbuffComponentValues, nullptr);
	if (buffComponentsValuesArray == nullptr)
	{
		return JNI_FALSE;
	}
	std::vector<int> buffComponentsValues(buffComponentsValuesArray, &buffComponentsValuesArray[countValues]);
	env->ReleaseIntArrayElements(jbuffComponentValues, buffComponentsValuesArray, JNI_ABORT);
	for (int j = 0; j < countKeys; ++j)
	{
		JavaString jTempString(static_cast<jstring>(env->GetObjectArrayElement(jbuffComponentKeys, j)));
		if (JavaLibrary::convert(jTempString, tempString))
		{
			buffComponents[tempString].first = buffComponentsValues[static_cast<unsigned int>(j)];
			buffComponents[tempString].second = currentSession.buffComponents[tempString].second;
		}
	}
	session.buffComponents = buffComponents;

	bool const result = ServerBuffBuilderManager::makeChanges(session);
	if(!result)
	{
		Chat::sendSystemMessage(*bufferObj, SharedStringIds::buffbuilder_session_not_valid, Unicode::emptyString);
		if(bufferObj != recipientObj)
		{
			Chat::sendSystemMessage(*recipientObj, SharedStringIds::buffbuilder_session_not_valid, Unicode::emptyString);
		}
		ServerBuffBuilderManager::cancelSession(session.bufferId, session.recipientId);
	}
	else
	{
		//send the final change message to the buffer to indicate acceptance
		
		Controller * const bufferController = bufferObj ? bufferObj->getController() : nullptr;
		if(bufferController)
		{
			BuffBuilderChangeMessage * outMsg = new BuffBuilderChangeMessage();

			outMsg->setBufferId(session.bufferId);
			outMsg->setRecipientId(session.recipientId);
			outMsg->setOrigin(BuffBuilderChangeMessage::O_SERVER);
			outMsg->setAccepted(true);
			bufferController->appendMessage(static_cast<int>(CM_buffBuilderChange), 0.0f, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}

	if(result)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

//------------------------------------------------------------------------
