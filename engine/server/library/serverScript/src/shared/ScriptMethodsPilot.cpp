// ======================================================================
//
// ScriptMethodsPilot.cpp
// Copyright 2003, Sony Online Entertainment, Inc.
// All rights reserved.
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AiShipAttackTargetList.h"
#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceDockingManager.h"
#include "serverGame/SpacePath.h"
#include "serverGame/SpacePathManager.h"
#include "serverGame/SpaceSquad.h"
#include "serverGame/SpaceSquadManager.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"
#include "sharedMath/Transform.h"
#include "sharedObject/Appearance.h"
#include "sharedTerrain/TerrainObject.h"

using namespace JNIWrappersNamespace;


// ======================================================================

namespace ScriptMethodsPilotNamespace
{
	bool verifyShipsEnabled()
	{
		if (!ConfigServerGame::getShipsEnabled())
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("Ship support is not enabled.  Make sure [GameServer] shipsEnabled=1 is in your config file."));
			return false;
		}

		return true;
	}

	bool install();

	jint       JNICALL spaceUnitGetBehavior(JNIEnv * env, jobject self, jlong unit);
	void       JNICALL spaceUnitIdle(JNIEnv * env, jobject self, jlong unit);
	void       JNICALL spaceUnitTrack(JNIEnv * env, jobject self, jlong unit, jlong target);
	void       JNICALL spaceUnitMoveTo(JNIEnv * env, jobject self, jlong unit, jobjectArray path);
	void       JNICALL spaceUnitAddPatrolPath(JNIEnv * env, jobject self, jlong unit, jobjectArray path);
	void       JNICALL spaceUnitClearPatrolPath(JNIEnv * env, jobject self, jlong unit);
	void       JNICALL spaceUnitFollow(JNIEnv * env, jobject self, jlong unit, jlong followedUnit, jobject direction_o, jfloat distance);
	void       JNICALL spaceUnitAddDamageTaken(JNIEnv * env, jobject self, jlong unit, jlong targetUnit, jfloat damage);
	void       JNICALL spaceUnitSetAttackOrders(JNIEnv * env, jobject self, jlong unit, jint attackOrders);
	void       JNICALL spaceUnitSetPilotType(JNIEnv * env, jobject self, jlong unit, jstring pilotType);
	jstring    JNICALL spaceUnitGetPilotType(JNIEnv * env, jobject self, jlong unit);
	jlong      JNICALL spaceUnitGetPrimaryAttackTarget(JNIEnv * env, jobject self, jlong unit);
	jlongArray JNICALL spaceUnitGetAttackTargetList(JNIEnv * env, jobject self, jlong unit);
	jlongArray JNICALL spaceUnitGetWhoIsTargetingMe(JNIEnv * env, jobject self, jlong unit);
	jboolean   JNICALL spaceUnitIsAttacking(JNIEnv * env, jobject self, jlong unit);
	void       JNICALL spaceUnitRemoveAttackTarget(JNIEnv * env, jobject self, jlong unit, jlong targetUnit);
	void       JNICALL spaceUnitSetLeashDistance(JNIEnv * env, jobject self, jlong unit, jfloat distance);
	void       JNICALL spaceUnitSetSquadId(JNIEnv * env, jobject self, jlong unit, jint squadId);
	jint       JNICALL spaceUnitGetSquadId(JNIEnv * env, jobject self, jlong unit);
	void       JNICALL spaceUnitDock(JNIEnv * env, jobject self, jlong unit, jlong dockTarget, jfloat timeAtDock);
	void       JNICALL spaceUnitUnDock(JNIEnv * env, jobject self, jlong unit);
	jboolean   JNICALL spaceUnitIsDocked(JNIEnv * env, jobject self, jlong unit);
	jboolean   JNICALL spaceUnitIsDocking(JNIEnv * env, jobject self, jlong unit);
	void       JNICALL spaceUnitSetAutoAggroImmuneTime(JNIEnv * env, jobject self, jlong unit, jfloat time);
	jboolean   JNICALL spaceUnitIsAutoAggroImmune(JNIEnv * env, jobject self, jlong unit);
	void       JNICALL spaceUnitSetDamageAggroImmune(JNIEnv * env, jobject self, jlong unit, jboolean enabled);
	jobject    JNICALL spaceUnitGetDockTransform(JNIEnv * env, jobject self, jlong dockTarget, jlong dockingUnit);
	void       JNICALL spaceUnitAddExclusiveAggro(JNIEnv * env, jobject self, jlong unit, jlong pilot);
	void       JNICALL spaceUnitRemoveExclusiveAggro(JNIEnv * env, jobject self, jlong unit, jlong pilot);
	jint       JNICALL spaceSquadCreateSquadId(JNIEnv * env, jobject self);
	jint       JNICALL spaceSquadRemoveUnit(JNIEnv * env, jobject self, jlong unit);
	jint       JNICALL spaceSquadCombine(JNIEnv * env, jobject self, jint squadId1, jint squadId2);
	jint       JNICALL spaceSquadGetSize(JNIEnv * env, jobject self, jint squadId);
	jlongArray JNICALL spaceSquadGetUnitList(JNIEnv * env, jobject self, jint squadId);
	void       JNICALL spaceSquadSetFormation(JNIEnv * env, jobject self, jint squadId, jint formation);
	void       JNICALL spaceSquadSetFormationSpacing(JNIEnv * env, jobject self, jint squadId, jfloat scale);
	jint       JNICALL spaceSquadGetFormation(JNIEnv * env, jobject self, jint squadId);
	void       JNICALL spaceSquadSetLeader(JNIEnv * env, jobject self, jint squadId, jlong unit);
	jlong      JNICALL spaceSquadGetLeader(JNIEnv * env, jobject self, jint squadId);
	void       JNICALL spaceSquadIdle(JNIEnv * env, jobject self, jint squadId);
	void       JNICALL spaceSquadTrack(JNIEnv * env, jobject self, jint squadId, jlong target);
	void       JNICALL spaceSquadMoveTo(JNIEnv * env, jobject self, jint squadId, jobjectArray path);
	void       JNICALL spaceSquadAddPatrolPath(JNIEnv * env, jobject self, jint squadId, jobjectArray path);
	void       JNICALL spaceSquadClearPatrolPath(JNIEnv * env, jobject self, jint squadId);
	void       JNICALL spaceSquadFollow(JNIEnv * env, jobject self, jint squadId, jlong followedUnit, jobject direction_o, jfloat distance);
	void       JNICALL spaceSquadSetGuardTarget(JNIEnv * env, jobject self, jint squad, jint targetSquad);
	jint       JNICALL spaceSquadGetGuardTarget(JNIEnv * env, jobject self, jint squad);
	void       JNICALL spaceSquadRemoveGuardTarget(JNIEnv * env, jobject self, jint squad);
	jboolean   JNICALL spaceSquadIsSquadIdValid(JNIEnv *env, jobject self, jint squadId);
	void       JNICALL setShipAggroDistance(JNIEnv *env, jobject self, jlong shipId, jfloat aggroDistance);
}

using namespace ScriptMethodsPilotNamespace;


//========================================================================
// install
//========================================================================

bool ScriptMethodsPilotNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsPilotNamespace::c)}
	JF("__spaceUnitGetBehavior", "(J)I", spaceUnitGetBehavior),
	JF("__spaceUnitIdle", "(J)V", spaceUnitIdle),
	JF("__spaceUnitTrack", "(JJ)V", spaceUnitTrack),
	JF("__spaceUnitMoveTo", "(J[Lscript/transform;)V", spaceUnitMoveTo),
	JF("__spaceUnitAddPatrolPath", "(J[Lscript/transform;)V", spaceUnitAddPatrolPath),
	JF("__spaceUnitClearPatrolPath", "(J)V", spaceUnitClearPatrolPath),
	JF("__spaceUnitFollow", "(JJLscript/vector;F)V", spaceUnitFollow),
	JF("__spaceUnitAddDamageTaken", "(JJF)V", spaceUnitAddDamageTaken),
	JF("__spaceUnitSetAttackOrders", "(JI)V", spaceUnitSetAttackOrders),
	JF("__spaceUnitSetPilotType", "(JLjava/lang/String;)V", spaceUnitSetPilotType),
	JF("__spaceUnitGetPilotType", "(J)Ljava/lang/String;", spaceUnitGetPilotType),
	JF("__spaceUnitGetPrimaryAttackTarget", "(J)J", spaceUnitGetPrimaryAttackTarget),
	JF("__spaceUnitGetAttackTargetList", "(J)[J", spaceUnitGetAttackTargetList),
	JF("__spaceUnitGetWhoIsTargetingMe", "(J)[J", spaceUnitGetWhoIsTargetingMe),
	JF("__spaceUnitIsAttacking", "(J)Z", spaceUnitIsAttacking),
	JF("__spaceUnitRemoveAttackTarget", "(JJ)V", spaceUnitRemoveAttackTarget),
	JF("__spaceUnitSetLeashDistance", "(JF)V", spaceUnitSetLeashDistance),
	JF("__spaceUnitSetSquadId", "(JI)V", spaceUnitSetSquadId),
	JF("__spaceUnitGetSquadId", "(J)I", spaceUnitGetSquadId),
	JF("__spaceUnitDock", "(JJF)V", spaceUnitDock),
	JF("__spaceUnitUnDock", "(J)V", spaceUnitUnDock),
	JF("__spaceUnitIsDocked", "(J)Z", spaceUnitIsDocked),
	JF("__spaceUnitIsDocking", "(J)Z", spaceUnitIsDocking),
	JF("__spaceUnitSetAutoAggroImmuneTime", "(JF)V", spaceUnitSetAutoAggroImmuneTime),
	JF("__spaceUnitIsAutoAggroImmune", "(J)Z", spaceUnitIsAutoAggroImmune),
	JF("__spaceUnitSetDamageAggroImmune", "(JZ)V", spaceUnitSetDamageAggroImmune),
	JF("__spaceUnitGetDockTransform", "(JJ)Lscript/transform;", spaceUnitGetDockTransform),
	JF("__spaceUnitAddExclusiveAggro", "(JJ)V", spaceUnitAddExclusiveAggro),
	JF("__spaceUnitRemoveExclusiveAggro", "(JJ)V", spaceUnitRemoveExclusiveAggro),
	JF("_spaceSquadCreateSquadId", "()I", spaceSquadCreateSquadId),
	JF("__spaceSquadRemoveUnit", "(J)I", spaceSquadRemoveUnit),
	JF("_spaceSquadCombine", "(II)I", spaceSquadCombine),
	JF("_spaceSquadGetSize", "(I)I", spaceSquadGetSize),
	JF("__spaceSquadGetUnitList", "(I)[J", spaceSquadGetUnitList),
	JF("_spaceSquadSetFormation", "(II)V", spaceSquadSetFormation),
	JF("_spaceSquadSetFormationSpacing", "(IF)V", spaceSquadSetFormationSpacing),
	JF("_spaceSquadGetFormation", "(I)I", spaceSquadGetFormation),
	JF("__spaceSquadSetLeader", "(IJ)V", spaceSquadSetLeader),
	JF("__spaceSquadGetLeader", "(I)J", spaceSquadGetLeader),
	JF("_spaceSquadIdle", "(I)V", spaceSquadIdle),
	JF("__spaceSquadTrack", "(IJ)V", spaceSquadTrack),
	JF("_spaceSquadMoveTo", "(I[Lscript/transform;)V", spaceSquadMoveTo),
	JF("_spaceSquadAddPatrolPath", "(I[Lscript/transform;)V", spaceSquadAddPatrolPath),
	JF("_spaceSquadClearPatrolPath", "(I)V", spaceSquadClearPatrolPath),
	JF("__spaceSquadFollow", "(IJLscript/vector;F)V", spaceSquadFollow),
	JF("_spaceSquadSetGuardTarget", "(II)V", spaceSquadSetGuardTarget),
	JF("_spaceSquadGetGuardTarget", "(I)I", spaceSquadGetGuardTarget),
	JF("_spaceSquadRemoveGuardTarget", "(I)V", spaceSquadRemoveGuardTarget),
	JF("_spaceSquadIsSquadIdValid", "(I)Z", spaceSquadIsSquadIdValid),
	JF("_setShipAggroDistance", "(JF)V", setShipAggroDistance),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPilotNamespace::spaceUnitGetBehavior(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitGetBehavior()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitGetBehavior() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitGetBehavior() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	return aiShipController->getBehaviorType();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsPilotNamespace::spaceUnitGetPrimaryAttackTarget(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitGetPrimaryAttackTarget()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitGetPrimaryAttackTarget() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitGetPrimaryAttackTarget() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	return (aiShipController->getPrimaryAttackTarget()).getValue();
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsPilotNamespace::spaceUnitGetAttackTargetList(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitGetAttackTargetList()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitGetAttackTargetList() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitGetAttackTargetList() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	AiShipAttackTargetList::SortedTargetList targetList;
	aiShipController->getAttackTargetList().getSortedTargetList(targetList);

	LocalLongArrayRefPtr attackTargetList = createNewLongArray(static_cast<long>(targetList.size()));
	AiShipAttackTargetList::SortedTargetList::const_iterator iterTargetList = targetList.begin();
	int index = 0;

	jlong jlongTmp;
	for (; iterTargetList != targetList.end(); ++iterTargetList)
	{
		jlongTmp = ((iterTargetList->first).getValue());
		setLongArrayRegion(*attackTargetList, index, 1, &jlongTmp);
		++index;
	}

	return attackTargetList->getReturnValue();
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsPilotNamespace::spaceUnitGetWhoIsTargetingMe(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitGetWhoIsTargetingMe()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitGetWhoIsTargetingMe() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	ShipController * const shipController = shipObject->getController()->asShipController();
	if (!shipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitGetWhoIsTargetingMe() called on unit(%s) without ShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	ShipController::CachedNetworkIdList const & aiTargetingMeList = shipController->getAiTargetingMeList();

#ifdef _DEBUG
	if (ConfigServerGame::isSpaceAiLoggingEnabled())
	{
		ShipController::CachedNetworkIdList::const_iterator iterAiTargetingMeList = aiTargetingMeList.begin();
		int index = 0;

		for (; iterAiTargetingMeList != aiTargetingMeList.end(); ++iterAiTargetingMeList)
		{
			LOG("debug_ai", ("ScriptMethodsPilot::spaceUnitGetWhoIsTargetingMe() [%d] unit(%s)", ++index, iterAiTargetingMeList->getValueString().c_str()));
		}
	}
#endif // _DEBUG

	LocalLongArrayRefPtr attackTargetList = createNewLongArray(static_cast<long>(aiTargetingMeList.size()));
	ShipController::CachedNetworkIdList::const_iterator iterAiTargetingMeList = aiTargetingMeList.begin();
	int index = 0;

	jlong jlongTmp;
	for (; iterAiTargetingMeList != aiTargetingMeList.end(); ++iterAiTargetingMeList)
	{
		jlongTmp = (*iterAiTargetingMeList).getValue();
		setLongArrayRegion(*attackTargetList, index, 1, &jlongTmp);
		++index;
	}

	return attackTargetList->getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPilotNamespace::spaceUnitIsAttacking(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitIsAttacking()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitIsAttacking() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitIsAttacking() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	return aiShipController->isAttacking();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitIdle(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitIdle()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitIdle() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitIdle() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	aiShipController->idle();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitTrack(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jlong jobject_target)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitTrack()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitTrack() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitTrack() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	Object * target = nullptr;
	if (!JavaLibrary::getObject(jobject_target, target))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitTrack() The target did not resolve to an Object"));
		return;
	}

	aiShipController->track(*target);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitMoveTo(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jobjectArray jobjectArray_path)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitMoveTo()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitMoveTo() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitMoveTo() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Convert the path

	AiShipController::TransformList convertedPath;

	if (!ScriptConversion::convert(jobjectArray_path, convertedPath))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitMoveTo() unit(%s) scene(%s) Failed to convert the path to a vector of transforms.", shipObject->getDebugInformation().c_str(), ServerWorld::getSceneId().c_str()));
		return;
	}

	if (convertedPath.empty())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitMoveTo() unit(%s) scene(%s) Specifying an empty path transform list. Are you sure you wanted to do that?", shipObject->getDebugInformation().c_str(), ServerWorld::getSceneId().c_str()));
		return;
	}

	SpacePath * const path = SpacePathManager::fetch(nullptr, aiShipController->getOwner(), aiShipController->getShipRadius());
	AiShipController::TransformList::const_iterator iterTransformList = convertedPath.begin();

	for (; iterTransformList != convertedPath.end(); ++iterTransformList)
	{
		path->addTransform(*iterTransformList);
	}

	aiShipController->moveTo(path);
	SpacePathManager::release(path, aiShipController->getOwner());
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitAddPatrolPath(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jobjectArray jobjectArray_path)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitAddPatrolPath()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitAddPatrolPath() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitAddPatrolPath() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	//-- Convert the path

	AiShipController::TransformList convertedPath;

	if (!ScriptConversion::convert(jobjectArray_path, convertedPath))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitAddPatrolPath() unit(%s) scene(%s) Failed to convert the path to a vector of transforms.", shipObject->getDebugInformation().c_str(), ServerWorld::getSceneId().c_str()));
		return;
	}

	if (convertedPath.empty())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitAddPatrolPath() unit(%s) scene(%s) Specifying an empty path transform list. Are you sure you wanted to do that?", shipObject->getDebugInformation().c_str(), ServerWorld::getSceneId().c_str()));
		return;
	}

	SpacePath * const path = SpacePathManager::fetch(aiShipController->getPath(), aiShipController->getOwner(), aiShipController->getShipRadius());
	AiShipController::TransformList::const_iterator iterTransformList = convertedPath.begin();

	for (; iterTransformList != convertedPath.end(); ++iterTransformList)
	{
		path->addTransform(*iterTransformList);
	}

	aiShipController->addPatrolPath(path);
	SpacePathManager::release(path, aiShipController->getOwner());
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitClearPatrolPath(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitClearPatrolPath()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitClearPatrolPath() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitClearPatrolPath() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	aiShipController->clearPatrolPath();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitFollow(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jlong jobject_followedUnit, jobject jobject_direction_o, jfloat jfloat_distance)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitFollow()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitFollow() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitFollow() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	Object * followedUnit = nullptr;
	if (!JavaLibrary::getObject(jobject_followedUnit, followedUnit))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitFollow() followedUnit did not resolve to an Object"));
		return;
	}

	Vector direction_o;
	if (!ScriptConversion::convert(jobject_direction_o, direction_o))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitFollow() direction did not resolve to a Vector"));
		return;
	}

	NetworkId const followedUnitId(jobject_followedUnit);
	if (followedUnitId == NetworkId::cms_invalid)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitFollow() DB failed to convert parameter 2 (followedUnit) to NetworkId"));
		return;
	}

	aiShipController->follow(followedUnitId, direction_o, jfloat_distance);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitAddDamageTaken(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jlong jobject_targetUnit, jfloat jfloat_damage)
{
	if (!verifyShipsEnabled())
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitAddDamageTaken() ERROR: Ships are disabled."));
		return;
	}

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitAddDamageTaken() unit did not resolve to a ShipObject");
	if (!shipObject)
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitAddDamageTaken() ERROR: Unable to resolve the attacking unit to a ShipObject."));
		return;
	}

	ShipObject * const targetShipObject = JavaLibrary::getShipThrow(env, jobject_targetUnit, "ScriptMethodsPilot::spaceUnitAddDamageTaken() targetUnit did not resolve to a ShipObject");
	if (!targetShipObject)
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitAddDamageTaken() ERROR: Unable to resolve target_unit to a ShipObject."));
		return;
	}

	if (shipObject->getNetworkId() == targetShipObject->getNetworkId())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitAddDamageTaken() unit(%s) is attempting to attack itself", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	{
		float const chassisHitPointsCurrent = targetShipObject->getCurrentChassisHitPoints();
		float const chassisHitPointsMaximum = targetShipObject->getMaximumChassisHitPoints();
		float const currentChassisPercent = (chassisHitPointsMaximum > 0.0f) ? (chassisHitPointsCurrent / chassisHitPointsMaximum) * 100.0f : -1.0f;

		float const frontArmorCurrent = targetShipObject->getComponentHitpointsCurrent(ShipChassisSlotType::SCST_armor_0);
		float const frontArmorMaximum = targetShipObject->getComponentHitpointsMaximum(ShipChassisSlotType::SCST_armor_0);
		float const frontArmorPercent = (frontArmorMaximum > 0.0f) ? (frontArmorCurrent / frontArmorMaximum) * 100.0f : -1.0f;

		float const backArmorCurrent = targetShipObject->getComponentHitpointsCurrent(ShipChassisSlotType::SCST_armor_1);
		float const backArmorMaximum = targetShipObject->getComponentHitpointsMaximum(ShipChassisSlotType::SCST_armor_1);
		float const backArmorPercent = (backArmorMaximum > 0.0f) ? (backArmorCurrent / backArmorMaximum) * 100.0f : -1.0f;

		float const frontShieldCurrent = targetShipObject->getShieldHitpointsFrontCurrent();
		float const frontShieldMaximum = targetShipObject->getShieldHitpointsFrontMaximum();
		float const frontShieldPercent = (frontShieldMaximum > 0.0f) ? (frontShieldCurrent / frontShieldMaximum) * 100.0f : -1.0f;

		float const backShieldCurrent = targetShipObject->getShieldHitpointsFrontCurrent();
		float const backShieldMaximum = targetShipObject->getShieldHitpointsBackMaximum();
		float const backShieldPercent = (backShieldMaximum > 0.0f) ? (backShieldCurrent / backShieldMaximum) * 100.0f : -1.0f;

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitAddDamageTaken() target(%s) dmg(%.2f) ch(%.0f) af(%.0f) ab(%.0f) sf(%.0f) sb(%.0f)", targetShipObject->getNetworkId().getValueString().c_str(), jfloat_damage, currentChassisPercent, frontArmorPercent, backArmorPercent, frontShieldPercent, backShieldPercent));
	}

	ShipController * const shipController = shipObject->getController()->asShipController();
	AiShipController * const aiShipController = (shipController != nullptr) ? shipController->asAiShipController() : nullptr;
	bool const verifyAttacker = true;

	if (aiShipController != nullptr)
	{
		// This adds damage to an AI unit

		bool const notifySquad = true;
		bool const checkPlayerAttacker = true;

		if (!aiShipController->addDamageTaken(targetShipObject->getNetworkId(), jfloat_damage, verifyAttacker, notifySquad, checkPlayerAttacker))
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitAddDamageTaken() targetUnit(%s) is not attackable", targetShipObject->getNetworkId().getValueString().c_str()));
		}
	}
	else if (shipController != nullptr)
	{
		// This adds damage to a player unit

		if (!shipController->addDamageTaken(targetShipObject->getNetworkId(), jfloat_damage, verifyAttacker))
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitAddDamageTaken() targetUnit(%s) is not attackable", targetShipObject->getNetworkId().getValueString().c_str()));
		}
	}
	else
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitAddDamageTaken() unit(%s) Why does this ship not have a ShipController?", shipObject->getNetworkId().getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitSetAttackOrders(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jint jint_attackOrders)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitSetAttackOrder()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitSetAttackOrder() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitSetAttackOrder() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	if (   (jint_attackOrders < 0)
	    || (jint_attackOrders >= static_cast<int>(AiShipController::AO_count)))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitSetAttackOrder() called on unit(%s) with out of range attack orders %d valid range [0...%d]", shipObject->getNetworkId().getValueString().c_str(), jint_attackOrders, static_cast<int>(AiShipController::AO_count) - 1));
		return;
	}

	aiShipController->setAttackOrders(static_cast<AiShipController::AttackOrders>(jint_attackOrders));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitSetPilotType(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jstring jstring_pilotType)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitSetPilotType()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitSetPilotType() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitSetPilotType() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	JavaStringParam javaStringParam_pilotType (jstring_pilotType);
	std::string pilotType;
	if (!JavaLibrary::convert (javaStringParam_pilotType, pilotType))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitSetPilotType() DB failed to convert parameter 2 (pilotType) to string"));
		return;
	}

	if (pilotType.empty())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::spaceUnitSetPilotType empty pilotType for ship [%s]",
			shipObject ? shipObject->getDebugInformation().c_str() : "NONE"));
		return;
	}

	aiShipController->setPilotType(pilotType);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsPilotNamespace::spaceUnitGetPilotType(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitGetPilotType()"));

	if (!verifyShipsEnabled())
		return nullptr;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitGetPilotType() unit did not resolve to a ShipObject");
	if (!shipObject)
		return nullptr;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitGetPilotType() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	return JavaString(aiShipController->getPilotType()).getReturnValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitRemoveAttackTarget(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jlong jobject_targetUnit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitRemoveAttackTarget()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitRemoveAttackTarget() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitRemoveAttackTarget() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	NetworkId const targetUnit(jobject_targetUnit);
	if (targetUnit == NetworkId::cms_invalid)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitRemoveAttackTarget() DB failed to convert parameter 2 (targetUnit) to NetworkId"));
		return;
	}

	if (targetUnit == shipObject->getNetworkId())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitRemoveAttackTarget() unit(%s) is attempting to remove itself from its target list", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	if (!aiShipController->removeAttackTarget(targetUnit))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitRemoveAttackTarget() The targetUnit(%s) is not in the units(%s) target list", targetUnit.getValueString().c_str(), shipObject->getNetworkId().getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitSetLeashDistance(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jfloat jfloat_radius)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitSetLeashDistance()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitSetLeashDistance() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitSetLeashDistance() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	aiShipController->setLeashRadius(jfloat_radius);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitSetSquadId(JNIEnv * /*env*/, jobject /*self*/, jlong jobject_unit, jint jint_squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitSetSquadId()"));

	if (!verifyShipsEnabled())
		return;

	NetworkId unit(jobject_unit);

	if (!SpaceSquadManager::setSquadId(unit, jint_squadId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitSetSquadId() called on unit(%s) without AiShipController", unit.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPilotNamespace::spaceUnitGetSquadId(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitGetSquadId()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitGetSquadId() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitGetSquadId() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	return aiShipController->getSquad().getId();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadMoveTo(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId, jobjectArray jobjectArray_path)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadMoveTo()"));

	if (!verifyShipsEnabled())
		return;

	//-- Convert the path

	AiShipController::TransformList convertedPath;

	if (!ScriptConversion::convert(jobjectArray_path, convertedPath))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadMoveTo() squadId(%d) scene(%s) Failed to convert the path to a vector of transforms.", jint_squadId, ServerWorld::getSceneId().c_str()));
		return;
	}

	if (convertedPath.empty())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadMoveTo() squadId(%d) scene(%s) Specifying an empty path transform list. Are you sure you wanted to do that?", jint_squadId, ServerWorld::getSceneId().c_str()));
		return;
	}

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadMoveTo() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	float const largestShipRadius = squad->getLargestShipRadius();
	SpacePath * const path = SpacePathManager::fetch(nullptr, squad, largestShipRadius);
	AiShipController::TransformList::const_iterator iterTransformList = convertedPath.begin();

	for (; iterTransformList != convertedPath.end(); ++iterTransformList)
	{
		path->addTransform(*iterTransformList);
	}

	squad->moveTo(path);
	SpacePathManager::release(path, squad);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadAddPatrolPath(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId, jobjectArray jobjectArray_path)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadAddPatrolPath()"));

	if (!verifyShipsEnabled())
		return;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadAddPatrolPath() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	//-- Convert the path

	AiShipController::TransformList convertedPath;

	if (!ScriptConversion::convert(jobjectArray_path, convertedPath))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadAddPatrolPath() squadId(%d) scene(%s) Failed to convert the path to a vector of transforms.", jint_squadId, ServerWorld::getSceneId().c_str()));
		return;
	}

	if (convertedPath.empty())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadAddPatrolPath() squadId(%d) scene(%s) Specifying an empty path transform list. Are you sure you wanted to do that?", jint_squadId, ServerWorld::getSceneId().c_str()));
		return;
	}

	//-- Add the transforms to the path

	float const largestShipRadius = squad->getLargestShipRadius();
	SpacePath * const path = SpacePathManager::fetch(squad->getPath(), squad, largestShipRadius);
	AiShipController::TransformList::const_iterator iterTransformList = convertedPath.begin();

	for (; iterTransformList != convertedPath.end(); ++iterTransformList)
	{
		path->addTransform(*iterTransformList);
	}

	//-- Patrol

	squad->addPatrolPath(path);
	SpacePathManager::release(path, squad);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPilotNamespace::spaceSquadCreateSquadId(JNIEnv * /*env*/, jobject /*self*/)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadCreateSquadId()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	return SpaceSquadManager::createSquadId();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPilotNamespace::spaceSquadRemoveUnit(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadRemoveUnit()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceSquadRemoveUnit() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadRemoveUnit() called on unit(%s) without AiShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	SpaceSquad * const squad = SpaceSquadManager::createSquad();

	IGNORE_RETURN(SpaceSquadManager::setSquadId(shipObject->getNetworkId(), squad->getId()));

	return squad->getId();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPilotNamespace::spaceSquadCombine(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId1, jint jint_squadId2)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadCombine()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	SpaceSquad * const squad1 = SpaceSquadManager::getSquad(jint_squadId1);

	if (!squad1)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadCombine() Unable to resolve squadId1(%d) to a SpaceSquad", jint_squadId1));
		return JNI_FALSE;
	}

	SpaceSquad * const squad2 = SpaceSquadManager::getSquad(jint_squadId2);

	if (!squad2)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadCombine() Unable to resolve squadId2(%d) to a SpaceSquad", jint_squadId2));
		return JNI_FALSE;
	}

	squad1->combineWith(*squad2);

	return squad1->getId();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPilotNamespace::spaceSquadGetSize(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadGetSize()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadGetSize() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return JNI_FALSE;
	}

	return squad->getUnitCount();
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsPilotNamespace::spaceSquadGetUnitList(JNIEnv * env, jobject /*self*/, jint jint_squadId)
{
	UNREF(env);
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadGetUnitList()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadGetUnitList() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return JNI_FALSE;
	}

	SpaceSquad::UnitList unitList;

	squad->getUnitList(unitList);

	LocalLongArrayRefPtr resultUnitList = createNewLongArray(static_cast<long>(unitList.size()));

	SpaceSquad::UnitList::const_iterator iterUnitList = unitList.begin();
	int index = 0;

	jlong jlongTmp;
	for (; iterUnitList != unitList.end(); ++iterUnitList)
	{
		jlongTmp = (*iterUnitList).getValue();
		setLongArrayRegion(*resultUnitList, index, 1, &jlongTmp);
		++index;
	}

	return resultUnitList->getReturnValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadSetFormation(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId, jint jint_formation)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadSetFormation()"));

	if (!verifyShipsEnabled())
		return;

	if (   (jint_formation <= static_cast<int>(Formation::S_invalid))
	    || (jint_formation >= static_cast<int>(Formation::S_count)))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetFormation() Invalid formation(%d) specified", jint_formation));
		return;
	}

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetFormation() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	squad->getFormation().setShape(static_cast<Formation::Shape>(jint_formation));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadSetFormationSpacing(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId, jfloat jfloat_scale)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadSetFormationSpacing()"));

	if (!verifyShipsEnabled())
		return;

	if (jfloat_scale < 1.0f)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetFormationSpacing() Invalid scale specified(%.2f) specified. Valid range is [1.0f...n]", jfloat_scale));
		return;
	}

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetFormationSpacing() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	squad->getFormation().setSpacing(jfloat_scale);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPilotNamespace::spaceSquadGetFormation(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadGetFormation()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadGetFormation() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return JNI_FALSE;
	}

	return static_cast<int>(squad->getFormation().getShape());
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadIdle(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadIdle()"));

	if (!verifyShipsEnabled())
		return;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadIdle() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	squad->idle();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadTrack(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId, jlong jobject_target)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadTrack()"));

	if (!verifyShipsEnabled())
		return;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadTrack() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	Object * target = nullptr;
	if (!JavaLibrary::getObject(jobject_target, target))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadTrack() The target did not resolve to an Object"));
		return;
	}

	squad->track(*target);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadSetLeader(JNIEnv * env, jobject /*self*/, jint jint_squadId, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadSetLeader()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const leaderShipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceSquadSetLeader() unit did not resolve to a ShipObject");
	if (!leaderShipObject)
		return;

	AiShipController * const aiShipController = (leaderShipObject->getController()->asShipController() != nullptr) ? leaderShipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetLeader() called on unit(%s) without AiShipController", leaderShipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetLeader() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	bool const rebuildFormation = true;

	if (!squad->setLeader(leaderShipObject->getNetworkId(), rebuildFormation))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetLeader() The specified leader(%s) is not a member of the squad(%i).", leaderShipObject->getNetworkId().getValueString().c_str(), jint_squadId));
	}
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsPilotNamespace::spaceSquadGetLeader(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadGetLeader()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadGetLeader() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return JNI_FALSE;
	}

	return (squad->getLeader()).getValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadClearPatrolPath(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadClearPatrolPath()"));

	if (!verifyShipsEnabled())
		return;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadClearPatrolPath() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	squad->clearPatrolPath();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadFollow(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId, jlong jobject_followedUnit, jobject jobject_direction_o, jfloat jfloat_distance)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadFollow()"));

	if (!verifyShipsEnabled())
		return;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadFollow() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	Object * followedUnit = nullptr;
	if (!JavaLibrary::getObject(jobject_followedUnit, followedUnit))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadFollow() followedUnit did not resolve to an Object"));
		return;
	}

	Vector direction_o;
	if (!ScriptConversion::convert(jobject_direction_o, direction_o))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadFollow() direction did not resolve to a Vector"));
		return;
	}

	NetworkId const followedUnitId(jobject_followedUnit);
	if (followedUnitId == NetworkId::cms_invalid)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadFollow() DB failed to convert parameter 2 (followedUnit) to NetworkId"));
		return;
	}

	squad->follow(followedUnitId, direction_o, jfloat_distance);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitDock(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jlong jobject_dockTarget, jfloat jfloat_timeAtDock)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitDock()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitDock() The unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	ShipController * const shipController = shipObject->getController()->asShipController();
	if (!shipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitDock() called on unit(%s) without ShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	ShipObject * const targetShipObject = JavaLibrary::getShipThrow(env, jobject_dockTarget, "ScriptMethodsPilot::spaceUnitDock() The dockTarget did not resolve to a ShipObject");
	if (!targetShipObject)
		return;

	ShipController * const targetShipController = targetShipObject->getController()->asShipController();
	if (!targetShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitDock() called on target(%s) without ShipController", targetShipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	shipController->dock(*targetShipObject, jfloat_timeAtDock);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitUnDock(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitUnDock()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitUnDock() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	ShipController * const shipController = shipObject->getController()->asShipController();
	if (!shipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitUnDock() called on unit(%s) without a ShipController", shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	shipController->unDock();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPilotNamespace::spaceUnitIsDocked(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitDock()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitDock() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	ShipController * const shipController = shipObject->getController()->asShipController();
	if (!shipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitIsDocked() called on unit(%s) without ShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	return shipController->isDocked();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPilotNamespace::spaceUnitIsDocking(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitIsDocking()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitIsDocking() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	ShipController * const shipController = shipObject->getController()->asShipController();
	if (!shipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitIsDocking() called on unit(%s) without ShipController", shipObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	return shipController->isDocking();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadSetGuardTarget(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId, jint jint_targetSquadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadSetGuardTarget()"));

	if (!verifyShipsEnabled())
		return;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetGuardTarget() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	if (!squad->setGuardTarget(jint_targetSquadId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadSetGuardTarget() Unable to resolve targetSquadId(%d) to a SpaceSquad", jint_targetSquadId));
	}
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPilotNamespace::spaceSquadGetGuardTarget(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadGetGuardTarget()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadGetGuardTarget() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return JNI_FALSE;
	}

	if (squad->getGuardTarget() == nullptr)
	{
		return 0;
	}

	return squad->getGuardTarget()->getId();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceSquadRemoveGuardTarget(JNIEnv * /*env*/, jobject /*self*/, jint jint_squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceSquadRemoveGuardTarget()"));

	if (!verifyShipsEnabled())
		return;

	SpaceSquad * const squad = SpaceSquadManager::getSquad(jint_squadId);

	if (!squad)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceSquadRemoveGuardTarget() Unable to resolve squadId(%d) to a SpaceSquad", jint_squadId));
		return;
	}

	squad->removeGuardTarget();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPilotNamespace::spaceSquadIsSquadIdValid(JNIEnv * env, jobject /*self*/, jint squadId)
{
	if (SpaceSquadManager::getSquad(squadId))
		return static_cast<jboolean>(JNI_TRUE);
	return static_cast<jboolean>(JNI_FALSE);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::setShipAggroDistance(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jfloat jfloat_aggroRadius)
{
	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::setShipAggroDistance(): jobject_unit did not resolve to a ShipObject", false);
	if (!shipObject)
		return;

	AiShipController * const aiShipController = (shipObject->getController()->asShipController() != nullptr) ? shipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!aiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::setShipAggroDistance(): unit(%s) does not have an AiShipController",shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	TerrainObject const * const terrain = TerrainObject::getInstance();

	if (!terrain)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::setShipAggroDistance(): unit(%s) Unable to get the terrain object to determine the map side length.",shipObject->getNetworkId().getValueString().c_str()));
		return;
	}

	if (jfloat_aggroRadius > terrain->getMapWidthInMeters())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::setShipAggroDistance(): unit(%s) aggroRadius(%f) > mapSideLength(%f).",shipObject->getNetworkId().getValueString().c_str(), jfloat_aggroRadius, terrain->getMapWidthInMeters()));
		return;
	}

	if (jfloat_aggroRadius <= 1.0f)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::setShipAggroDistance(): unit(%s) aggroRadius(%f) <= 1.",shipObject->getNetworkId().getValueString().c_str(), jfloat_aggroRadius));
		return;
	}

	aiShipController->setAggroRadius(jfloat_aggroRadius);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitSetAutoAggroImmuneTime(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jfloat jfloat_time)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitSetAutoAggroImmuneTime() time(%.2f)", jfloat_time));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitSetAutoAggroImmuneTime() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	if (   (jfloat_time < 0.0f)
	    && !WithinEpsilonInclusive(-1.0f, jfloat_time, 0.00001f))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitSetAutoAggroImmuneTime(): unit(%s) The time(%f) must be >= 0.0f", shipObject->getNetworkId().getValueString().c_str(), jfloat_time));
		return;
	}

	shipObject->setAutoAggroImmuneTime(jfloat_time);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPilotNamespace::spaceUnitIsAutoAggroImmune(JNIEnv * env, jobject /*self*/, jlong jobject_unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitIsAutoAggroImmune()"));

	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitIsAutoAggroImmune() unit did not resolve to a ShipObject");
	if (!shipObject)
		return JNI_FALSE;

	return shipObject->isAutoAggroImmune();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitSetDamageAggroImmune(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jboolean jboolean_enabled)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitSetDamageAggroImmune() enabled(%s)", jboolean_enabled ? "yes" : "no"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitSetDamageAggroImmune() unit did not resolve to a ShipObject");
	if (!shipObject)
		return;

	shipObject->setDamageAggroImmune(jboolean_enabled);
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsPilotNamespace::spaceUnitGetDockTransform(JNIEnv * /*env*/, jobject /*self*/, jlong jobject_dockTarget, jlong jobject_dockingUnit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitGetDockTransform()"));

	if (!verifyShipsEnabled())
		return 0;

	Object * dockTarget = nullptr;
	if (!JavaLibrary::getObject(jobject_dockTarget, dockTarget))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitGetDockTransform() dockTarget did not resolve to an Object"));
		return 0;
	}

	Object * dockingUnit = nullptr;
	if (!JavaLibrary::getObject(jobject_dockingUnit, dockingUnit))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitGetDockTransform() dockingUnit did not resolve to an Object"));
		return 0;
	}

	Transform dockHardPoint;
	SpaceDockingManager::HardPointList approachHardPointList;
	SpaceDockingManager::HardPointList exitHardPointList;

	SpaceDockingManager::fetchDockingProcedure(*dockingUnit, *dockTarget, dockHardPoint, approachHardPointList, exitHardPointList);
	SpaceDockingManager::releaseDockingProcedure(dockingUnit->getNetworkId(), dockTarget->getNetworkId());

	LocalRefPtr transform;
	if (ScriptConversion::convert(dockHardPoint, transform))
		return transform->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitAddExclusiveAggro(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jlong jobject_pilot)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitAddExclusiveAggro()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const unitShipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitAddExclusiveAggro() unit did not resolve to a ShipObject");

	if (!unitShipObject)
	{
		return;
	}

	AiShipController * const unitAiShipController = (unitShipObject->getController()->asShipController() != nullptr) ? unitShipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!unitAiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitAddExclusiveAggro() called on unit(%s) without AiShipController", unitShipObject->getDebugInformation().c_str()));
		return;
	}

	Object * pilotObject = nullptr;
	if (!JavaLibrary::getObject(jobject_pilot, pilotObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitAddExclusiveAggro() pilot did not resolve to an Object"));
		return;
	}

	ServerObject * const pilotServerObject = pilotObject->asServerObject();

	if (   !pilotServerObject
	    || !pilotServerObject->isPlayerControlled())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitAddExclusiveAggro() pilot(%s) did not resolve to player-controlled ServerObject", pilotObject->getDebugInformation().c_str()));
		return;
	}

	unitAiShipController->addExclusiveAggro(pilotObject->getNetworkId());
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPilotNamespace::spaceUnitRemoveExclusiveAggro(JNIEnv * env, jobject /*self*/, jlong jobject_unit, jlong jobject_pilot)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("ScriptMethodsPilot::spaceUnitRemoveExclusiveAggro()"));

	if (!verifyShipsEnabled())
		return;

	ShipObject * const unitShipObject = JavaLibrary::getShipThrow(env, jobject_unit, "ScriptMethodsPilot::spaceUnitRemoveExclusiveAggro() unit did not resolve to a ShipObject");

	if (!unitShipObject)
	{
		return;
	}

	AiShipController * const unitAiShipController = (unitShipObject->getController()->asShipController() != nullptr) ? unitShipObject->getController()->asShipController()->asAiShipController() : nullptr;
	if (!unitAiShipController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitRemoveExclusiveAggro() called on unit(%s) without AiShipController", unitShipObject->getDebugInformation().c_str()));
		return;
	}

	Object * pilotObject = nullptr;
	if (!JavaLibrary::getObject(jobject_pilot, pilotObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitRemoveExclusiveAggro() pilot did not resolve to an Object"));
		return;
	}

	ServerObject * const pilotServerObject = pilotObject->asServerObject();

	if (   !pilotServerObject
	    || !pilotServerObject->isPlayerControlled())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsPilot::spaceUnitRemoveExclusiveAggro() pilot(%s) did not resolve to player-controlled ServerObject", pilotObject->getDebugInformation().c_str()));
		return;
	}

	unitAiShipController->removeExclusiveAggro(pilotObject->getNetworkId());
}

// ======================================================================
