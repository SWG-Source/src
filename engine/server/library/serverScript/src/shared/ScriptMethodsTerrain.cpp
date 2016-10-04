// ======================================================================
//
// ScriptMethodsTerrain.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/LocationRequest.h"
#include "serverPathfinding/ServerPathBuilder.h"
#include "serverUtility/ServerClock.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/FloorContactShape.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/Footprint.h"
#include "sharedCollision/MultiList.h"
#include "sharedDebug/Profiler.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedGame/SharedTerrainSurfaceObjectTemplate.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Sphere.h"
#include "sharedNetworkMessages/CreateClientPathMessage.h"
#include "sharedNetworkMessages/DestroyClientPathMessage.h"
#include "sharedNetworkMessages/EnterStructurePlacementModeMessage.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/LotManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/StructureFootprint.h"
#include "sharedTerrain/TerrainObject.h"

#include <algorithm>
#include <list>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsTerrainNamespace
// ======================================================================

namespace ScriptMethodsTerrainNamespace
{
	bool install();

	jfloat       JNICALL getHeightAtLocation (JNIEnv* env, jobject self, jfloat x, jfloat z);
	jfloat       JNICALL getCoverAtLocation (JNIEnv* env, jobject self, jfloat x, jfloat z);
	jobject      JNICALL getGoodLocation (JNIEnv* env, jobject self, jfloat hintX, jfloat hintY, jobject searchRectLowerLeftLocation, jobject searchRectUpperRightLocation, jboolean dontCheckWater, jboolean dontCheckSlope);
	jobject      JNICALL getGoodLocationAvoidCollidables (JNIEnv* env, jobject self, jfloat hintX, jfloat hintY, jobject searchRectLowerLeftLocation, jobject searchRectUpperRightLocation, jboolean dontCheckWater, jboolean dontCheckSlope, jfloat staticObjDistance);
	jboolean     JNICALL enterClientStructurePlacementMode (JNIEnv* env, jobject self, jlong player, jlong deed, jstring serverObjectTemplateName);
	jfloat       JNICALL canPlaceStructure (JNIEnv* env, jobject self, jstring serverObjectTemplateName, jobject position, jint rotation);
	jlong        JNICALL createTemporaryStructure (JNIEnv* env, jobject self, jstring serverObjectTemplateName, jobject position, jint rotation);
	jint         JNICALL getNumberOfLots (JNIEnv* env, jobject self, jstring serverObjectTemplateName);
	jfloat       JNICALL getLocalTime (JNIEnv* env, jobject self);
	jfloat       JNICALL getLocalDayLength (JNIEnv* env, jobject self);
	jboolean     JNICALL setWeatherData (JNIEnv* env, jobject self, jint index, jfloat windVelocityX, jfloat windVelocityZ);
	jboolean     JNICALL isBelowWater (JNIEnv* env, jobject self, jobject location);
	jboolean     JNICALL isWaterInRect (JNIEnv* env, jobject self, jfloat x0, jfloat y0, jfloat x1, jfloat y1);
	jboolean     JNICALL createClientPath (JNIEnv * env, jobject self, jlong player, jobject start, jobject end);
	jboolean     JNICALL createClientPathAdvanced(JNIEnv * env, jobject self, jlong player, jobject locationStart, jobject locationEnd, jstring appearanceTableEntry);
	jboolean     JNICALL destroyClientPath (JNIEnv * env, jobject self, jlong player);
	jfloat       JNICALL getWaterTableHeight (JNIEnv* env, jobject self, jobject location);
	jboolean     JNICALL requestLocation (JNIEnv * env, jobject self, jlong scriptObject, jstring locationId, jobject searchLocation, jfloat searchRadius, jfloat locationReservationRadius, jboolean checkWater, jboolean checkSlope);
	jboolean     JNICALL isRelativePointOnSameFloorAsObject(JNIEnv * env, jobject self, jlong scriptObject, jfloat relativeXLocation, jfloat relativeZLocation);
	jfloat       JNICALL getFloorHeightAtRelativePointOnSameFloorAsObject(JNIEnv * env, jobject self, jlong scriptObject, jfloat relativeXLocation, jfloat relativeZLocation);
	jboolean     JNICALL isOnAFloor (JNIEnv * env, jobject self, jlong scriptObject);
	jboolean     JNICALL setCreatureCoverDefault(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setCreatureCover(JNIEnv *env, jobject self, jlong target, jint cover);
	void         JNICALL setCreatureCoverVisibility(JNIEnv * env, jobject self, jlong target, jboolean isVisibile);
	jboolean     JNICALL getCreatureCoverVisibility(JNIEnv * env, jobject self, jlong target);
	jlong        JNICALL getStandingOn(JNIEnv *env, jobject self, jlong target);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsTerrainNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsTerrainNamespace::c)}
	JF("getHeightAtLocation",               "(FF)F",                                                     getHeightAtLocation),
	JF("getCoverAtLocation",                "(FF)F",                                                     getCoverAtLocation),
	JF("getGoodLocation",                   "(FFLscript/location;Lscript/location;ZZ)Lscript/location;", getGoodLocation),
	JF("getGoodLocationAvoidCollidables",   "(FFLscript/location;Lscript/location;ZZF)Lscript/location;", getGoodLocationAvoidCollidables),
	JF("_enterClientStructurePlacementMode", "(JJLjava/lang/String;)Z",       enterClientStructurePlacementMode),
	JF("canPlaceStructure",                 "(Ljava/lang/String;Lscript/location;I)F",                   canPlaceStructure),
	JF("_createTemporaryStructure",          "(Ljava/lang/String;Lscript/location;I)J",     createTemporaryStructure),
	JF("getNumberOfLots",                   "(Ljava/lang/String;)I",                                     getNumberOfLots),
	JF("getLocalTime",                      "()F",                                                       getLocalTime),
	JF("getLocalDayLength",                 "()F",                                                       getLocalDayLength),
	JF("setWeatherData",                    "(IFF)Z",                                                    setWeatherData),
	JF("isBelowWater",                      "(Lscript/location;)Z",                                      isBelowWater),
	JF("isWaterInRect",						"(FFFF)Z",													 isWaterInRect),
	JF("_createClientPath",                  "(JLscript/location;Lscript/location;)Z",      createClientPath),
	JF("_createClientPathAdvanced",          "(JLscript/location;Lscript/location;Ljava/lang/String;)Z", createClientPathAdvanced),
	JF("_destroyClientPath",                 "(J)Z",                                        destroyClientPath),
	JF("getWaterTableHeight",               "(Lscript/location;)F",                                      getWaterTableHeight),
	JF("_requestLocation", "(JLjava/lang/String;Lscript/location;FFZZ)Z", requestLocation),
	JF("_isRelativePointOnSameFloorAsObject",               "(JFF)Z",                       isRelativePointOnSameFloorAsObject),
	JF("_getFloorHeightAtRelativePointOnSameFloorAsObject", "(JFF)F",                       getFloorHeightAtRelativePointOnSameFloorAsObject),
	JF("_isOnAFloor",                                       "(J)Z",                         isOnAFloor),
	JF("_setCreatureCover", "(J)Z", setCreatureCoverDefault),
	JF("_setCreatureCover", "(JI)Z", setCreatureCover),
	JF("_setCreatureCoverVisibility", "(JZ)V", setCreatureCoverVisibility),
	JF("_getCreatureCoverVisibility", "(J)Z", getCreatureCoverVisibility),
	JF("_getStandingOn", "(J)J", getStandingOn),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jfloat JNICALL ScriptMethodsTerrainNamespace::getHeightAtLocation(JNIEnv * /*env*/, jobject /*self*/, jfloat x, jfloat z)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getHeightAtLocation");
	float height = 0.f;
	const bool result = TerrainObject::getInstance()->getHeightForceChunkCreation(Vector(x, 0.f, z), height);
	DEBUG_WARNING (!result, ("JavaLibrary::getHeightAtLocation: failed for position %1.2f, %1.2f", x, z));
	UNREF (result);

	return height;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsTerrainNamespace::getCoverAtLocation(JNIEnv * /*env*/, jobject /*self*/, jfloat x, jfloat z)
{
	const Vector position (x, 0.f, z);

	float cover = 0.f;
	const SharedTerrainSurfaceObjectTemplate* const sharedTerrainSurfaceObjectTemplate = safe_cast<const SharedTerrainSurfaceObjectTemplate*> (TerrainObject::getInstance ()->getSurfaceProperties (position));
	if (sharedTerrainSurfaceObjectTemplate)
		cover += sharedTerrainSurfaceObjectTemplate->getCover ();

	float height = 0.f;
	if (TerrainObject::getInstance ()->getHeight (position, height))
	{
		const Sphere sphere (Vector (x, height, z), 4.f);

		ObjectVec objectList;
		if (CollisionWorld::query (sphere, &objectList) && !objectList.empty ())
			cover += 1.f;
	}

	return 100.f * clamp (0.f, cover, 1.f);
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsTerrainNamespace::getGoodLocation(JNIEnv * /*env*/, jobject /*self*/, jfloat areaSizeX, jfloat areaSizeZ, jobject searchRectLowerLeftLocation, jobject searchRectUpperRightLocation, jboolean dontCheckWater, jboolean dontCheckSlope)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getGoodLocation");

	//validate scripter's input
	if (searchRectLowerLeftLocation == nullptr)
	{
		DEBUG_WARNING (true, ("getGoodLocation (): DB lower left is nullptr"));
		return nullptr;
	}

	if (searchRectUpperRightLocation == nullptr)
	{
		DEBUG_WARNING (true, ("getGoodLocation (): DB upper right is nullptr"));
		return nullptr;
	}

	//pull the data from the java Location objects. sceneid and cell are not checked or used, but sent back in the good location, if one exists
	std::string sceneId;

	Vector srLLLocationVec;
	if (!ScriptConversion::convertWorld(searchRectLowerLeftLocation, srLLLocationVec, sceneId))
	{
		DEBUG_WARNING (true, ("getGoodLocation (): DB could not convert lower left coordinates"));
		return nullptr;
	}

	Vector srURLocationVec;
	if (!ScriptConversion::convertWorld(searchRectUpperRightLocation, srURLocationVec, sceneId))
	{
		DEBUG_WARNING (true, ("getGoodLocation (): DB could not convert upper right coordinates"));
		return nullptr;
	}

	// get the good location from ServerWorld
	Vector successLoc3d = ServerWorld::getGoodLocation(areaSizeX, areaSizeZ, srLLLocationVec, srURLocationVec, dontCheckWater, dontCheckSlope);
	if (successLoc3d.x == 0 && successLoc3d.y == 0 && successLoc3d.z == 0)
	{
		// no good location available
		return nullptr;
	}

	//convert it back into scripter lingo (a "location"), using the default scene and cell ids
	LocalRefPtr goodLocation;
	if (!ScriptConversion::convert(successLoc3d, sceneId, NetworkId(), goodLocation))
	{
		DEBUG_WARNING (true, ("getGoodLocation (): PB could not convert result back to a location"));
		return nullptr;
	}

	return goodLocation->getReturnValue();
}

jobject JNICALL ScriptMethodsTerrainNamespace::getGoodLocationAvoidCollidables(JNIEnv * /*env*/, jobject /*self*/, jfloat areaSizeX, jfloat areaSizeZ, jobject searchRectLowerLeftLocation, jobject searchRectUpperRightLocation, jboolean dontCheckWater, jboolean dontCheckSlope, jfloat staticObjDistance)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getGoodLocationAvoidCollidables");

	//validate scripter's input
	if (searchRectLowerLeftLocation == nullptr)
	{
		DEBUG_WARNING (true, ("getGoodLocationAvoidCollidables (): DB lower left is nullptr"));
		return nullptr;
	}

	if (searchRectUpperRightLocation == nullptr)
	{
		DEBUG_WARNING (true, ("getGoodLocationAvoidCollidables (): DB upper right is nullptr"));
		return nullptr;
	}

	//pull the data from the java Location objects. sceneid and cell are not checked or used, but sent back in the good location, if one exists
	std::string sceneId;

	Vector srLLLocationVec;
	if (!ScriptConversion::convertWorld(searchRectLowerLeftLocation, srLLLocationVec, sceneId))
	{
		DEBUG_WARNING (true, ("getGoodLocationAvoidCollidables (): DB could not convert lower left coordinates"));
		return nullptr;
	}

	Vector srURLocationVec;
	if (!ScriptConversion::convertWorld(searchRectUpperRightLocation, srURLocationVec, sceneId))
	{
		DEBUG_WARNING (true, ("getGoodLocationAvoidCollidables (): DB could not convert upper right coordinates"));
		return nullptr;
	}

	// get the good location from ServerWorld
	Vector successLoc3d = ServerWorld::getGoodLocationAvoidCollidables(areaSizeX, areaSizeZ, srLLLocationVec, srURLocationVec, dontCheckWater, dontCheckSlope, staticObjDistance);
	if (successLoc3d.x == 0 && successLoc3d.y == 0 && successLoc3d.z == 0)
	{
		// no good location available
		return nullptr;
	}

	//convert it back into scripter lingo (a "location"), using the default scene and cell ids
	LocalRefPtr goodLocation;
	if (!ScriptConversion::convert(successLoc3d, sceneId, NetworkId(), goodLocation))
	{
		DEBUG_WARNING (true, ("getGoodLocationAvoidCollidables (): PB could not convert result back to a location"));
		return nullptr;
	}

	return goodLocation->getReturnValue();
}
// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::enterClientStructurePlacementMode (JNIEnv* /*env*/, jobject /*self*/, jlong jobject_player, jlong jobject_deed, jstring jstring_serverObjectTemplateName)
{
	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_WARNING (true, ("enterClientStructurePlacementMode (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client* const client = player->getClient ();
	if (!client)
	{
		DEBUG_WARNING (true, ("enterClientStructurePlacementMode (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- make sure we have a valid object template name
	JavaStringParam javaStringParam_serverObjectTemplateName (jstring_serverObjectTemplateName);
	std::string     serverObjectTemplateName;
	if (!JavaLibrary::convert (javaStringParam_serverObjectTemplateName, serverObjectTemplateName))
	{
		DEBUG_WARNING (true, ("enterClientStructurePlacementMode (): DB failed to convert parameter 3 (serverObjectTemplateName) to string"));
		return JNI_FALSE;
	}

	//-- make sure the name is not empty
	if (serverObjectTemplateName.empty ())
	{
		DEBUG_WARNING (true, ("enterClientStructurePlacementMode (): DB serverObjectTemplateName is empty"));
		return JNI_FALSE;
	}

	//-- fetch the object template
	const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (serverObjectTemplateName);
	if (!objectTemplate)
	{
		DEBUG_WARNING (true, ("enterClientStructurePlacementMode (): DB serverObjectTemplate [%s] does not exist", serverObjectTemplateName.c_str ()));
		return JNI_FALSE;
	}

	//-- fetch the server object template
	const ServerObjectTemplate* const serverObjectTemplate = dynamic_cast<const ServerObjectTemplate*> (objectTemplate);
	if (!serverObjectTemplate)
	{
		objectTemplate->releaseReference ();

		DEBUG_WARNING (true, ("enterClientStructurePlacementMode (): DB objectTemplate [%s] exists, but is not a valid server object template", serverObjectTemplateName.c_str ()));
		return JNI_FALSE;
	}

	//-- fetch the shared object template
	const std::string sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate ();
	serverObjectTemplate->releaseReference ();
	if (sharedObjectTemplateName.empty ())
	{
		DEBUG_WARNING (true, ("enterClientStructurePlacementMode (): DB sharedObjectTemplateName specified by serverObjectTemplate [%s] is empty", serverObjectTemplateName.c_str ()));
		return JNI_FALSE;
	}

	//-- get the id of the deed
	const NetworkId deedNetworkId(jobject_deed);
	if (deedNetworkId == NetworkId::cms_invalid)
	{
		DEBUG_WARNING (true, ("enterClientStructurePlacementMode (): DB jobject_deed is not a valid object"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	client->send (EnterStructurePlacementModeMessage (deedNetworkId, sharedObjectTemplateName), true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsTerrainNamespace::canPlaceStructure (JNIEnv* /*env*/, jobject /*self*/, jstring jstring_serverObjectTemplateName, jobject jobject_location, jint jint_rotation)
{
	//-- has the position to lot x, z
	const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): PB there is no terrain"));
		return -9997.f;
	}

	const LotManager* const lotManager = ServerWorld::getConstLotManager ();
	if (!lotManager)
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): PB there is no lot system"));
		return -9997.f;
	}

	//-- make sure we have a valid object template name
	JavaStringParam javaStringParam_serverObjectTemplateName (jstring_serverObjectTemplateName);
	std::string     serverObjectTemplateName;
	if (!JavaLibrary::convert (javaStringParam_serverObjectTemplateName, serverObjectTemplateName))
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): DB failed to convert parameter 3 (serverObjectTemplateName) to string"));
		return -9998.f;
	}

	//-- make sure the name is not empty
	if (serverObjectTemplateName.empty ())
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): DB serverObjectTemplateName is empty"));
		return -9998.f;
	}

	//-- fetch the object template
	const ObjectTemplate* objectTemplate = ObjectTemplateList::fetch (serverObjectTemplateName);
	if (!objectTemplate)
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): DB serverObjectTemplate [%s] does not exist", serverObjectTemplateName.c_str ()));
		return -9998.f;
	}

	//-- fetch the server object template
	const ServerObjectTemplate* const serverObjectTemplate = dynamic_cast<const ServerObjectTemplate*> (objectTemplate);
	if (!serverObjectTemplate)
	{
		objectTemplate->releaseReference ();

		DEBUG_WARNING (true, ("canPlaceStructure (): DB objectTemplate [%s] exists, but is not a valid server object template", serverObjectTemplateName.c_str ()));
		return -9998.f;
	}

	//-- fetch the shared object template name
	const std::string sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate ();
	serverObjectTemplate->releaseReference ();
	if (sharedObjectTemplateName.empty ())
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): DB sharedObjectTemplateName specified by serverObjectTemplate [%s] is empty", serverObjectTemplateName.c_str ()));
		return -9998.f;
	}

	//-- fetch the object template
	objectTemplate = ObjectTemplateList::fetch (sharedObjectTemplateName);
	if (!objectTemplate)
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): DB sharedObjectTemplate [%s] does not exist", sharedObjectTemplateName.c_str ()));
		return -9998.f;
	}

	//-- fetch the shared object template
	const SharedTangibleObjectTemplate* const tangibleObjectTemplate = dynamic_cast<const SharedTangibleObjectTemplate*> (objectTemplate);
	if (!tangibleObjectTemplate)
	{
		objectTemplate->releaseReference ();

		DEBUG_WARNING (true, ("canPlaceStructure (): DB objectTemplate [%s] exists, but is not a valid shared tangible object template", sharedObjectTemplateName.c_str ()));
		return -9998.f;
	}

	//-- get the structure footprint
	const StructureFootprint* const structureFootprint = tangibleObjectTemplate->getStructureFootprint ();
	if (!structureFootprint)
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): DB objectTemplate [%s] does not have a structure footprint", sharedObjectTemplateName.c_str ()));
		return -9998.f;
	}

	std::string sceneId;
	Vector position_w;
	if (!ScriptConversion::convertWorld (jobject_location, position_w, sceneId))
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): DB failed to convert parameter 2 (location) to sceneId and position_w"));
		return -9998.f;
	}

	if (jint_rotation < 0 || jint_rotation > 3)
	{
		DEBUG_WARNING (true, ("canPlaceStructure (): DB failed to convert parameter 3 (rotation) [%i] to valid rotation (0, 1, 2, or 3)", jint_rotation));
		return -9998.f;
	}

	const int x = terrainObject->calculateChunkX (position_w);
	const int z = terrainObject->calculateChunkZ (position_w);
	const RotationType rotationType = static_cast<RotationType> (jint_rotation);

	float height_w = 0.f;
	if (lotManager->canPlace (structureFootprint, x, z, rotationType, height_w))
		return height_w;

	return -9999.f;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsTerrainNamespace::createTemporaryStructure (JNIEnv* /*env*/, jobject /*self*/, jstring jstring_serverObjectTemplateName, jobject jobject_position, jint jint_rotation)
{
	UNREF (jstring_serverObjectTemplateName);
	UNREF (jobject_position);
	UNREF (jint_rotation);

	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsTerrainNamespace::getNumberOfLots (JNIEnv* /*env*/, jobject /*self*/, jstring jstring_serverObjectTemplateName)
{
	//-- make sure we have a valid object template name
	JavaStringParam javaStringParam_serverObjectTemplateName (jstring_serverObjectTemplateName);
	std::string     serverObjectTemplateName;
	if (!JavaLibrary::convert (javaStringParam_serverObjectTemplateName, serverObjectTemplateName))
	{
		DEBUG_WARNING (true, ("getNumberOfLots (): DB failed to convert parameter 3 (serverObjectTemplateName) to string"));
		return 0;
	}

	//-- make sure the name is not empty
	if (serverObjectTemplateName.empty ())
	{
		DEBUG_WARNING (true, ("getNumberOfLots (): DB serverObjectTemplateName is empty"));
		return 0;
	}

	//-- fetch the object template
	const ObjectTemplate* objectTemplate = ObjectTemplateList::fetch (serverObjectTemplateName);
	if (!objectTemplate)
	{
		DEBUG_WARNING (true, ("getNumberOfLots (): DB serverObjectTemplate [%s] does not exist", serverObjectTemplateName.c_str ()));
		return 0;
	}

	//-- fetch the server object template
	const ServerObjectTemplate* const serverObjectTemplate = dynamic_cast<const ServerObjectTemplate*> (objectTemplate);
	if (!serverObjectTemplate)
	{
		objectTemplate->releaseReference ();

		DEBUG_WARNING (true, ("getNumberOfLots (): DB objectTemplate [%s] exists, but is not a valid server object template", serverObjectTemplateName.c_str ()));
		return 0;
	}

	//-- fetch the shared object template name
	const std::string sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate ();
	serverObjectTemplate->releaseReference ();
	if (sharedObjectTemplateName.empty ())
	{
		DEBUG_WARNING (true, ("getNumberOfLots (): DB sharedObjectTemplateName specified by serverObjectTemplate [%s] is empty", serverObjectTemplateName.c_str ()));
		return 0;
	}

	//-- fetch the object template
	objectTemplate = ObjectTemplateList::fetch (sharedObjectTemplateName);
	if (!objectTemplate)
	{
		DEBUG_WARNING (true, ("getNumberOfLots (): DB sharedObjectTemplate [%s] does not exist", sharedObjectTemplateName.c_str ()));
		return 0;
	}

	//-- fetch the shared object template
	const SharedTangibleObjectTemplate* const tangibleObjectTemplate = dynamic_cast<const SharedTangibleObjectTemplate*> (objectTemplate);
	if (!tangibleObjectTemplate)
	{
		objectTemplate->releaseReference ();

		DEBUG_WARNING (true, ("getNumberOfLots (): DB objectTemplate [%s] exists, but is not a valid shared tangible object template", sharedObjectTemplateName.c_str ()));
		return 0;
	}

	//-- get the structure footprint
	const StructureFootprint* const structureFootprint = tangibleObjectTemplate->getStructureFootprint ();
	if (!structureFootprint)
	{
		DEBUG_WARNING (true, ("getNumberOfLots (): DB objectTemplate [%s] does not have a structure footprint", sharedObjectTemplateName.c_str ()));
		return 0;
	}

	//-- return the number of lots
	return structureFootprint->getNumberOfStructureReservations ();
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsTerrainNamespace::getLocalTime (JNIEnv* /*env*/, jobject /*self*/)
{
	const TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("getLocalTime (): PB no terrain"));
		return 0.f;
	}

	const float environmentCycleTime = terrainObject->getEnvironmentCycleTime ();
	if (environmentCycleTime == 0.f)
	{
		DEBUG_WARNING (true, ("getLocalTime (): DB terrain exists, but environmentCycleTime is 0"));
		return 0.f;
	}

	return fmodf (static_cast<float> (ServerClock::getInstance ().getGameTimeSeconds ()), environmentCycleTime) / environmentCycleTime;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsTerrainNamespace::getLocalDayLength (JNIEnv* /*env*/, jobject /*self*/)
{
	const TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("getLocalDayLength (): PB no terrain"));
		return 0.f;
	}

	return 0.5f;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::setWeatherData (JNIEnv* env, jobject self, jint index,
	jfloat windVelocityX, jfloat windVelocityZ)
{
	UNREF(env);
	UNREF(self);

	PlanetObject * planet = ServerUniverse::getInstance().getCurrentPlanet();
	if (planet != nullptr)
	{
		planet->setWeather(index, windVelocityX, 0.f, windVelocityZ);
		return JNI_TRUE;
	}
	else
	{
		WARNING(true, ("JavaLibrary::setWeatherData could not get current planet"));
		return JNI_FALSE;
	}
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::isBelowWater (JNIEnv* /*env*/, jobject /*self*/, jobject location)
{
	if (location == nullptr)
	{
		DEBUG_WARNING(true, ("[designer bug] isBelowWater script hook was passed a nullptr location reference"));
		return JNI_FALSE;
	}

	Vector loc;
	std::string sceneId;
	NetworkId cell;
	if(!ScriptConversion::convert(location, loc, sceneId, cell))
	{
		DEBUG_WARNING(true, ("isBelowWater could not convert given location to internal Vector/scene/cell format, was a location object passed in?"));
		return JNI_FALSE;
	}

	const CellProperty* worldCell = CellProperty::getWorldCellProperty();
	if(!worldCell)
	{
		DEBUG_WARNING(true, ("isBelowWater got a nullptr worldCell back from CellProperty::getWorldCellProperty()"));
		return JNI_FALSE;
	}
	const NetworkId& worldCellId = worldCell->getOwner().getNetworkId();
	if(worldCellId != cell)
	{
		DEBUG_WARNING(true, ("isBelowWater was given a location not in the world cell"));
		return JNI_FALSE;
	}

	const TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING(true, ("isBelowWater can't get the TerrainObject"));
		return JNI_FALSE;
	}

	return terrainObject->isBelowWater(loc);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::isWaterInRect (JNIEnv* /*env*/, jobject /*self*/, jfloat x0, jfloat y0, jfloat x1, jfloat y1)
{
	const TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING(true, ("isWaterInRect can't get the TerrainObject"));
		return JNI_FALSE;
	}

	Rectangle2d rect(x0,y0,x1,y1);
	return terrainObject->getWater(rect);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::createClientPath (JNIEnv * /*env*/, jobject /*self*/, jlong jobject_player, jobject jobject_start, jobject jobject_end)
{
	//-- get the player object
	CreatureObject * player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_WARNING (true, ("createClientPath (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client * const client = player->getClient ();
	if (!client)
	{
		DEBUG_WARNING (true, ("createClientPath (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- get the start position
	std::string sceneId;
	Vector startPosition_w;
	if (!ScriptConversion::convertWorld (jobject_start, startPosition_w, sceneId))
	{
		DEBUG_WARNING (true, ("createClientPath (): DB failed to convert parameter 2 (start) to sceneId and position_w"));
		return JNI_FALSE;
	}

	//-- get the end position
	Vector endPosition_w;
	if (!ScriptConversion::convertWorld (jobject_end, endPosition_w, sceneId))
	{
		DEBUG_WARNING (true, ("createClientPath (): DB failed to convert parameter 2 (end) to sceneId and position_w"));
		return JNI_FALSE;
	}

	//-- send path to client
	std::vector<Vector> pointList;

	ServerPathBuilder builder;

	AiLocation const startAiLocation(CellProperty::getWorldCellProperty(), startPosition_w);
	AiLocation const endAiLocation(CellProperty::getWorldCellProperty(), endPosition_w);

	builder.buildWorldPath(startAiLocation, endAiLocation);

	if (builder.buildFailed())
	{
		DEBUG_WARNING(true, ("createClientPath (): Path search failed to find a path to the goal\n"));
		return JNI_FALSE;
	}

	AiPath * const path = builder.getPath();

	for (AiPath::iterator it = path->begin(); it != path->end(); ++it)
	{
		AiLocation const &loc = (*it);
		Vector position_w(loc.getPosition_w());
		position_w.y = 0.0f; // let the client resolve the height.
		pointList.push_back(position_w);
	}

	//-- send the message to the client
	CreateClientPathMessage const createClientPathMessage(pointList);
	client->send(createClientPathMessage, true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::destroyClientPath (JNIEnv * /*env*/, jobject /*self*/, jlong jobject_player)
{
	//-- get the player object
	ServerObject * player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_WARNING (true, ("destroyClientPath (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client * const client = player->getClient ();
	if (!client)
	{
		DEBUG_WARNING (true, ("destroyClientPath (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	client->send(DestroyClientPathMessage(), true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsTerrainNamespace::getWaterTableHeight (JNIEnv* /*env*/, jobject /*self*/, jobject location)
{
	float zero = 0.0f;

	if (location == nullptr)
	{
		DEBUG_WARNING(true, ("[designer bug] getWaterTableHeight script hook was passed a nullptr location reference"));
		return zero;
	}

	Vector loc;
	std::string sceneId;
	NetworkId cell;
	if(!ScriptConversion::convert(location, loc, sceneId, cell))
	{
		DEBUG_WARNING(true, ("getWaterTableHeight could not convert given location to internal Vector/scene/cell format, was a location object passed in?"));
		return zero;
	}

	const CellProperty* worldCell = CellProperty::getWorldCellProperty();
	if(!worldCell)
	{
		DEBUG_WARNING(true, ("getWaterTableHeight got a nullptr worldCell back from CellProperty::getWorldCellProperty()"));
		return zero;
	}
	const NetworkId& worldCellId = worldCell->getOwner().getNetworkId();
	if(worldCellId != cell)
	{
		DEBUG_WARNING(true, ("getWaterTableHeight was given a location not in the world cell"));
		return zero;
	}

	const TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING(true, ("getWaterTableHeight can't get the TerrainObject"));
		return zero;
	}

	float height = 0.0f;
	bool success =	terrainObject->getWaterHeight(loc, height);
	if(!success)
	{
		DEBUG_WARNING(true, ("getWaterTableHeight failed TerrainObject call"));
		return zero;
	}

	return height;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::requestLocation (JNIEnv * /*env*/, jobject /*self*/, jlong jobject_scriptObject, jstring jstring_locationId, jobject jobject_searchLocation, jfloat searchRadius, jfloat locationReservationRadius, jboolean checkWater, jboolean checkSlope)
{
	//-- make sure we have a valid trigger object
	ServerObject * scriptObject = 0;
	if (!JavaLibrary::getObject (jobject_scriptObject, scriptObject))
	{
		DEBUG_WARNING (true, ("requestLocation (): DB could not find scriptObject"));
		return JNI_FALSE;
	}

	//-- extract the locationId
	JavaStringParam javaStringParam_locationId (jstring_locationId);
	std::string locationId;
	if (!JavaLibrary::convert (javaStringParam_locationId, locationId))
	{
		DEBUG_WARNING (true, ("requestLocation (): DB failed to convert parameter 2 (locationId) to string"));
		return JNI_FALSE;
	}

	//-- extract the scene and the position
	std::string sceneId;
	Vector position_w;
	if (!ScriptConversion::convertWorld (jobject_searchLocation, position_w, sceneId))
	{
		DEBUG_WARNING (true, ("requestLocation (): DB failed to convert parameter 3 (searchLocation) to scene and position"));
		return JNI_FALSE;
	}

	//-- make sure the location is on the same planet
	if (sceneId != ServerWorld::getSceneId ())
	{
		DEBUG_WARNING (true, ("requestLocation (): DB parameter 3 (searchLocation) specified a different planet (%s != %s)", sceneId.c_str(), ServerWorld::getSceneId ().c_str()));
		return JNI_FALSE;
	}

	LocationRequest const locationRequest (
		GameServer::getInstance ().getProcessId (),
		scriptObject->getNetworkId (),
		locationId,
		position_w.x,
		position_w.z,
		searchRadius,
		locationReservationRadius,
		checkWater,
		checkSlope);

	GameServer::getInstance ().sendToPlanetServer (locationRequest);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::isOnAFloor (JNIEnv * env, jobject self, jlong scriptObject)
{
	//-- make sure we have a valid object
	ServerObject * theObject = nullptr;
	if (!JavaLibrary::getObject (scriptObject, theObject))
	{
		DEBUG_WARNING (true, ("isOnAFloor (): could not find scriptObject"));
		return JNI_FALSE;
	}

	CollisionProperty * const objectCollisionProp = theObject->getCollisionProperty();
	if(!objectCollisionProp)
	{
		DEBUG_WARNING (true, ("isOnAFloor (): could not get the collision property for scriptObject"));
		return JNI_FALSE;
	}

	if(objectCollisionProp->getStandingOn() != nullptr)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::isRelativePointOnSameFloorAsObject(JNIEnv * env, jobject self, jlong scriptObject, jfloat relativeXLocation, jfloat relativeZLocation)
{
	if(!isOnAFloor(env, self, scriptObject))
		return JNI_FALSE;

	//-- make sure we have a valid object
	ServerObject * theObject = nullptr;
	if (!JavaLibrary::getObject (scriptObject, theObject))
	{
		DEBUG_WARNING (true, ("isRelativePointOnSameFloorAsObject (): could not find scriptObject"));
		return JNI_FALSE;
	}

	Floor const * floorObjectStandingOn = CollisionWorld::getFloorStandingOn (*theObject);

	Vector testPosition_w = theObject->getPosition_w();
	testPosition_w.x += relativeXLocation;
	testPosition_w.z += relativeZLocation;

	Vector capsuleTopLocation = testPosition_w;
	testPosition_w.y += 5000.0f;
	Vector capsuleBottomLocation = testPosition_w;
	testPosition_w.y -= 5000.0f;

	SpatialDatabase const * database = CollisionWorld::getDatabase();
	if(!database)
	{
		DEBUG_WARNING (true, ("isRelativePointOnSameFloorAsObject (): could not get the database"));
		return JNI_FALSE;
	}

	FloorVec results;
	Capsule cap(capsuleBottomLocation, capsuleTopLocation, 1.0f);
	database->queryFloors(cap, &results);
	for(uint i = 0; i < results.size(); ++i)
	{
		Floor const * const queriedFloor = results.at(i);
		if(queriedFloor == floorObjectStandingOn)
			return JNI_TRUE;
	}

	return JNI_FALSE;
}

// ----------------------------------------------------------------------

jfloat   JNICALL ScriptMethodsTerrainNamespace::getFloorHeightAtRelativePointOnSameFloorAsObject(JNIEnv * env, jobject self, jlong scriptObject, jfloat relativeXLocation, jfloat relativeZLocation)
{
	float result = 0.0f;

	if(!isOnAFloor(env, self, scriptObject))
		return JNI_FALSE;

	//-- make sure we have a valid object
	ServerObject * theObject = nullptr;
	if (!JavaLibrary::getObject (scriptObject, theObject))
	{
		DEBUG_WARNING (true, ("getFloorHeightAtRelativePointOnSameFloorAsObject (): could not find scriptObject"));
		return result;
	}

	Floor const * floorObjectStandingOn = CollisionWorld::getFloorStandingOn (*theObject);

	Vector testPosition_p = theObject->getPosition_p();
	testPosition_p.x += relativeXLocation;
	testPosition_p.z += relativeZLocation;

	Vector capsuleTopLocation = testPosition_p;
	testPosition_p.y += 5000.0f;
	Vector capsuleBottomLocation = testPosition_p;
	testPosition_p.y -= 5000.0f;

	SpatialDatabase const * database = CollisionWorld::getDatabase();
	if(!database)
	{
		DEBUG_WARNING (true, ("getFloorHeightAtRelativePointOnSameFloorAsObject (): could not get the database"));
		return result;
	}

	FloorVec results;
	Capsule cap(capsuleBottomLocation, capsuleTopLocation, 1.0f);
	database->queryFloors(cap, &results);
	for(uint i = 0; i < results.size(); ++i)
	{
		Floor const * const queriedFloor = results.at(i);
		if(queriedFloor == floorObjectStandingOn)
		{
			FloorLocator tempLoc;
			if(queriedFloor->dropTest(testPosition_p,tempLoc))
			{
				result = tempLoc.getPosition_w().y;
				break;
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::createClientPathAdvanced(JNIEnv * /*env*/, jobject /*self*/, jlong jPlayer, jobject jStartLocation, jobject jEndLocation, jstring /*jAppearanceTableEntry*/)
{
	//-- get the player object
	CreatureObject * player = 0;
	if (!JavaLibrary::getObject(jPlayer, player))
	{
		DEBUG_WARNING(true,("createClientPathAdvanced(): Could not find player object"));
		return JNI_FALSE;
	}
	
	//-- get the client associated with the player
	Client * const client = player->getClient();
	if (!client)
	{
		DEBUG_WARNING(true,("createClientPathAdvanced(): Could get the player's client object"));
		return JNI_FALSE;
	}
	
	//-- get the start location
	Location startLocation;
	if (!ScriptConversion::convert(LocalRefParam(jStartLocation), startLocation))
	{
		DEBUG_WARNING(true,("createClientPathAdvanced(): Failed to convert jStartLocation to startLocation"));
		return JNI_FALSE;
	}
	
	//-- get the end location
	Location endLocation;
	if (!ScriptConversion::convert(LocalRefParam(jEndLocation), endLocation))
	{
		DEBUG_WARNING(true,("createClientPathAdvanced(): Failed to convert jEndLocation to endLocation"));
		return JNI_FALSE;
	}
	
	//-- send path to client
	std::vector<Vector> pointList;
	
	ServerPathBuilder builder;

	AiLocation const startAiLocation(startLocation.getCell(), startLocation.getCoordinates());
	AiLocation const endAiLocation(endLocation.getCell(), endLocation.getCoordinates());
	
	builder.buildPathImmediate(startAiLocation, endAiLocation);
	
	if (builder.buildFailed())
	{
		DEBUG_WARNING(true,("createClientPathAdvanced(): Path search failed to find a path to the goal\n"));
		return JNI_FALSE;
	}

	// add the start position.
	{
		Vector startPosition_w(startAiLocation.getPosition_w());
		if (!startAiLocation.getCell() || startAiLocation.getCell()->isWorldCell()) 
		{
			startPosition_w.y = 0.0f;
		}
		pointList.push_back(startPosition_w);
	}

	AiPath * const path = builder.getPath();
	if (path) 
	{
		// erase the first and last nodes because they are wrong.
		if (!path->empty()) 
		{
			path->pop_front();
		}

		if (!path->empty()) 
		{
			path->pop_back();
		}

		// iterate over the remaining nodes.
		for(AiPath::iterator it = path->begin(); it != path->end(); ++it)
		{
			AiLocation const &loc =(*it);		

			Vector position_w(loc.getPosition_w());

			CellProperty const * const cell = loc.getCell();
			if (!cell || cell->isWorldCell()) 
			{
				position_w.y = 0.0f; // let the client resolve the height.
			}

			pointList.push_back(position_w);
		}
	}

	// add the end position.
	{
		Vector endPosition_w(endAiLocation.getPosition_w());
		if (!endAiLocation.getCell() || endAiLocation.getCell()->isWorldCell()) 
		{
			endPosition_w.y = 0.0f;
		}
		pointList.push_back(endAiLocation.getPosition_w());
	}
		
	//-- send the message to the client
	CreateClientPathMessage const createClientPathMessage(pointList);
	client->send(createClientPathMessage, true);

	return JNI_TRUE;
}

//-----------------------------------------------------------------------

/**
 * Sets a creature's cover value based on it's current chunk.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to set
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsTerrainNamespace::setCreatureCoverDefault(JNIEnv *env, jobject self, jlong target)
{
	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	jint cover = 0;
	if (creature->getParentCell() == CellProperty::getWorldCellProperty())
	{
		const Vector pos = creature->getPosition_w();
		cover = static_cast<jint>(getCoverAtLocation(env, self, pos.x, pos.z) * 100);
	}
	creature->setCover(cover);

	return JNI_TRUE;
}	// JavaLibrary::setCreatureCoverDefault

//-----------------------------------------------------------------------

/**
 * Sets a creature's cover value.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to set
 * @param cover			the cover value
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsTerrainNamespace::setCreatureCover(JNIEnv *env, jobject self, jlong target, jint cover)
{
	UNREF(self);

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->setCover(cover);
	return JNI_TRUE;
}	// JavaLibrary::setCreatureCover

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsTerrainNamespace::setCreatureCoverVisibility(JNIEnv * env, jobject self, jlong target, jboolean isVisible)
{
	CreatureObject * creature = nullptr;
	if(!JavaLibrary::getObject(target, creature))
		return;

	creature->setCoverVisibility(isVisible);
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsTerrainNamespace::getCreatureCoverVisibility(JNIEnv * env, jobject self, jlong target)
{
	CreatureObject * creature = nullptr;
	if(!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if(creature->getCoverVisibility())
		return JNI_TRUE;

	return JNI_FALSE;
}

//-----------------------------------------------------------------------

/**
* Returns the id of the object that target is standing on
*
* @param env         Java environment
* @param self        class calling this function
* @param target      object we want to check
*
* @return id of the object that target is standing on or invalid if target is not standing on an object (e.g. standing on terrain)
*/
jlong JNICALL ScriptMethodsTerrainNamespace::getStandingOn(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject const * creature  = 0;

	if (JavaLibrary::getObject(target, creature))
	{
		CollisionProperty const * const collision = creature->getCollisionProperty();

		if (collision)
		{
			Object const * const object = collision->getStandingOn();

			if (object)
				return object->getNetworkId().getValue();
		}
	}

	return NetworkId::cms_invalid.getValue();
}

// ======================================================================
