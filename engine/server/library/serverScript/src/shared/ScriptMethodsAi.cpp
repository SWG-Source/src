// ======================================================================
//
// ScriptMethodsAi.cpp
// Copyright 2005 Sony Online Eentertainment, Inc.
// All rights reserved.
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AiCreatureCombatProfile.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/AiLogManager.h"
#include "serverGame/CellObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/HateList.h"
#include "serverGame/WeaponObject.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedLog/Log.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsAiNamespace
// ======================================================================

namespace ScriptMethodsAiNamespace
{
	bool install();
	AICreatureController * const getAiCreatureController(jlong ai);
	
	jint     JNICALL aiGetMovementState(JNIEnv *env, jobject self, jlong ai);
	jboolean JNICALL aiLoggingEnabled(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiIsFrozen(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiIsAggressive(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiIsAssist(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiIsStalker(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiIsKiller(JNIEnv * env, jobject self, jlong ai);
	void     JNICALL aiTether(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiIsTethered(JNIEnv * env, jobject self, jlong ai);
	void     JNICALL aiSetHomeLocation(JNIEnv * env, jobject self, jlong ai, jobject location);
	jobject  JNICALL aiGetHomeLocation(JNIEnv * env, jobject self, jlong ai);
	jobject  JNICALL aiGetLeashAnchorLocation(JNIEnv * env, jobject self, jlong ai);
	jfloat   JNICALL aiGetLeashRadius(JNIEnv * env, jobject self);
	jfloat   JNICALL aiGetRespectRadius(JNIEnv * env, jobject self, jlong ai, jlong target);
	jfloat   JNICALL aiGetAggroRadius(JNIEnv * env, jobject self, jlong ai);
	void     JNICALL aiEquipPrimaryWeapon(JNIEnv * env, jobject self, jlong ai);
	void     JNICALL aiEquipSecondaryWeapon(JNIEnv * env, jobject self, jlong ai);
	void     JNICALL aiUnEquipWeapons(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiHasPrimaryWeapon(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiHasSecondaryWeapon(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiUsingPrimaryWeapon(JNIEnv * env, jobject self, jlong ai);
	jboolean JNICALL aiUsingSecondaryWeapon(JNIEnv * env, jobject self, jlong ai);
	jlong    JNICALL aiGetPrimaryWeapon(JNIEnv * env, jobject self, jlong ai);
	jlong    JNICALL aiGetSecondaryWeapon(JNIEnv * env, jobject self, jlong ai);
	float    JNICALL aiGetMovementSpeedPercent(JNIEnv * env, jobject self, jlong ai);
	jstring  JNICALL aiGetCombatAction(JNIEnv * env, jobject self, jlong ai);
	jint     JNICALL aiGetKnockDownRecoveryTime(JNIEnv * env, jobject self, jlong ai);
	void     JNICALL pathToLocation(JNIEnv *env, jobject self, jlong mob, jobject location);
	void     JNICALL pathToName(JNIEnv *env, jobject self, jlong objectId, jstring targetName);
	void     JNICALL patrolToLocation(JNIEnv *env, jobject self, jlong ai, jobjectArray targets, jboolean random, jboolean flip, jboolean repeat, jint startPoint);
	void     JNICALL patrolToName(JNIEnv *env, jobject self, jlong ai, jobjectArray targetNames, jboolean random, jboolean flip, jboolean repeat, jint startPoint);
	void     JNICALL loiterLocation(JNIEnv *env, jobject self, jlong ai, jobject anchorLocation, jfloat minDistance, jfloat maxDistance, jfloat minDelay, jfloat maxDelay);
	void     JNICALL loiterTarget(JNIEnv *env, jobject self, jlong ai, jlong target, jfloat minDistance, jfloat maxDistance, jfloat minDelay, jfloat maxDelay);
	jboolean JNICALL setHibernationDelay(JNIEnv *env, jobject self, jlong creature, jfloat delay);
}

using namespace ScriptMethodsAiNamespace;

//========================================================================
// install
//========================================================================

bool ScriptMethodsAiNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsAiNamespace::c)}
	JF("_aiGetMovementState", "(J)I", aiGetMovementState),
	JF("_aiLoggingEnabled", "(J)Z", aiLoggingEnabled),
	JF("_aiIsFrozen", "(J)Z", aiIsFrozen),
	JF("_aiIsAggressive", "(J)Z", aiIsAggressive),
	JF("_aiIsAssist", "(J)Z", aiIsAssist),
	JF("_aiIsStalker", "(J)Z", aiIsStalker),
	JF("_aiIsKiller", "(J)Z", aiIsKiller),
	JF("_aiTether", "(J)V", aiTether),
	JF("_aiIsTethered", "(J)Z", aiIsTethered),
	JF("_aiSetHomeLocation", "(JLscript/location;)V", aiSetHomeLocation),
	JF("_aiGetHomeLocation", "(J)Lscript/location;", aiGetHomeLocation),
	JF("_aiGetLeashAnchorLocation", "(J)Lscript/location;", aiGetLeashAnchorLocation),
	JF("aiGetLeashRadius", "()F", aiGetLeashRadius),
	JF("_aiGetRespectRadius", "(JJ)F", aiGetRespectRadius),
	JF("_aiGetAggroRadius", "(J)F", aiGetAggroRadius),
	JF("_aiEquipPrimaryWeapon", "(J)V", aiEquipPrimaryWeapon),
	JF("_aiEquipSecondaryWeapon", "(J)V", aiEquipSecondaryWeapon),
	JF("_aiUnEquipWeapons", "(J)V", aiUnEquipWeapons),
	JF("_aiUsingPrimaryWeapon", "(J)Z", aiUsingPrimaryWeapon),
	JF("_aiHasPrimaryWeapon", "(J)Z", aiHasPrimaryWeapon),
	JF("_aiHasSecondaryWeapon", "(J)Z", aiHasSecondaryWeapon),
	JF("_aiUsingSecondaryWeapon", "(J)Z", aiUsingSecondaryWeapon),
	JF("_aiGetPrimaryWeapon", "(J)J", aiGetPrimaryWeapon),
	JF("_aiGetSecondaryWeapon", "(J)J", aiGetSecondaryWeapon),
	JF("_aiGetMovementSpeedPercent", "(J)F", aiGetMovementSpeedPercent),
	JF("_aiGetCombatAction", "(J)Ljava/lang/String;", aiGetCombatAction),
	JF("_aiGetKnockDownRecoveryTime", "(J)I", aiGetKnockDownRecoveryTime),
	JF("_pathTo", "(JLscript/location;)V", pathToLocation),
	JF("_pathTo", "(JLjava/lang/String;)V", pathToName),
	JF("_patrol", "(J[Lscript/location;ZZZI)V", patrolToLocation),
	JF("_patrol", "(J[Ljava/lang/String;ZZZI)V", patrolToName),
	JF("_loiterLocation", "(JLscript/location;FFFF)V", loiterLocation),
	JF("_loiterTarget", "(JJFFFF)V", loiterTarget),
	JF("_setHibernationDelay", "(JF)Z", setHibernationDelay),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

// ----------------------------------------------------------------------

AICreatureController * const ScriptMethodsAiNamespace::getAiCreatureController(jlong ai)
{
	NetworkId const aiNetworkId(ai);
	CreatureObject * const aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

	if (aiCreatureObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("Unable to resolve the ai(%s) to a CreatureObject.", aiNetworkId.getValueString().c_str()));
		return nullptr;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(aiCreatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("Unable to get the ai's(%s) AiCreatureController.", aiCreatureObject->getDebugInformation().c_str()));
		return nullptr;
	}

	return aiCreatureController;
}

// ======================================================================
//
// ScriptMethodsAi
//
// ======================================================================

// ----------------------------------------------------------------------
jint JNICALL ScriptMethodsAiNamespace::aiGetMovementState(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	NetworkId const aiNetworkId(ai);
	CreatureObject * const aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

	if (aiCreatureObject == nullptr)
	{
		return AMT_invalid;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(aiCreatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		return AMT_invalid;
	}

	jint const nextState = aiCreatureController->getPendingMovementType();

	// If we will change states next frame, return the pending state

	if (nextState != AMT_invalid)
	{
		return nextState;
	}

	jint const currentState = aiCreatureController->getMovementType();

	return currentState;
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiLoggingEnabled(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	NetworkId const aiNetworkId(ai);
	CreatureObject * const aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

	if (aiCreatureObject == nullptr)
	{
		return JNI_FALSE;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(aiCreatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	if (ConfigServerGame::isAiLoggingEnabled())
	{
		return JNI_TRUE;
	}

	return aiCreatureController->isLogging();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiIsFrozen(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->isFrozen();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiIsAggressive(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	NetworkId const aiNetworkId(ai);
	CreatureObject * const aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

	if (aiCreatureObject == nullptr)
	{
		return JNI_FALSE;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(aiCreatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->isAggressive();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiIsAssist(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	NetworkId const aiNetworkId(ai);
	CreatureObject * const aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

	if (aiCreatureObject == nullptr)
	{
		return JNI_FALSE;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(aiCreatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->isAssist();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiIsStalker(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->isStalker();
}


// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiIsKiller(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	NetworkId const aiNetworkId(ai);
	CreatureObject * const aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

	if (aiCreatureObject == nullptr)
	{
		return JNI_FALSE;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(aiCreatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->isKiller();
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsAiNamespace::aiTether(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	aiCreatureController->setRetreating(true);
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiIsTethered(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	NetworkId const aiNetworkId(ai);
	CreatureObject * const aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

	if (aiCreatureObject == nullptr)
	{
		return JNI_FALSE;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(aiCreatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->isRetreating();
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsAiNamespace::aiSetHomeLocation(JNIEnv * /*env*/, jobject /*self*/, jlong ai, jobject location)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	Location homeLocation;
	if (!ScriptConversion::convert(LocalRefParam(location), homeLocation))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsAi::aiSetHomeLocation() Unable to resolve the ai's(%s) location to a Location.", aiCreatureController->getCreature()->getDebugInformation().c_str()));
		return;
	}

	aiCreatureController->setHomeLocation(homeLocation);
}

// ----------------------------------------------------------------------
jobject JNICALL ScriptMethodsAiNamespace::aiGetHomeLocation(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return JavaLibrary::getVector(Vector::zero)->getReturnValue();
	}

	Location const & homeLocation = aiCreatureController->getHomeLocation();
	LocalRefPtr location;
	ScriptConversion::convert(homeLocation, location);

	return location->getReturnValue();
}

// ----------------------------------------------------------------------
jobject JNICALL ScriptMethodsAiNamespace::aiGetLeashAnchorLocation(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return JavaLibrary::getVector(Vector::zero)->getReturnValue();
	}

	Location const & startCombatLocation = aiCreatureController->getCombatStartLocation();
	LocalRefPtr location;
	ScriptConversion::convert(startCombatLocation, location);

	return location->getReturnValue();
}

// ----------------------------------------------------------------------
jfloat JNICALL ScriptMethodsAiNamespace::aiGetLeashRadius(JNIEnv * /*env*/, jobject /*self*/)
{
	return AICreatureController::getLeashRadius();
}

// ----------------------------------------------------------------------
jfloat JNICALL ScriptMethodsAiNamespace::aiGetRespectRadius(JNIEnv * /*env*/, jobject /*self*/, jlong ai, jlong target)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return 0.0f;
	}

	NetworkId const targetNetworkId(target);

	return aiCreatureController->getRespectRadius(targetNetworkId);
}

// ----------------------------------------------------------------------
jfloat JNICALL ScriptMethodsAiNamespace::aiGetAggroRadius(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return 0.0f;
	}

	return aiCreatureController->getAggroRadius();
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsAiNamespace::aiEquipPrimaryWeapon(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	aiCreatureController->equipPrimaryWeapon();
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsAiNamespace::aiEquipSecondaryWeapon(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	aiCreatureController->equipSecondaryWeapon();
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsAiNamespace::aiUnEquipWeapons(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	aiCreatureController->unEquipWeapons();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiHasPrimaryWeapon(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->hasPrimaryWeapon();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiHasSecondaryWeapon(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->hasSecondaryWeapon();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiUsingPrimaryWeapon(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->usingPrimaryWeapon();
}

// ----------------------------------------------------------------------
jboolean JNICALL ScriptMethodsAiNamespace::aiUsingSecondaryWeapon(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return JNI_FALSE;
	}

	return aiCreatureController->usingSecondaryWeapon();
}

// ----------------------------------------------------------------------
jlong JNICALL ScriptMethodsAiNamespace::aiGetPrimaryWeapon(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return 0;
	}

	return (aiCreatureController->getPrimaryWeapon()).getValue();
}

// ----------------------------------------------------------------------
jlong JNICALL ScriptMethodsAiNamespace::aiGetSecondaryWeapon(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return 0;
	}

	return (aiCreatureController->getSecondaryWeapon()).getValue();
}

// ----------------------------------------------------------------------
jfloat JNICALL ScriptMethodsAiNamespace::aiGetMovementSpeedPercent(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return 0.0f;
	}

	return aiCreatureController->getMovementSpeedPercent();
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsAiNamespace::loiterLocation(JNIEnv * /*env*/, jobject /*self*/, jlong ai, jobject anchorLocation, jfloat minDistance, jfloat maxDistance, jfloat minDelay, jfloat maxDelay)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	if (aiCreatureController->isRetreating())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(ConfigServerGame::getMovementWhileRetreatingThrowsException(),
		                            ("ScriptMethodsAi::loiterLocation() ai(%s) Calling a movement command on a retreating AI is not allowed.", aiCreatureController->getCreature()->getDebugInformation().c_str()));
		return;
	}

	Vector position_c;
	NetworkId cellId;

	if (ScriptConversion::convert(anchorLocation, position_c, cellId))
	{
		if (cellId == NetworkId::cms_invalid)
		{
			LOGC(AiLogManager::isLogging(aiCreatureController->getOwner()->getNetworkId()), "debug_ai", ("ScriptMethodsAi::loiterLocation() ai(%s) WORLD CELL", aiCreatureController->getOwner()->getNetworkId().getValueString().c_str()));

			CellProperty const * const cellProperty = CellProperty::getWorldCellProperty();

			aiCreatureController->loiter(cellProperty, position_c, minDistance, maxDistance, minDelay, maxDelay);
		}
		else
		{
			LOGC(AiLogManager::isLogging(aiCreatureController->getOwner()->getNetworkId()), "debug_ai", ("ScriptMethodsAi::loiterLocation() ai(%s) NON-WORLD CELL", aiCreatureController->getOwner()->getNetworkId().getValueString().c_str()));

			CellObject const * const cellObject = CellObject::getCellObject(cellId);

			if (cellObject != nullptr)
			{
				CellProperty const * const cellProperty = cellObject->getCellProperty();

				aiCreatureController->loiter(cellProperty, position_c, minDistance, maxDistance, minDelay, maxDelay);
			}
			else
			{
				JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsAi::loiterLocation() ai(%s) Unable to resolve the cellId(%s) to a CellObject", aiCreatureController->getCreature()->getDebugInformation().c_str(), cellId.getValueString().c_str()));
			}
		}
	}
	else
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsAi::loiterLocation() ai(%s) Unable to resolve the loiter location", aiCreatureController->getCreature()->getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsAiNamespace::loiterTarget(JNIEnv * /*env*/, jobject /*self*/, jlong ai, jlong target, jfloat minDistance, jfloat maxDistance, jfloat minDelay, jfloat maxDelay)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	if (aiCreatureController->isRetreating())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(ConfigServerGame::getMovementWhileRetreatingThrowsException(),
		                            ("ScriptMethodsAi::loiterTarget() ai(%s) Calling a movement command on a retreating AI is not allowed.", aiCreatureController->getCreature()->getDebugInformation().c_str()));
		return;
	}

	NetworkId const targetNetworkId(static_cast<NetworkId::NetworkIdType>(target));
	Object * const targetObject = NetworkIdManager::getObjectById(targetNetworkId);

	if (targetObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsAi::loiterTarget() ai(%s) Unable to resolve the target(%s) to a Object", aiCreatureController->getCreature()->getDebugInformation().c_str(), targetNetworkId.getValueString().c_str()));
		return;
	}

	if (aiCreatureController->getCreature()->getNetworkId() == targetNetworkId)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsAi::loiterTarget() ai(%s) The AI was told to loiter itself", aiCreatureController->getCreature()->getDebugInformation().c_str()));
		return;
	}

	aiCreatureController->loiter(targetObject->getNetworkId(), minDistance, maxDistance, minDelay, maxDelay);
}

// ----------------------------------------------------------------------
void JNICALL ScriptMethodsAiNamespace::pathToLocation(JNIEnv * /*env*/, jobject /*self*/, jlong ai, jobject location)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	if (aiCreatureController->isRetreating())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(ConfigServerGame::getMovementWhileRetreatingThrowsException(),
		                            ("ScriptMethodsAi::pathToLocation() ai(%s) Calling a movement command on a retreating AI is not allowed.", aiCreatureController->getCreature()->getDebugInformation().c_str()));
		return;
	}

	Vector position_c;
	NetworkId cellId;

	if (ScriptConversion::convert(location, position_c, cellId))
	{
		if (cellId != NetworkId::cms_invalid)
		{
			CellObject const * const cellObject = CellObject::getCellObject(cellId);

			if (cellObject != nullptr)
			{
				CellProperty const * const cellProperty = cellObject->getCellProperty();

				aiCreatureController->moveTo(cellProperty, position_c);
			}
			else
			{
				JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsAi::pathToLocation() ai(%s) Unable to resolve the cellId(%s) to a CellObject", aiCreatureController->getCreature()->getDebugInformation().c_str(), cellId.getValueString().c_str()));
				return;
			}
		}
		else
		{
			aiCreatureController->moveTo(CellProperty::getWorldCellProperty(), position_c);
		}
	}
	else
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsAi::pathToLocation() ai(%s) Unable to resolve the destination location", aiCreatureController->getCreature()->getDebugInformation().c_str()));
		return;
	}
}

// ----------------------------------------------------------------------
/**
 * Plots a path from an object's current location in the world to a path node with the given name
 *
 */
void JNICALL ScriptMethodsAiNamespace::pathToName(JNIEnv * /*env*/, jobject /*self*/, jlong ai, jstring targetName)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return;
	}

	if (aiCreatureController->isRetreating())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(ConfigServerGame::getMovementWhileRetreatingThrowsException(),
		                            ("ScriptMethodsAi::pathToName() ai(%s) Calling a movement command on a retreating AI is not allowed.", aiCreatureController->getCreature()->getDebugInformation().c_str()));
		return;
	}

	JavaStringParam localName(targetName);
	Unicode::String nameString;
	JavaLibrary::convert(localName, nameString);

	aiCreatureController->moveTo(nameString);
}

// ----------------------------------------------------------------------

/**
 * Causes an AI to move along a series of points.
 *
 * @param ai			the AI to move
 * @param targets		the points the AI will move along
 * @param random		flag to tell the AI to choose a random target to move to next, instead of the next one in the targets array
 * @param flip			if true, when the AI gets to the last point in the targets array, it will then go along them in reverse order
 * @param repeat		if true, the AI will go back to the first point after reaching the last one; if false, the ai will stop at the last point
 * @param startPoint	which point the ai will start its patrol at (base-0)
 */
void JNICALL ScriptMethodsAiNamespace::patrolToLocation(JNIEnv *, jobject, jlong ai, jobjectArray targets, jboolean random, jboolean flip, jboolean repeat, jint startPoint)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);
	if (aiCreatureController == nullptr)
		return;

	std::vector<Location> locations;
	if (!ScriptConversion::convert(LocalObjectArrayRefParam(targets), locations))
		return;

	aiCreatureController->patrol(locations, random, flip, repeat, startPoint);
}	// JavaLibrary::patrolToLocation

// ----------------------------------------------------------------------

/**
 * Causes an AI to move along a series of points.
 * @param ai			the AI to move
 * @param targetNames	the waypoint names the AI will move along
 * @param random		flag to tell the AI to choose a random target to move to next, instead of the next one in the targets array
 * @param flip			if true, when the AI gets to the last point in the targets array, it will then go along them in reverse order
 * @param repeat		if true, the AI will go back to the first point after reaching the last one; if false, the ai will stop at the last point
 * @param startPoint	which point the ai will start its patrol at (base-0)
 */
void JNICALL ScriptMethodsAiNamespace::patrolToName(JNIEnv *, jobject, jlong ai, jobjectArray targetNames, jboolean random, jboolean flip, jboolean repeat, jint startPoint)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);
	if (aiCreatureController == nullptr)
		return;

	std::vector<const Unicode::String *> locations;
	getStringArray(LocalObjectArrayRefParam(targetNames), locations);

	// we have to copy the string pointers to strings, which sucks
	int count = locations.size();
	std::vector<Unicode::String> realLocations;
	realLocations.reserve(count);
	for (int i = 0; i < count; ++i)
	{
		const Unicode::String * location = locations[i];
		if (location != nullptr)
		{
			realLocations.push_back(*location);
			delete location;
			locations[i] = nullptr;
		}
	}
	aiCreatureController->patrol(realLocations, random, flip, repeat, startPoint);
}	// JavaLibrary::patrolToName

// ----------------------------------------------------------------------
jstring JNICALL ScriptMethodsAiNamespace::aiGetCombatAction(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return nullptr;
	}

	PersistentCrcString const & result = aiCreatureController->getCombatAction();

	if (result.isEmpty())
	{
		return nullptr;
	}

	JavaString javaString(result.getString());

	return javaString.getReturnValue();
}

// ----------------------------------------------------------------------
jint JNICALL ScriptMethodsAiNamespace::aiGetKnockDownRecoveryTime(JNIEnv * /*env*/, jobject /*self*/, jlong ai)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(ai);

	if (aiCreatureController == nullptr)
	{
		return 0;
	}

	return int(aiCreatureController->getKnockDownRecoveryTime());
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsAiNamespace::setHibernationDelay(JNIEnv *env, jobject self, jlong creature, jfloat delay)
{
	AICreatureController * const aiCreatureController = ScriptMethodsAiNamespace::getAiCreatureController(creature);

	if (aiCreatureController == nullptr)
		return JNI_FALSE;

	aiCreatureController->setHibernationDelay(delay);
	return JNI_TRUE;
}

// ======================================================================
