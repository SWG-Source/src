//========================================================================
//
// ScriptMethodsWorldInfo.cpp - implements script methods for querying
// world objects
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "UnicodeUtils.h"
#include "serverGame/ContainerInterface.h"
#include "sharedDebug/Profiler.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ServerBuildoutManager.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"

#include <list>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsWorldInfoNamespace
// ======================================================================

namespace ScriptMethodsWorldInfoNamespace
{
	bool install();

	jlongArray   JNICALL getObjectsInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range);
	jlongArray   JNICALL getObjectsInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range);
	jlongArray   JNICALL getCreaturesInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range);
	jlongArray   JNICALL getCreaturesInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range);
	jlongArray   JNICALL getCreaturesOfNicheInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range, jint niche, jint mask);
	jlongArray   JNICALL getCreaturesOfNicheInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range, jint niche, jint mask);
	jlongArray   JNICALL getCreaturesOfSpeciesInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range, jint species);
	jlongArray   JNICALL getCreaturesOfSpeciesInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range, jint species);
	jlongArray   JNICALL getCreaturesOfRaceInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range, jint species, jint race);
	jlongArray   JNICALL getCreaturesOfRaceInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range, jint species, jint race);
	jlongArray   JNICALL getNonCreaturesInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range);
	jlongArray   JNICALL getNonCreaturesInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range);
	jlongArray   JNICALL getNPCsInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range);
	jlongArray   JNICALL getNPCsInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range);
	jlongArray   JNICALL getPlayerCreaturesInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range);
	jlongArray   JNICALL getPlayerCreaturesInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range);
	jlongArray   JNICALL getObjectsInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle);
	jlongArray   JNICALL getObjectsInConeLocation(JNIEnv *env, jobject self, jlong coneCenterObjectId, jobject coneDirectionLocation, jfloat range, jfloat angle);
	jlongArray   JNICALL getCreaturesInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle);
	jlongArray   JNICALL getCreaturesInConeLocation(JNIEnv *env, jobject self, jlong coneCenterObjectId, jobject coneDirectionLocation, jfloat range, jfloat angle);
	jlongArray   JNICALL getCreaturesOfNicheInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle, jint niche, jint mask);
	jlongArray   JNICALL getCreaturesOfSpeciesInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle, jint species);
	jlongArray   JNICALL getCreaturesOfRaceInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle, jint species, jint race);
	jlongArray   JNICALL getNonCreaturesInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle);
	jlongArray   JNICALL getNPCsInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle);
	jlongArray   JNICALL getPlayerCreaturesInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle);
	jlongArray   JNICALL getPerformanceListenersInRange(JNIEnv *env, jobject self, jlong performer, jfloat range);
	jlongArray   JNICALL getPerformanceWatchersInRange(JNIEnv *env, jobject self, jlong performer, jfloat range);
	jboolean     JNICALL isSpaceBattlefieldZone(JNIEnv *env, jobject self);
	jboolean     JNICALL isSpaceScene(JNIEnv *env, jobject self);
	jboolean     JNICALL isValidLocation(JNIEnv *env, jobject self, jobject objectPosition, jfloat objectRadius );
	jobject      JNICALL getValidLocation(JNIEnv *env, jobject self, jobject corralCenter, jfloat corralRadius, jobject desiredPosition, jfloat objectRadius );
	jboolean     JNICALL canSee(JNIEnv *env, jobject self, jlong source, jlong target);
	jboolean     JNICALL canSeeLocation(JNIEnv *env, jobject self, jlong source, jobject target);
	jboolean     JNICALL canManipulate(JNIEnv *env, jobject self, jlong player, jlong target, jboolean move, jboolean doPermission, float distance, jboolean sendMessage, jboolean skipNoTradeCheck);
	jfloat       JNICALL getElevation(JNIEnv *env, jobject self, jobject loc);
	jfloat       JNICALL getDirection(JNIEnv *env, jobject self, jobject loc1, jobject loc2);
	jobject      JNICALL findSpotNear(JNIEnv *env, jobject self, jlong target, jobject loc, jfloat radius);
	jlongArray   JNICALL getAllNpcs(JNIEnv *env, jobject self, jobject loc, jfloat radius);
	jlongArray   JNICALL getAllPlayers(JNIEnv *env, jobject self, jobject loc, jfloat radius);
	jlong        JNICALL getClosestMobile(JNIEnv *env, jobject self, jobject loc);
	jlong        JNICALL getClosestPlayer(JNIEnv *env, jobject self, jobject loc);
	jlong        JNICALL getFirstObjectWithScript(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring script);
	jlong        JNICALL getFirstObjectWithObjVar(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring objvar);
	jlong        JNICALL getFirstObjectWithTemplate(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring templateName);
	jlongArray   JNICALL getAllObjectsWithScript(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring script);
	jlongArray   JNICALL getAllObjectsWithObjVar(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring objvar);
	jlongArray   JNICALL getAllObjectsWithTemplate(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring templateName);
	jobject      JNICALL getLocationBetweenLocs(JNIEnv *env, jobject self, jobject target1, jobject target2);
	jlong        JNICALL getPlanetByName(JNIEnv * env, jobject self, jstring name);
	jstring      JNICALL getNameForPlanetObject(JNIEnv * env, jobject self, jlong planet);
	jboolean     JNICALL getCollidesWithObject(JNIEnv * env, jobject self, jobject loc, jfloat radius);
	jboolean     JNICALL startUniverseWideEvent(JNIEnv * env, jobject self, jstring eventName);
	jboolean     JNICALL stopUniverseWideEvent(JNIEnv * env, jobject self, jstring eventName);
	jstring      JNICALL getCurrentUniverseWideEvents(JNIEnv * env, jobject self);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsWorldInfoNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsWorldInfoNamespace::c)}
	JF("_getObjectsInRange", "(JF)[J", getObjectsInRangeObject),
	JF("_getObjectsInRange", "(Lscript/location;F)[J", getObjectsInRangeLocation),
	JF("_getCreaturesInRange", "(JF)[J", getCreaturesInRangeObject),
	JF("_getCreaturesInRange", "(Lscript/location;F)[J", getCreaturesInRangeLocation),
	JF("_getCreaturesOfNicheInRange", "(JFII)[J", getCreaturesOfNicheInRangeObject),
	JF("_getCreaturesOfNicheInRange", "(Lscript/location;FII)[J", getCreaturesOfNicheInRangeLocation),
	JF("_getCreaturesOfSpeciesInRange", "(JFI)[J", getCreaturesOfSpeciesInRangeObject),
	JF("_getCreaturesOfSpeciesInRange", "(Lscript/location;FI)[J", getCreaturesOfSpeciesInRangeLocation),
	JF("_getCreaturesOfRaceInRange", "(JFII)[J", getCreaturesOfRaceInRangeObject),
	JF("_getCreaturesOfRaceInRange", "(Lscript/location;FII)[J", getCreaturesOfRaceInRangeLocation),
	JF("_getNonCreaturesInRange", "(JF)[J", getNonCreaturesInRangeObject),
	JF("_getNonCreaturesInRange", "(Lscript/location;F)[J", getNonCreaturesInRangeLocation),
	JF("_getNPCsInRange", "(JF)[J", getNPCsInRangeObject),
	JF("_getNPCsInRange", "(Lscript/location;F)[J", getNPCsInRangeLocation),
	JF("_getPlayerCreaturesInRange", "(JF)[J", getPlayerCreaturesInRangeObject),
	JF("_getPlayerCreaturesInRange", "(Lscript/location;F)[J", getPlayerCreaturesInRangeLocation),
	JF("_getPerformanceListenersInRange", "(JF)[J", getPerformanceListenersInRange),
	JF("_getPerformanceWatchersInRange", "(JF)[J", getPerformanceWatchersInRange),
	JF("isSpaceBattlefieldZone", "()Z", isSpaceBattlefieldZone),
	JF("isSpaceScene", "()Z", isSpaceScene),
	JF("_getObjectsInCone", "(JJFF)[J", getObjectsInCone),
	JF("_getObjectsInCone", "(JLscript/location;FF)[J", getObjectsInConeLocation),
	JF("_getCreaturesInCone", "(JJFF)[J", getCreaturesInCone),
	JF("_getCreaturesInCone", "(JLscript/location;FF)[J", getCreaturesInConeLocation),
	JF("_getCreaturesOfNicheInCone", "(JJFFII)[J", getCreaturesOfNicheInCone),
	JF("_getCreaturesOfSpeciesInCone", "(JJFFI)[J", getCreaturesOfSpeciesInCone),
	JF("_getCreaturesOfRaceInCone", "(JJFFII)[J", getCreaturesOfRaceInCone),
	JF("_getNonCreaturesInCone", "(JJFF)[J", getNonCreaturesInCone),
	JF("_getNPCsInCone", "(JJFF)[J", getNPCsInCone),
	JF("_getPlayerCreaturesInCone", "(JJFF)[J", getPlayerCreaturesInCone),
	JF("isValidLocation","(Lscript/location;F)Z",isValidLocation),
	JF("getValidLocation","(Lscript/location;FLscript/location;F)Lscript/location;",getValidLocation),
	JF("_canSee", "(JJ)Z", canSee),
	JF("_canSee", "(JLscript/location;)Z", canSeeLocation),
	JF("_canManipulate", "(JJZZFZZ)Z", canManipulate),
	JF("getElevation", "(Lscript/location;)F",getElevation),
	JF("getDirection", "(Lscript/location;Lscript/location;)F",getDirection),
	JF("_findSpotNear", "(JLscript/location;F)Lscript/location;",findSpotNear),
	JF("_getAllNpcs", "(Lscript/location;F)[J",getAllNpcs),
	JF("_getAllPlayers", "(Lscript/location;F)[J",getAllPlayers),
	JF("_getClosestMobile", "(Lscript/location;)J",getClosestMobile),
	JF("_getClosestPlayer", "(Lscript/location;)J",getClosestPlayer),
	JF("_getFirstObjectWithScript", "(Lscript/location;FLjava/lang/String;)J",getFirstObjectWithScript),
	JF("_getFirstObjectWithObjVar", "(Lscript/location;FLjava/lang/String;)J",getFirstObjectWithObjVar),
	JF("_getFirstObjectWithTemplate", "(Lscript/location;FLjava/lang/String;)J",getFirstObjectWithTemplate),
	JF("_getAllObjectsWithScript", "(Lscript/location;FLjava/lang/String;)[J",getAllObjectsWithScript),
	JF("_getAllObjectsWithObjVar", "(Lscript/location;FLjava/lang/String;)[J",getAllObjectsWithObjVar),
	JF("_getAllObjectsWithTemplate", "(Lscript/location;FLjava/lang/String;)[J",getAllObjectsWithTemplate),
	JF("getLocationBetweenLocs", "(Lscript/location;Lscript/location;)Lscript/location;",getLocationBetweenLocs),
	JF("_getPlanetByName",             "(Ljava/lang/String;)J",  getPlanetByName),
	JF("_getNameForPlanetObject",      "(J)Ljava/lang/String;",  getNameForPlanetObject),
	JF("_getCollidesWithObject", "(Lscript/location;F)Z", getCollidesWithObject),
	JF("_startUniverseWideEvent", "(Ljava/lang/String;)Z", startUniverseWideEvent),
	JF("_stopUniverseWideEvent", "(Ljava/lang/String;)Z", stopUniverseWideEvent),
	JF("_getCurrentUniverseWideEvents", "()Ljava/lang/String;",  getCurrentUniverseWideEvents),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI world info callback methods
//========================================================================

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		location that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getObjectsInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getObjectsInRange");
	UNREF(self);

	if (location == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;
	ServerWorld::findObjectsInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		object that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getObjectsInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getObjectsInRange");
	UNREF(self);

	if (object == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(object, target))
		return 0;
	ServerWorld::findObjectsInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		location that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range)
{
	UNREF(self);

	if (location == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;
	ServerWorld::findCreaturesInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		object that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range)
{
	UNREF(self);

	if (object == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(object, target))
		return 0;
	ServerWorld::findCreaturesInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		location that is the center of the search area
 * @param range			radius of search area
 * @param type
 * @param mask
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfNicheInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range, jint type, jint mask)
{
	UNREF(self);

	if (location == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;
	ServerWorld::findCreaturesOfNicheInRange(target, range, type, mask, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		object that is the center of the search area
 * @param range			radius of search area
 * @param type
 * @param mask
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfNicheInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range, jint type, jint mask)
{
	UNREF(self);

	if (object == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(object, target))
		return 0;
	ServerWorld::findCreaturesOfNicheInRange(target, range, type, mask, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		location that is the center of the search area
 * @param range			radius of search area
 * @param species
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfSpeciesInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range, jint species)
{
	UNREF(self);

	if (location == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;
	ServerWorld::findCreaturesOfSpeciesInRange(target, range, species, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		object that is the center of the search area
 * @param range			radius of search area
 * @param species
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfSpeciesInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range, jint species)
{
	UNREF(self);

	if (object == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(object, target))
		return 0;
	ServerWorld::findCreaturesOfSpeciesInRange(target, range, species, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		location that is the center of the search area
 * @param range			radius of search area
 * @param species
 * @param race
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfRaceInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range, jint species, jint race)
{
	UNREF(self);

	if (location == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;
	ServerWorld::findCreaturesOfRaceInRange(target, range, species, race, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		object that is the center of the search area
 * @param range			radius of search area
 * @param species
 * @param race
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfRaceInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range, jint species, jint race)
{
	UNREF(self);

	if (object == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(object, target))
		return 0;
	ServerWorld::findCreaturesOfRaceInRange(target, range, species, race, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		location that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getNonCreaturesInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range)
{
	UNREF(self);

	if (location == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;
	ServerWorld::findNonCreaturesInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		object that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getNonCreaturesInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range)
{
	UNREF(self);

	if (object == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(object, target))
		return 0;
	ServerWorld::findNonCreaturesInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		location that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getNPCsInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range)
{
	UNREF(self);

	if (location == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;
	ServerWorld::findNPCsInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		object that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getNPCsInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range)
{
	UNREF(self);

	if (object == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(object, target))
		return 0;
	ServerWorld::findNPCsInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		location that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getPlayerCreaturesInRangeLocation(JNIEnv *env, jobject self, jobject location, jfloat range)
{
	UNREF(self);

	if (location == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(location, target))
		return 0;
	ServerWorld::findPlayerCreaturesInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param location		object that is the center of the search area
 * @param range			radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getPlayerCreaturesInRangeObject(JNIEnv *env, jobject self, jlong object, jfloat range)
{
	UNREF(self);

	if (object == 0)
		return 0;

	std::vector<ServerObject *> results;
	Vector target;
	if (!ScriptConversion::convertWorld(object, target))
		return 0;
	ServerWorld::findPlayerCreaturesInRange(target, range, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getObjectsInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	Object * coneCenterObject    = 0;
	Object * coneDirectionObject = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findObjectsInCone(*coneCenterObject, *coneDirectionObject, range, convertDegreesToRadians(angle),
		results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Get an array of objects which are in a cone of a location.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getObjectsInConeLocation(JNIEnv *env, jobject self, jlong coneCenterObjectId, jobject coneDirectionLocation, jfloat range, jfloat angle)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionLocation)
		return 0;

	Object * coneCenterObject    = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	Location coneDirection;
	if (!ScriptConversion::convert(LocalRefParam(coneDirectionLocation), coneDirection))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findObjectsInCone(*coneCenterObject, coneDirection, range, convertDegreesToRadians(angle),
		results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	Object * coneCenterObject    = 0;
	Object * coneDirectionObject = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findCreaturesInCone(*coneCenterObject, *coneDirectionObject, range,
		convertDegreesToRadians(angle), results);

	// Convert C-style results to Java.
	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * Get an array of creatures which are in a specified cone.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesInConeLocation(JNIEnv *env, jobject self, jlong coneCenterObjectId, jobject coneDirectionLocation, jfloat range, jfloat angle)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionLocation)
		return 0;

	Object * coneCenterObject    = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	Location coneDirection;
	if (!ScriptConversion::convert(LocalRefParam(coneDirectionLocation), coneDirection))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findCreaturesInCone(*coneCenterObject, coneDirection, range,
		convertDegreesToRadians(angle), results);

	// Convert C-style results to Java.
	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfNicheInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle, jint niche, jint mask)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	Object * coneCenterObject    = 0;
	Object * coneDirectionObject = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findCreaturesOfNicheInCone(*coneCenterObject, *coneDirectionObject,
		range, convertDegreesToRadians(angle), niche, mask, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfSpeciesInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle, jint species)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	Object * coneCenterObject    = 0;
	Object * coneDirectionObject = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findCreaturesOfSpeciesInCone(*coneCenterObject, *coneDirectionObject,
		range, convertDegreesToRadians(angle), species, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getCreaturesOfRaceInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle, jint species, jint race)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	Object * coneCenterObject    = 0;
	Object * coneDirectionObject = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findCreaturesOfRaceInCone(*coneCenterObject, *coneDirectionObject,
		range, convertDegreesToRadians(angle), species, race, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getNonCreaturesInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	Object * coneCenterObject    = 0;
	Object * coneDirectionObject = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findNonCreaturesInCone(*coneCenterObject, *coneDirectionObject,
		range, convertDegreesToRadians(angle), results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getNPCsInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	Object * coneCenterObject    = 0;
	Object * coneDirectionObject = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findNPCsInCone(*coneCenterObject, *coneDirectionObject,
		range, convertDegreesToRadians(angle), results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 *
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getPlayerCreaturesInCone(JNIEnv *env, jobject self, jlong coneCenterObjectId, jlong coneDirectionObjectId, jfloat range, jfloat angle)
{
	UNREF(self);

	// Validate preconditions.
	if (!coneCenterObjectId || !coneDirectionObjectId)
		return 0;

	Object * coneCenterObject    = 0;
	Object * coneDirectionObject = 0;

	// Get the cone and base objects.
	if (!JavaLibrary::getObject(coneCenterObjectId, coneCenterObject))
		return 0;

	if (!JavaLibrary::getObject(coneDirectionObjectId, coneDirectionObject))
		return 0;

	// Query for the results.
	std::vector<ServerObject *>  results;

	ServerWorld::findPlayerCreaturesInCone(*coneCenterObject, *coneDirectionObject,
		range, convertDegreesToRadians(angle), results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsWorldInfoNamespace::getElevation(JNIEnv *env, jobject self, jobject loc)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getElevation");
	Vector where;
	NetworkId cell;
	if (!ScriptConversion::convert(loc, where, cell))
		return 0.0f;

	const TerrainObject *terrain=TerrainObject::getConstInstance ();
	NOT_NULL(terrain);

	float height;
	if (terrain->getHeight(where,height))
		return height;
	else
		return 0.0f;
}

jfloat JNICALL ScriptMethodsWorldInfoNamespace::getDirection(JNIEnv *env, jobject self, jobject loc1, jobject loc2)
{
	Vector from;
	if (!ScriptConversion::convertWorld(loc1, from))
	{
		return 0.0f;
	}

	Vector to;
	if (!ScriptConversion::convertWorld(loc2, to))
	{
		return 0.0f;
	}

	Vector difference=(to-from);
	float theta=convertRadiansToDegrees(difference.theta());
	if (theta > 0)
		return theta;
	else
		return (360.0f+theta);
}

jobject JNICALL ScriptMethodsWorldInfoNamespace::findSpotNear(JNIEnv *env, jobject self, jlong target, jobject loc, jfloat radius)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::findSpotNear");
	Vector where;
	if (!ScriptConversion::convertWorld(loc, where))
	{
		return 0;
	}

	Vector result(where.x+Random::randomReal(radius*2) - radius,0,where.z+Random::randomReal(radius*2) - radius);

	const TerrainObject *terrain=TerrainObject::getConstInstance();
	NOT_NULL(terrain);

	float height;
	if (terrain->getHeight(where,height))
	{
		result.y=height;
		LocalRefPtr temp;
		if (ScriptConversion::convert(result, NetworkId(), temp))
			return temp->getReturnValue();
		else
			return 0;
	}
	else
		return 0;
}

jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getAllNpcs(JNIEnv *env, jobject self, jobject loc, jfloat radius)
{
	UNREF(self);

	std::vector<ServerObject *> results;

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	ServerWorld::findNPCsInRange(target, radius, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getAllPlayers(JNIEnv *env, jobject self, jobject loc, jfloat radius)
{
	UNREF(self);

	std::vector<ServerObject *> results;

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	ServerWorld::findPlayerCreaturesInRange(target, radius, results);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

jlong JNICALL ScriptMethodsWorldInfoNamespace::getClosestMobile(JNIEnv *env, jobject self, jobject loc)
	//TODO:  Rename getClosestNPC for consistency
{
	jfloat radius=128;  //TODO:  make this a paramter
	UNREF(self);

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	ServerObject * npc = ServerWorld::findClosestNPC(target, radius);
	if (npc == nullptr)
		return 0;
	return (npc->getNetworkId()).getValue();
}

jlong JNICALL ScriptMethodsWorldInfoNamespace::getClosestPlayer(JNIEnv *env, jobject self, jobject loc)
{
	jfloat radius=128;  //TODO:  make this a paramter
	UNREF(self);

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	ServerObject * player = ServerWorld::findClosestPlayer(target, radius);
	if (player == nullptr)
		return 0;
	return (player->getNetworkId()).getValue();
}

jlong JNICALL ScriptMethodsWorldInfoNamespace::getFirstObjectWithScript(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring script)
{
	JavaStringParam localScript(script);

	UNREF(self);

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	std::string scriptName;
	if (!JavaLibrary::convert(localScript, scriptName))
		return 0;

	typedef std::vector<ServerObject *> ResultsType;
	ResultsType results;
	ServerWorld::findObjectsInRange(target, range, results);

	for (ResultsType::iterator i=results.begin(); i!=results.end(); ++i)
	{
		if (*i != nullptr && (*i)->getScriptObject()->hasScript(scriptName))
			return ((*i)->getNetworkId()).getValue();
	}

	return 0;
}

jlong JNICALL ScriptMethodsWorldInfoNamespace::getFirstObjectWithObjVar(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring objvar)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getFirstObjectWithObjvar");
	JavaStringParam localObjvar(objvar);

	UNREF(self);

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	std::string objvarName;
	if (!JavaLibrary::convert(localObjvar, objvarName))
		return 0;

	typedef std::vector<ServerObject *> ResultsType;
	ResultsType results;
	ServerWorld::findObjectsInRange(target, range, results);

	for (ResultsType::iterator i=results.begin(); i!=results.end(); ++i)
	{
		if (*i != nullptr && (*i)->getObjVars().hasItem(objvarName))
			return ((*i)->getNetworkId()).getValue();
	}

	return 0;
}

jlong JNICALL ScriptMethodsWorldInfoNamespace::getFirstObjectWithTemplate(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring templateName)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getFirstObjectWithTemplate");
	JavaStringParam localTemplateName(templateName);

	UNREF(self);

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	std::string templateStr;
	if (!JavaLibrary::convert(localTemplateName, templateStr))
		return 0;

	typedef std::vector<ServerObject *> ResultsType;
	ResultsType results;
	ServerWorld::findObjectsInRange(target, range, results);

	for (ResultsType::iterator i=results.begin(); i!=results.end(); ++i)
	{
		if (*i != nullptr && templateStr == (*i)->getTemplateName())
			return ((*i)->getNetworkId()).getValue();
	}

	return 0;
}

jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getAllObjectsWithScript(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring script)
{
	JavaStringParam localScript(script);

	UNREF(self);

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	std::string scriptName;
	if (!JavaLibrary::convert(localScript,scriptName))
		return 0;

	typedef std::vector<ServerObject *> ResultsType;
	ResultsType results, filteredResults;
	ServerWorld::findObjectsInRange(target, range, results);

	for (ResultsType::iterator i=results.begin(); i!=results.end(); ++i)
	{
		if ((*i)->getScriptObject()->hasScript(scriptName))
			filteredResults.push_back(*i);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getAllObjectsWithObjVar(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring objvar)
{
	UNREF(self);

	PROFILER_AUTO_BLOCK_DEFINE("JNI::getObjectsWithObjvar");

	JavaStringParam localObjvar(objvar);

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	std::string objvarName;
	if (!JavaLibrary::convert(localObjvar, objvarName))
		return 0;

	typedef std::vector<ServerObject *> ResultsType;
	ResultsType results, filteredResults;
	ServerWorld::findObjectsInRange(target, range, results);

	for (ResultsType::iterator i=results.begin(); i!=results.end(); ++i)
	{
		if ((*i)->getObjVars().hasItem(objvarName))
			filteredResults.push_back(*i);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getAllObjectsWithTemplate(JNIEnv *env, jobject self, jobject loc, jfloat range, jstring templateName)
{
	UNREF(self);

	PROFILER_AUTO_BLOCK_DEFINE("JNI::getAllObjectsWithTemplate");

	JavaStringParam localTemplateName(templateName);

	Vector target;
	if (!ScriptConversion::convertWorld(loc, target))
		return 0;

	std::string templateStr;
	if (!JavaLibrary::convert(localTemplateName, templateStr))
		return 0;

	typedef std::vector<ServerObject *> ResultsType;
	ResultsType results, filteredResults;
	ServerWorld::findObjectsInRange(target, range, results);

	for (ResultsType::iterator i=results.begin(); i!=results.end(); ++i)
	{
		if (templateStr == (*i)->getTemplateName())
			filteredResults.push_back(*i);
	}

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(filteredResults, rv))
		return rv->getReturnValue();
	return 0;
}

jobject JNICALL ScriptMethodsWorldInfoNamespace::getLocationBetweenLocs(JNIEnv *env, jobject self, jobject target1, jobject target2)
{
	Vector loc1;
	if (!ScriptConversion::convertWorld(target1, loc1))
		return 0;
	Vector loc2;
	if (!ScriptConversion::convertWorld(target2, loc2))
		return 0;

	float factor=Random::randomReal(); // how far along the line between the two points
	Vector result(loc1.x+(loc2.x-loc1.x)*factor,0,loc1.z+(loc2.z-loc1.z)*factor);

	const TerrainObject *terrain=TerrainObject::getConstInstance();
	NOT_NULL(terrain);

	float height;
	if (terrain->getHeight(result,height))
	{
		result.y=height;
		LocalRefPtr temp;
		// @todo: what cell do we use?
		if (ScriptConversion::convert(result, NetworkId::cms_invalid, temp))
			return temp->getReturnValue();
		else
		{
			DEBUG_REPORT_LOG(true,("(ScriptConversion::convert(result,temp)) failed.\n"));
		}
	}
	DEBUG_REPORT_LOG(true,("(terrain->getHeight(result,height)) failed\n"));
   	return 0;
}

//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsWorldInfoNamespace::getPlanetByName(JNIEnv * env, jobject self, jstring name)
{
	JavaStringParam localName(name);

	PROFILER_AUTO_BLOCK_DEFINE("JNI::getPlanetByName");

	jlong result = 0;
	std::string n;
	if(JavaLibrary::convert(localName, n))
	{
		PlanetObject * p = ServerUniverse::getInstance().getPlanetByName(n);
		if (p)
		{
			result = (p->getNetworkId()).getValue();
		}
	}
	return result;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsWorldInfoNamespace::getNameForPlanetObject(JNIEnv * env, jobject self, jlong planet)
{
	NetworkId planetObjId(planet);
	if (planetObjId!=NetworkId::cms_invalid)
	{
		const PlanetObject *planetObj = dynamic_cast<const PlanetObject*>(NetworkIdManager::getObjectById(planetObjId));
		if (planetObj)
		{
			JavaString str(Unicode::narrowToWide(planetObj->getName()));
			return str.getReturnValue();
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWorldInfoNamespace::isValidLocation(JNIEnv *env, jobject self, jobject jObjectPos, jfloat jObjectRadius )
{
	return true;
}

jobject JNICALL ScriptMethodsWorldInfoNamespace::getValidLocation(JNIEnv *env, jobject self, jobject jCorralCenter, jfloat jCorralRadius, jobject jDesiredPos, jfloat jObjectRadius )
{
	// convert java objects to C++ objects

	PROFILER_AUTO_BLOCK_DEFINE("JNI::getValidLocation");

	Vector corralCenter;
	float corralRadius = jCorralRadius;

	Vector desiredPos;
	float objectRadius = jObjectRadius;

	UNREF(corralRadius);
	UNREF(objectRadius);

	ScriptConversion::convertWorld(jCorralCenter, corralCenter);
	ScriptConversion::convertWorld(jDesiredPos, desiredPos);

	// ----------

	const TerrainObject *terrain=TerrainObject::getConstInstance ();
	NOT_NULL(terrain);

	for (int testCount = 0; testCount < 20; testCount++)
	{
		// ----------
		// Find a random point inside the corral

		float rad = (corralRadius - objectRadius);
		float rad2 = rad * rad;

		Vector corralPoint;

		do
		{
			corralPoint.x = Random::randomReal(-rad,rad);
			corralPoint.y = 0;
			corralPoint.z = Random::randomReal(-rad,rad);
		}
		while(corralPoint.magnitudeSquared() > rad2);

		corralPoint += corralCenter;

		// ----------
		// CorralPoint is now a random point in the corral.

		// Pop CorralPoint down onto the terrain. If we can't, try and find another point.

		if (!terrain->getHeight(corralPoint,corralPoint.y))
		{
			continue;
		}

		// Determine if it's a valid point, and if not search for another one.

		bool isValid = !CollisionWorld::query( Sphere(corralPoint,objectRadius),nullptr);

		// ----------

		if(isValid)
		{
			LocalRefPtr result;
			if (ScriptConversion::convert(corralPoint, NetworkId(), result))
				return result->getReturnValue();
		}
		else
		{
			continue;
		}
	}

	// We failed to find a valid location

	return nullptr;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWorldInfoNamespace::getCollidesWithObject(JNIEnv * env, jobject self, jobject jlocation, jfloat jradius)
{
	if (jlocation == nullptr)
		return JNI_FALSE;

	float const radius = jradius;
	Vector location;
	if (!ScriptConversion::convertWorld(jlocation, location))
		return JNI_FALSE;

	bool const result = CollisionWorld::query(Sphere(location, radius), nullptr);

	if(result)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

// ----------------------------------------------------------------------

/**
 * Tests to see if an object can see another object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param source		object that is looking
 * @param target		object that is being looked at
 *
 * @return JNI_TRUE if the source can see the target, JNI_FALSE otherwise
 */
jboolean JNICALL ScriptMethodsWorldInfoNamespace::canSee(JNIEnv *env, jobject self, jlong source, jlong target)
{
	UNREF(self);

	const ServerObject * sourceObject = nullptr;
	if (!JavaLibrary::getObject(source, sourceObject))
	{
		NetworkId id(source);
		if (id == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true,("JavaLibrary::canSee - source object %s doesn't exist, returning true as a workaround\n",id.getValueString().c_str()));
		}
		else
		{
			DEBUG_WARNING(true,("JavaLibrary::canSee - source object doesn't exist and we can't get a network ID for it, returning true as a workaround\n"));
		}

		return JNI_TRUE;
	}

	const ServerObject * targetObject = nullptr;
	if (!JavaLibrary::getObject(target, targetObject))
	{
		NetworkId id(target);
		if (id == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true,("JavaLibrary::canSee - target object %s doesn't exist, returning true as a workaround\n",id.getValueString().c_str()));
		}
		else
		{
			DEBUG_WARNING(true,("JavaLibrary::canSee - target object doesn't exist and we can't get a network ID for it, returning true as a workaround\n"));
		}

		return JNI_TRUE;
	}

	return sourceObject->checkLOSTo(*targetObject);

}	// JavaLibrary::canSee

// ----------------------------------------------------------------------

/**
 * Tests to see if an object can see a location.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param source		object that is looking
 * @param target		location the source is looking at
 *
 * @return JNI_TRUE if the source can see the target, JNI_FALSE otherwise
 */
jboolean JNICALL ScriptMethodsWorldInfoNamespace::canSeeLocation(JNIEnv *env, jobject self, jlong source, jobject target)
{
	UNREF(self);

	const ServerObject * sourceObject = nullptr;
	if (!JavaLibrary::getObject(source, sourceObject))
	{
		NetworkId id(source);
		if (id == NetworkId::cms_invalid)
		{
			DEBUG_WARNING(true,("JavaLibrary::canSee - source object %s doesn't exist, returning true as a workaround\n",id.getValueString().c_str()));
		}
		else
		{
			DEBUG_WARNING(true,("JavaLibrary::canSee - source object doesn't exist and we can't get a network ID for it, returning true as a workaround\n"));
		}

		return JNI_TRUE;
	}

	Location targetLocation;
	if (!ScriptConversion::convert(LocalRefParam(target), targetLocation))
	{
		DEBUG_WARNING(true,("JavaLibrary::canSeeLocation - can't convert target location, returning true as a workaround\n"));
		return JNI_TRUE;
	}

	return sourceObject->checkLOSTo(targetLocation);
}	// JavaLibrary::canSeeLocation

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWorldInfoNamespace::canManipulate(JNIEnv *env, jobject self, jlong player, jlong target, jboolean move, jboolean doPermission, float distance, jboolean sendMessage, jboolean skipNoTradeCheck)
{
	UNREF(self);

	const CreatureObject * sourceObject = nullptr;
	if (!JavaLibrary::getObject(player, sourceObject))
		return JNI_FALSE;

	const ServerObject * targetObject = nullptr;
	if (!JavaLibrary::getObject(target, targetObject))
		return JNI_FALSE;

	Container::ContainerErrorCode code = Container::CEC_Success;
	bool retval =  sourceObject->canManipulateObject(*targetObject, move, doPermission, doPermission, distance, code, skipNoTradeCheck);
	if (!retval && sendMessage && sourceObject->getClient())
	{
		ContainerInterface::sendContainerMessageToClient(*sourceObject, code);
	}
	return retval;
}

/**
 * @param env       Java environment
 * @param self      class calling this function
 * @param performer performer we are looking for listeners of
 * @param range     radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getPerformanceListenersInRange(JNIEnv *env, jobject self, jlong performer, jfloat range)
{
	UNREF(self);

	if (performer == 0)
		return 0;

	std::vector<ServerObject *> creatures;
	const TangibleObject * object = 0;
	if (!JavaLibrary::getObject(performer, object))
		return 0;
	ServerWorld::findCreaturesInRange(object->getPosition_w(), range, creatures);

	std::vector<ServerObject *> results;
	for (std::vector<ServerObject*>::iterator i = creatures.begin(); i != creatures.end(); ++i)
		if (safe_cast<CreatureObject*>(*i)->getPerformanceListenTarget() == object->getNetworkId())
			results.push_back(*i);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

/**
 * @param env       Java environment
 * @param self      class calling this function
 * @param performer performer we are looking for watchers of
 * @param range     radius of search area
 *
 * @return array of obj_ids of the objects in range, or nullptr on error
 */
jlongArray JNICALL ScriptMethodsWorldInfoNamespace::getPerformanceWatchersInRange(JNIEnv *env, jobject self, jlong performer, jfloat range)
{
	UNREF(self);

	if (performer == 0)
		return 0;

	std::vector<ServerObject *> creatures;
	const TangibleObject * object = 0;
	if (!JavaLibrary::getObject(performer, object))
		return 0;
	ServerWorld::findCreaturesInRange(object->getPosition_w(), range, creatures);

	std::vector<ServerObject *> results;
	for (std::vector<ServerObject*>::iterator i = creatures.begin(); i != creatures.end(); ++i)
		if (safe_cast<CreatureObject*>(*i)->getPerformanceWatchTarget() == object->getNetworkId())
			results.push_back(*i);

	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWorldInfoNamespace::isSpaceBattlefieldZone(JNIEnv *, jobject)
{
	return static_cast<jboolean>(ServerWorld::isSpaceBattlefieldZone());
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWorldInfoNamespace::isSpaceScene(JNIEnv *, jobject)
{
	return static_cast<jboolean>(ServerWorld::isSpaceScene());
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWorldInfoNamespace::startUniverseWideEvent(JNIEnv * env, jobject self, jstring eventName)
{
	JavaStringParam localEventName(eventName);
	std::string eventString;

	if (!JavaLibrary::convert(localEventName, eventString))
		return JNI_FALSE;

	ServerUniverse::addUniverseEvent(eventString);
	//ServerBuildoutManager::onEventStarted(eventString);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWorldInfoNamespace::stopUniverseWideEvent(JNIEnv * env, jobject self, jstring eventName)
{
	JavaStringParam localEventName(eventName);
	std::string eventString;

	if (!JavaLibrary::convert(localEventName, eventString))
		return JNI_FALSE;

	ServerUniverse::removeUniverseEvent(eventString);
	//ServerBuildoutManager::onEventStopped(eventString);


	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsWorldInfoNamespace::getCurrentUniverseWideEvents(JNIEnv * env, jobject self)
{
	const std::string events  = ServerUniverse::getCurrentEvents();

	JavaString str(Unicode::narrowToWide(events));
	
	return str.getReturnValue();

}

// ======================================================================

