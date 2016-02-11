//========================================================================
//
// ScriptMethodsObjectMove.cpp - implements script methods dealing with object
// movement.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/CellObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/NewbieTutorial.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PositionUpdateTracker.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverGame/TangibleController.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/LocationData.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Footprint.h"
#include "sharedLog/Log.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsObjectMoveNamespace
// ======================================================================

namespace ScriptMethodsObjectMoveNamespace
{
	bool install();
	bool testInvalidObjectMove(const ServerObject * object);

	jboolean     JNICALL setLocationFromPos(JNIEnv *env, jobject self, jlong objectId, jobject to);
	jboolean     JNICALL setLocationFromObj(JNIEnv *env, jobject self, jlong objectId, jlong toId);
	jobject      JNICALL getLocation(JNIEnv *env, jobject self, jlong objectId);
	jobject      JNICALL getWorldLocation(JNIEnv *env, jobject self, jlong objectId);
	jobject      JNICALL getHeading(JNIEnv *env, jobject self, jlong objectId);
	jobject      JNICALL getTransform_o2w(JNIEnv *env, jobject self, jlong objectId);
	jobject      JNICALL getTransform_o2p(JNIEnv *env, jobject self, jlong objectId);
	jlong        JNICALL getAttachedTo(JNIEnv *env, jobject self, jlong objectId);
	void         JNICALL setTransform_o2p(JNIEnv *env, jobject self, jlong objectId, jobject transform);
	jboolean     JNICALL faceToObj(JNIEnv *env, jobject self, jlong mob, jlong target);
	jboolean     JNICALL faceToPos(JNIEnv *env, jobject self, jlong mob, jobject target);
	jboolean     JNICALL faceToBehavior(JNIEnv *env, jobject self, jlong mob, jlong target);
	jint         JNICALL elevatorMove(JNIEnv * env, jobject self, jlong mob, jint nFloors);
	void         JNICALL setClientUsesAnimationLocomotion(JNIEnv *env, jobject self, jlong target, jboolean enabled);
	jfloat       JNICALL getMovementSpeed(JNIEnv *env, jobject self, jlong object);
	jfloat       JNICALL getWalkSpeed(JNIEnv *env, jobject self, jlong object);
	jfloat       JNICALL getRunSpeed(JNIEnv *env, jobject self, jlong object);
	void         JNICALL setBaseWalkSpeed(JNIEnv *env, jobject self, jlong object, jfloat speed);
	jfloat       JNICALL getBaseWalkSpeed(JNIEnv *env, jobject self, jlong object);
	void         JNICALL setBaseRunSpeed(JNIEnv *env, jobject self, jlong object, jfloat speed);
	jfloat       JNICALL getBaseRunSpeed(JNIEnv *env, jobject self, jlong object);
	void         JNICALL setMovementWalk(JNIEnv *env, jobject self, jlong object);
	void         JNICALL setMovementRun(JNIEnv *env, jobject self, jlong object);
	void         JNICALL hideFromClient(JNIEnv *env, jobject self, jlong object, jboolean hide);
	jboolean     JNICALL stopFloating(JNIEnv *env, jobject self, jlong objectId);
	jboolean     JNICALL setFloatingTime(JNIEnv *env, jobject self, jlong objectId, jint seconds );
	jfloat       JNICALL getYaw(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setYaw(JNIEnv *env, jobject self, jlong target, jfloat scale);
	void         JNICALL modifyYaw(JNIEnv *env, jobject self, jlong target, jfloat degrees);
	void         JNICALL modifyPitch(JNIEnv *env, jobject self, jlong target, jfloat degrees);
	void         JNICALL modifyRoll(JNIEnv *env, jobject self, jlong target, jfloat degrees);
	jfloatArray  JNICALL getQuaternion(JNIEnv *env, jobject self, jlong target);
	void         JNICALL setQuaternion(JNIEnv *env, jobject self, jlong target, jfloat qw, jfloat qx, jfloat qy, jfloat qz);
	jint         JNICALL getFurnitureRotationDegree(JNIEnv *env, jobject self, jlong player);
	void         JNICALL saveDecorationLayout(JNIEnv *env, jobject self, jlong player, jlong pob, jint saveSlotNumber, jstring description);
	void         JNICALL restoreDecorationLayout(JNIEnv *env, jobject self, jlong player, jlong pob, jint saveSlotNumber);
	jboolean     JNICALL setSlopeModAngle(JNIEnv *env, jobject self, jlong mob, jfloat value);
	jfloat       JNICALL getSlopeModAngle(JNIEnv *env, jobject self, jlong mob);
	jfloat       JNICALL getTemplateSlopeModAngle(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL setSlopeModPercent(JNIEnv *env, jobject self, jlong mob, jfloat value);
	jfloat       JNICALL getSlopeModPercent(JNIEnv *env, jobject self, jlong mob);
	jfloat       JNICALL getTemplateSlopeModPercent(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL setWaterModPercent(JNIEnv *env, jobject self, jlong mob, jfloat value);
	jfloat       JNICALL getWaterModPercent(JNIEnv *env, jobject self, jlong mob);
	jfloat       JNICALL getTemplateWaterModPercent(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL setMovementPercent(JNIEnv *env, jobject self, jlong mob, jfloat value);
	jfloat       JNICALL getMovementPercent(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL setAccelPercent(JNIEnv *env, jobject self, jlong mob, jfloat value);
	jfloat       JNICALL getAccelPercent(JNIEnv *env, jobject self, jlong mob);
	jboolean     JNICALL monitorCreatureMovement(JNIEnv *env, jobject self, jlong mob, jlong target, jfloat skittishness, jfloat curve);
	jboolean     JNICALL ignoreCreatureMovement(JNIEnv *env, jobject self, jlong mob, jlong target);
	jobject      JNICALL getApproachSpeed(JNIEnv *env, jobject self, jlong i_mob, jobject i_mobDelta, jlong i_target, jobject i_targetDelta, jfloat i_time);
	jfloat	     JNICALL getTargetBearing(JNIEnv *env, jobject self, jlong i_mob, jlong i_target);
	void         JNICALL warpPlayerCellId(JNIEnv *env, jobject self, jlong player, jstring scene, jfloat x_w, jfloat y_w, jfloat z_w, jlong cell, jfloat x_p, jfloat y_p, jfloat z_p, jstring callback, jboolean forceLoadScreen);
	void         JNICALL warpPlayerCellName(JNIEnv *env, jobject self, jlong player, jstring scene, jfloat x_w, jfloat y_w, jfloat z_w, jlong building, jstring cell, jfloat x_p, jfloat y_p, jfloat z_p, jstring callback, jboolean forceLoadScreen);
	void         JNICALL disconnectPlayer(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL isPlayerConnected(JNIEnv *env, jobject self, jlong player);
	jobject      JNICALL getConnectedPlayerLocation(JNIEnv * env, jobject self, jlong player);
	jboolean     JNICALL isPlayerActive(JNIEnv *env, jobject self, jlong player);
	void         JNICALL addLocationTarget(JNIEnv * env, jobject self, jlong target, jstring name, jobject location, jfloat radius, jboolean is3d);
	void         JNICALL removeLocationTarget(JNIEnv * env, jobject self, jlong target, jstring name);
	void         JNICALL sendPlayerToTutorial(JNIEnv * env, jobject self, jlong player);
	void         JNICALL addPassiveReveal(JNIEnv * env, jobject self, jlong object, jlong target, jint range);
	void         JNICALL removePassiveReveal(JNIEnv * env, jobject self, jlong object, jlong target);
	void         JNICALL clearPassiveRevealList(JNIEnv * env, jobject self, jlong object);
	jint         JNICALL getPassiveRevealRange(JNIEnv * env, jobject self, jlong object, jlong target);
	jobject      JNICALL getPassiveRevealList(JNIEnv * env, jobject self, jlong object);
	void         JNICALL requestSendPositionUpdate(JNIEnv * env, jobject self, jlong object);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsObjectMoveNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsObjectMoveNamespace::c)}
	JF("_setLocation", "(JLscript/location;)Z", setLocationFromPos),
	JF("_setLocation", "(JJ)Z", setLocationFromObj),
	JF("_getLocation", "(J)Lscript/location;", getLocation),
	JF("_getWorldLocation", "(J)Lscript/location;", getWorldLocation),
	JF("_getHeading", "(J)Lscript/location;", getHeading),
	JF("_getTransform_o2w", "(J)Lscript/transform;", getTransform_o2w),
	JF("_getTransform_o2p", "(J)Lscript/transform;", getTransform_o2p),
	JF("_getAttachedTo", "(J)J", getAttachedTo),
	JF("_setTransform_o2p", "(JLscript/transform;)V", setTransform_o2p),
	JF("_faceTo", "(JJ)Z", faceToObj),
	JF("_faceTo", "(JLscript/location;)Z", faceToPos),
	JF("_faceToBehavior", "(JJ)Z", faceToBehavior),
	JF("_elevatorMove", "(JI)I", elevatorMove),
	JF("_getMovementSpeed", "(J)F", getMovementSpeed),
	JF("_getWalkSpeed", "(J)F", getWalkSpeed),
	JF("_getRunSpeed", "(J)F", getRunSpeed),
	JF("_setClientUsesAnimationLocomotion", "(JZ)V", setClientUsesAnimationLocomotion),
	JF("_setBaseWalkSpeed", "(JF)V", setBaseWalkSpeed),
	JF("_getBaseWalkSpeed", "(J)F", getBaseWalkSpeed),
	JF("_setBaseRunSpeed", "(JF)V", setBaseRunSpeed),
	JF("_getBaseRunSpeed", "(J)F", getBaseRunSpeed),
	JF("_setMovementWalk", "(J)V", setMovementWalk),
	JF("_setMovementRun", "(J)V", setMovementRun),
	JF("_hideFromClient", "(JZ)V", hideFromClient),
	JF("_stopFloating", "(J)Z", stopFloating),
	JF("_setFloatingTime", "(JI)Z", setFloatingTime),
	JF("_getYaw", "(J)F", getYaw),
	JF("_setYaw", "(JF)Z", setYaw),
	JF("_modifyYaw", "(JF)V", modifyYaw),
	JF("_modifyPitch", "(JF)V", modifyPitch),
	JF("_modifyRoll", "(JF)V", modifyRoll),
	JF("_getQuaternion", "(J)[F", getQuaternion),
	JF("_setQuaternion", "(JFFFF)V", setQuaternion),
	JF("_getFurnitureRotationDegree", "(J)I", getFurnitureRotationDegree),
	JF("_saveDecorationLayout", "(JJILjava/lang/String;)V", saveDecorationLayout),
	JF("_restoreDecorationLayout", "(JJI)V", restoreDecorationLayout),
	JF("_setSlopeModAngle", "(JF)Z", setSlopeModAngle),
	JF("_getSlopeModAngle", "(J)F",getSlopeModAngle),
	JF("_getTemplateSlopeModAngle", "(J)F",getTemplateSlopeModAngle),
	JF("_setSlopeModPercent", "(JF)Z", setSlopeModPercent),
	JF("_getSlopeModPercent", "(J)F",getSlopeModPercent),
	JF("_getTemplateSlopeModPercent", "(J)F",getTemplateSlopeModPercent),
	JF("_setWaterModPercent", "(JF)Z", setWaterModPercent),
	JF("_getWaterModPercent", "(J)F",getWaterModPercent),
	JF("_getTemplateWaterModPercent", "(J)F",getTemplateWaterModPercent),
	JF("_setMovementPercent", "(JF)Z", setMovementPercent),
	JF("_getMovementPercent", "(J)F",getMovementPercent),
	JF("_setAccelPercent", "(JF)Z", setAccelPercent),
	JF("_getAccelPercent", "(J)F",getAccelPercent),
	JF("__addLocationTarget",               "(JLjava/lang/String;Lscript/location;FZ)V",          addLocationTarget),
	JF("__removeLocationTarget",            "(JLjava/lang/String;)V",                             removeLocationTarget),
	JF("_monitorCreatureMovement", "(JJFF)Z", monitorCreatureMovement),
	JF("_ignoreCreatureMovement", "(JJ)Z", ignoreCreatureMovement),
	JF("_getApproachSpeed", "(JLscript/location;JLscript/location;F)Lscript/location;", getApproachSpeed),
	JF("_getTargetBearing", "(JJ)F", getTargetBearing),
	JF("_warpPlayer", "(JLjava/lang/String;FFFJFFFLjava/lang/String;Z)V", warpPlayerCellId),
	JF("_warpPlayer", "(JLjava/lang/String;FFFJLjava/lang/String;FFFLjava/lang/String;Z)V", warpPlayerCellName),
	JF("_disconnectPlayer", "(J)V", disconnectPlayer),
	JF("_isPlayerConnected", "(J)Z", isPlayerConnected),
	JF("_getConnectedPlayerLocation", "(J)Lscript/dictionary;", getConnectedPlayerLocation),
	JF("_isPlayerActive", "(J)Z", isPlayerActive),
	JF("_sendPlayerToTutorial", "(J)V", sendPlayerToTutorial),
	JF("_addPassiveReveal", "(JJI)V", addPassiveReveal),
	JF("_removePassiveReveal", "(JJ)V", removePassiveReveal),
	JF("_clearPassiveRevealList", "(J)V", clearPassiveRevealList),
	JF("_getPassiveRevealRange", "(JJ)I", getPassiveRevealRange),
	JF("_getPassiveRevealList", "(J)Lscript/dictionary;", getPassiveRevealList),
	JF("_requestSendPositionUpdate", "(J)V", requestSendPositionUpdate),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// helper functions
//========================================================================

/**
 * Tests if an object can be moved. Throws a Java error if the object can't be
 * moved.
 *
 * @param object		the object to test
 *
 * @return true if the object can be moved, false if not
 */
bool ScriptMethodsObjectMoveNamespace::testInvalidObjectMove(const ServerObject * object)
{
	if (object->getCacheVersion() > 0 || object->getCellProperty() != nullptr)
	{
		char buffer[1024];
		sprintf(buffer, "A script is trying to move object %s, which is a cached "
			"or cell object", object->getNetworkId().getValueString().c_str());
		WARNING(true, (buffer));
		JavaLibrary::throwInternalScriptError(buffer);
		return false;
	}
	return true;
}	// testInvalidObjectMove


//========================================================================
// class JavaLibrary JNI object location callback methods
//========================================================================

/**
 * Sets an object's location in the world based on a world coordinate.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to move
 * @param to			where to move the object
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectMoveNamespace::setLocationFromPos(JNIEnv *env, jobject self, jlong objectId, jobject to)
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		return JNI_FALSE;

	if (!testInvalidObjectMove(object))
		return JNI_FALSE;

	Vector newPosition_p;
	std::string sceneId;
	NetworkId cell;
	ScriptConversion::convert(to, newPosition_p, sceneId, cell);

	if (object->getSceneId() != sceneId)
	{
		// @todo: can we move the object to a different scene?
		// no, we can't without having a world location specified
	}

	if (cell == NetworkId::cms_invalid)
		object->teleportObject(newPosition_p, NetworkId::cms_invalid, "", Vector(), "");
	else
	{
		CellObject *destCell = dynamic_cast<CellObject *>(NetworkIdManager::getObjectById(cell));
		if (!destCell)
		{
			JavaLibrary::throwInternalScriptError("JavaLibrary::setLocationFromPos tried to teleport to a non-cell or unloaded cell");
			return JNI_FALSE;
		}
		Object *topmost = ContainerInterface::getTopmostContainer(*destCell);
		object->teleportObject(topmost->getPosition_w(), cell, "", newPosition_p, "");
	}

	// fix any scene ids that may be incorrect - this was causing an issue with pack houses
	object->setSceneIdOnThisAndContents(object->getSceneId());
	
	return JNI_TRUE;
}	// JavaLibrary::setLocationFromPos

/**
 * Sets an object's location in the world based on another object's position.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to move
 * @param toId			object to move our object to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectMoveNamespace::setLocationFromObj(JNIEnv *env, jobject self, jlong objectId, jlong toId)
{
	UNREF(self);

	ServerObject * object = 0;
	ServerObject * toObject = 0;
	if (!JavaLibrary::getObject(objectId, object) || !JavaLibrary::getObject(toId, toObject))
		return JNI_FALSE;

	if (!testInvalidObjectMove(object))
		return JNI_FALSE;

	Object *toCell = ContainerInterface::getContainedByObject(*toObject);
	Object *topmost = ContainerInterface::getTopmostContainer(*toObject);
	object->teleportObject(topmost->getPosition_w(), toCell ? toCell->getNetworkId() : NetworkId::cms_invalid, "", toObject->getPosition_p(), "");

	// fix any scene ids that may be incorrect - this was causing an issue with pack houses
	object->setSceneIdOnThisAndContents(object->getSceneId());

	return JNI_TRUE;
}	// JavaLibrary::setLocationFromObj

/**
 * Returns an object's location.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to get
 *
 * @return the object's location, or nullptr on error
 */
jobject JNICALL ScriptMethodsObjectMoveNamespace::getLocation(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		return 0;

	//-- Retrieve the object's cell.  This is not necessarily the object's immediate container Object.
	//   (e.g. for mounts, the immediate Container is the mount).
	NOT_NULL(object);
	CellProperty const *const cellProperty = object->getParentCell();
	Object const *const cell = (cellProperty != nullptr) ? &(cellProperty->getOwner()) : nullptr;

	// Remember, just because we're not in a cell doesn't mean we're not contained by something else,
	// particularly in the case of a rider mounted where the mount is in the world cell.
	Vector const positionRelativeToCellOrWorld = (cell != nullptr) ? object->getPosition_c() :
		object->getPosition_w();
	NetworkId const &networkIdForCellOrWorld = (cell != nullptr) ? cell->getNetworkId() :
		NetworkId::cms_invalid;

	LocalRefPtr location;
	if (!ScriptConversion::convert(positionRelativeToCellOrWorld, object->getSceneId(),
		networkIdForCellOrWorld, location))
	{
		return 0;
	}
	return location->getReturnValue();
}	// JavaLibrary::getLocation

/**
 * Returns an object's world location.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to get
 *
 * @return the object's world location, or nullptr on error
 */
jobject JNICALL ScriptMethodsObjectMoveNamespace::getWorldLocation(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		return 0;

	LocalRefPtr location;
	if (!ScriptConversion::convert(object->getPosition_w(), object->getSceneId(), NetworkId::cms_invalid, location))
	{
		return 0;
	}
	return location->getReturnValue();
}	// JavaLibrary::getWorldLocation

/**
 * Returns an object's heading.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to get
 *
 * @return the object's location, or nullptr on error
 */
jobject JNICALL ScriptMethodsObjectMoveNamespace::getHeading(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		return 0;

	const Object * cell = ContainerInterface::getContainedByObject(*object);

	LocalRefPtr location;
	if (!ScriptConversion::convert(object->getObjectFrameK_p(), object->getSceneId(),
		cell ? cell->getNetworkId() : NetworkId::cms_invalid, location))
	{
		return 0;
	}

	return location->getReturnValue();
}	// JavaLibrary::getLocation

//========================================================================

jboolean JNICALL ScriptMethodsObjectMoveNamespace::stopFloating(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object)) return JNI_FALSE;

	CollisionProperty * collision = object->getCollisionProperty();
	if(!collision) return JNI_FALSE;

	Footprint * foot = collision->getFootprint();
	if(!foot) return JNI_FALSE;

	foot->stopFloating();

	return JNI_TRUE;
}

//========================================================================

jboolean JNICALL ScriptMethodsObjectMoveNamespace::setFloatingTime(JNIEnv *env, jobject self, jlong objectId, jint seconds )
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object)) return JNI_FALSE;

	CollisionProperty * collision = object->getCollisionProperty();
	if(!collision) return JNI_FALSE;

	Footprint * foot = collision->getFootprint();
	if(!foot) return JNI_FALSE;

	foot->setFloatingTime(static_cast<int>(seconds));

	return JNI_TRUE;
}

//========================================================================
// class JavaLibrary JNI object movement callback methods
//========================================================================

/**
 * Moves an object to face another object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param mob			object to move
 * @param target		object to face our object to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectMoveNamespace::faceToObj(JNIEnv *env, jobject self, jlong mob, jlong target)
{
	UNREF(self);

	ServerObject * object = 0;
	Object * targetObject = 0;
	if (!JavaLibrary::getObject(mob, object) || !JavaLibrary::getObject(target, targetObject))
		 return JNI_FALSE;

	if (!testInvalidObjectMove(object))
		return JNI_FALSE;

	object->lookAt_p(object->rotateTranslate_w2p(targetObject->getPosition_w ()));
	return JNI_TRUE;
}

/**
 * Moves an object to face a position.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param mob			object to move
 * @param target		location to face our object to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectMoveNamespace::faceToPos(JNIEnv *env, jobject self, jlong mob, jobject target)
{
	UNREF(self);

	ServerObject * object = 0;
	if (!JavaLibrary::getObject(mob, object))
		 return JNI_FALSE;

	if (!testInvalidObjectMove(object))
		return JNI_FALSE;

	Vector location;
	NetworkId cellId;

	if (ScriptConversion::convert(target,location,cellId))
	{
		object->lookAt_p(location);

		return JNI_TRUE;
	}
	return JNI_FALSE;
}

// ----------------------------------------------------------------------
/**
 * Puts an AI into the faceTo behavior - they'll stand in place and keep
 * turning to face the target until you tell them to do something else
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param mob			object to move
 * @param target		object to face our object to
 *
 * @return true on success, false on fail
 */

jboolean JNICALL ScriptMethodsObjectMoveNamespace::faceToBehavior(JNIEnv *env, jobject self, jlong mob, jlong target)
{
	UNREF(self);

	ServerObject * object = 0;
	Object * targetObject = 0;
	if (!JavaLibrary::getObject(mob, object) || !JavaLibrary::getObject(target, targetObject))
		 return JNI_FALSE;

	if (!testInvalidObjectMove(object))
		return JNI_FALSE;

	AICreatureController * controller = 0;
	if (JavaLibrary::getObjectController(mob, object, controller))
	{
		controller->faceTo(NetworkId(target));
		return JNI_TRUE;
	}
	else
	{
		return JNI_FALSE;
	}
}

// ----------------------------------------------------------------

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getYaw(JNIEnv *env, jobject self, jlong target)
{
	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return -1.0f;

	return convertRadiansToDegrees (object->getObjectFrameK_p ().theta ());
}

// ----------------------------------------------------------------

jboolean JNICALL ScriptMethodsObjectMoveNamespace::setYaw(JNIEnv *env, jobject self, jlong target, jfloat yaw)
{
	PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::setYaw");
	ServerObject * object = nullptr;
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::setYaw::getObject");
		if (!JavaLibrary::getObject(target, object))
		{
			return JNI_FALSE;
		}
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::setYaw::testInvalidObjectMove");
		if (!testInvalidObjectMove(object))
		{
			return JNI_FALSE;
		}
	}

	float radians = convertDegreesToRadians(yaw) - object->getObjectFrameK_p().theta();
	if (object->getClient())
	{
		Transform t(object->getTransform_o2p());
		if (radians != 0.0f)
		{
			t.yaw_l(radians);
			t.reorthonormalize();
			ServerController *controller = safe_cast<ServerController*>(object->getController());
			ServerObject *destContainer = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*object));
			controller->teleport(t, destContainer);
		}
	}
	else
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::setYaw::objectYaw");
		object->yaw_o(radians);
	}
	return JNI_TRUE;
}

// ----------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::modifyYaw(JNIEnv *env, jobject self, jlong target, jfloat degrees)
{
	PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyYaw");
	ServerObject * object = nullptr;
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyYaw::getObject");
		if (!JavaLibrary::getObject(target, object))
		{
			return;
		}
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyYaw::testInvalidObjectMove");
		if (!testInvalidObjectMove(object))
		{
			return;
		}
	}

	float const radians = convertDegreesToRadians(degrees);
	if (object->getClient())
	{
		Transform t(object->getTransform_o2p());
		if (radians != 0.0f)
		{
			t.yaw_l(radians);
			t.reorthonormalize();
			ServerController *controller = safe_cast<ServerController*>(object->getController());
			ServerObject *destContainer = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*object));
			controller->teleport(t, destContainer);
		}
	}
	else
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyYaw::objectYaw");
		object->yaw_o(radians);
	}
}

// ----------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::modifyPitch(JNIEnv *env, jobject self, jlong target, jfloat degrees)
{
	PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyPitch");
	ServerObject * object = nullptr;
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyPitch::getObject");
		if (!JavaLibrary::getObject(target, object))
		{
			return;
		}
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyPitch::testInvalidObjectMove");
		if (!testInvalidObjectMove(object))
		{
			return;
		}
	}

	float const radians = convertDegreesToRadians(degrees);
	if (object->getClient())
	{
		Transform t(object->getTransform_o2p());
		if (radians != 0.0f)
		{
			t.pitch_l(radians);
			t.reorthonormalize();
			ServerController *controller = safe_cast<ServerController*>(object->getController());
			ServerObject *destContainer = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*object));
			controller->teleport(t, destContainer);
		}
	}
	else
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyPitch::objectPitch");
		object->pitch_o(radians);
	}
}

// ----------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::modifyRoll(JNIEnv *env, jobject self, jlong target, jfloat degrees)
{
	PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyRoll");
	ServerObject * object = nullptr;
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyRoll::getObject");
		if (!JavaLibrary::getObject(target, object))
		{
			return;
		}
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyRoll::testInvalidObjectMove");
		if (!testInvalidObjectMove(object))
		{
			return;
		}
	}

	float const radians = convertDegreesToRadians(degrees);
	if (object->getClient())
	{
		Transform t(object->getTransform_o2p());
		if (radians != 0.0f)
		{
			t.roll_l(radians);
			t.reorthonormalize();
			ServerController *controller = safe_cast<ServerController*>(object->getController());
			ServerObject *destContainer = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*object));
			controller->teleport(t, destContainer);
		}
	}
	else
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::modifyRoll::objectRoll");
		object->roll_o(radians);
	}
}

// ----------------------------------------------------------------

jfloatArray JNICALL ScriptMethodsObjectMoveNamespace::getQuaternion(JNIEnv *env, jobject self, jlong target)
{
	PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::getQuaternion");
	ServerObject * object = nullptr;
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::getQuaternion::getObject");
		if (!JavaLibrary::getObject(target, object))
		{
			return 0;
		}
	}

	Quaternion const q(object->getTransform_o2p());

	LocalFloatArrayRefPtr valueArray = createNewFloatArray(static_cast<jsize>(4));
	if (valueArray == LocalFloatArrayRef::cms_nullPtr)
	{
		return 0;
	}

	jfloat value = static_cast<jfloat>(q.w);
	setFloatArrayRegion(*valueArray, 0, 1, &value);

	value = static_cast<jfloat>(q.x);
	setFloatArrayRegion(*valueArray, 1, 1, &value);

	value = static_cast<jfloat>(q.y);
	setFloatArrayRegion(*valueArray, 2, 1, &value);

	value = static_cast<jfloat>(q.z);
	setFloatArrayRegion(*valueArray, 3, 1, &value);

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::setQuaternion(JNIEnv *env, jobject self, jlong target, jfloat qw, jfloat qx, jfloat qy, jfloat qz)
{
	PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::setQuaternion");
	ServerObject * object = nullptr;
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::setQuaternion::getObject");
		if (!JavaLibrary::getObject(target, object))
		{
			return;
		}
	}

	Quaternion const q(static_cast<real>(qw), static_cast<real>(qx), static_cast<real>(qy), static_cast<real>(qz));
	Transform t;
	q.getTransform(&t);
	t.setPosition_p(object->getTransform_o2p().getPosition_p());

	if (object->getClient())
	{
		t.reorthonormalize();
		ServerController *controller = safe_cast<ServerController*>(object->getController());
		ServerObject *destContainer = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*object));
		controller->teleport(t, destContainer);
	}
	else
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::setQuaternion::objectSetQuaternion");
		object->setTransform_o2p(t);
	}
}

// ----------------------------------------------------------------

jint JNICALL ScriptMethodsObjectMoveNamespace::getFurnitureRotationDegree(JNIEnv *env, jobject self, jlong player)
{
	PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::getFurnitureRotationDegree");
	ServerObject * object = nullptr;
	{
		PROFILER_AUTO_BLOCK_DEFINE("JavaLibrary::getFurnitureRotationDegree::getObject");
		if (!JavaLibrary::getObject(player, object))
		{
			return 90;
		}
	}

	if (object->getClient())
		return abs(object->getClient()->getFurnitureRotationDegree());

	return 90;
}

// ----------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::saveDecorationLayout(JNIEnv *env, jobject self, jlong player, jlong pob, jint saveSlotNumber, jstring description)
{
	CreatureObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject) || !playerObject)
		return;

	ServerObject const * pobObject = nullptr;
	if (!JavaLibrary::getObject(pob, pobObject) || !pobObject)
		return;

	JavaStringParam localDescription(description);
	std::string descriptionStr;
	if (!JavaLibrary::convert(localDescription, descriptionStr))
		return;

	playerObject->saveDecorationLayout(*pobObject, saveSlotNumber, descriptionStr);
}

// ----------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::restoreDecorationLayout(JNIEnv *env, jobject self, jlong player, jlong pob, jint saveSlotNumber)
{
	CreatureObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject) || !playerObject)
		return;

	ServerObject const * pobObject = nullptr;
	if (!JavaLibrary::getObject(pob, pobObject) || !pobObject)
		return;

	playerObject->restoreDecorationLayout(*pobObject, saveSlotNumber);
}

// ----------------------------------------------------------------

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectMoveNamespace::setSlopeModAngle(JNIEnv *env, jobject self, jlong objectId, jfloat value)
{
	UNREF(self);

	CreatureObject * object = 0;
	CreatureController * controller = 0;
	if (!JavaLibrary::getObjectController(objectId, object, controller))
		 return JNI_FALSE;

	return controller->setSlopeModAngle(value * PI / 180);
}

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jfloat JNICALL ScriptMethodsObjectMoveNamespace::getSlopeModAngle(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		 return 0.0f;

	return object->getSlopeModAngle() * 180 / PI;
}

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jfloat JNICALL ScriptMethodsObjectMoveNamespace::getTemplateSlopeModAngle(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		 return 0.0f;

	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(object->getSharedTemplate());

	return sharedTemplate->getSlopeModAngle() * 180 / PI;
}

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectMoveNamespace::setWaterModPercent(JNIEnv *env, jobject self, jlong objectId, jfloat value)
{
	UNREF(self);

	CreatureObject * object = 0;
	CreatureController * controller = 0;
	if (!JavaLibrary::getObjectController(objectId, object, controller))
		 return JNI_FALSE;

	return controller->setWaterModPercent(value);
}

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jfloat JNICALL ScriptMethodsObjectMoveNamespace::getWaterModPercent(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		 return 0.0f;

	return object->getWaterModPercent();
}

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jfloat JNICALL ScriptMethodsObjectMoveNamespace::getTemplateWaterModPercent(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		 return 0.0f;

	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(object->getSharedTemplate());

	return sharedTemplate->getWaterModPercent();
}

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsObjectMoveNamespace::setSlopeModPercent(JNIEnv *env, jobject self, jlong objectId, jfloat value)
{
	UNREF(self);

	CreatureObject * object = 0;
	CreatureController * controller = 0;
	if (!JavaLibrary::getObjectController(objectId, object, controller))
		 return JNI_FALSE;

	return controller->setSlopeModPercent(value);
}

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jfloat JNICALL ScriptMethodsObjectMoveNamespace::getSlopeModPercent(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		 return 0.0f;

	return object->getSlopeModPercent();
}

/**
 * Sets the slope aversion for an object.  This may cause a repath to take place.  The aversion is calculated by adding the
 * height delta times the aversion value to the actual distance.  So a 5 meter edge with a 1 meter slope difference would
 * be pathed as 6 meters with a 1 aversion factor.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param objectId		object to change the slope aversion
 * @param value			the aversion value. 0 is no aversion.
 *
 * @return true on success, false on fail
 */
jfloat JNICALL ScriptMethodsObjectMoveNamespace::getTemplateSlopeModPercent(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		 return 0.0f;

	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(object->getSharedTemplate());

	return sharedTemplate->getSlopeModPercent();
}

jboolean JNICALL ScriptMethodsObjectMoveNamespace::setMovementPercent(JNIEnv *, jobject, jlong objectId, jfloat value)
{
	TangibleObject *tangibleTarget = 0;
	if (JavaLibrary::getObject(objectId, tangibleTarget))
	{
		if (tangibleTarget->asShipObject())
			return tangibleTarget->asShipObject()->setMovementPercent(value);
		else if (tangibleTarget->asCreatureObject())
			return tangibleTarget->asCreatureObject()->setMovementPercent(value);
	}
	return static_cast<jboolean>(JNI_FALSE);
}

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getMovementPercent(JNIEnv *, jobject, jlong objectId)
{
	TangibleObject *tangibleTarget = 0;
	if (JavaLibrary::getObject(objectId, tangibleTarget))
	{
		if (tangibleTarget->asShipObject())
			return tangibleTarget->asShipObject()->getMovementPercent();
		else if (tangibleTarget->asCreatureObject())
			return tangibleTarget->asCreatureObject()->getMovementPercent();
	}
	return 0.f;
}

jboolean JNICALL ScriptMethodsObjectMoveNamespace::setAccelPercent(JNIEnv *env, jobject self, jlong objectId, jfloat value)
{
	UNREF(self);

	CreatureObject * object = 0;
	CreatureController * controller = 0;
	if (!JavaLibrary::getObjectController(objectId, object, controller))
		 return JNI_FALSE;

	return controller->setAccelPercent(value);
}

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getAccelPercent(JNIEnv *env, jobject self, jlong objectId)
{
	UNREF(self);

	CreatureObject * object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		 return 0.0f;

	return object->getAccelPercent();
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::addLocationTarget(JNIEnv * env, jobject self, jlong target, jstring name, jobject location, jfloat radius, jboolean is3d)
{
	JavaStringParam localName(name);

	if(! self)
		return;
	if(!name)
		return;
	if(!location)
		return;

	Vector pos;
	if (ScriptConversion::convertWorld(location, pos))
	{
		LocationData d;
		// @todo : check scene id!
		std::string s;
		JavaStringPtr scene = getStringField(LocalRefParam(location), JavaLibrary::getFidLocationArea());
		if(JavaLibrary::convert(*scene, s))
		{
			d.scene = s;

			// 3d location targets are denoted by y coordinates > 50k, with 100k added to them
			// location targets with y coordinates less than 50k have their y coordinates ignored during comparisons.
			if (is3d)
				pos.y += 100000.0f;
			d.location.setCenter(pos);
			d.location.setRadius(radius);
			Unicode::String n;
			if(JavaLibrary::convert(localName, n))
			{
				TangibleObject * t = 0;
				d.name = n;
				if(JavaLibrary::getObject(target, t))
				{
					if(t)
					{
						if (testInvalidObjectMove(t))
							t->addLocationTarget(d);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::removeLocationTarget(JNIEnv * env, jobject self, jlong target, jstring name)
{
	JavaStringParam localName(name);

	if(! self)
		return;
	if(! name)
		return;

	Unicode::String n;
	if(JavaLibrary::convert(localName, n))
	{
		TangibleObject * t = 0;
		if(JavaLibrary::getObject(target, t))
		{
			if(t)
			{
				if (testInvalidObjectMove(t))
					t->removeLocationTarget(n);
			}
		}
	}
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectMoveNamespace::elevatorMove(JNIEnv *env, jobject self, jlong mob, jint nFloors )
{
	UNREF(env);
	UNREF(self);

	CreatureObject * creature = 0;
	ServerController * controller = 0;

	if (JavaLibrary::getObjectController(mob, creature, controller))
	{
		int movedFloors = controller->elevatorMove(static_cast<int>(nFloors));

		return static_cast<jint>(movedFloors);
	}

	return 0;
}


jboolean JNICALL ScriptMethodsObjectMoveNamespace::monitorCreatureMovement(JNIEnv *env, jobject self, jlong mob, jlong target, jfloat skittishness, jfloat curve)
{
	UNREF(env);
	UNREF(self);

	CreatureObject * creature = 0;
	if (JavaLibrary::getObject(mob, creature))
	{
		CachedNetworkId targetId(target);
		if (!targetId)
			return false;
		return creature->monitorCreatureMovement(targetId, skittishness, curve);
	}
	return false;
}

jboolean JNICALL ScriptMethodsObjectMoveNamespace::ignoreCreatureMovement(JNIEnv *env, jobject self, jlong mob, jlong target)
{
	UNREF(env);
	UNREF(self);

	CreatureObject * creature = 0;
	if (JavaLibrary::getObject(mob, creature))
	{
		CachedNetworkId targetId(target);
		if (targetId == NetworkId::cms_invalid)
			return false;
		return creature->ignoreCreatureMovement(targetId);
	}
	return false;
}

jobject JNICALL ScriptMethodsObjectMoveNamespace::getApproachSpeed(JNIEnv *env, jobject self, jlong i_mob, jobject i_mobDelta, jlong i_target, jobject i_targetDelta, jfloat i_time)
{
	UNREF(env);
	UNREF(self);

	Vector resultVector(Vector::zero);

	CreatureObject * mob = 0;
	Vector mobDelta;
	NetworkId mobCell;
	CreatureObject * target = 0;
	Vector targetDelta;
	NetworkId targetCell;
	if (i_time > 0 && JavaLibrary::getObject(i_mob, mob) && ScriptConversion::convert(i_mobDelta, mobDelta, mobCell) &&
		  JavaLibrary::getObject(i_target, target) && ScriptConversion::convert(i_targetDelta, targetDelta, targetCell))
	{
		Vector mobPosition = mob->getPosition_w();
		Vector targetPosition = target->getPosition_w();
		float originalDistance = (targetPosition - targetDelta).magnitudeBetween(mobPosition - mobDelta);
		float currentDistance = targetPosition.magnitudeBetween(mobPosition);
		float approachVelocity = (originalDistance - currentDistance) / i_time;
		float tangentVelocity = sqrt(((mobDelta - targetDelta)/i_time).magnitudeSquared() - approachVelocity * approachVelocity);
		resultVector.x = approachVelocity;
		resultVector.y = tangentVelocity;
		resultVector.z = currentDistance;
	}

	LocalRefPtr result;
	if (!ScriptConversion::convert(resultVector, NetworkId::cms_invalid, result))
		return 0;
	return result->getReturnValue();
}

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getTargetBearing(JNIEnv *env, jobject self, jlong i_mob, jlong i_target)
{
	UNREF(env);
	UNREF(self);

	ServerObject * mob = 0;
	ServerObject * target = 0;
	float result=0;
	if (JavaLibrary::getObject(i_mob, mob) && JavaLibrary::getObject(i_target, target))
	{
		Vector localPosition = mob->rotateTranslate_w2o(target->getPosition_w());
		result = float(atan2(localPosition.x, localPosition.z)) * 180 / PI;
	}
	return result;
}

void JNICALL ScriptMethodsObjectMoveNamespace::warpPlayerCellId(JNIEnv *env, jobject self, jlong player, jstring scene, jfloat x_w, jfloat y_w, jfloat z_w, jlong cell, jfloat x_p, jfloat y_p, jfloat z_p, jstring callback, jboolean forceLoadScreen)
{
	ServerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
		return;

	JavaStringParam localScene(scene);
	std::string sceneName;
	if (!JavaLibrary::convert(localScene, sceneName))
		return;

	std::string callbackName;

	if (callback)
	{
		JavaStringParam localCallback(callback);
		if (!JavaLibrary::convert(localCallback, callbackName))
			return;
	}

	NetworkId cellId(cell);
	GameServer::getInstance().requestSceneWarp(
		CachedNetworkId(*playerObject),
		sceneName,
		Vector(x_w, y_w, z_w),
		cellId,
		Vector(x_p, y_p, z_p),
		callbackName.c_str(),
		forceLoadScreen);
}

void JNICALL ScriptMethodsObjectMoveNamespace::warpPlayerCellName(JNIEnv *env, jobject self, jlong player, jstring scene, jfloat x_w, jfloat y_w, jfloat z_w, jlong building, jstring cell, jfloat x_p, jfloat y_p, jfloat z_p, jstring callback, jboolean forceLoadScreen)
{
	ServerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
		return;

	JavaStringParam localScene(scene);
	std::string sceneName;
	if (!JavaLibrary::convert(localScene, sceneName))
		return;

	JavaStringParam localCell(cell);
	std::string cellName;
	if (!JavaLibrary::convert(localCell, cellName))
		return;

	NetworkId buildingId(building);
	if (buildingId == NetworkId::cms_invalid)
		return;

	std::string callbackName;
	if (callback)
	{
		JavaStringParam localCallback(callback);
		if (!JavaLibrary::convert(localCallback, callbackName))
			return;
	}

	GameServer::getInstance().requestSceneWarp(
		CachedNetworkId(*playerObject),
		sceneName,
		Vector(x_w, y_w, z_w),
		buildingId,
		cellName,
		Vector(x_p, y_p, z_p),
		callbackName.c_str(),
		forceLoadScreen);
}

void JNICALL ScriptMethodsObjectMoveNamespace::disconnectPlayer(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject * playerCreature = 0;
	if (JavaLibrary::getObject(player, playerCreature))
		playerCreature->disconnect();
}

jboolean JNICALL ScriptMethodsObjectMoveNamespace::isPlayerConnected(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject const * playerCreature = 0;
	if (JavaLibrary::getObject(player, playerCreature) && playerCreature)
	{
		PlayerObject const * const playerPlayer = PlayerCreatureController::getPlayerObject(playerCreature);
		if (playerPlayer && (playerPlayer->getSessionStartPlayTime() != 0))
		{
			return JNI_TRUE;
		}
		else
		{
			return JNI_FALSE;
		}
	}

	// player may be on a different game server, check the cluster wide LFG data
	if (ServerUniverse::getConnectedCharacterLfgData().count(NetworkId(static_cast<NetworkId::NetworkIdType>(player))) > 0)
		return JNI_TRUE;

	return JNI_FALSE;
}

jobject JNICALL ScriptMethodsObjectMoveNamespace::getConnectedPlayerLocation(JNIEnv * env, jobject self, jlong player)
{
	std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
	std::map<NetworkId, LfgCharacterData>::const_iterator const iterFind = connectedCharacterLfgData.find(NetworkId(static_cast<NetworkId::NetworkIdType>(player)));
	if (iterFind == connectedCharacterLfgData.end())
		return nullptr;

	LocalRefPtr dictionary = createNewObject(JavaLibrary::getClsDictionary(), JavaLibrary::getMidDictionary());
	if (dictionary == LocalRef::cms_nullPtr)
		return nullptr;

	callObjectMethod(*dictionary, JavaLibrary::getMidDictionaryPut(), JavaString("planet").getValue(), JavaString(iterFind->second.locationPlanet).getValue());

	if (!iterFind->second.locationRegion.empty())
		callObjectMethod(*dictionary, JavaLibrary::getMidDictionaryPut(), JavaString("region").getValue(), JavaString(iterFind->second.locationRegion).getValue());

	if (!iterFind->second.locationPlayerCity.empty())
		callObjectMethod(*dictionary, JavaLibrary::getMidDictionaryPut(), JavaString("playerCity").getValue(), JavaString(iterFind->second.locationPlayerCity).getValue());

	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	return dictionary->getReturnValue();
}

jboolean JNICALL ScriptMethodsObjectMoveNamespace::isPlayerActive(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject const * playerCreature = 0;
	if (JavaLibrary::getObject(player, playerCreature) && playerCreature)
	{
		PlayerObject const * const playerPlayer = PlayerCreatureController::getPlayerObject(playerCreature);
		if (playerPlayer && (playerPlayer->getSessionLastActiveTime() != 0))
		{
			return JNI_TRUE;
		}
		else
		{
			return JNI_FALSE;
		}
	}

	// player may be on a different game server, check the cluster wide LFG data
	std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
	std::map<NetworkId, LfgCharacterData>::const_iterator iter = connectedCharacterLfgData.find(NetworkId(static_cast<NetworkId::NetworkIdType>(player)));
	if ((iter != connectedCharacterLfgData.end()) && iter->second.active)
		return JNI_TRUE;

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getMovementSpeed(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	CreatureController * const creatureController = CreatureController::getCreatureController(networkId);

	return (creatureController != nullptr) ? creatureController->getCurrentVelocity().magnitude() : 0.0f;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getWalkSpeed(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	return (creatureObject != nullptr) ? creatureObject->getWalkSpeed() : 0.0f;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getRunSpeed(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	return (creatureObject != nullptr) ? creatureObject->getRunSpeed() : 0.0f;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::setBaseWalkSpeed(JNIEnv * /*env*/, jobject /*self*/, jlong object, jfloat speed)
{
	NetworkId const networkId(object);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	if (creatureObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsObjectMove::setBaseWalkSpeed() Unable to resolve the object(%s) to a CreatureObject.", networkId.getValueString().c_str()));
		return;
	}

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsObjectMove::setBaseWalkSpeed() object(%s) speed(%.2f)", networkId.getValueString().c_str(), speed));

	creatureObject->setBaseWalkSpeed(speed);
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getBaseWalkSpeed(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	return (creatureObject != nullptr) ? creatureObject->getBaseWalkSpeed() : 0.0f;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::setBaseRunSpeed(JNIEnv * /*env*/, jobject /*self*/, jlong object, jfloat speed)
{
	NetworkId const networkId(object);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	if (creatureObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsObjectMove::setBaseRunSpeed() Unable to resolve the object(%s) to a CreatureObject.", networkId.getValueString().c_str()));
		return;
	}

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsObjectMove::setBaseRunSpeed() object(%s) speed(%.2f)", networkId.getValueString().c_str(), speed));

	creatureObject->setBaseRunSpeed(speed);
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsObjectMoveNamespace::getBaseRunSpeed(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	return (creatureObject != nullptr) ? creatureObject->getBaseRunSpeed() : 0.0f;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::setMovementWalk(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	if (creatureObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsObjectMove::setMovementWalk() Unable to resolve the object(%s) to a CreatureObject.", networkId.getValueString().c_str()));
		return;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(creatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsObjectMove::setMovementWalk() Unable to resolve the object's(%s) controller to an AiCreatureController.", creatureObject->getDebugInformation().c_str()));
		return;
	}

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsObjectMove::setMovementWalk() object(%s)", networkId.getValueString().c_str()));

	aiCreatureController->setMovementWalk();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::setMovementRun(JNIEnv * /*env*/, jobject /*self*/, jlong object)
{
	NetworkId const networkId(object);
	CreatureObject * const creatureObject = CreatureObject::getCreatureObject(networkId);

	if (creatureObject == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsObjectMove::setMovementRun() Unable to resolve the object(%s) to a CreatureObject.", networkId.getValueString().c_str()));
		return;
	}

	AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(creatureObject->getController());

	if (aiCreatureController == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("ScriptMethodsObjectMove::setMovementRun() Unable to resolve the object's(%s) controller to an AiCreatureController.", creatureObject->getDebugInformation().c_str()));
		return;
	}

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("ScriptMethodsObjectMove::setMovementRun() object(%s)", networkId.getValueString().c_str()));

	aiCreatureController->setMovementRun();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::hideFromClient(JNIEnv *env, jobject self, jlong target, jboolean hide)
{
	TangibleObject * tangibleObject = 0;
	if (JavaLibrary::getObject(target, tangibleObject))
		tangibleObject->setHidden(hide);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::setClientUsesAnimationLocomotion(JNIEnv *env, jobject self, jlong target, jboolean enabled)
{
	CreatureObject * creature = 0;
	if (JavaLibrary::getObject(target, creature))
	{
		if (creature->isAuthoritative())
		{
			creature->setClientUsesAnimationLocomotion(enabled);
			return;
		}
	}
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsObjectMoveNamespace::getTransform_o2w(JNIEnv *env, jobject self, jlong objectId)
{
	ServerObject *object = 0;
	if (!JavaLibrary::getObject(objectId, object))
	{
		JavaLibrary::throwInternalScriptError("JavaLibrary::getTransform_o2w could not resolve object");
		return 0;
	}

	LocalRefPtr transform;
	if (ScriptConversion::convert(object->getTransform_o2w(), transform))
		return transform->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsObjectMoveNamespace::getTransform_o2p(JNIEnv *env, jobject self, jlong objectId)
{
	ServerObject *object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		JavaLibrary::throwInternalScriptError("JavaLibrary::getTransform_o2p could not resolve object");
	else
	{
		LocalRefPtr transform;
		if (ScriptConversion::convert(object->getTransform_o2p(), transform))
			return transform->getReturnValue();
	}
	return 0;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsObjectMoveNamespace::getAttachedTo(JNIEnv *env, jobject self, jlong objectId)
{
	ServerObject *object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		JavaLibrary::throwInternalScriptError("JavaLibrary::getAttachedTo could not resolve object");
	else
	{
		Object *attachedTo = object->getAttachedTo();
		if (attachedTo)
			return (attachedTo->getNetworkId()).getValue();
	}
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::setTransform_o2p(JNIEnv *env, jobject self, jlong objectId, jobject transform)
{
	ServerObject *object = 0;
	if (!JavaLibrary::getObject(objectId, object))
		JavaLibrary::throwInternalScriptError("JavaLibrary::setTransform_o2p could not resolve object");
	else if (testInvalidObjectMove(object))
	{
		Transform newTransform;
		ScriptConversion::convert(transform, newTransform);
		safe_cast<ServerController *>(object->getController())->teleport(newTransform, safe_cast<ServerObject *>(object->getAttachedTo()));
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::sendPlayerToTutorial(JNIEnv *env, jobject self, jlong player)
{
	CreatureObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
		return;
	
	NewbieTutorial::setupCharacterForTutorial(playerObject);
	GameServer::getInstance().requestSceneWarp(CachedNetworkId(*playerObject), NewbieTutorial::getSceneId(), NewbieTutorial::getTutorialLocation(), NetworkId::cms_invalid, Vector(0,0,0));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::addPassiveReveal(JNIEnv * env, jobject self, jlong object, jlong target, jint range)
{
	TangibleObject * toObject = 0;
	if (!JavaLibrary::getObject(object, toObject))
		return;

	TangibleObject * toTarget = 0;
	if (!JavaLibrary::getObject(target, toTarget))
		return;

	toObject->addPassiveReveal(*toTarget, range);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::removePassiveReveal(JNIEnv * env, jobject self, jlong object, jlong target)
{
	TangibleObject * toObject = 0;
	if (!JavaLibrary::getObject(object, toObject))
		return;

	toObject->removePassiveReveal(NetworkId(static_cast<NetworkId::NetworkIdType>(target)));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::clearPassiveRevealList(JNIEnv * env, jobject self, jlong object)
{
	TangibleObject * toObject = 0;
	if (!JavaLibrary::getObject(object, toObject))
		return;

	toObject->removeAllPassiveReveal();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsObjectMoveNamespace::getPassiveRevealRange(JNIEnv * env, jobject self, jlong object, jlong target)
{
	TangibleObject * toObject = 0;
	if (!JavaLibrary::getObject(object, toObject))
		return -1;

	return toObject->getPassiveRevealRange(NetworkId(static_cast<NetworkId::NetworkIdType>(target)));
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsObjectMoveNamespace::getPassiveRevealList(JNIEnv * env, jobject self, jlong object)
{
	TangibleObject * toObject = 0;
	if (!JavaLibrary::getObject(object, toObject))
		return 0;

	std::map<NetworkId, int> const & passiveReveal = toObject->getPassiveReveal();
	if (passiveReveal.empty())
		return 0;

	// params (see below) will be responsible for destructing these
	std::vector<NetworkId> * id = new std::vector<NetworkId>;
	std::vector<int> * range = new std::vector<int>;

	for (std::map<NetworkId, int>::const_iterator iter = passiveReveal.begin(); iter != passiveReveal.end(); ++iter)
	{
		id->push_back(iter->first);
		range->push_back(iter->second);
	}

	ScriptParams params;
	params.addParam(*id, "id", true);
	params.addParam(*range, "range", true);

	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(params, dictionary);
	return dictionary->getReturnValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsObjectMoveNamespace::requestSendPositionUpdate(JNIEnv * env, jobject self, jlong object)
{
	ServerObject * soObject = 0;
	if (!JavaLibrary::getObject(object, soObject))
		return;

	PositionUpdateTracker::positionChanged(*soObject);
}

// ======================================================================
