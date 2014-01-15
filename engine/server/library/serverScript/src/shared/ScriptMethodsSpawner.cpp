// ScriptMethodsSpawner.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/ConfigFile.h"

//-----------------------------------------------------------------------

namespace ScriptMethodsSpawnerNamespace
{
	bool install();

	jint JNICALL getServerSpawnLimit(JNIEnv * env, jobject self);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsSpawnerNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsSpawnerNamespace::c)}
	JF("getServerSpawnLimit", "()I", getServerSpawnLimit),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsSpawnerNamespace::getServerSpawnLimit(JNIEnv * env, jobject self)
{
	UNREF(env);
	UNREF(self);

	return GameServer::getServerSpawnLimit();
}

//-----------------------------------------------------------------------
