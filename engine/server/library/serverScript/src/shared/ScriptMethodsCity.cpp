// ======================================================================
//
// ScriptMethodsCity.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/CityInterface.h"
#include "serverGame/CityObject.h"
#include "serverGame/ServerUniverse.h"
#include "sharedGame/CitizenRankDataTable.h"

using namespace JNIWrappersNamespace;


// ======================================================================

static int closestInt(float f)
{
	if (f < 0.0f)
		return static_cast<int>(f-0.5f);
	else
		return static_cast<int>(f+0.5f);
}

// ======================================================================
// ScriptMethodsCityNamespace
// ======================================================================

namespace ScriptMethodsCityNamespace
{
	bool install();

	jint         JNICALL getCitizenOfCityId(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getLocatedInCityId(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getMilitiaOfCityId(JNIEnv *env, jobject self, jlong target);
	jintArray    JNICALL getAllCityIds(JNIEnv *env, jobject self);
	jlong        JNICALL getMasterCityObject(JNIEnv *env, jobject self);
	jint         JNICALL findCityByName(JNIEnv *env, jobject self, jstring cityName);
	jint         JNICALL findCityByCityHall(JNIEnv *env, jobject self, jlong cityHallId);
	jint         JNICALL getCityAtLocation(JNIEnv *env, jobject self, jobject cityLocation, jint radius);
	jint         JNICALL createCity(JNIEnv *env, jobject self, jstring cityName, jlong cityHallId, jobject cityLocation, jint radius, jlong leaderId, jint incomeTax, jint propertyTax, jint salesTax, jobject travelLocation, jint travelCost, jboolean travelInterplanetary, jobject cloneLoc, jobject cloneRespawn, jlong cloneId);
	void         JNICALL removeCity(JNIEnv *env, jobject self, jint cityId);
	jboolean     JNICALL cityExists(JNIEnv *env, jobject self, jint cityId);
	jstring      JNICALL cityGetName(JNIEnv *env, jobject self, jint cityId);
	jlong        JNICALL cityGetCityHall(JNIEnv *env, jobject self, jint cityId);
	jobject      JNICALL cityGetLocation(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetRadius(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetFaction(JNIEnv *env, jobject self, jint cityId);
	jstring      JNICALL cityGetGcwDefenderRegion(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetTimeJoinedGcwDefenderRegion(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetCreationTime(JNIEnv *env, jobject self, jint cityId);
	jlong        JNICALL cityGetLeader(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetIncomeTax(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetPropertyTax(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetSalesTax(JNIEnv *env, jobject self, jint cityId);
	jobject      JNICALL cityGetTravelLocation(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetTravelCost(JNIEnv *env, jobject self, jint cityId);
	jboolean     JNICALL cityGetTravelInterplanetary(JNIEnv *env, jobject self, jint cityId);
	jobject      JNICALL cityGetCloneLoc(JNIEnv *env, jobject self, jint cityId);
	jobject      JNICALL cityGetCloneRespawn(JNIEnv *env, jobject self, jint cityId);
	jlong        JNICALL cityGetCloneId(JNIEnv *env, jobject self, jint cityId);
	jlongArray   JNICALL cityGetCitizenIds(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetCitizenCount(JNIEnv *env, jobject self, jint cityId);
	jstring      JNICALL cityGetCitizenName(JNIEnv *env, jobject self, jint cityId, jlong citizenId);
	jstring      JNICALL cityGetCitizenProfession(JNIEnv *env, jobject self, jint cityId, jlong citizenId);
	jint         JNICALL cityGetCitizenLevel(JNIEnv *env, jobject self, jint cityId, jlong citizenId);
	jstring      JNICALL cityGetCitizenTitle(JNIEnv *env, jobject self, jint cityId, jlong citizenId);
	jlong        JNICALL cityGetCitizenAllegiance(JNIEnv *env, jobject self, jint cityId, jlong citizenId);
	jint         JNICALL cityGetCitizenPermissions(JNIEnv *env, jobject self, jint cityId, jlong citizenId);
	jlongArray   JNICALL cityGetStructureIds(JNIEnv *env, jobject self, jint cityId);
	jint         JNICALL cityGetStructureType(JNIEnv *env, jobject self, jint cityId, jlong structureId);
	jboolean     JNICALL cityGetStructureValid(JNIEnv *env, jobject self, jint cityId, jlong structureId);
	void         JNICALL citySetName(JNIEnv *env, jobject self, jint cityId, jstring cityName);
	void         JNICALL citySetCityHall(JNIEnv *env, jobject self, jint cityId, jlong cityHallId);
	void         JNICALL citySetLocation(JNIEnv *env, jobject self, jint cityId, jobject cityLocation);
	void         JNICALL citySetRadius(JNIEnv *env, jobject self, jint cityId, jint radius);
	void         JNICALL citySetFaction(JNIEnv *env, jobject self, jint cityId, jint factionId, jboolean notifyCitizens);
	void         JNICALL citySetGcwDefenderRegion(JNIEnv *env, jobject self, jint cityId, jstring gcwDefenderRegion, jint timeJoined, jboolean notifyCitizens);
	void         JNICALL citySetLeader(JNIEnv *env, jobject self, jint cityId, jlong leaderId);
	void         JNICALL citySetIncomeTax(JNIEnv *env, jobject self, jint cityId, jint incomeTax);
	void         JNICALL citySetPropertyTax(JNIEnv *env, jobject self, jint cityId, jint propertyTax);
	void         JNICALL citySetSalesTax(JNIEnv *env, jobject self, jint cityId, jint salesTax);
	void         JNICALL citySetTravelInfo(JNIEnv *env, jobject self, jint cityId, jobject travelLocation, jint travelCost, jboolean travelInterplanetary);
	void         JNICALL citySetCloneInfo(JNIEnv *env, jobject self, jint cityId, jobject cloneLoc, jobject cloneRespawn, jlong cloneId);
	void         JNICALL citySetCitizenInfo(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring citizenName, jlong allegiance, jint permissions);
	void         JNICALL citySetCitizenTitle(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring title);
	jobjectArray JNICALL cityGetAllCitizenRanks(JNIEnv *env, jobject self);
	jobjectArray JNICALL cityGetTitleForCitizenRank(JNIEnv *env, jobject self, jstring rank);
	jobjectArray JNICALL cityGetCitizenRank(JNIEnv *env, jobject self, jint cityId, jlong citizenId);
	jboolean     JNICALL cityHasCitizenRank(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring rank);
	void         JNICALL cityAddCitizenRank(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring rank);
	void         JNICALL cityRemoveCitizenRank(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring rank);
	void         JNICALL citySetStructureInfo(JNIEnv *env, jobject self, jint cityId, jlong structureId, jint structureType, jboolean structureValid);
	void         JNICALL cityRemoveCitizen(JNIEnv *env, jobject self, jint cityId, jlong citizenId);
	void         JNICALL cityRemoveStructure(JNIEnv *env, jobject self, jint cityId, jlong structureId);
	jboolean     JNICALL cityIsInactivePackupActive(JNIEnv *env, jobject self);
	jint         JNICALL cityGetInactivePackupInactiveTimeSeconds(JNIEnv *env, jobject self);
	jlongArray   JNICALL pgcGetChroniclerId(JNIEnv *env, jobject self, jstring chroniclerName);
	jstring      JNICALL pgcGetChroniclerName(JNIEnv *env, jobject self, jlong chroniclerId);
	jintArray    JNICALL pgcGetRating(JNIEnv *env, jobject self, jlong chroniclerId);
	void         JNICALL pgcAdjustRating(JNIEnv *env, jobject self, jlong chroniclerId, jstring chroniclerName, jint adjustment);
	jintArray    JNICALL pgcGetRatingData(JNIEnv *env, jobject self, jlong chroniclerId);
	void         JNICALL pgcAdjustRatingData(JNIEnv *env, jobject self, jlong chroniclerId, jstring chroniclerName, jint index, jint adjustment);
	void         JNICALL pgcSetRatingData(JNIEnv *env, jobject self, jlong chroniclerId, jstring chroniclerName, jint index, jint value);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsCityNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsCityNamespace::c)}
	JF("_getCitizenOfCityId", "(J)I", getCitizenOfCityId),
	JF("_getLocatedInCityId", "(J)I", getLocatedInCityId),
	JF("_getMilitiaOfCityId", "(J)I", getMilitiaOfCityId),
	JF("getAllCityIds", "()[I", getAllCityIds),
	JF("_getMasterCityObject", "()J", getMasterCityObject),
	JF("findCityByName", "(Ljava/lang/String;)I", findCityByName),
	JF("_findCityByCityHall", "(J)I", findCityByCityHall),
	JF("getCityAtLocation", "(Lscript/location;I)I", getCityAtLocation),
	JF("_createCity", "(Ljava/lang/String;JLscript/location;IJIIILscript/location;IZLscript/location;Lscript/location;J)I", createCity),
	JF("removeCity", "(I)V", removeCity),
	JF("cityExists", "(I)Z", cityExists),
	JF("cityGetName", "(I)Ljava/lang/String;", cityGetName),
	JF("_cityGetCityHall", "(I)J", cityGetCityHall),
	JF("cityGetLocation", "(I)Lscript/location;", cityGetLocation),
	JF("cityGetRadius", "(I)I", cityGetRadius),
	JF("cityGetFaction", "(I)I", cityGetFaction),
	JF("cityGetGcwDefenderRegion", "(I)Ljava/lang/String;", cityGetGcwDefenderRegion),
	JF("cityGetTimeJoinedGcwDefenderRegion", "(I)I", cityGetTimeJoinedGcwDefenderRegion),
	JF("cityGetCreationTime", "(I)I", cityGetCreationTime),
	JF("_cityGetLeader", "(I)J", cityGetLeader),
	JF("cityGetIncomeTax", "(I)I", cityGetIncomeTax),
	JF("cityGetPropertyTax", "(I)I", cityGetPropertyTax),
	JF("cityGetSalesTax", "(I)I", cityGetSalesTax),
	JF("cityGetTravelLocation", "(I)Lscript/location;", cityGetTravelLocation),
	JF("cityGetTravelCost", "(I)I", cityGetTravelCost),
	JF("cityGetTravelInterplanetary", "(I)Z", cityGetTravelInterplanetary),
	JF("cityGetCloneLoc", "(I)Lscript/location;", cityGetCloneLoc),
	JF("cityGetCloneRespawn", "(I)Lscript/location;", cityGetCloneRespawn),
	JF("_cityGetCloneId", "(I)J", cityGetCloneId),
	JF("_cityGetCitizenIds", "(I)[J", cityGetCitizenIds),
	JF("cityGetCitizenCount", "(I)I", cityGetCitizenCount),
	JF("_cityGetCitizenName", "(IJ)Ljava/lang/String;", cityGetCitizenName),
	JF("_cityGetCitizenProfession", "(IJ)Ljava/lang/String;", cityGetCitizenProfession),
	JF("_cityGetCitizenLevel", "(IJ)I", cityGetCitizenLevel),
	JF("_cityGetCitizenTitle", "(IJ)Ljava/lang/String;", cityGetCitizenTitle),
	JF("_cityGetCitizenAllegiance", "(IJ)J", cityGetCitizenAllegiance),
	JF("_cityGetCitizenPermissions", "(IJ)I", cityGetCitizenPermissions),
	JF("_cityGetStructureIds", "(I)[J", cityGetStructureIds),
	JF("_cityGetStructureType", "(IJ)I", cityGetStructureType),
	JF("_cityGetStructureValid", "(IJ)Z", cityGetStructureValid),
	JF("citySetName", "(ILjava/lang/String;)V", citySetName),
	JF("_citySetCityHall", "(IJ)V", citySetCityHall),
	JF("citySetLocation", "(ILscript/location;)V", citySetLocation),
	JF("citySetRadius", "(II)V", citySetRadius),
	JF("citySetFaction", "(IIZ)V", citySetFaction),
	JF("citySetGcwDefenderRegion", "(ILjava/lang/String;IZ)V", citySetGcwDefenderRegion),
	JF("_citySetLeader", "(IJ)V", citySetLeader),
	JF("citySetIncomeTax", "(II)V", citySetIncomeTax),
	JF("citySetPropertyTax", "(II)V", citySetPropertyTax),
	JF("citySetSalesTax", "(II)V", citySetSalesTax),
	JF("citySetTravelInfo", "(ILscript/location;IZ)V", citySetTravelInfo),
	JF("_citySetCloneInfo", "(ILscript/location;Lscript/location;J)V", citySetCloneInfo),
	JF("_citySetCitizenInfo", "(IJLjava/lang/String;JI)V", citySetCitizenInfo),
	JF("_citySetCitizenTitle", "(IJLjava/lang/String;)V", citySetCitizenTitle),
	JF("cityGetAllCitizenRanks", "()[Ljava/lang/String;", cityGetAllCitizenRanks),
	JF("cityGetTitleForCitizenRank", "(Ljava/lang/String;)[Ljava/lang/String;", cityGetTitleForCitizenRank),
	JF("_cityGetCitizenRank", "(IJ)[Ljava/lang/String;", cityGetCitizenRank),
	JF("_cityHasCitizenRank", "(IJLjava/lang/String;)Z", cityHasCitizenRank),
	JF("_cityAddCitizenRank", "(IJLjava/lang/String;)V", cityAddCitizenRank),
	JF("_cityRemoveCitizenRank", "(IJLjava/lang/String;)V", cityRemoveCitizenRank),
	JF("_citySetStructureInfo", "(IJIZ)V", citySetStructureInfo),
	JF("_cityRemoveCitizen", "(IJ)V", cityRemoveCitizen),
	JF("_cityRemoveStructure", "(IJ)V", cityRemoveStructure),
	JF("cityIsInactivePackupActive", "()Z", cityIsInactivePackupActive),
	JF("cityGetInactivePackupInactiveTimeSeconds", "()I", cityGetInactivePackupInactiveTimeSeconds),
	JF("_pgcGetChroniclerId", "(Ljava/lang/String;)[J", pgcGetChroniclerId),
	JF("_pgcGetChroniclerName", "(J)Ljava/lang/String;", pgcGetChroniclerName),
	JF("_pgcGetRating", "(J)[I", pgcGetRating),
	JF("_pgcAdjustRating", "(JLjava/lang/String;I)V", pgcAdjustRating),
	JF("_pgcGetRatingData", "(J)[I", pgcGetRatingData),
	JF("_pgcAdjustRatingData", "(JLjava/lang/String;II)V", pgcAdjustRatingData),
	JF("_pgcSetRatingData", "(JLjava/lang/String;II)V", pgcSetRatingData),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jint JNICALL ScriptMethodsCityNamespace::getCitizenOfCityId(JNIEnv *env, jobject self, jlong target)
{
	std::vector<int> const & cityId = CityInterface::getCitizenOfCityId(NetworkId(target));
	if (!cityId.empty())
		return cityId.front();

	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::getLocatedInCityId(JNIEnv *env, jobject self, jlong target)
{
	CreatureObject *targetObj = 0;
	if (JavaLibrary::getObject(target, targetObj))
		return targetObj->getLocatedInCityId();
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::getMilitiaOfCityId(JNIEnv *env, jobject self, jlong target)
{
	CreatureObject *targetObj = 0;
	if (JavaLibrary::getObject(target, targetObj))
		return targetObj->getMilitiaOfCityId();
	return 0;
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsCityNamespace::getAllCityIds(JNIEnv *env, jobject self)
{
	std::vector<int> cityIds;
	CityInterface::getAllCityIds(cityIds);
	LocalIntArrayRefPtr ret = createNewIntArray(cityIds.size());
	if (!cityIds.empty())
		setIntArrayRegion(*ret, 0, cityIds.size(), reinterpret_cast<jint *>(&cityIds[0]));
	return ret->getReturnValue();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsCityNamespace::getMasterCityObject(JNIEnv *env, jobject self)
{
	CityObject *masterCityObject = ServerUniverse::getInstance().getMasterCityObject();
	return masterCityObject ? (masterCityObject->getNetworkId()).getValue() : (NetworkId::cms_invalid).getValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::findCityByName(JNIEnv *env, jobject self, jstring cityName)
{
	JavaStringParam localNameStr(cityName);
	std::string cityNameStr;
	if (JavaLibrary::convert(localNameStr, cityNameStr))
		return CityInterface::findCityByName(cityNameStr);
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::findCityByCityHall(JNIEnv *env, jobject self, jlong cityHallId)
{
	return CityInterface::findCityByCityHall(NetworkId(cityHallId));
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::getCityAtLocation(JNIEnv *env, jobject self, jobject cityLocation, jint radius)
{
	Vector cityPosition;
	std::string cityPlanet;
	NetworkId cell;
	ScriptConversion::convert(cityLocation, cityPosition, cityPlanet, cell);
	if (cell == NetworkId::cms_invalid)
		return CityInterface::getCityAtLocation(cityPlanet, closestInt(cityPosition.x), closestInt(cityPosition.z), radius);
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::createCity(JNIEnv *env, jobject self, jstring cityName, jlong cityHallId, jobject cityLocation, jint radius, jlong leaderId, jint incomeTax, jint propertyTax, jint salesTax, jobject travelLocation, jint travelCost, jboolean travelInterplanetary, jobject cloneLoc, jobject cloneRespawn, jlong cloneId)
{
	JavaStringParam localNameStr(cityName);
	std::string cityNameStr;
	if (JavaLibrary::convert(localNameStr, cityNameStr))
	{
		Vector cityPosition;
		std::string cityPlanet;
		NetworkId cityCell;
		ScriptConversion::convert(cityLocation, cityPosition, cityPlanet, cityCell);
		if (cityCell != NetworkId::cms_invalid)
			return 0;
		Vector travelPosition(Vector::zero);
		std::string travelPlanet;
		NetworkId travelCell;
		if (travelCost > 0)
		{
			ScriptConversion::convert(travelLocation, travelPosition, travelPlanet, travelCell);
			if (travelCell != NetworkId::cms_invalid)
			{
				travelCost = 0;
				travelPosition = Vector::zero;
			}
		}
		Vector cloneLocPosition(Vector::zero);
		std::string cloneLocPlanet;
		NetworkId cloneLocCell;
		ScriptConversion::convert(cloneLoc, cloneLocPosition, cloneLocPlanet, cloneLocCell);
		Vector cloneRespawnPosition(Vector::zero);
		std::string cloneRespawnPlanet;
		NetworkId cloneRespawnCell;
		ScriptConversion::convert(cloneRespawn, cloneRespawnPosition, cloneRespawnPlanet, cloneRespawnCell);

		return CityInterface::createCity(
			cityNameStr, NetworkId(cityHallId),
			cityPlanet, closestInt(cityPosition.x), closestInt(cityPosition.z), radius,
			NetworkId(leaderId),
			incomeTax, propertyTax, salesTax,
			travelPosition, travelCost, travelInterplanetary,
			cloneLocPosition, cloneRespawnPosition, cloneRespawnCell, NetworkId(cloneId));
	}
	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::removeCity(JNIEnv *env, jobject self, jint cityId)
{
	CityInterface::removeCity(cityId);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCityNamespace::cityExists(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::cityExists(cityId);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsCityNamespace::cityGetName(JNIEnv *env, jobject self, jint cityId)
{
	return JavaString(CityInterface::getCityInfo(cityId).getCityName().c_str()).getReturnValue();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsCityNamespace::cityGetCityHall(JNIEnv *env, jobject self, jint cityId)
{
	return (CityInterface::getCityInfo(cityId).getCityHallId()).getValue();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsCityNamespace::cityGetLocation(JNIEnv *env, jobject self, jint cityId)
{
	CityInfo const &ci = CityInterface::getCityInfo(cityId);
	LocalRefPtr location;
	if (ScriptConversion::convert(
		Vector(static_cast<float>(ci.getX()), 0.0f, static_cast<float>(ci.getZ())),
		ci.getPlanet(),
		NetworkId::cms_invalid,
		location))
	{
		return location->getReturnValue();
	}
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetRadius(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCityInfo(cityId).getRadius();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetFaction(JNIEnv *env, jobject self, jint cityId)
{
	return static_cast<jint>(CityInterface::getCityInfo(cityId).getFaction());
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsCityNamespace::cityGetGcwDefenderRegion(JNIEnv *env, jobject self, jint cityId)
{
	return JavaString(CityInterface::getCityInfo(cityId).getGcwDefenderRegion().c_str()).getReturnValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetTimeJoinedGcwDefenderRegion(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCityInfo(cityId).getTimeJoinedGcwDefenderRegion();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetCreationTime(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCityInfo(cityId).getCreationTime();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsCityNamespace::cityGetLeader(JNIEnv *env, jobject self, jint cityId)
{
	return (CityInterface::getCityInfo(cityId).getLeaderId()).getValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetIncomeTax(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCityInfo(cityId).getIncomeTax();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetPropertyTax(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCityInfo(cityId).getPropertyTax();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetSalesTax(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCityInfo(cityId).getSalesTax();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsCityNamespace::cityGetTravelLocation(JNIEnv *env, jobject self, jint cityId)
{
	CityInfo const &ci = CityInterface::getCityInfo(cityId);
	LocalRefPtr location;
	if (ScriptConversion::convert(
		ci.getTravelLoc(),
		ci.getPlanet(),
		NetworkId::cms_invalid,
		location))
	{
		return location->getReturnValue();
	}
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetTravelCost(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCityInfo(cityId).getTravelCost();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCityNamespace::cityGetTravelInterplanetary(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCityInfo(cityId).getTravelInterplanetary();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsCityNamespace::cityGetCloneLoc(JNIEnv *env, jobject self, jint cityId)
{
	CityInfo const &ci = CityInterface::getCityInfo(cityId);
	LocalRefPtr location;
	if (ScriptConversion::convert(
		ci.getCloneLoc(),
		ci.getPlanet(),
		NetworkId::cms_invalid,
		location))
	{
		return location->getReturnValue();
	}
	return 0;
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsCityNamespace::cityGetCloneRespawn(JNIEnv *env, jobject self, jint cityId)
{
	CityInfo const &ci = CityInterface::getCityInfo(cityId);
	LocalRefPtr location;
	if (ScriptConversion::convert(
		ci.getCloneRespawn(),
		ci.getPlanet(),
		ci.getCloneRespawnCell(),
		location))
	{
		return location->getReturnValue();
	}
	return 0;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsCityNamespace::cityGetCloneId(JNIEnv *env, jobject self, jint cityId)
{
	return (CityInterface::getCityInfo(cityId).getCloneId()).getValue();
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsCityNamespace::cityGetCitizenIds(JNIEnv *env, jobject self, jint cityId)
{
	std::vector<NetworkId> results;
	CityInterface::getCitizenIds(cityId, results);
	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetCitizenCount(JNIEnv *env, jobject self, jint cityId)
{
	return CityInterface::getCitizenCount(cityId);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsCityNamespace::cityGetCitizenName(JNIEnv *env, jobject self, jint cityId, jlong citizenId)
{
	CitizenInfo const *ci = CityInterface::getCitizenInfo(cityId, NetworkId(citizenId));
	if (ci)
		return JavaString(ci->m_citizenName).getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsCityNamespace::cityGetCitizenProfession(JNIEnv *env, jobject self, jint cityId, jlong citizenId)
{
	CitizenInfo const *ci = CityInterface::getCitizenInfo(cityId, NetworkId(citizenId));
	if (ci)
		return JavaString(ci->m_citizenProfessionSkillTemplate).getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetCitizenLevel(JNIEnv *env, jobject self, jint cityId, jlong citizenId)
{
	CitizenInfo const *ci = CityInterface::getCitizenInfo(cityId, NetworkId(citizenId));
	if (ci)
		return ci->m_citizenLevel;
	return 0;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsCityNamespace::cityGetCitizenTitle(JNIEnv *env, jobject self, jint cityId, jlong citizenId)
{
	CitizenInfo const *ci = CityInterface::getCitizenInfo(cityId, NetworkId(citizenId));
	if (ci)
		return JavaString(ci->m_citizenTitle).getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsCityNamespace::cityGetCitizenAllegiance(JNIEnv *env, jobject self, jint cityId, jlong citizenId)
{
	CitizenInfo const *ci = CityInterface::getCitizenInfo(cityId, NetworkId(citizenId));
	if (ci)
		return (ci->m_citizenAllegiance).getValue();
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetCitizenPermissions(JNIEnv *env, jobject self, jint cityId, jlong citizenId)
{
	CitizenInfo const *ci = CityInterface::getCitizenInfo(cityId, NetworkId(citizenId));
	if (ci)
		return ci->m_citizenPermissions;
	return 0;
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsCityNamespace::cityGetStructureIds(JNIEnv *env, jobject self, jint cityId)
{
	std::vector<NetworkId> results;
	CityInterface::getCityStructureIds(cityId, results);
	LocalLongArrayRefPtr rv;
	if (ScriptConversion::convert(results, rv))
		return rv->getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetStructureType(JNIEnv *env, jobject self, jint cityId, jlong structureId)
{
	CityStructureInfo const *csi = CityInterface::getCityStructureInfo(cityId, NetworkId(structureId));
	if (csi)
		return csi->getStructureType();
	return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCityNamespace::cityGetStructureValid(JNIEnv *env, jobject self, jint cityId, jlong structureId)
{
	CityStructureInfo const *csi = CityInterface::getCityStructureInfo(cityId, NetworkId(structureId));
	if (csi)
		return csi->getStructureValid();
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetName(JNIEnv *env, jobject self, jint cityId, jstring cityName)
{
	JavaStringParam localNameStr(cityName);
	std::string cityNameStr;
	if (!JavaLibrary::convert(localNameStr, cityNameStr))
		JavaLibrary::throwInternalScriptError("JavaLibrary::citySetName: invalid cityName");
	else
		CityInterface::setCityName(cityId, cityNameStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetCityHall(JNIEnv *env, jobject self, jint cityId, jlong cityHallId)
{
	CityInterface::setCityHall(cityId, NetworkId(cityHallId));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetLocation(JNIEnv *env, jobject self, jint cityId, jobject cityLocation)
{
	Vector cityPosition;
	std::string cityPlanet;
	NetworkId cell;
	if (!ScriptConversion::convert(cityLocation, cityPosition, cityPlanet, cell))
		JavaLibrary::throwInternalScriptError("JavaLibrary::citySetLocation: bad location");
	else if (cell != NetworkId::cms_invalid)
		JavaLibrary::throwInternalScriptError("JavaLibrary::citySetLocation: tried to set a location in an interior");
	else
		CityInterface::setCityLocation(cityId, cityPlanet, closestInt(cityPosition.x), closestInt(cityPosition.z));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetRadius(JNIEnv *env, jobject self, jint cityId, jint radius)
{
	if (radius <= 0)
		JavaLibrary::throwInternalScriptError("JavaLibrary::citySetLocation: tried to set a nonpositive radius");
	else
		CityInterface::setCityRadius(cityId, radius);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetFaction(JNIEnv *env, jobject self, jint cityId, jint factionId, jboolean notifyCitizens)
{
	CityInterface::setCityFaction(cityId, static_cast<uint32>(factionId), (notifyCitizens != JNI_FALSE));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetGcwDefenderRegion(JNIEnv *env, jobject self, jint cityId, jstring gcwDefenderRegion, jint timeJoined, jboolean notifyCitizens)
{
	JavaStringParam localGcwDefenderRegionStr(gcwDefenderRegion);
	std::string gcwDefenderRegionStr;
	if (JavaLibrary::convert(localGcwDefenderRegionStr, gcwDefenderRegionStr))
	{
		if (!gcwDefenderRegionStr.empty())
		{
			Pvp::GcwScoreCategory const * const gcwCategory = Pvp::getGcwScoreCategory(gcwDefenderRegionStr);
			if (!gcwCategory)
			{
				DEBUG_WARNING(true, ("JavaLibrary::citySetGcwDefenderRegion: invalid GCW score category (%s)", gcwDefenderRegionStr.c_str()));
				return;
			}

			if (!gcwCategory->gcwRegionDefender)
			{
				DEBUG_WARNING(true, ("JavaLibrary::citySetGcwDefenderRegion: GCW score category (%s) is not flagged as GcwRegionDefender", gcwDefenderRegionStr.c_str()));
				return;
			}
		}
	}
	else
	{
		gcwDefenderRegionStr.clear();
	}

	CityInterface::setCityGcwDefenderRegion(cityId, gcwDefenderRegionStr, timeJoined, (notifyCitizens != JNI_FALSE));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetLeader(JNIEnv *env, jobject self, jint cityId, jlong leaderId)
{
	CityInterface::setCityLeader(cityId, NetworkId(leaderId));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetIncomeTax(JNIEnv *env, jobject self, jint cityId, jint incomeTax)
{
	CityInterface::setCityIncomeTax(cityId, incomeTax);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetPropertyTax(JNIEnv *env, jobject self, jint cityId, jint propertyTax)
{
	CityInterface::setCityPropertyTax(cityId, propertyTax);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetSalesTax(JNIEnv *env, jobject self, jint cityId, jint salesTax)
{
	CityInterface::setCitySalesTax(cityId, salesTax);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetTravelInfo(JNIEnv *env, jobject self, jint cityId, jobject travelLocation, jint travelCost, jboolean travelInterplanetary)
{
	Vector travelPosition(Vector::zero);
	std::string travelPlanet;
	NetworkId travelCell;
	if (!ScriptConversion::convert(travelLocation, travelPosition, travelPlanet, travelCell))
		JavaLibrary::throwInternalScriptError("JavaLibrary::citySetTravelInfo: bad location");
	else if (travelCell != NetworkId::cms_invalid)
		JavaLibrary::throwInternalScriptError("JavaLibrary::citySetTravelInfo: tried to set a location in an interior");
	else
		CityInterface::setCityTravelInfo(cityId, travelPosition, travelCost, travelInterplanetary ? true : false);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetCloneInfo(JNIEnv *env, jobject self, jint cityId, jobject cloneLoc, jobject cloneRespawn, jlong cloneId)
{
	Vector cloneLocPosition(Vector::zero);
	std::string cloneLocPlanet;
	NetworkId cloneLocCell;
	ScriptConversion::convert(cloneLoc, cloneLocPosition, cloneLocPlanet, cloneLocCell);
	Vector cloneRespawnPosition(Vector::zero);
	std::string cloneRespawnPlanet;
	NetworkId cloneRespawnCell;
	ScriptConversion::convert(cloneRespawn, cloneRespawnPosition, cloneRespawnPlanet, cloneRespawnCell);
	CityInterface::setCityCloneInfo(cityId, cloneLocPosition, cloneRespawnPosition, cloneRespawnCell, NetworkId(cloneId));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetCitizenInfo(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring citizenName, jlong allegiance, jint permissions)
{
	JavaStringParam localNameStr(citizenName);
	std::string citizenNameStr;
	if (JavaLibrary::convert(localNameStr, citizenNameStr))
		CityInterface::setCitizenInfo(cityId, NetworkId(citizenId), citizenNameStr, NetworkId(allegiance), permissions);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetCitizenTitle(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring title)
{
	JavaStringParam localTitleStr(title);
	std::string titleStr;
	if (JavaLibrary::convert(localTitleStr, titleStr))
		CityInterface::setCitizenTitle(cityId, NetworkId(citizenId), titleStr);
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCityNamespace::cityGetAllCitizenRanks(JNIEnv *env, jobject self)
{
	std::vector<CitizenRankDataTable::CitizenRank const *> const & ranks = CitizenRankDataTable::getAllRanks();
	if (ranks.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(ranks.size(), JavaLibrary::getClsString());
	for (std::vector<CitizenRankDataTable::CitizenRank const *>::const_iterator iter = ranks.begin(); iter != ranks.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCityNamespace::cityGetTitleForCitizenRank(JNIEnv *env, jobject self, jstring rank)
{
	JavaStringParam localRankStr(rank);
	std::string rankStr;
	if (!JavaLibrary::convert(localRankStr, rankStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::cityGetTitleForCitizenRank: invalid rank"));
		return 0;
	}

	CitizenRankDataTable::CitizenRank const * rankInfo = CitizenRankDataTable::getRank(rankStr);
	if (!rankInfo || rankInfo->titles.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(rankInfo->titles.size(), JavaLibrary::getClsString());
	for (std::vector<std::string>::const_iterator iter = rankInfo->titles.begin(); iter != rankInfo->titles.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCityNamespace::cityGetCitizenRank(JNIEnv *env, jobject self, jint cityId, jlong citizenId)
{
	std::vector<std::string> ranks;
	CityInterface::getCitizenRank(cityId, NetworkId(static_cast<NetworkId::NetworkIdType>(citizenId)), ranks);
	if (ranks.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(ranks.size(), JavaLibrary::getClsString());
	for (std::vector<std::string>::const_iterator iter = ranks.begin(); iter != ranks.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCityNamespace::cityHasCitizenRank(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring rank)
{
	NetworkId citizen(citizenId);

	JavaStringParam localRankStr(rank);
	std::string rankStr;
	if (!JavaLibrary::convert(localRankStr, rankStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::cityHasCitizenRank: invalid rank"));
		return JNI_FALSE;
	}

	return CityInterface::hasCitizenRank(cityId, citizen, rankStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::cityAddCitizenRank(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring rank)
{
	NetworkId citizen(citizenId);

	JavaStringParam localRankStr(rank);
	std::string rankStr;
	if (!JavaLibrary::convert(localRankStr, rankStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::cityAddCitizenRank: invalid rank"));
		return;
	}

	CityInterface::addCitizenRank(cityId, citizen, rankStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::cityRemoveCitizenRank(JNIEnv *env, jobject self, jint cityId, jlong citizenId, jstring rank)
{
	NetworkId citizen(citizenId);

	JavaStringParam localRankStr(rank);
	std::string rankStr;
	if (!JavaLibrary::convert(localRankStr, rankStr))
	{
		DEBUG_WARNING(true, ("JavaLibrary::cityRemoveCitizenRank: invalid rank"));
		return;
	}

	CityInterface::removeCitizenRank(cityId, citizen, rankStr);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::citySetStructureInfo(JNIEnv *env, jobject self, jint cityId, jlong structureId, jint structureType, jboolean structureValid)
{
	CityInterface::setCityStructureInfo(cityId, NetworkId(structureId), structureType, structureValid);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::cityRemoveCitizen(JNIEnv *env, jobject self, jint cityId, jlong citizenId)
{
	CityInterface::removeCitizen(cityId, NetworkId(citizenId));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::cityRemoveStructure(JNIEnv *env, jobject self, jint cityId, jlong structureId)
{
	CityInterface::removeCityStructure(cityId, NetworkId(structureId));
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCityNamespace::cityIsInactivePackupActive(JNIEnv *env, jobject self)
{
	return (ConfigServerGame::getCityCitizenshipInactivePackupStartTimeEpoch() <= static_cast<int>(::time(nullptr)));
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsCityNamespace::cityGetInactivePackupInactiveTimeSeconds(JNIEnv *env, jobject self)
{
	return ConfigServerGame::getCityCitizenshipInactivePackupInactiveTimeSeconds();
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsCityNamespace::pgcGetChroniclerId(JNIEnv *env, jobject self, jstring chroniclerName)
{
	JavaStringParam localChroniclerNameStr(chroniclerName);
	std::string chroniclerNameStr;
	if (!JavaLibrary::convert(localChroniclerNameStr, chroniclerNameStr) || chroniclerNameStr.empty())
		return 0;

	std::vector<NetworkId> chroniclerIds;
	CityInterface::getPgcChroniclerId(chroniclerNameStr, chroniclerIds);

	if (chroniclerIds.empty())
		return 0;

	LocalLongArrayRefPtr idArray;
	if (!ScriptConversion::convert(chroniclerIds, idArray))
		return 0;

	return idArray->getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsCityNamespace::pgcGetChroniclerName(JNIEnv *env, jobject self, jlong chroniclerId)
{
	PgcRatingInfo const * const pgcRating = CityInterface::getPgcRating(NetworkId(chroniclerId));
	if (pgcRating)
		return JavaString(pgcRating->m_chroniclerName).getReturnValue();
	return 0;
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsCityNamespace::pgcGetRating(JNIEnv *env, jobject self, jlong chroniclerId)
{
	PgcRatingInfo const * const pgcRating = CityInterface::getPgcRating(NetworkId(chroniclerId));
	if (pgcRating)
	{
		std::vector<int> result(3);
		result[0] = static_cast<int>(pgcRating->m_ratingCount);
		result[1] = static_cast<int>(pgcRating->m_ratingTotal);
		result[2] = pgcRating->m_lastRatingTime;
		
		LocalIntArrayRefPtr ret = createNewIntArray(result.size());
		setIntArrayRegion(*ret, 0, result.size(), reinterpret_cast<jint *>(&result[0]));
		return ret->getReturnValue();
	}

	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::pgcAdjustRating(JNIEnv *env, jobject self, jlong chroniclerId, jstring chroniclerName, jint adjustment)
{
	std::string chroniclerNameStr;
	if (chroniclerName != 0)
	{
		JavaStringParam localChroniclerNameStr(chroniclerName);
		if (!JavaLibrary::convert(localChroniclerNameStr, chroniclerNameStr))
			return;
	}

	CityInterface::adjustPgcRating(NetworkId(chroniclerId), chroniclerNameStr, adjustment);
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsCityNamespace::pgcGetRatingData(JNIEnv *env, jobject self, jlong chroniclerId)
{
	PgcRatingInfo const * const pgcRating = CityInterface::getPgcRating(NetworkId(chroniclerId));
	if (pgcRating && !pgcRating->m_data.empty())
	{
		LocalIntArrayRefPtr ret = createNewIntArray(pgcRating->m_data.size());
		setIntArrayRegion(*ret, 0, pgcRating->m_data.size(), reinterpret_cast<jint *>(const_cast<int *>(&pgcRating->m_data[0])));
		return ret->getReturnValue();
	}

	return 0;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::pgcAdjustRatingData(JNIEnv *env, jobject self, jlong chroniclerId, jstring chroniclerName, jint index, jint adjustment)
{
	std::string chroniclerNameStr;	
	if (chroniclerName != 0)
	{
		JavaStringParam localChroniclerNameStr(chroniclerName);
		if (!JavaLibrary::convert(localChroniclerNameStr, chroniclerNameStr))
			return;
	}

	CityInterface::adjustPgcRatingData(NetworkId(chroniclerId), chroniclerNameStr, index, adjustment);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsCityNamespace::pgcSetRatingData(JNIEnv *env, jobject self, jlong chroniclerId, jstring chroniclerName, jint index, jint value)
{
	std::string chroniclerNameStr;
	if (chroniclerName != 0)
	{
		JavaStringParam localChroniclerNameStr(chroniclerName);
		if (!JavaLibrary::convert(localChroniclerNameStr, chroniclerNameStr))
			return;
	}

	CityInterface::setPgcRatingData(NetworkId(chroniclerId), chroniclerNameStr, index, value);
}

// ======================================================================
