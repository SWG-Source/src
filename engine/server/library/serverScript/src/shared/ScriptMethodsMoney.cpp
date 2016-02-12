//========================================================================
//
// ScriptMethodsMoney.cpp - implements script methods dealing with money
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/GetMoneyFromOfflineObjectMessage.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsMoneyNamespace
// ======================================================================

namespace ScriptMethodsMoneyNamespace
{
	bool install();

	jboolean     JNICALL transferCashTo        (JNIEnv *env, jobject self, jlong source, jlong target, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData);
	jboolean     JNICALL transferBankCreditsTo (JNIEnv *env, jobject self, jlong source, jlong target, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData);
	jboolean     JNICALL transferBankCreditsToNamedAccount (JNIEnv *env, jobject self, jlong source, jstring target, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData);
	jboolean     JNICALL transferBankCreditsFromNamedAccount (JNIEnv *env, jobject self, jstring source, jlong target, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData);
	jboolean     JNICALL withdrawCashFromBank  (JNIEnv *env, jobject self, jlong source, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData);
	jboolean     JNICALL depositCashToBank     (JNIEnv *env, jobject self, jlong source, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData);
	void         JNICALL depositToGalacticReserve(JNIEnv *env, jobject self, jlong player);
	void         JNICALL withdrawFromGalacticReserve(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL canAccessGalacticReserve(JNIEnv *env, jobject self, jlong player);
	jint         JNICALL getCashBalance        (JNIEnv *env, jobject self, jlong source);
	jint         JNICALL getBankBalance        (JNIEnv *env, jobject self, jlong source);
	jint         JNICALL getTotalMoney         (JNIEnv *env, jobject self, jlong source);
	void         JNICALL getMoneyFromOfflineObject(JNIEnv *env, jobject self, jlong sourceObject, jint amount, jstring targetAccount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsMoneyNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsMoneyNamespace::c)}
	JF("_transferCashTo", "(JJILjava/lang/String;Ljava/lang/String;Lscript/dictionary;)Z",transferCashTo),
	JF("_transferBankCreditsTo", "(JJILjava/lang/String;Ljava/lang/String;Lscript/dictionary;)Z",transferBankCreditsTo),
	JF("_transferBankCreditsToNamedAccount", "(JLjava/lang/String;ILjava/lang/String;Ljava/lang/String;Lscript/dictionary;)Z",transferBankCreditsToNamedAccount),
	JF("_transferBankCreditsFromNamedAccount", "(Ljava/lang/String;JILjava/lang/String;Ljava/lang/String;Lscript/dictionary;)Z",transferBankCreditsFromNamedAccount),
	JF("_withdrawCashFromBank", "(JILjava/lang/String;Ljava/lang/String;Lscript/dictionary;)Z",withdrawCashFromBank),
	JF("_depositCashToBank", "(JILjava/lang/String;Ljava/lang/String;Lscript/dictionary;)Z",depositCashToBank),
	JF("_depositToGalacticReserve", "(J)V",depositToGalacticReserve),
	JF("_withdrawFromGalacticReserve", "(J)V",withdrawFromGalacticReserve),
	JF("_canAccessGalacticReserve", "(J)Z",canAccessGalacticReserve),
	JF("_getCashBalance", "(J)I",getCashBalance),
	JF("_getBankBalance", "(J)I",getBankBalance),
	JF("_getTotalMoney", "(J)I",getTotalMoney),
	JF("_getMoneyFromOfflineObject", "(JILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Lscript/dictionary;)V",getMoneyFromOfflineObject),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jboolean JNICALL ScriptMethodsMoneyNamespace::transferCashTo(JNIEnv *env, jobject self, jlong source, jlong target, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData)
{
	UNREF(self);

	// Check for missing/invalid parameters

	if (source == 0 || target == 0)
		return JNI_FALSE;
	if  (successCallbackFunction != 0 || failCallbackFunction != 0)
	{
		if (callbackData == 0)
			return JNI_FALSE;
		if (!env->IsInstanceOf(callbackData, JavaLibrary::getClsDictionary()))
			return JNI_FALSE;
	}

	// Convert parameters from Java to C++

	NetworkId         sourceObjId(source);
	NetworkId         targetObjId(target);
	NetworkId         ownerOID(JavaLibrary::getNetworkId(*callStaticObjectMethod(JavaLibrary::getClsScriptEntry(), JavaLibrary::getMidScriptEntryGetOwnerContext())));
	std::vector<int8> packedDictionary;
	std::string       successCallback;
	std::string       failCallback;

	if (callbackData != 0)
	{
		JavaDictionary const javaDictionary(callbackData);
		JavaLibrary::instance()->packDictionary(javaDictionary, packedDictionary);
	}
	if (successCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(successCallbackFunction), successCallback))
		return JNI_FALSE;
	if (failCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(failCallbackFunction), failCallback))
		return JNI_FALSE;
	ServerObject *sourceObj = ServerWorld::findObjectByNetworkId(sourceObjId);

	if (sourceObj == nullptr || targetObjId == NetworkId::cms_invalid || ownerOID == NetworkId::cms_invalid)
		return JNI_FALSE;

	// Do money transfer (will be forwarded if object is not authoritative)

	sourceObj->scriptTransferCashTo(targetObjId, amount, ownerOID, successCallback, failCallback, packedDictionary);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMoneyNamespace::transferBankCreditsTo(JNIEnv *env, jobject self, jlong source, jlong target, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData)
{
	UNREF(self);

	// Check for missing/invalid parameters

	if (source == 0 || target == 0)
		return JNI_FALSE;
	if  (successCallbackFunction != 0 || failCallbackFunction != 0)
	{
		if (callbackData == 0)
			return JNI_FALSE;
		if (!env->IsInstanceOf(callbackData, JavaLibrary::getClsDictionary()))
			return JNI_FALSE;
	}

	// Convert parameters from Java to C++

	NetworkId         sourceObjId(source);
	NetworkId         targetObjId(target);
	NetworkId         ownerOID(JavaLibrary::getNetworkId(*callStaticObjectMethod(JavaLibrary::getClsScriptEntry(), JavaLibrary::getMidScriptEntryGetOwnerContext())));
	std::vector<int8> packedDictionary;
	std::string       successCallback;
	std::string       failCallback;

	if (callbackData != 0)
	{
		JavaDictionary const javaDictionary(callbackData);
		JavaLibrary::instance()->packDictionary(javaDictionary, packedDictionary);
	}
	if (successCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(successCallbackFunction), successCallback))
		return JNI_FALSE;
	if (failCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(failCallbackFunction), failCallback))
		return JNI_FALSE;
	ServerObject *sourceObj = ServerWorld::findObjectByNetworkId(sourceObjId);

	if (sourceObj == nullptr || targetObjId == NetworkId::cms_invalid || ownerOID == NetworkId::cms_invalid)
		return JNI_FALSE;

	// Do money transfer (will be forwarded if object is not authoritative)

	sourceObj->scriptTransferBankCreditsTo(targetObjId, amount, ownerOID, successCallback, failCallback, packedDictionary);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMoneyNamespace::withdrawCashFromBank (JNIEnv *env, jobject self, jlong source, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData)
{
	UNREF(self);

	// Check for missing/invalid parameters

	if (source == 0)
		return JNI_FALSE;
	if  (successCallbackFunction != 0 || failCallbackFunction != 0)
	{
		if (callbackData == 0)
			return JNI_FALSE;
		if (!env->IsInstanceOf(callbackData, JavaLibrary::getClsDictionary()))
			return JNI_FALSE;
	}

	// Convert parameters from Java to C++

	NetworkId         sourceObjId(source);
	NetworkId         ownerOID(JavaLibrary::getNetworkId(*callStaticObjectMethod(JavaLibrary::getClsScriptEntry(), JavaLibrary::getMidScriptEntryGetOwnerContext())));
	std::vector<int8> packedDictionary;
	std::string       successCallback;
	std::string       failCallback;

	if (callbackData != 0)
	{
		JavaDictionary const javaDictionary(callbackData);
		JavaLibrary::instance()->packDictionary(javaDictionary, packedDictionary);
	}
	if (successCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(successCallbackFunction), successCallback))
		return JNI_FALSE;
	if (failCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(failCallbackFunction), failCallback))
		return JNI_FALSE;
	ServerObject *sourceObj = ServerWorld::findObjectByNetworkId(sourceObjId);

	if (sourceObj == nullptr  || ownerOID == NetworkId::cms_invalid)
		return JNI_FALSE;

	// Do money transfer (will be forwarded if object is not authoritative)

	sourceObj->scriptWithdrawCashFromBank(amount, ownerOID, successCallback, failCallback, packedDictionary);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMoneyNamespace::depositCashToBank(JNIEnv *env, jobject self, jlong source, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData)
{
	UNREF(self);

	// Check for missing/invalid parameters

	if (source == 0)
		return JNI_FALSE;
	if (successCallbackFunction != 0 || failCallbackFunction != 0)
	{
		if (callbackData == 0)
			return JNI_FALSE;
		if (!env->IsInstanceOf(callbackData, JavaLibrary::getClsDictionary()))
			return JNI_FALSE;
	}

	// Convert parameters from Java to C++
	NetworkId ownerOID(JavaLibrary::getNetworkId(*callStaticObjectMethod(JavaLibrary::getClsScriptEntry(), JavaLibrary::getMidScriptEntryGetOwnerContext())));
	if (ownerOID == NetworkId::cms_invalid)
		return JNI_FALSE;

	std::string successCallback;
	if (successCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(successCallbackFunction), successCallback))
		return JNI_FALSE;

	std::string failCallback;
	if (failCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(failCallbackFunction), failCallback))
		return JNI_FALSE;

	ServerObject *sourceObj = nullptr;
	if (!JavaLibrary::getObject(source, sourceObj))
		return JNI_FALSE;

	JavaDictionary callbackDictionary(callbackData);
	callbackDictionary.serialize();

	// Do money transfer (will be forwarded if object is not authoritative)
	sourceObj->scriptDepositCashToBank(amount, ownerOID, successCallback, failCallback,
		callbackDictionary.getSerializedData());
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsMoneyNamespace::depositToGalacticReserve(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject * objPlayer = nullptr;
	if (!JavaLibrary::getObject(player, objPlayer) || !objPlayer)
		return;

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(objPlayer);
	if (!playerObject)
		return;

	playerObject->depositToGalacticReserve(false);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsMoneyNamespace::withdrawFromGalacticReserve(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject * objPlayer = nullptr;
	if (!JavaLibrary::getObject(player, objPlayer) || !objPlayer)
		return;

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(objPlayer);
	if (!playerObject)
		return;

	playerObject->withdrawFromGalacticReserve();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMoneyNamespace::canAccessGalacticReserve(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject * objPlayer = nullptr;
	if (!JavaLibrary::getObject(player, objPlayer) || !objPlayer)
		return JNI_FALSE;

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(objPlayer);
	if (!playerObject)
		return JNI_FALSE;

	Vector const positionW = objPlayer->getPosition_w();
	return PlayerObject::canAccessGalacticReserve(positionW.x, positionW.z);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsMoneyNamespace::getCashBalance(JNIEnv *env, jobject self, jlong source)
{
	ServerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(source, sourceObj))
		return JNI_FALSE;

	return sourceObj->getCashBalance();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsMoneyNamespace::getBankBalance(JNIEnv *env, jobject self, jlong source)
{
	ServerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(source, sourceObj))
		return JNI_FALSE;

	return sourceObj->getBankBalance();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsMoneyNamespace::getTotalMoney(JNIEnv *env, jobject self, jlong source)
{
	ServerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(source, sourceObj))
		return JNI_FALSE;

	return sourceObj->getTotalMoney();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMoneyNamespace::transferBankCreditsToNamedAccount(JNIEnv *env, jobject self, jlong source, jstring target, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData)
{
	UNREF(self);

	// Check for missing/invalid parameters

	if (source == 0 || target == 0)
		return JNI_FALSE;
	if (successCallbackFunction != 0 || failCallbackFunction != 0)
	{
		// if you specify a callback, data must be valid
		if(callbackData == 0)
			return JNI_FALSE;
		if (!env->IsInstanceOf(callbackData, JavaLibrary::getClsDictionary()))
			return JNI_FALSE;
	}

	// Convert parameters from Java to C++

	NetworkId         sourceObjId(source);
	std::string       targetString;
	NetworkId         ownerOID(JavaLibrary::getNetworkId(*callStaticObjectMethod(JavaLibrary::getClsScriptEntry(), JavaLibrary::getMidScriptEntryGetOwnerContext())));
	std::vector<int8> packedDictionary;
	std::string       successCallback;
	std::string       failCallback;

	if (callbackData != 0)
	{
		JavaDictionary const javaDictionary(callbackData);
		JavaLibrary::instance()->packDictionary(javaDictionary, packedDictionary);
	}
	if (successCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(successCallbackFunction), successCallback))
		return JNI_FALSE;
	if (failCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(failCallbackFunction), failCallback))
		return JNI_FALSE;
	if (!JavaLibrary::convert(JavaStringParam(target), targetString))
		return JNI_FALSE;
	ServerObject *sourceObj = ServerWorld::findObjectByNetworkId(sourceObjId);

	if (sourceObj == nullptr || targetString.size() == 0 || ownerOID == NetworkId::cms_invalid)
		return JNI_FALSE;

	// Do money transfer (will be forwarded if object is not authoritative)

	sourceObj->scriptTransferBankCreditsTo(targetString, amount, ownerOID, successCallback, failCallback, packedDictionary);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsMoneyNamespace::transferBankCreditsFromNamedAccount(JNIEnv *env, jobject self, jstring source, jlong target, jint amount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData)
{
	UNREF(self);

	// Check for missing/invalid parameters

	if (source == 0 || target == 0)
		return JNI_FALSE;
	if (successCallbackFunction != 0 || failCallbackFunction != 0)
	{
		// if you specify a callback, data must be valid
		if(callbackData == 0)
			return JNI_FALSE;
		if (!env->IsInstanceOf(callbackData, JavaLibrary::getClsDictionary()))
			return JNI_FALSE;
	}

	// Convert parameters from Java to C++

	std::string       sourceString;
	NetworkId         targetObjId(target);
	NetworkId         ownerOID(JavaLibrary::getNetworkId(*callStaticObjectMethod(JavaLibrary::getClsScriptEntry(), JavaLibrary::getMidScriptEntryGetOwnerContext())));
	std::vector<int8> packedDictionary;
	std::string       successCallback;
	std::string       failCallback;

	if (callbackData != 0)
	{
		JavaDictionary const javaDictionary(callbackData);
		JavaLibrary::instance()->packDictionary(javaDictionary, packedDictionary);
	}
	if (successCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(successCallbackFunction), successCallback))
		return JNI_FALSE;
	if (failCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(failCallbackFunction), failCallback))
		return JNI_FALSE;
	if (!JavaLibrary::convert(JavaStringParam(source), sourceString))
		return JNI_FALSE;
	ServerObject *targetObj = ServerWorld::findObjectByNetworkId(targetObjId);

	if (targetObj == nullptr || sourceString.size() == 0 || ownerOID == NetworkId::cms_invalid)
		return JNI_FALSE;

	// Do money transfer (will be forwarded if object is not authoritative)

	targetObj->scriptTransferBankCreditsFrom(sourceString, amount, ownerOID, successCallback, failCallback, packedDictionary);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsMoneyNamespace::getMoneyFromOfflineObject(JNIEnv *env, jobject self, jlong sourceObject, jint amount, jstring targetAccount, jstring successCallbackFunction, jstring failCallbackFunction, jobject callbackData)
{
	// Convert parameters from Java types to C types
	DEBUG_REPORT_LOG(true,("getMoneyFromOfflineObject\n"));

	NetworkId const sourceObjectC(sourceObject);
	NetworkId const replyToC(JavaLibrary::getNetworkId(*callStaticObjectMethod(JavaLibrary::getClsScriptEntry(), JavaLibrary::getMidScriptEntryGetOwnerContext())));
	std::string targetAccountC;
	std::string successCallbackFunctionC;
	std::string failCallbackFunctionC;
	std::vector<int8> callbackDataC;

	if (targetAccount == 0 || !JavaLibrary::convert(JavaStringParam(targetAccount), targetAccountC))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("getMoneyFromOfflineObject:  targetAccount was not specified or could not be converted to a string."));
		return;
	}
	if (successCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(successCallbackFunction), successCallbackFunctionC))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("getMoneyFromOfflineObject:  successCallbackFunction could not be converted to a string."));
		return;
	}
	if (failCallbackFunction != 0 && !JavaLibrary::convert(JavaStringParam(failCallbackFunction), failCallbackFunctionC))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("getMoneyFromOfflineObject:  failCallbackFunction could not be converted to a string."));
		return;
	}
	if (callbackData != 0)
	{
		JavaDictionary const javaDictionary(callbackData);
		JavaLibrary::instance()->packDictionary(javaDictionary, callbackDataC);
	}

	// Do the money grab
	GetMoneyFromOfflineObjectMessage msg(sourceObjectC, amount, replyToC, successCallbackFunctionC, failCallbackFunctionC, callbackDataC, false);
	GameServer::getInstance().sendToDatabaseServer(msg);

	LOG("CustomerService", ("Money: offline transfer from %s to %s, amount %i", sourceObjectC.getValueString().c_str(), targetAccountC.c_str(), amount));
}

// ======================================================================
