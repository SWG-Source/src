//-----------------------------------------------------------------------
// ScriptMethodsRegion.cpp
// Copyright 2002, Sony Online Entertainment Inc., all rights reserved.
//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "sharedDebug/Profiler.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/UniverseObject.h"
#include "sharedMath/MxCifQuadTreeBounds.h"
#include "sharedRandom/Random.h"
#include <list>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsRegionNamespace
// ======================================================================

namespace ScriptMethodsRegionNamespace
{
	bool                   install();
	LocalObjectArrayRefPtr _getRegionsAtPoint(jobject location);

	void         JNICALL createRectRegion(JNIEnv *env, jobject self, jobject lowerLeftLocation, jobject upperRightLocation, jstring name, jint pvp, jint buildable, jint municipal, jint geography, jint minDifficulty, jint maxDifficulty, jint spawnable, jint mission, jboolean visible, jboolean notify);
	void         JNICALL createCircleRegion(JNIEnv *env, jobject self, jobject center, jfloat radius, jstring name, jint pvp, jint buildable, jint municipal, jint geography, jint minDifficulty, jint maxDifficulty, jint spawnable, jint mission, jboolean visible, jboolean notify);
	jlong        JNICALL createCircleRegionWithSpawn(JNIEnv *env, jobject self, jobject center, jfloat radius, jstring name, jint pvp, jint buildable, jint municipal, jint geography, jint minDifficulty, jint maxDifficulty, jint spawnable, jint mission, jboolean visible, jboolean notify, jstring spawnTable, jint duration);
	jobject      JNICALL getRegion(JNIEnv *env, jobject self, jstring planet, jstring region);
	jobjectArray JNICALL getRegionsAtPoint(JNIEnv *env, jobject self, jobject location);
	jobjectArray JNICALL getRegions(JNIEnv *env, jobject self, jstring planet);
	jobjectArray JNICALL getRegionsWithPvP(JNIEnv *env, jobject self, jstring planet, jint value);
	jobjectArray JNICALL getRegionsWithBuildable(JNIEnv *env, jobject self, jstring planet, jint value);
	jobjectArray JNICALL getRegionsWithMunicipal(JNIEnv *env, jobject self, jstring planet, jint value);
	jobjectArray JNICALL getRegionsWithGeographical(JNIEnv *env, jobject self, jstring planet, jint value);
	jobjectArray JNICALL getRegionsWithDifficulty(JNIEnv *env, jobject self, jstring planet, jint value);
	jobjectArray JNICALL getRegionsWithSpawnable(JNIEnv *env, jobject self, jstring planet, jint value);
	jobjectArray JNICALL getRegionsWithMission(JNIEnv *env, jobject self, jstring planet, jint value);
	jobjectArray JNICALL getRegionsWithPvPAtPoint(JNIEnv *env, jobject self, jobject location, jint value);
	jobjectArray JNICALL getRegionsWithBuildableAtPoint(JNIEnv *env, jobject self, jobject location, jint value);
	jobjectArray JNICALL getRegionsWithMunicipalAtPoint(JNIEnv *env, jobject self, jobject location, jint value);
	jobjectArray JNICALL getRegionsWithGeographicalAtPoint(JNIEnv *env, jobject self, jobject location, jint value);
	jobjectArray JNICALL getRegionsWithDifficultyAtPoint(JNIEnv *env, jobject self, jobject location, jint value);
	jobjectArray JNICALL getRegionsWithSpawnableAtPoint(JNIEnv *env, jobject self, jobject location, jint value);
	jobjectArray JNICALL getRegionsWithMissionAtPoint(JNIEnv *env, jobject self, jobject location, jint value);
	jobject      JNICALL getSmallestRegionAtPoint(JNIEnv *env, jobject self, jobject location);
	jobject      JNICALL getSmallestVisibleRegionAtPoint(JNIEnv *env, jobject self, jobject location);
	jboolean     JNICALL wipeRegionsFromPlanet(JNIEnv *env, jobject self, jstring planet);
	jboolean     JNICALL deleteRegion(JNIEnv *env, jobject self, jobject region);
	jobject      JNICALL findPointInRegion(JNIEnv *env, jobject self, jobject region);
	jobjectArray JNICALL getRegionExtent(JNIEnv *env, jobject self, jobject region);
	jboolean     JNICALL isNotifyRegion(JNIEnv *env, jobject self, jobject region);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsRegionNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsRegionNamespace::c)}
	JF("createRectRegion",           "(Lscript/location;Lscript/location;Ljava/lang/String;IIIIIIIIZZ)V",createRectRegion),
	JF("createCircleRegion",         "(Lscript/location;FLjava/lang/String;IIIIIIIIZZ)V",      createCircleRegion),
	JF("_createCircleRegionWithSpawn","(Lscript/location;FLjava/lang/String;IIIIIIIIZZLjava/lang/String;I)J",      createCircleRegionWithSpawn),
	JF("getRegion",                  "(Ljava/lang/String;Ljava/lang/String;)Lscript/region;", getRegion),
	JF("getRegionsAtPoint",          "(Lscript/location;)[Lscript/region;",                   getRegionsAtPoint),
	JF("getRegions",                 "(Ljava/lang/String;)[Lscript/region;",                  getRegions),
	JF("getRegionsWithPvP",          "(Ljava/lang/String;I)[Lscript/region;",                 getRegionsWithPvP),
	JF("getRegionsWithBuildable",    "(Ljava/lang/String;I)[Lscript/region;",                 getRegionsWithBuildable),
	JF("getRegionsWithMunicipal",    "(Ljava/lang/String;I)[Lscript/region;",                 getRegionsWithMunicipal),
	JF("getRegionsWithGeographical", "(Ljava/lang/String;I)[Lscript/region;",                 getRegionsWithGeographical),
	JF("getRegionsWithDifficulty",   "(Ljava/lang/String;I)[Lscript/region;",                 getRegionsWithDifficulty),
	JF("getRegionsWithSpawnable",    "(Ljava/lang/String;I)[Lscript/region;",                 getRegionsWithSpawnable),
	JF("getRegionsWithMission",      "(Ljava/lang/String;I)[Lscript/region;",                 getRegionsWithMission),
	JF("getRegionsWithPvPAtPoint",   "(Lscript/location;I)[Lscript/region;",                  getRegionsWithPvPAtPoint),
	JF("getRegionsWithBuildableAtPoint",    "(Lscript/location;I)[Lscript/region;",           getRegionsWithBuildableAtPoint),
	JF("getRegionsWithMunicipalAtPoint",    "(Lscript/location;I)[Lscript/region;",           getRegionsWithMunicipalAtPoint),
	JF("getRegionsWithGeographicalAtPoint", "(Lscript/location;I)[Lscript/region;",           getRegionsWithGeographicalAtPoint),
	JF("getRegionsWithDifficultyAtPoint",   "(Lscript/location;I)[Lscript/region;",           getRegionsWithDifficultyAtPoint),
	JF("getRegionsWithSpawnableAtPoint",    "(Lscript/location;I)[Lscript/region;",           getRegionsWithSpawnableAtPoint),
	JF("getRegionsWithMissionAtPoint",      "(Lscript/location;I)[Lscript/region;",           getRegionsWithMissionAtPoint),
	JF("getSmallestRegionAtPoint",          "(Lscript/location;)Lscript/region;",             getSmallestRegionAtPoint),
	JF("getSmallestVisibleRegionAtPoint",   "(Lscript/location;)Lscript/region;",             getSmallestVisibleRegionAtPoint),
	JF("wipeRegionsFromPlanet",      "(Ljava/lang/String;)Z",                                 wipeRegionsFromPlanet),
	JF("deleteRegion",               "(Lscript/region;)Z",                                    deleteRegion),
	JF("findPointInRegion",          "(Lscript/region;)Lscript/location;",                    findPointInRegion),
	JF("getRegionExtent",            "(Lscript/region;)[Lscript/location;",                   getRegionExtent),
	JF("isNotifyRegion",             "(Lscript/region;)Z",                                    isNotifyRegion),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

/** Get all the regions at the given location
 */
LocalObjectArrayRefPtr ScriptMethodsRegionNamespace::_getRegionsAtPoint(jobject location)
{
	//pull the data from the java Location objects
	Vector locationVec;
	std::string sceneId;
	if(!ScriptConversion::convertWorld(location, locationVec, sceneId))
		return LocalObjectArrayRef::cms_nullPtr;

	std::vector<const Region *> result;
	RegionMaster::getRegionsAtPoint(sceneId, locationVec.x, locationVec.z, result);

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return LocalObjectArrayRef::cms_nullPtr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *it;
		if (r != nullptr)
		{
			if (!ScriptConversion::convert(*r, javaRegion))
				return LocalObjectArrayRef::cms_nullPtr;
			setObjectArrayElement(*regions, index, *javaRegion);
		}
		else
			setObjectArrayElement(*regions, index, LocalRef(0));
	}
	return regions;
}

//-----------------------------------------------------------------------

/** Create a rectangular region given the locations.
 */
void JNICALL ScriptMethodsRegionNamespace::createRectRegion(JNIEnv *env, jobject self,
	jobject lowerLeftLocation, jobject upperRightLocation,
	jstring name, jint pvp, jint buildable, jint municipal, jint geography,
	jint minDifficulty, jint maxDifficulty, jint spawnable, jint mission,
	jboolean visible, jboolean notify)
{
	UNREF(self);

	//validate scripter's input
	if (lowerLeftLocation == 0 || upperRightLocation == 0 || name == 0)
		return;

	JavaStringParam jname(name);
	Unicode::String regionName;
	if (!JavaLibrary::convert(jname, regionName))
		return;

	//pull the data from the java Location objects
	Vector llLocationVec;
	std::string llSceneId;
	NetworkId llCell;
	if (!ScriptConversion::convert(lowerLeftLocation, llLocationVec, llSceneId, llCell))
		return;
	Vector urLocationVec;
	std::string urSceneId;
	NetworkId urCell;
	if(!ScriptConversion::convert(upperRightLocation, urLocationVec, urSceneId, urCell))
		return;

	//locations must be on the same planet
	if (llSceneId != urSceneId)
		return;

	//create the region
	RegionMaster::createNewDynamicRegion(llLocationVec.x,
		llLocationVec.z, urLocationVec.x, urLocationVec.z, regionName, llSceneId,
		pvp, buildable, municipal, geography, minDifficulty, maxDifficulty,
		spawnable, mission, visible, notify);
}

//-----------------------------------------------------------------------

/** Create a circular region given the location and radius.
 */
void JNICALL ScriptMethodsRegionNamespace::createCircleRegion(JNIEnv *env, jobject self,
	jobject center, jfloat radius, jstring name, jint pvp, jint buildable,
	jint municipal, jint geography, jint minDifficulty, jint maxDifficulty,
	jint spawnable, jint mission, jboolean visible, jboolean notify)
{
	UNREF(self);

	// validate scripter's input
	if (center == nullptr || radius <= 0 || name == 0)
		return;

	JavaStringParam jname(name);
	Unicode::String regionName;
	if (!JavaLibrary::convert(jname, regionName))
		return;

	// pull the data from the java Location objects
	Vector locationVec;
	std::string sceneId;
	if (!ScriptConversion::convertWorld(center, locationVec, sceneId))
		return;

	// create the region
	RegionMaster::createNewDynamicRegion(locationVec.x, locationVec.z,
		radius, regionName, sceneId, pvp, buildable, municipal, geography, minDifficulty,
		maxDifficulty, spawnable, mission, visible, notify);
}

//-----------------------------------------------------------------------

/**
 * Gets the region object for a given planet and region name.
 */
jobject JNICALL ScriptMethodsRegionNamespace::getRegion(JNIEnv *env, jobject self, jstring planet,
	jstring region)
{
	UNREF(self);

	if (planet == 0 || region == 0)
		return 0;

	JavaStringParam jplanet(planet);
	std::string planetName;
	if (!JavaLibrary::convert(jplanet, planetName))
		return 0;

	JavaStringParam jregion(region);
	Unicode::String regionName;
	if (!JavaLibrary::convert(jregion, regionName))
		return 0;

	const Region * r = RegionMaster::getRegionByName(planetName, regionName);
	if (r == nullptr)
		return 0;

	LocalRefPtr jr;
	if (!ScriptConversion::convert(*r, jr))
		return 0;
	return jr->getReturnValue();
}	// JavaLibrary::getRegion

//-----------------------------------------------------------------------

/** Get all the regions at the given location
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsAtPoint(JNIEnv *env, jobject self, jobject location)
{
	LocalObjectArrayRefPtr regions = _getRegionsAtPoint(location);
	if (regions.get() == nullptr)
		return 0;
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Return all the regions on the given planet
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegions(JNIEnv *env, jobject self, jstring planet)
{
	UNREF(self);

	JavaStringParam localPlanet(planet);

	//get the planet name and the planet object
	std::string sceneId;
	if (!JavaLibrary::convert(localPlanet, sceneId))
		return 0;

	//get the planet's regions
	std::vector<const Region*> result;
	RegionMaster::getRegionsForPlanet(sceneId, result);

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return nullptr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator i = result.begin(); i != result.end(); ++i, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *i;
		if (r != nullptr)
		{
			if (!ScriptConversion::convert(*r, javaRegion))
			{
				return nullptr;
			}
		}
		setObjectArrayElement(*regions, index, *javaRegion);
	}
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Get all the regions with the PvP property set to the given value on the given planet
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithPvP(JNIEnv *env, jobject self, jstring planet, jint value)
{
	UNREF(self);

	JavaStringParam localPlanet(planet);

	//get the planet name
	std::string sceneId;
	JavaLibrary::convert(localPlanet, sceneId);

	//get the planet's regions
	std::vector<const Region*> regions;
	RegionMaster::getRegionsForPlanet(sceneId, regions);

	// get the regions that have the right property
	std::vector<const Region*> goodRegions;
	for (std::vector<const Region*>::iterator i = regions.begin(); i != regions.end(); ++i)
	{
		if ((*i)->getPvp() == value)
			goodRegions.push_back(*i);
	}

	//-- return nullptr instead of a zero-length array
	if (goodRegions.empty())
		return 0;

	//put them into the jobjectarray
	int index = 0;
	LocalObjectArrayRefPtr result = createNewObjectArray(goodRegions.size(), JavaLibrary::getClsRegion());
	for (std::vector<const Region*>::iterator it = goodRegions.begin();
		it != goodRegions.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		if (!ScriptConversion::convert(**it, javaRegion))
		{
			return nullptr;
		}
		setObjectArrayElement(*result, index, *javaRegion);
	}
	return result->getReturnValue();
}

//-----------------------------------------------------------------------

/** Get all the regions with the Buildable property set to the given value on the given planet
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithBuildable(JNIEnv *env, jobject self, jstring planet, jint value)
{
	UNREF(self);

	JavaStringParam localPlanet(planet);

	//get the planet name
	std::string sceneId;
	JavaLibrary::convert(localPlanet, sceneId);

	//get the planet's regions
	std::vector<const Region*> regions;
	RegionMaster::getRegionsForPlanet(sceneId, regions);

	// get the regions that have the right property
	std::vector<const Region*> goodRegions;
	for (std::vector<const Region*>::iterator i = regions.begin(); i != regions.end(); ++i)
	{
		if ((*i)->getBuildable() == value)
			goodRegions.push_back(*i);
	}

	//-- return nullptr instead of a zero-length array
	if (goodRegions.empty())
		return 0;

	//put them into the jobjectarray
	int index = 0;
	LocalObjectArrayRefPtr result = createNewObjectArray(goodRegions.size(), JavaLibrary::getClsRegion());
	for (std::vector<const Region*>::iterator it = goodRegions.begin();
		it != goodRegions.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		if (!ScriptConversion::convert(**it, javaRegion))
		{
			return nullptr;
		}
		setObjectArrayElement(*result, index, *javaRegion);
	}
	return result->getReturnValue();
}

//-----------------------------------------------------------------------

/** Get all the regions with the Municipal property set to the given value on the given planet
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithMunicipal(JNIEnv *env, jobject self, jstring planet, jint value)
{
	UNREF(self);

	JavaStringParam localPlanet(planet);

	//get the planet name
	std::string sceneId;
	JavaLibrary::convert(localPlanet, sceneId);

	//get the planet's regions
	std::vector<const Region*> regions;
	RegionMaster::getRegionsForPlanet(sceneId, regions);

	// get the regions that have the right property
	std::vector<const Region*> goodRegions;
	for (std::vector<const Region*>::iterator i = regions.begin(); i != regions.end(); ++i)
	{
		if ((*i)->getMunicipal() == value)
			goodRegions.push_back(*i);
	}

	//-- return nullptr instead of a zero-length array
	if (goodRegions.empty())
		return 0;

	//put them into the jobjectarray
	int index = 0;
	LocalObjectArrayRefPtr result = createNewObjectArray(goodRegions.size(), JavaLibrary::getClsRegion());
	for (std::vector<const Region*>::iterator it = goodRegions.begin();
		it != goodRegions.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		if (!ScriptConversion::convert(**it, javaRegion))
		{
			return nullptr;
		}
		setObjectArrayElement(*result, index, *javaRegion);
	}
	return result->getReturnValue();
}

//-----------------------------------------------------------------------

/** Get all the regions with the Geographical property set to the given value on the given planet
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithGeographical(JNIEnv *env, jobject self, jstring planet, jint value)
{
	UNREF(self);

	JavaStringParam localPlanet(planet);

	//get the planet name
	std::string sceneId;
	JavaLibrary::convert(localPlanet, sceneId);

	//get the planet's regions
	std::vector<const Region*> regions;
	RegionMaster::getRegionsForPlanet(sceneId, regions);

	// get the regions that have the right property
	std::vector<const Region*> goodRegions;
	for (std::vector<const Region*>::iterator i = regions.begin(); i != regions.end(); ++i)
	{
		if ((*i)->getGeography() == value)
			goodRegions.push_back(*i);
	}

	//-- return nullptr instead of a zero-length array
	if (goodRegions.empty())
		return 0;

	//put them into the jobjectarray
	int index = 0;
	LocalObjectArrayRefPtr result = createNewObjectArray(goodRegions.size(), JavaLibrary::getClsRegion());
	for (std::vector<const Region*>::iterator it = goodRegions.begin();
		it != goodRegions.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		if (!ScriptConversion::convert(**it, javaRegion))
		{
			return nullptr;
		}
		setObjectArrayElement(*result, index, *javaRegion);
	}
	return result->getReturnValue();
}

//-----------------------------------------------------------------------

/** Get all the regions with the Difficulty property set to the given value on the given planet
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithDifficulty(JNIEnv *env, jobject self, jstring planet, jint value)
{
	UNREF(self);

	JavaStringParam localPlanet(planet);

	//get the planet name
	std::string sceneId;
	JavaLibrary::convert(localPlanet, sceneId);

	//get the planet's regions
	std::vector<const Region*> regions;
	RegionMaster::getRegionsForPlanet(sceneId, regions);

	// get the regions that have the right property
	std::vector<const Region*> goodRegions;
	for (std::vector<const Region*>::iterator i = regions.begin(); i != regions.end(); ++i)
	{
		if (value >= (*i)->getMinDifficulty() && value <= (*i)->getMaxDifficulty())
			goodRegions.push_back(*i);
	}

	//-- return nullptr instead of a zero-length array
	if (goodRegions.empty())
		return 0;

	//put them into the jobjectarray
	int index = 0;
	LocalObjectArrayRefPtr result = createNewObjectArray(goodRegions.size(), JavaLibrary::getClsRegion());
	for (std::vector<const Region*>::iterator it = goodRegions.begin();
		it != goodRegions.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		if (!ScriptConversion::convert(**it, javaRegion))
		{
			return nullptr;
		}
		setObjectArrayElement(*result, index, *javaRegion);
	}
	return result->getReturnValue();
}

//-----------------------------------------------------------------------

/** Get all the regions with the Spawnable property set to the given value on the given planet
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithSpawnable(JNIEnv *env, jobject self, jstring planet, jint value)
{
	UNREF(self);

	JavaStringParam localPlanet(planet);

	//get the planet name
	std::string sceneId;
	JavaLibrary::convert(localPlanet, sceneId);

	//get the planet's regions
	std::vector<const Region*> regions;
	RegionMaster::getRegionsForPlanet(sceneId, regions);

	// get the regions that have the right property
	std::vector<const Region*> goodRegions;
	for (std::vector<const Region*>::iterator i = regions.begin(); i != regions.end(); ++i)
	{
		if ((*i)->getSpawn() == value)
			goodRegions.push_back(*i);
	}

	//-- return nullptr instead of a zero-length array
	if (goodRegions.empty())
		return 0;

	//put them into the jobjectarray
	int index = 0;
	LocalObjectArrayRefPtr result = createNewObjectArray(goodRegions.size(), JavaLibrary::getClsRegion());
	for (std::vector<const Region*>::iterator it = goodRegions.begin();
		it != goodRegions.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		if (!ScriptConversion::convert(**it, javaRegion))
		{
			return nullptr;
		}
		setObjectArrayElement(*result, index, *javaRegion);
	}
	return result->getReturnValue();
}

//-----------------------------------------------------------------------

/** Get all the regions with the Mission property set to the given value on the given planet
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithMission(JNIEnv *env, jobject self, jstring planet, jint value)
{
	UNREF(self);

	JavaStringParam localPlanet(planet);

	//get the planet name
	std::string sceneId;
	JavaLibrary::convert(localPlanet, sceneId);

	//get the planet's regions
	std::vector<const Region*> regions;
	RegionMaster::getRegionsForPlanet(sceneId, regions);

	// get the regions that have the right property
	std::vector<const Region*> goodRegions;
	for (std::vector<const Region*>::iterator i = regions.begin(); i != regions.end(); ++i)
	{
		if ((*i)->getMission() == value)
			goodRegions.push_back(*i);
	}

	//-- return nullptr instead of a zero-length array
	if (goodRegions.empty())
		return 0;

	//put them into the jobjectarray
	int index = 0;
	LocalObjectArrayRefPtr result = createNewObjectArray(goodRegions.size(), JavaLibrary::getClsRegion());
	for (std::vector<const Region*>::iterator it = goodRegions.begin();
		it != goodRegions.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		if (!ScriptConversion::convert(**it, javaRegion))
		{
			return nullptr;
		}
		setObjectArrayElement(*result, index, *javaRegion);
	}
	return result->getReturnValue();
}

//-----------------------------------------------------------------------

/** Delete all the regions on the given planet
 */
jboolean JNICALL ScriptMethodsRegionNamespace::wipeRegionsFromPlanet(JNIEnv *env, jobject self, jstring planet)
{
	UNREF(self);
	return JNI_FALSE;

	//@todo implement this
}

//-----------------------------------------------------------------------

/** Delete the given region
 * @return true on success, false on failure (if the region doesn't point to a valid const Region, if region isn't of the script.region class, etc.)
 */
jboolean JNICALL ScriptMethodsRegionNamespace::deleteRegion(JNIEnv *env, jobject self, jobject region)
{

	UNREF(self);
	if (region == 0)
		return JNI_FALSE;

	const Region* r = 0;
	if (!ScriptConversion::convert(region, r))
		return JNI_FALSE;

	if (!r->isDynamic())
		return JNI_FALSE;

	UniverseObject * regionObject = dynamic_cast<UniverseObject *>(r->getDynamicRegionId().getObject());
	if (regionObject == nullptr)
		return JNI_FALSE;
	regionObject->permanentlyDestroy(DeleteReasons::Script);
	return JNI_TRUE;
}

//-----------------------------------------------------------------------

/** Find the extent of the given region
 * @return true on success, false on failure (if the region doesn't point to a valid const Region, if region isn't of the script.region class, etc.)
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionExtent(JNIEnv *env, jobject self, jobject region)
{
	UNREF (self);

	if (env->IsInstanceOf (region, JavaLibrary::getClsRegion()) != JNI_TRUE)
	{
		DEBUG_WARNING (true, ("getRegionExtent (): jobject region is not a region"));
		return 0;
	}

	const Region * r = 0;
	if (!ScriptConversion::convert(region, r))
	{
		DEBUG_WARNING (true, ("getRegionExtent (): could not convert region to a const Region"));
		return 0;
	}
	const MxCifQuadTreeBounds & bounds = r->getBounds();

	LocalObjectArrayRefPtr locations = createNewObjectArray (2, JavaLibrary::getClsLocation());

	LocalRefPtr lowerLeftLocation;
	if (!ScriptConversion::convert (Vector(bounds.getMinX(), 0.f, bounds.getMinY()), r->getPlanet(), NetworkId::cms_invalid, lowerLeftLocation))
	{
		DEBUG_WARNING (true, ("getRegionExtent (): could not convert lower-left location"));
		return 0;
	}

	LocalRefPtr upperRightLocation;
	if (!ScriptConversion::convert (Vector (bounds.getMaxX(), 0.f, bounds.getMaxY()), r->getPlanet(), NetworkId::cms_invalid, upperRightLocation))
	{
		DEBUG_WARNING (true, ("getRegionExtent (): could not convert upper-right location"));
		return 0;
	}

	setObjectArrayElement(*locations, 0, *lowerLeftLocation);
	setObjectArrayElement(*locations, 1, *upperRightLocation);

	return locations->getReturnValue();
}

//-----------------------------------------------------------------------

/** Find a random point in the given region
 *  @return a script.location inside the region, or a nullptr reference if any problems occur
 */
jobject JNICALL ScriptMethodsRegionNamespace::findPointInRegion(JNIEnv *env, jobject self, jobject region)
{
	UNREF(self);

	const Region* r = 0;
	if (!ScriptConversion::convert(region, r))
		return 0;
	const MxCifQuadTreeBounds & bounds = r->getBounds();

	float x,z;
	do
	{
		x = Random::randomReal(bounds.getMinX(), bounds.getMaxX());
		z = Random::randomReal(bounds.getMinY(), bounds.getMaxY());
	} while (!bounds.isPointIn(x, z));

	Vector loc3d(x, 0, z);
	LocalRefPtr location;
	if (!ScriptConversion::convert(loc3d, r->getPlanet(), NetworkId::cms_invalid, location))
		return nullptr;
	return location->getReturnValue();
}

//-----------------------------------------------------------------------

/** Return an array of all the regions with the given value for the PvP property that cover the given point
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithPvPAtPoint(JNIEnv *env, jobject self, jobject location, jint value)
{
	//get all the regions at the given point
	LocalObjectArrayRefPtr regionsAtPoint = _getRegionsAtPoint(location);
	if (regionsAtPoint.get() == nullptr || regionsAtPoint == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	//parse out the ones with the right property
	int count = getArrayLength(*regionsAtPoint);
	std::vector<const Region*> result;
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr region = getObjectArrayElement(*regionsAtPoint, i);
		const Region* r = 0;
		if (!ScriptConversion::convert(*region, r))
			return 0;
		if(r->getPvp() == value)
			result.push_back(r);
	}

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return nullptr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *it;
		if (r != nullptr)
		{
			if (!ScriptConversion::convert(*r, javaRegion))
				return nullptr;
		}
		setObjectArrayElement(*regions, index, *javaRegion);
	}
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Return an array of all the regions with the given value for the Buildable property that cover the given point
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithBuildableAtPoint(JNIEnv *env, jobject self, jobject location, jint value)
{
	//get all the regions at the given point
	LocalObjectArrayRefPtr regionsAtPoint = _getRegionsAtPoint(location);
	if (regionsAtPoint.get() == nullptr || regionsAtPoint == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	//parse out the ones with the right property
	int count = getArrayLength(*regionsAtPoint);
	std::vector<const Region*> result;
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr region = getObjectArrayElement(*regionsAtPoint, i);
		const Region* r = 0;
		if (!ScriptConversion::convert(*region, r))
			return 0;
		if(r->getBuildable() == value)
			result.push_back(r);
	}

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return nullptr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *it;
		if (r != nullptr)
		{
			if(!ScriptConversion::convert(*r, javaRegion))
				return nullptr;
		}
		setObjectArrayElement(*regions, index, *javaRegion);
	}
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Return an array of all the regions with the given value for the Municipal property that cover the given point
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithMunicipalAtPoint(JNIEnv *env, jobject self, jobject location, jint value)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getRegionsWithMunicipalAtPoint");
	//get all the regions at the given point
	LocalObjectArrayRefPtr regionsAtPoint = _getRegionsAtPoint(location);
	if (regionsAtPoint.get() == nullptr || regionsAtPoint == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	//parse out the ones with the right property
	int count = getArrayLength(*regionsAtPoint);
	std::vector<const Region*> result;
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr region = getObjectArrayElement(*regionsAtPoint, i);
		const Region* r = 0;
		if (!ScriptConversion::convert(*region, r))
			return 0;
		if(r->getMunicipal() == value)
			result.push_back(r);
	}

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return nullptr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *it;
		if (r != nullptr)
		{
			if(!ScriptConversion::convert(*r, javaRegion))
				return nullptr;
		}
		setObjectArrayElement(*regions, index, *javaRegion);
	}
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Return an array of all the regions with the given value for the Geographical property that cover the given point
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithGeographicalAtPoint(JNIEnv *env, jobject self, jobject location, jint value)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::getRegionsWithGeographicalAtPoint");
	//get all the regions at the given point
	LocalObjectArrayRefPtr regionsAtPoint = _getRegionsAtPoint(location);
	if (regionsAtPoint.get() == nullptr || regionsAtPoint == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	//parse out the ones with the right property
	int count = getArrayLength(*regionsAtPoint);
	std::vector<const Region*> result;
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr region = getObjectArrayElement(*regionsAtPoint, i);
		const Region* r = 0;
		if (!ScriptConversion::convert(*region, r))
			return 0;
		if(r->getGeography() == value)
			result.push_back(r);
	}

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return nullptr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *it;
		if (r != nullptr)
		{
			if(!ScriptConversion::convert(*r, javaRegion))
				return nullptr;
		}
		setObjectArrayElement(*regions, index, *javaRegion);
	}
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Return an array of all the regions with the given value for the Difficulty property that cover the given point
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithDifficultyAtPoint(JNIEnv *env, jobject self, jobject location, jint value)
{
	//get all the regions at the given point
	LocalObjectArrayRefPtr regionsAtPoint = _getRegionsAtPoint(location);
	if (regionsAtPoint.get() == nullptr || regionsAtPoint == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	//parse out the ones with the right property
	int count = getArrayLength(*regionsAtPoint);
	std::vector<const Region*> result;
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr region = getObjectArrayElement(*regionsAtPoint, i);
		const Region* r = 0;
		if (!ScriptConversion::convert(*region, r))
			return 0;
		if (value >= r->getMinDifficulty() && value <= r->getMaxDifficulty())
			result.push_back(r);
	}

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return nullptr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *it;
		if (r != nullptr)
		{
			if(!ScriptConversion::convert(*r, javaRegion))
				return nullptr;
		}
		setObjectArrayElement(*regions, index, *javaRegion);
	}
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Return an array of all the regions with the given value for the Spawnable property that cover the given point
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithSpawnableAtPoint(JNIEnv *env, jobject self, jobject location, jint value)
{
	//get all the regions at the given point
	LocalObjectArrayRefPtr regionsAtPoint = _getRegionsAtPoint(location);
	if (regionsAtPoint.get() == nullptr || regionsAtPoint == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	//parse out the ones with the right property
	int count = getArrayLength(*regionsAtPoint);
	std::vector<const Region*> result;
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr region = getObjectArrayElement(*regionsAtPoint, i);
		const Region* r = 0;
		if (!ScriptConversion::convert(*region, r))
			return 0;
		if (r->getSpawn() == value)
			result.push_back(r);
	}

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return nullptr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *it;
		if (r != nullptr)
		{
			if(!ScriptConversion::convert(*r, javaRegion))
				return nullptr;
		}
		setObjectArrayElement(*regions, index, *javaRegion);
	}
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Return an array of all the regions with the given value for the Mission property that cover the given point
 */
jobjectArray JNICALL ScriptMethodsRegionNamespace::getRegionsWithMissionAtPoint(JNIEnv *env, jobject self, jobject location, jint value)
{
	//get all the regions at the given point
	LocalObjectArrayRefPtr regionsAtPoint = _getRegionsAtPoint(location);
	if (regionsAtPoint.get() == nullptr || regionsAtPoint == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	//parse out the ones with the right property
	int count = getArrayLength(*regionsAtPoint);
	std::vector<const Region*> result;
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr region = getObjectArrayElement(*regionsAtPoint, i);
		const Region* r = 0;
		if (!ScriptConversion::convert(*region, r))
			return 0;
		if(r->getMission() == value)
			result.push_back(r);
	}

	//-- return nullptr instead of a zero-length array
	if (result.empty ())
		return nullptr;

	//put them into the jobjectarray
	LocalObjectArrayRefPtr regions = createNewObjectArray(result.size(), JavaLibrary::getClsRegion());

	int index = 0;
	for(std::vector<const Region*>::iterator it = result.begin(); it != result.end(); ++it, ++index)
	{
		LocalRefPtr javaRegion;
		const Region * r = *it;
		if (r != nullptr)
		{
			if(!ScriptConversion::convert(*r, javaRegion))
				return nullptr;
		}
		setObjectArrayElement(*regions, index, *javaRegion);
	}
	return regions->getReturnValue();
}

//-----------------------------------------------------------------------

/** Get the "most appropriate" region at the given location.  Specifically, return the region that:
 *  1: has a value for its fictional name, and
 *  2: of the above, has the smallest area
 */
jobject JNICALL ScriptMethodsRegionNamespace::getSmallestRegionAtPoint(JNIEnv *env, jobject self,
	jobject location)
{
	Vector locationVec;
	std::string sceneId;
	if(!ScriptConversion::convertWorld(location, locationVec, sceneId))
		return nullptr;

	const Region * r = RegionMaster::getSmallestRegionAtPoint(sceneId, locationVec.x,
		locationVec.z);
	if (r == nullptr)
		return nullptr;

	LocalRefPtr region;
	if (!ScriptConversion::convert(*r, region))
		return nullptr;
	return region->getReturnValue();
}

//-----------------------------------------------------------------------

/** Same as getSmallestRegionAtPoint, but the region must be flagged as visible.
 */
jobject JNICALL ScriptMethodsRegionNamespace::getSmallestVisibleRegionAtPoint(JNIEnv *env, jobject self,
	jobject location)
{
	Vector locationVec;
	std::string sceneId;
	if(!ScriptConversion::convertWorld(location, locationVec, sceneId))
		return nullptr;

	const Region * r = RegionMaster::getSmallestVisibleRegionAtPoint(sceneId, locationVec.x,
		locationVec.z);
	if (r == nullptr)
		return nullptr;

	LocalRefPtr region;
	if (!ScriptConversion::convert(*r, region))
		return nullptr;
	return region->getReturnValue();
}

//-----------------------------------------------------------------------

/** Checks to see if a region is a notify region
* @return true or false
*/
jboolean JNICALL ScriptMethodsRegionNamespace::isNotifyRegion(JNIEnv *env, jobject self, jobject region)
{
	UNREF(self);
	if (region == 0)
		return JNI_FALSE;

	const Region* r = 0;
	if (!ScriptConversion::convert(region, r))
		return JNI_FALSE;

	return (r->getNotify());
}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

/** Create a circular region given the location and radius and spawn creatures with it!
*/
jlong JNICALL ScriptMethodsRegionNamespace::createCircleRegionWithSpawn(JNIEnv *env, jobject self,
															  jobject center, jfloat radius, jstring name, jint pvp, jint buildable,
															  jint municipal, jint geography, jint minDifficulty, jint maxDifficulty,
															  jint spawnable, jint mission, jboolean visible, jboolean notify, jstring spawnTable, jint duration)
{
	UNREF(self);

	// validate scripter's input
	if (center == nullptr || radius <= 0 || name == 0)
		return 0;

	JavaStringParam jname(name);
	Unicode::String regionName;
	if (!JavaLibrary::convert(jname, regionName))
		return 0;

	// pull the data from the java Location objects
	Vector locationVec;
	std::string sceneId;
	if (!ScriptConversion::convertWorld(center, locationVec, sceneId))
		return 0;

	JavaStringParam spawn(spawnTable);
	Unicode::String spawnDataTable;
	if(!JavaLibrary::convert(spawn, spawnDataTable))
		return 0;

	// create the region
	NetworkId regionObject = RegionMaster::createNewDynamicRegionWithSpawn(locationVec.x, locationVec.y, locationVec.z,
		radius, regionName, sceneId, pvp, buildable, municipal, geography, minDifficulty,
		maxDifficulty, spawnable, mission, visible, notify, Unicode::wideToNarrow(spawnDataTable), duration);

	return regionObject.getValue();
}
