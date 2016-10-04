// ======================================================================
//
// ScriptMethodsRegion3d.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/Region3d.h"
#include "serverGame/Region3dMaster.h"
#include "serverGame/RegionBox.h"
#include "serverGame/RegionSphere.h"
#include "serverScript/ScriptParameters.h"

#include <algorithm>

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsRegion3dNamespace
// ======================================================================

namespace ScriptMethodsRegion3dNamespace
{
	bool install();
	ScriptParams *convertRegionDictionaryToScriptParams(jobject regionDictionary);

	jobject      JNICALL get3dRegionByName(JNIEnv *env, jobject self, jstring regionName);
	jobject      JNICALL getSmallest3dRegionAtPoint(JNIEnv *env, jobject self, jobject location);
	jobjectArray JNICALL get3dRegionsAtPoint(JNIEnv *env, jobject self, jobject location);
	void         JNICALL createSphereRegion(JNIEnv *env, jobject self, jstring regionName, jobject centerJ, jfloat radius, jobject regionDictionary);
	void         JNICALL createBoxRegion(JNIEnv *env, jobject self, jstring regionName, jobject extentMinJ, jobject extentMaxJ, jobject regionDictionary);
	void         JNICALL destroy3dRegion(JNIEnv *env, jobject self, jstring regionName);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsRegion3dNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsRegion3dNamespace::c)}
	JF("get3dRegionByName", "(Ljava/lang/String;)Lscript/dictionary;", get3dRegionByName),
	JF("getSmallest3dRegionAtPoint", "(Lscript/location;)Lscript/dictionary;", getSmallest3dRegionAtPoint),
	JF("get3dRegionsAtPoint", "(Lscript/location;)[Lscript/dictionary;", get3dRegionsAtPoint),
	JF("createSphereRegion", "(Ljava/lang/String;Lscript/vector;FLscript/dictionary;)V", createSphereRegion),
	JF("createBoxRegion", "(Ljava/lang/String;Lscript/vector;Lscript/vector;Lscript/dictionary;)V", createBoxRegion),
	JF("destroy3dRegion", "(Ljava/lang/String;)V", destroy3dRegion),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

static JavaDictionaryPtr get3dRegionScriptDataReturnValue(Region3d const *region)
{
	if (region)
	{
		ScriptParams const *params = region->getRegionScriptData();
		if (params)
		{
			JavaDictionaryPtr dict;
			JavaLibrary::instance()->convert(*params, dict);
			return dict;
		}
	}
	return JavaDictionary::cms_nullPtr;
}

// ----------------------------------------------------------------------

struct Region3dPtrVolumeComparator
{
	bool operator()(Region3d const *lhs, Region3d const *rhs) const
	{
		return lhs->getVolume() < rhs->getVolume();
	}
};

// ======================================================================

ScriptParams *ScriptMethodsRegion3dNamespace::convertRegionDictionaryToScriptParams(jobject regionDictionary)
{
	// If we're passed a nullptr dictionary, we just don't have any extra data but
	// we still want to return a ScriptParams to fill in the standard values.
	if (!regionDictionary)
		return new ScriptParams;

	LocalRefParam regionDictionaryParam(regionDictionary);

	if (!isInstanceOf(regionDictionaryParam, JavaLibrary::getClsDictionary()))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::convertRegionDictionaryToScriptParams bad regionDictionary"));
		return 0;
	}

	ScriptParams *regionScriptData = new ScriptParams;

	LocalRefPtr keyEnumeration = callObjectMethod(regionDictionaryParam, JavaLibrary::getMidDictionaryKeys());
	while (callBooleanMethod(*keyEnumeration, JavaLibrary::getMidEnumerationHasMoreElements()))
	{
		LocalRefPtr key = callObjectMethod(*keyEnumeration, JavaLibrary::getMidEnumerationNextElement());
		if (!isInstanceOf(*key, JavaLibrary::getClsString()))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::convertRegionDictionaryToScriptParams regionDictionary has bad key"));
			delete regionScriptData;
			return 0;
		}
		JavaStringParam localKeyStr(*key);
		std::string keyStr;
		JavaLibrary::convert(localKeyStr, keyStr);
		LocalRefPtr value = callObjectMethod(regionDictionaryParam, JavaLibrary::getMidDictionaryGet(), key->getValue());
		if (isInstanceOf(*value, JavaLibrary::getClsInteger()))
		{
			jint intValue = callIntMethod(*value, JavaLibrary::getMidIntegerIntValue());
			regionScriptData->addParam(intValue, keyStr, true);
		}
		else if (isInstanceOf(*value, JavaLibrary::getClsFloat()))
		{
			jfloat floatValue = callFloatMethod(*value, JavaLibrary::getMidFloatFloatValue());
			regionScriptData->addParam(floatValue, keyStr, true);
		}
		else if (isInstanceOf(*value, JavaLibrary::getClsBoolean()))
		{
			jboolean booleanValue = callBooleanMethod(*value, JavaLibrary::getMidBooleanBooleanValue());
			regionScriptData->addParam(booleanValue, keyStr, true);
		}
		else if (isInstanceOf(*value, JavaLibrary::getClsString()))
		{
			JavaStringParam localValueStr(*value);
			std::string valueStr;
			JavaLibrary::convert(localValueStr, valueStr);
			char *s = new char[valueStr.size()+1];
			strcpy(s, valueStr.c_str());
			regionScriptData->addParam(s, keyStr, true);
		}
		else
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::convertRegionDictionaryToScriptParams bad type for regionDictionary value, key=%s", keyStr.c_str()));
			delete regionScriptData;
			return 0;
		}
	}

	return regionScriptData;
}

// ======================================================================

jobject JNICALL ScriptMethodsRegion3dNamespace::get3dRegionByName(JNIEnv *env, jobject self, jstring regionName)
{
	JavaStringParam regionNameParam(regionName);
	std::string regionNameString;
	if (JavaLibrary::convert(regionNameParam, regionNameString))
		return get3dRegionScriptDataReturnValue(Region3dMaster::getRegionByName(regionNameString))->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsRegion3dNamespace::getSmallest3dRegionAtPoint(JNIEnv *env, jobject self, jobject location)
{
	Vector target;
	if (ScriptConversion::convertWorld(location, target))
		return get3dRegionScriptDataReturnValue(Region3dMaster::getSmallestRegionAtPoint(target))->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsRegion3dNamespace::get3dRegionsAtPoint(JNIEnv *env, jobject self, jobject location)
{
	Vector target;
	if (ScriptConversion::convertWorld(location, target))
	{
		std::vector<Region3d const *> results;
		Region3dMaster::getRegionsAtPoint(target, results);
		if (!results.empty())
		{
			std::sort(results.begin(), results.end(), Region3dPtrVolumeComparator());
			LocalObjectArrayRefPtr regions = createNewObjectArray(results.size(), JavaLibrary::getClsDictionary());
			for (unsigned int i = 0; i < results.size(); ++i)
				setObjectArrayElement(*regions, i, *get3dRegionScriptDataReturnValue(results[i]));
			return regions->getReturnValue();
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsRegion3dNamespace::createSphereRegion(JNIEnv *env, jobject self, jstring regionName, jobject centerJ, jfloat radius, jobject regionDictionary)
{
	JavaStringParam localRegionNameStr((jstring)regionName);
	std::string regionNameStr;
	JavaLibrary::convert(localRegionNameStr, regionNameStr);

	Vector center;
	if (!ScriptConversion::convert(centerJ, center))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::createSphereRegion bad center vector, region '%s'", regionNameStr.c_str()));
	else if (radius <= 0.0f)
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::createSphereRegion non-positive radius, region '%s'", regionNameStr.c_str()));
	else
	{
		ScriptParams *regionScriptData = convertRegionDictionaryToScriptParams(regionDictionary);
		if (regionScriptData)
		{
			char *s = new char[regionNameStr.size()+1];
			strcpy(s, regionNameStr.c_str());
			regionScriptData->addParam(s, "regionName", true);
			regionScriptData->addParam(0, "geometry", true);
			regionScriptData->addParam(center.x, "x1", true);
			regionScriptData->addParam(center.y, "y1", true);
			regionScriptData->addParam(center.z, "z1", true);
			regionScriptData->addParam(static_cast<float>(radius), "radius", true);
			IGNORE_RETURN(new RegionSphere(regionNameStr, center, radius, regionScriptData));
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsRegion3dNamespace::createBoxRegion(JNIEnv *env, jobject self, jstring regionName, jobject extentMinJ, jobject extentMaxJ, jobject regionDictionary)
{
	JavaStringParam localRegionNameStr((jstring)regionName);
	std::string regionNameStr;
	JavaLibrary::convert(localRegionNameStr, regionNameStr);

	Vector extentMin, extentMax;
	if (!ScriptConversion::convert(extentMinJ, extentMin))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::createBoxRegion bad extentMin vector, region '%s'", regionNameStr.c_str()));
	else if (!ScriptConversion::convert(extentMaxJ, extentMax))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::createBoxRegion bad extentMax vector, region '%s'", regionNameStr.c_str()));
	else if (extentMax.x <= extentMin.x || extentMax.y <= extentMin.y || extentMax.z <= extentMin.z)
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::createBoxRegion has inverted coordinates, region '%s'", regionNameStr.c_str()));
	else
	{
		ScriptParams *regionScriptData = convertRegionDictionaryToScriptParams(regionDictionary);
		if (regionScriptData)
		{
			char *s = new char[regionNameStr.size()+1];
			strcpy(s, regionNameStr.c_str());
			regionScriptData->addParam(s, "regionName", true);
			regionScriptData->addParam(1, "geometry", true);
			regionScriptData->addParam(extentMin.x, "x1", true);
			regionScriptData->addParam(extentMin.y, "y1", true);
			regionScriptData->addParam(extentMin.z, "z1", true);
			regionScriptData->addParam(extentMax.x, "x2", true);
			regionScriptData->addParam(extentMax.y, "y2", true);
			regionScriptData->addParam(extentMax.z, "z2", true);
			IGNORE_RETURN(new RegionBox(regionNameStr, extentMin, extentMax, regionScriptData));
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsRegion3dNamespace::destroy3dRegion(JNIEnv *env, jobject self, jstring regionName)
{
	JavaStringParam localRegionNameStr((jstring)regionName);
	std::string regionNameStr;
	JavaLibrary::convert(localRegionNameStr, regionNameStr);
	
	Region3d const *reg = Region3dMaster::getRegionByName(regionNameStr);
	delete reg;
}

// ======================================================================

