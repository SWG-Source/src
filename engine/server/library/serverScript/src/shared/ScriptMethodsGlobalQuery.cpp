// ======================================================================
//
// ScriptMethodsGlobalQuery.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/GameServer.h"
#include "serverGame/NameManager.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsGlobalQueryNamespace
// ======================================================================

namespace ScriptMethodsGlobalQueryNamespace
{
	bool install();

	jlong      JNICALL getPlayerIdFromFirstName(JNIEnv *env, jobject self, jstring firstName);
	jboolean     JNICALL playerExists(JNIEnv *env, jobject self, jlong playerId);
	jstring      JNICALL localize(JNIEnv *env, jobject self, jobject id);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsGlobalQueryNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsGlobalQueryNamespace::c)}
	JF("_getPlayerIdFromFirstName", "(Ljava/lang/String;)J", getPlayerIdFromFirstName),
	JF("_playerExists", "(J)Z", playerExists),
	JF("localize", "(Lscript/string_id;)Ljava/lang/String;", localize),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jlong JNICALL ScriptMethodsGlobalQueryNamespace::getPlayerIdFromFirstName(JNIEnv *env, jobject self, jstring firstName)
{
	std::string firstNameString;
	JavaLibrary::convert(JavaStringParam(firstName), firstNameString);
	return (NameManager::getInstance().getPlayerId(NameManager::normalizeName(firstNameString))).getValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsGlobalQueryNamespace::localize(JNIEnv *env, jobject self, jobject id)
{
	StringId textStringId;
	if (!ScriptConversion::convert(id, textStringId))
		return 0;
	Unicode::String textString;
	if (!textStringId.localize(textString))
		return 0;
	return JavaString(textString).getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsGlobalQueryNamespace::playerExists(JNIEnv *env, jobject self, jlong playerId)
{
	NetworkId playerNetworkId(playerId);
	return !NameManager::getInstance().getPlayerName(playerNetworkId).empty();
}

// ======================================================================

