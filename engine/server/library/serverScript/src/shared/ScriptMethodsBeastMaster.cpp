//========================================================================
//
// ScriptMethodsBeastMaster.cpp - script methods dealing with beastmaster stuff.
//
// copyright 2007 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"
#include "serverScript/JNIWrappers.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "sharedNetworkMessages/IncubatorStartMessage.h"
#include "sharedObject/Controller.h"

// ======================================================================
// ScriptMethodsBeastMaster
// ======================================================================

using namespace JNIWrappersNamespace;

namespace ScriptMethodsBeastMasterNamespace
{
		bool install();

		void JNICALL incubatorStart_development(JNIEnv *env, jobject self, jlong jplayerId, jlong jterminalId );
		void JNICALL incubatorStart(JNIEnv *env, jobject self,
			jint jsessionNumber, // I
			jlong jplayerId, // J
			jlong jterminalId, // J
			jint jpowerGauge, // I
			jint jinitialPointsSurvival, //I
			jint jinitialPointsBeastialResilience, // I
			jint jinitialPointsCunning, // I
			jint jinitialPointsIntelligence, // I
			jint jinitialPointsAggression, // I
			jint jinitialPointsHuntersInstinct, // I
			jint jtemperatureGauge, // I
			jint jnutrientGauge, // I
			jint jinitialCreatureColorIndex, // I
			jstring jcreatureTemplateName // Ljava/lang/String
			);
	    jboolean JNICALL setBeastMasterPetCommands(JNIEnv *env, jobject self, jlong objId, jobjectArray value);
		jboolean JNICALL setBeastMasterToggledPetCommands(JNIEnv *env, jobject self, jlong objId, jobjectArray value);
		void JNICALL setIncubatorPowerResourceName(JNIEnv *env, jobject self, jlong incubatorId, jstring resourceName);
};

//========================================================================
// install
//========================================================================

bool ScriptMethodsBeastMasterNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsBeastMasterNamespace::c)}
	JF("_incubatorStart_development", "(JJ)V", incubatorStart_development),
	JF("_incubatorStart","(IJJIIIIIIIIIILjava/lang/String;)V",incubatorStart),
	JF("_setBeastmasterPetCommands", "(J[Ljava/lang/String;)Z", setBeastMasterPetCommands),
	JF("_setBeastmasterToggledPetCommands", "(J[Ljava/lang/String;)Z", setBeastMasterToggledPetCommands),
	JF("_setIncubatorPowerResourceName","(JLjava/lang/String;)V",setIncubatorPowerResourceName),
	};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

//========================================================================
// class JavaLibrary JNI beastmaster callback methods
//========================================================================

void JNICALL ScriptMethodsBeastMasterNamespace::incubatorStart_development(JNIEnv *env, jobject self, jlong jplayerId, jlong jterminalId)
{
	UNREF(env);
	UNREF(self);

	ServerObject * playerObj = 0;
	if (!JavaLibrary::getObject(jplayerId, playerObj))
	{
		WARNING(true, ("bad player passed to JavaLibrary::incubatorStart"));
		return;
	}

	ServerObject * terminalObj = 0;
	if (!JavaLibrary::getObject(jterminalId, terminalObj))
	{
		WARNING(true, ("bad terminal passed to JavaLibrary::incubatorStart"));
		return;
	}

	Controller * const playerController = playerObj->getController();
	if(playerController)
	{
		// JU_TODO: test data

		IncubatorStartMessage* const msg = new IncubatorStartMessage(
			1,								// sessionNumber
			playerObj->getNetworkId(),		// playerId
			NetworkId::cms_invalid,			// terminalId
			75,								// powerGauge
			2,								// initialPointsSurvival
			3,								// initialPointsBeastialReilience,
			4,								// initialPointsCunning
			5,								// initialPointsIntelligence
			6,								// initialPointsAggression
			7,								// initialPointsHuntersInstinct
			2,								// temperatureGauge
			1,								// nutrientGauge
			-1,								// initialCreatureColorIndex
			"foobar_deadbeef"				// creatureTemplateName
		);

		playerController->appendMessage(CM_incubatorStart, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

//------------------------------------------------------------------------

void JNICALL ScriptMethodsBeastMasterNamespace::incubatorStart(JNIEnv *env, jobject self,
			jint jsessionNumber, // I
			jlong jplayerId, // J
			jlong jterminalId, // J
			jint jpowerGauge, // I
			jint jinitialPointsSurvival, //I
			jint jinitialPointsBeastialResilience, // I
			jint jinitialPointsCunning, // I
			jint jinitialPointsIntelligence, // I
			jint jinitialPointsAggression, // I
			jint jinitialPointsHuntersInstinct, // I
			jint jtemperatureGauge, // I
			jint jnutrientGauge, // I
			jint jinitialCreatureColorIndex, // I
			jstring jcreatureTemplateName // Ljava/lang/String
			)
{
	UNREF(env);
	UNREF(self);

	ServerObject * playerObj = 0;
	if (!JavaLibrary::getObject(jplayerId, playerObj))
	{
		WARNING(true, ("bad player passed to JavaLibrary::incubatorStart"));
		return;
	}

	ServerObject * terminalObj = 0;
	if (!JavaLibrary::getObject(jterminalId, terminalObj))
	{
		WARNING(true, ("bad terminal passed to JavaLibrary::incubatorStart"));
		return;
		
	}

	JavaStringParam localCreatureTemplateName(jcreatureTemplateName);
	std::string creatureTemplateName;
	if (!JavaLibrary::convert(localCreatureTemplateName, creatureTemplateName))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JNICALL incubatorStart - could not resolve creature template name"));
		return;
	}

	Controller * const playerController = playerObj->getController();
	if(playerController)
	{
		IncubatorStartMessage* const msg = new IncubatorStartMessage(
			jsessionNumber,								// sessionNumber
			playerObj->getNetworkId(),					// playerId
			terminalObj->getNetworkId(),				// terminalId
			jpowerGauge,								// powerGauge
			jinitialPointsSurvival,						// initialPointsSurvival
			jinitialPointsBeastialResilience,			// initialPointsBeastialReilience,
			jinitialPointsCunning,						// initialPointsCunning
			jinitialPointsIntelligence,					// initialPointsIntelligence
			jinitialPointsAggression,					// initialPointsAggression
			jinitialPointsHuntersInstinct,				// initialPointsHuntersInstinct
			jtemperatureGauge,							// temperatureGauge
			jnutrientGauge,								// nutrientGauge
			jinitialCreatureColorIndex,                 // initialCreatureColorIndex
			creatureTemplateName						// creatureTemplateName
		);

		playerController->appendMessage(CM_incubatorStart, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

jboolean JNICALL ScriptMethodsBeastMasterNamespace::setBeastMasterPetCommands(JNIEnv *env, jobject self, jlong objId, jobjectArray value)
{
	if (objId == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsStringArray()) == JNI_FALSE)
		return JNI_FALSE;
	

	CreatureObject *creatureObject = 0;
	if (!JavaLibrary::getObject(objId, creatureObject))
	{
		WARNING(true, ("ScriptMethodsBeastMasterNamespace::setBeastMasterPetCommands() Object could not be resolved to a CreatureObject"));
		return JNI_FALSE;
	}

	PlayerObject *po = PlayerCreatureController::getPlayerObject(creatureObject);

	if(!po)
	{
		WARNING(true, ("ScriptMethodsBeastMasterNamespace::setBeastMasterPetCommands() Object has no player object"));
		return JNI_FALSE;
	}

	std::vector<std::string> commands;
	size_t size = env->GetArrayLength(value);
	commands.resize(size);

	for (size_t i = 0; i < size; ++i)
	{
		JavaStringPtr item = getStringArrayElement(LocalObjectArrayRefParam(value), i);
		if (item != JavaString::cms_nullPtr)
			JavaLibrary::convert(*item, commands[i]);
		else
			commands[i].clear();
	}
	po->setPetCommandList(commands);
	return JNI_TRUE;
}

jboolean JNICALL ScriptMethodsBeastMasterNamespace::setBeastMasterToggledPetCommands(JNIEnv *env, jobject self, jlong objId, jobjectArray value)
{
	if (objId == 0 || value == 0)
		return JNI_FALSE;

	if (env->IsInstanceOf(value, JavaLibrary::getClsStringArray()) == JNI_FALSE)
		return JNI_FALSE;


	CreatureObject *creatureObject = 0;
	if (!JavaLibrary::getObject(objId, creatureObject))
	{
		WARNING(true, ("ScriptMethodsBeastMasterNamespace::setBeastMasterToggledPetCommands() Object could not be resolved to a CreatureObject"));
		return JNI_FALSE;
	}

	PlayerObject *po = PlayerCreatureController::getPlayerObject(creatureObject);

	if(!po)
	{
		WARNING(true, ("ScriptMethodsBeastMasterNamespace::setBeastMasterToggledPetCommands() Object has no player object"));
		return JNI_FALSE;
	}

	std::vector<std::string> commands;
	size_t size = env->GetArrayLength(value);
	commands.resize(size);

	for (size_t i = 0; i < size; ++i)
	{
		JavaStringPtr item = getStringArrayElement(LocalObjectArrayRefParam(value), i);
		if (item != JavaString::cms_nullPtr)
			JavaLibrary::convert(*item, commands[i]);
		else
			commands[i].clear();
	}
	po->setPetToggledCommands(commands);
	return JNI_TRUE;
}

void JNICALL ScriptMethodsBeastMasterNamespace::setIncubatorPowerResourceName(JNIEnv *env, jobject self, jlong incubatorId, jstring resourceName)
{
	UNREF(env);
	UNREF(self);

	ServerObject * incubatorObj = 0;
	if (!JavaLibrary::getObject(incubatorId, incubatorObj))
		return;

	if (!incubatorObj->asTangibleObject())
		return;

	if (resourceName == 0)
		return;

	JavaStringParam jspResourceName(resourceName);
	std::string sResourceName;
	if (!JavaLibrary::convert(jspResourceName, sResourceName))
		return;

	if (sResourceName.empty())
		return;

	IGNORE_RETURN(incubatorObj->setObjVarItem("power.name", sResourceName));

	// store additional information if the resource is an imported resource from another galaxy
	ResourceTypeObject const * const rto = ServerUniverse::getInstance().getResourceTypeByName(sResourceName);
	if (rto && (rto->getNetworkId().getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId))
	{
		std::string const resourceData = rto->getResourceTypeDataForExport();

		if (!resourceData.empty())
			IGNORE_RETURN(incubatorObj->setObjVarItem("power.resourceData", resourceData));
	}
}
