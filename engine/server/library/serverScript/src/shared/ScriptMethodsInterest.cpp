//========================================================================
//
// ScriptMethodsInterest.cpp - implements script methods dealing with bank
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "sharedObject/NetworkIdManager.h"


// ======================================================================
// ScriptMethodsInterestNamespace
// ======================================================================

namespace ScriptMethodsInterestNamespace
{
	bool install();

	void JNICALL setAttributeAttained(JNIEnv *env, jobject script, jlong serverObject, jint attribute);
	void JNICALL clearAttributeAttained(JNIEnv *env, jobject script, jlong serverObject, jint attribute);
	bool JNICALL hasAttributeAttained(JNIEnv *env, jobject script, jlong serverObject, jint attribute);
	void JNICALL setAttributeInterested(JNIEnv *env, jobject script, jlong serverObject, jint attribute);
	void JNICALL clearAttributeInterested(JNIEnv *env, jobject script, jlong serverObject, jint attribute);
	bool JNICALL hasAttributeInterested(JNIEnv *env, jobject script, jlong serverObject, jint attribute);
	bool JNICALL isInterested(JNIEnv *env, jobject script, jlong serverObject, jlong serverObject2);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsInterestNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsInterestNamespace::c)}
	JF("_setAttributeAttained", "(JI)V",setAttributeAttained),
	JF("_clearAttributeAttained", "(JI)V",clearAttributeAttained),
	JF("_hasAttributeAttained", "(JI)Z",hasAttributeAttained),
	JF("_setAttributeInterested", "(JI)V",setAttributeInterested),
	JF("_clearAttributeInterested", "(JI)V",clearAttributeInterested),
	JF("_hasAttributeInterested", "(JI)Z",hasAttributeInterested),
	JF("_isInterested", "(JJ)Z",isInterested),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------

void JNICALL ScriptMethodsInterestNamespace::setAttributeAttained(JNIEnv *env, jobject script, jlong serverObject, jint attribute)
{
	ServerObject *obj1 = 0;
	if(!JavaLibrary::getObject(serverObject, obj1))
	{
		DEBUG_WARNING(true, ("[designer bug] JavaLibrary::setAttributeAttained object 1 is invalid"));
		return;
	}

	obj1->setAttributeAttained(attribute);
}

//-----------------------------------------------------------------

void JNICALL ScriptMethodsInterestNamespace::clearAttributeAttained(JNIEnv *env, jobject script, jlong serverObject, jint attribute)
{
	ServerObject *obj1 = 0;
	if(!JavaLibrary::getObject(serverObject, obj1))
	{
		DEBUG_WARNING(true, ("[designer bug] JavaLibrary::clearAttributeAttained object 1 is invalid"));
		return;
	}

	obj1->clearAttributeAttained(attribute);
}

//-----------------------------------------------------------------

bool JNICALL ScriptMethodsInterestNamespace::hasAttributeAttained(JNIEnv *env, jobject script, jlong serverObject, jint attribute)
{
	ServerObject *obj1 = 0;
	if(!JavaLibrary::getObject(serverObject, obj1))
	{
		DEBUG_WARNING(true, ("[designer bug] JavaLibrary::hasAttributeAttained object 1 is invalid"));
		return false;
	}

	return obj1->hasAttributeAttained(attribute);
}

//-----------------------------------------------------------------

void JNICALL ScriptMethodsInterestNamespace::setAttributeInterested(JNIEnv *env, jobject script, jlong serverObject, jint attribute)
{
	ServerObject *obj1 = 0;
	if(!JavaLibrary::getObject(serverObject, obj1))
	{
		DEBUG_WARNING(true, ("[designer bug] JavaLibrary::setAttributeInterested object 1 is invalid"));
		return;
	}

	obj1->setAttributeInterested(attribute);
}

//-----------------------------------------------------------------

void JNICALL ScriptMethodsInterestNamespace::clearAttributeInterested(JNIEnv *env, jobject script, jlong serverObject, jint attribute)
{
	ServerObject *obj1 = 0;
	if(!JavaLibrary::getObject(serverObject, obj1))
	{
		DEBUG_WARNING(true, ("[designer bug] JavaLibrary::clearAttributeInterested object 1 is invalid"));
		return;
	}

	obj1->clearAttributeInterested(attribute);
}

//-----------------------------------------------------------------

bool JNICALL ScriptMethodsInterestNamespace::hasAttributeInterested(JNIEnv *env, jobject script, jlong serverObject, jint attribute)
{
	ServerObject *obj1 = 0;
	if(!JavaLibrary::getObject(serverObject, obj1))
	{
		DEBUG_WARNING(true, ("[designer bug] JavaLibrary::hasAttributeInterested object 1 is invalid"));
		return false;
	}

	return obj1->hasAttributeInterest(attribute);
}

//-----------------------------------------------------------------

bool JNICALL ScriptMethodsInterestNamespace::isInterested(JNIEnv *env, jobject script, jlong serverObject, jlong serverObject2)
{
	ServerObject *obj1 = 0;
	if(!JavaLibrary::getObject(serverObject, obj1))
	{
		DEBUG_WARNING(true, ("[designer bug] JavaLibrary::isInterested object 1 is invalid"));
		return false;
	}

	ServerObject *obj2 = 0;
	if(!JavaLibrary::getObject(serverObject2, obj2))
	{
		DEBUG_WARNING(true, ("[designer bug] JavaLibrary::isInterested object 2 is invalid"));
		return false;
	}

	return obj1->isInterested(*obj2);
}

//==================================================================
