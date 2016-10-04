//========================================================================
//
// ScriptMethodsMap.cpp - implements script methods dealing with bank
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/PlanetMapManagerServer.h"
#include "serverGame/ServerObject.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "UnicodeUtils.h"
#include <vector>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsMapNamespace
// ======================================================================

namespace ScriptMethodsMapNamespace
{
	bool install();

	bool         JNICALL addPlanetaryMapLocation     (JNIEnv *env, jobject script, jlong locationId, jstring locationName, jint xLoc, jint yLoc, jstring category, jstring subCategory, jint mapLocationType, jbyte flags);
	bool         JNICALL addPlanetaryMapLocationIgnoreLocationCountLimits(JNIEnv *env, jobject script, jlong locationId, jstring locationName, jint xLoc, jint yLoc, jstring category, jstring subCategory, jint mapLocationType, jbyte flags);
	bool         JNICALL removePlanetaryMapLocation  (JNIEnv *env, jobject script, jlong locationId);
	jobjectArray JNICALL getPlanetaryMapLocations    (JNIEnv *env, jobject script, jstring category, jstring subCategory);
	jobjectArray JNICALL getPlanetaryMapCategories   (JNIEnv *env, jobject script, jstring category);
	jobject      JNICALL getPlanetaryMapLocation     (JNIEnv *env, jobject script, jlong locationId);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsMapNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsMapNamespace::c)}
	JF("_addPlanetaryMapLocation",    "(JLjava/lang/String;IILjava/lang/String;Ljava/lang/String;IB)Z",addPlanetaryMapLocation),
	JF("_addPlanetaryMapLocationIgnoreLocationCountLimits", "(JLjava/lang/String;IILjava/lang/String;Ljava/lang/String;IB)Z",addPlanetaryMapLocationIgnoreLocationCountLimits),
  	JF("_removePlanetaryMapLocation", "(J)Z",                                                          removePlanetaryMapLocation),
  	JF("getPlanetaryMapLocations",   "(Ljava/lang/String;Ljava/lang/String;)[Lscript/map_location;",   getPlanetaryMapLocations),
  	JF("getPlanetaryMapCategories",  "(Ljava/lang/String;)[Ljava/lang/String;",                        getPlanetaryMapCategories),
  	JF("_getPlanetaryMapLocation",    "(J)Lscript/map_location;",                                      getPlanetaryMapLocation),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ----------------------------------------------------------------------

bool JNICALL ScriptMethodsMapNamespace::addPlanetaryMapLocation(
	JNIEnv *env, jobject script, jlong locationId, jstring locationName, jint xLoc, jint yLoc, jstring jCategoryName, jstring jSubCategoryName, jint    mapLocationType, jbyte   flags)
{

	if (mapLocationType < 0 || mapLocationType >= PlanetMapManager::MLT_numTypes)
	{
		WARNING(true, ("[script bug] invalid mapLocationType [%d] passed to addPlanetaryMapLocation", mapLocationType));
		return false;
	}

	NetworkId id(locationId);
	if (id == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("[script bug] invalid locationId passed to addPlanetaryMapLocation"));
		return false;
	}

	JavaStringParam localLocationName(locationName);
	Unicode::String wideLocationName;
	if(!JavaLibrary::convert(localLocationName, wideLocationName))
	{
		DEBUG_WARNING(true, ("[script bug] invalid locationName passed to addPlanetaryMapLocation"));
		return false;
	}

	if (wideLocationName.empty ())
	{
		DEBUG_WARNING (true, ("[script bug] empty locationName passed to addPlanetaryMapLocation"));
		return false;
	}

	JavaStringParam localCategory(jCategoryName);
	std::string categoryName;
	if(!JavaLibrary::convert(localCategory, categoryName))
	{
		DEBUG_WARNING(true, ("[script bug] invalid category passed to addPlanetaryMapLocation"));
		return false;
	}

	std::string subCategoryName;
	if (jSubCategoryName)
	{
		JavaStringParam localSubCategory(jSubCategoryName);
		if(!JavaLibrary::convert(localSubCategory, subCategoryName))
		{
			DEBUG_WARNING(true, ("[script bug] invalid SubCategory passed to addPlanetaryMapLocation"));
			return false;
		}
	}

	const uint8 category = PlanetMapManager::findCategory (categoryName);
	if (!category)
	{
		WARNING(true, ("[script bug] invalid category [%s] passed to addPlanetaryMapLocation", categoryName.c_str ()));
		return false;

	}

	const uint8 subCategory = PlanetMapManager::findCategory (subCategoryName);

	if (!subCategory && !subCategoryName.empty ())
	{
		WARNING(true, ("[script bug] invalid subcategory [%s] passed to addPlanetaryMapLocation", categoryName.c_str ()));
		return false;
	}

	const Vector2d v2d (static_cast<float>(xLoc), static_cast<float>(yLoc));

	bool result = false;
	const MapLocation loc = PlanetMapManager::makeMapLocation (id, wideLocationName, v2d, categoryName, subCategoryName, flags, result);

	if (!result)
	{
		WARNING(true, ("[script bug] invalid location passed to addPlanetaryMapLocation"));
		return false;
	}

	PlanetMapManagerServer::addMapLocation (loc, mapLocationType, true);
	return true;
}

// ----------------------------------------------------------------------

bool JNICALL ScriptMethodsMapNamespace::addPlanetaryMapLocationIgnoreLocationCountLimits(
	JNIEnv *env, jobject script, jlong locationId, jstring locationName, jint xLoc, jint yLoc, jstring jCategoryName, jstring jSubCategoryName, jint    mapLocationType, jbyte   flags)
{

	if (mapLocationType < 0 || mapLocationType >= PlanetMapManager::MLT_numTypes)
	{
		WARNING(true, ("[script bug] invalid mapLocationType [%d] passed to addPlanetaryMapLocationIgnoreLocationCountLimits", mapLocationType));
		return false;
	}

	NetworkId id(locationId);
	if (id == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("[script bug] invalid locationId passed to addPlanetaryMapLocationIgnoreLocationCountLimits"));
		return false;
	}

	JavaStringParam localLocationName(locationName);
	Unicode::String wideLocationName;
	if(!JavaLibrary::convert(localLocationName, wideLocationName))
	{
		DEBUG_WARNING(true, ("[script bug] invalid locationName passed to addPlanetaryMapLocationIgnoreLocationCountLimits"));
		return false;
	}

	if (wideLocationName.empty ())
	{
		DEBUG_WARNING (true, ("[script bug] empty locationName passed to addPlanetaryMapLocationIgnoreLocationCountLimits"));
		return false;
	}

	JavaStringParam localCategory(jCategoryName);
	std::string categoryName;
	if(!JavaLibrary::convert(localCategory, categoryName))
	{
		DEBUG_WARNING(true, ("[script bug] invalid category passed to addPlanetaryMapLocationIgnoreLocationCountLimits"));
		return false;
	}

	std::string subCategoryName;
	if (jSubCategoryName)
	{
		JavaStringParam localSubCategory(jSubCategoryName);
		if(!JavaLibrary::convert(localSubCategory, subCategoryName))
		{
			DEBUG_WARNING(true, ("[script bug] invalid SubCategory passed to addPlanetaryMapLocationIgnoreLocationCountLimits"));
			return false;
		}
	}

	const uint8 category = PlanetMapManager::findCategory (categoryName);
	if (!category)
	{
		WARNING(true, ("[script bug] invalid category [%s] passed to addPlanetaryMapLocationIgnoreLocationCountLimits", categoryName.c_str ()));
		return false;

	}

	const uint8 subCategory = PlanetMapManager::findCategory (subCategoryName);

	if (!subCategory && !subCategoryName.empty ())
	{
		WARNING(true, ("[script bug] invalid subcategory [%s] passed to addPlanetaryMapLocationIgnoreLocationCountLimits", categoryName.c_str ()));
		return false;
	}

	const Vector2d v2d (static_cast<float>(xLoc), static_cast<float>(yLoc));

	bool result = false;
	const MapLocation loc = PlanetMapManager::makeMapLocation (id, wideLocationName, v2d, categoryName, subCategoryName, flags, result);

	if (!result)
	{
		WARNING(true, ("[script bug] invalid location passed to addPlanetaryMapLocationIgnoreLocationCountLimits"));
		return false;
	}

	PlanetMapManagerServer::addMapLocation (loc, mapLocationType, false);
	return true;
}

//----------------------------------------------------------------------

bool JNICALL ScriptMethodsMapNamespace::removePlanetaryMapLocation(JNIEnv *env, jobject script, jlong locationId)
{
	NetworkId id(locationId);
	if (id == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("[script bug] invalid locationId passed to removePlanetaryMapLocation"));
		return false;
	}

	PlanetMapManagerServer::removeMapLocation (id);
	return true;
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsMapNamespace::getPlanetaryMapLocations(
	JNIEnv *env,
	jobject script,
	jstring jCategoryName,
	jstring jSubCategoryName)

{
	std::string categoryName;
	if (jCategoryName)
	{
		JavaStringParam localCategory(jCategoryName);
		if(!JavaLibrary::convert(localCategory, categoryName))
		{
			DEBUG_WARNING(true, ("[script bug] invalid category passed to getPlanetaryMapLocations"));
			return nullptr;
		}
	}

	std::string subCategoryName;
	if (jSubCategoryName)
	{
		JavaStringParam localSubCategory(jSubCategoryName);
		if(!JavaLibrary::convert(localSubCategory, subCategoryName))
		{
			DEBUG_WARNING(true, ("[script bug] invalid SubCategory passed to getPlanetaryMapLocations"));
			return nullptr;
		}
	}

	static PlanetMapManagerServer::MapLocationVector locs;
	locs.clear ();
	//@todo: this must be passed in from script
	PlanetMapManagerServer::getMapLocationsByCategories (PlanetMapManager::MLT_static, categoryName, subCategoryName, locs);

	LocalObjectArrayRefPtr jlocs = createNewObjectArray(locs.size(), JavaLibrary::getClsMapLocation());

	int index = 0;
	for (PlanetMapManagerServer::MapLocationVector::const_iterator i = locs.begin(); i < locs.end(); ++i, ++index)
	{
		const MapLocation & mapLocation = *i;

		jlong jLocationId = mapLocation.getLocationId().getValue();
		if (!jLocationId)
			return nullptr;

		JavaString jNameString(mapLocation.getLocationName());

		if (jNameString.getValue() == 0)
			return nullptr;

		LocalRefPtr jMapLocation = createNewObject(JavaLibrary::getClsMapLocation(),
			JavaLibrary::getMidMapLocation(),
			jLocationId, jNameString.getValue(),
			jCategoryName,
			jSubCategoryName,
			static_cast<jlong> (mapLocation.getLocation().x),
			static_cast<jlong> (mapLocation.getLocation().y),
			static_cast<jbyte> (mapLocation.getFlags()));

		if (jMapLocation == LocalRef::cms_nullPtr)
			return nullptr;

		setObjectArrayElement(*jlocs, index, *jMapLocation);
	}

	return jlocs->getReturnValue();
}

//----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsMapNamespace::getPlanetaryMapCategories   (JNIEnv *env, jobject , jstring jCategoryName)
{
	std::string categoryName;
	if (jCategoryName)
	{
		JavaStringParam localCategory(jCategoryName);
		if(!JavaLibrary::convert(localCategory, categoryName))
		{
			DEBUG_WARNING(true, ("[script bug] invalid category passed to getPlanetaryMapCategories"));
			return nullptr;
		}
	}

	const int theCategory = PlanetMapManager::findCategory (categoryName);

	static PlanetMapManagerServer::IntVector iv;
	iv.clear ();
	PlanetMapManagerServer::getCategories (theCategory, iv);

	LocalObjectArrayRefPtr jcats = createNewObjectArray (iv.size (), JavaLibrary::getClsString());

	size_t i = 0;
	for (PlanetMapManagerServer::IntVector::const_iterator it = iv.begin (); it != iv.end (); ++it, ++i)
	{
		const int cat = *it;
		const std::string & catName = PlanetMapManager::findCategoryName (static_cast<uint8>(cat));
		const JavaString jval(catName);
		setObjectArrayElement(*jcats, i, jval);
	}

	return jcats->getReturnValue();
}

//----------------------------------------------------------------------

jobject      JNICALL ScriptMethodsMapNamespace::getPlanetaryMapLocation     (JNIEnv *env, jobject, jlong jLocationId)
{
	NetworkId id(jLocationId);
	if (id == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("[script bug] invalid locationId passed to getPlanetaryMapCategories"));
		return nullptr;
	}

	int mlt = 0;
	const MapLocation * const mapLocation = PlanetMapManagerServer::getLocation (id, mlt);

	if (!mapLocation)
		return nullptr;

	const JavaString jLocName            (mapLocation->m_locationName);
	const JavaString jLocCategoryName    (PlanetMapManager::findCategoryName (mapLocation->m_category));
	const JavaString jLocSubCategoryName (PlanetMapManager::findCategoryName (mapLocation->m_subCategory));

	LocalRefPtr jMapLocation = createNewObject(JavaLibrary::getClsMapLocation(),
		JavaLibrary::getMidMapLocation(),
		jLocationId,

		jLocName.getValue (),
		jLocCategoryName.getValue (),
		jLocSubCategoryName.getValue (),

		static_cast<jlong> (mapLocation->getLocation().x),
		static_cast<jlong> (mapLocation->getLocation().y),
		static_cast<jbyte> (mapLocation->getFlags()));

	return jMapLocation->getReturnValue();

}

//----------------------------------------------------------------------
