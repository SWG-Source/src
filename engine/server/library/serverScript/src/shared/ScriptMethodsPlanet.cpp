// ======================================================================
//
// ScriptMethodsPlanet.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/TravelManager.h"
#include "sharedGame/TravelPoint.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/EnterTicketPurchaseModeMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsPlanetNamespace
// ======================================================================

namespace ScriptMethodsPlanetNamespace
{
	bool install();

	jboolean     JNICALL addPlanetTravelPoint (JNIEnv* env, jobject self, jstring planetName, jstring travelPointName, jobject location, jint cost, jboolean interplanetary, jint type);
	jboolean     JNICALL removePlanetTravelPoint (JNIEnv* env, jobject self, jstring planetName, jstring travelPointName);
	jobjectArray JNICALL getPlanetTravelPoints (JNIEnv* env, jobject self, jstring planetName);
	jstring      JNICALL getPlanetTravelPointGcwContestedRegion (JNIEnv* env, jobject self, jstring planetName, jstring travelPointName);
	jobject      JNICALL getPlanetTravelPointLocation (JNIEnv* env, jobject self, jstring planetName, jstring travelPointName);
	jint         JNICALL getPlanetTravelPointCost (JNIEnv* env, jobject self, jstring planetName, jstring travelPointName);
	jboolean     JNICALL getPlanetTravelPointInterplanetary (JNIEnv* env, jobject self, jstring planetName, jstring travelPointName);
	jint         JNICALL getPlanetTravelCost (JNIEnv* env, jobject self, jstring sourcePlanetName, jstring destinationPlanetName);
	jboolean     JNICALL enterClientTicketPurchaseMode (JNIEnv* env, jobject self, jlong player, jstring startingPlanetName, jstring startingTravelPointName, jboolean instantTravel);
	jboolean     JNICALL isAreaTooFullForTravel (JNIEnv* env, jobject self, jstring planetName_j, jint x, jint z);
	jstring      JNICALL getBuildoutAreaName( JNIEnv *env, jobject self, jfloat x, jfloat z, jstring scene );
	jfloatArray  JNICALL getBuildoutAreaSizeAndCenter(JNIEnv *env, jobject self, jfloat x, jfloat z, jstring scene, jboolean ignoreInternal, jboolean allowComposite);
	void         JNICALL requestSameServer(JNIEnv *env, jobject self, jlong object1, jlong object2);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsPlanetNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsPlanetNamespace::c)}
	JF("addPlanetTravelPoint",                "(Ljava/lang/String;Ljava/lang/String;Lscript/location;IZI)Z", addPlanetTravelPoint),
	JF("removePlanetTravelPoint",             "(Ljava/lang/String;Ljava/lang/String;)Z",                 removePlanetTravelPoint),
	JF("getPlanetTravelPoints",               "(Ljava/lang/String;)[Ljava/lang/String;",                 getPlanetTravelPoints),
	JF("getPlanetTravelPointGcwContestedRegion", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", getPlanetTravelPointGcwContestedRegion),
	JF("getPlanetTravelPointLocation",        "(Ljava/lang/String;Ljava/lang/String;)Lscript/location;", getPlanetTravelPointLocation),
	JF("getPlanetTravelPointCost",            "(Ljava/lang/String;Ljava/lang/String;)I",                 getPlanetTravelPointCost),
	JF("getPlanetTravelPointInterplanetary",  "(Ljava/lang/String;Ljava/lang/String;)Z",                 getPlanetTravelPointInterplanetary),
	JF("getPlanetTravelCost",                 "(Ljava/lang/String;Ljava/lang/String;)I",                 getPlanetTravelCost),
	JF("_enterClientTicketPurchaseMode",      "(JLjava/lang/String;Ljava/lang/String;Z)Z",               enterClientTicketPurchaseMode),
	JF("isAreaTooFullForTravel",              "(Ljava/lang/String;II)Z",                                 isAreaTooFullForTravel),
	JF("getBuildoutAreaName",                 "(FFLjava/lang/String;)Ljava/lang/String;",                getBuildoutAreaName),
	JF("getBuildoutAreaSizeAndCenter",        "(FFLjava/lang/String;ZZ)[F",                              getBuildoutAreaSizeAndCenter),
	JF("_requestSameServer",                  "(JJ)V",                                                   requestSameServer),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jboolean JNICALL ScriptMethodsPlanetNamespace::addPlanetTravelPoint (JNIEnv* const /*env*/, jobject /*self*/, jstring jstring_planetName, jstring jstring_travelPointName, jobject jobject_location, jint cost, jboolean jboolean_interplanetary, jint type)
{
	//-- get the planet object
	JavaStringParam javaStringParam_planetName (jstring_planetName);
	std::string     planetName;
	if (!JavaLibrary::convert (javaStringParam_planetName, planetName))
	{
		DEBUG_WARNING (true, ("addPlanetTravelPoint (): DB failed to convert parameter 1 (planetName) to string"));
		return JNI_FALSE;
	}

	PlanetObject* const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		DEBUG_WARNING (true, ("addPlanetTravelPoint (): DB planet %s does not exist", planetName.c_str ()));
		return JNI_FALSE;
	}

	//-- get the name of the travel point
	JavaStringParam javaStringParam_travelPointName (jstring_travelPointName);
	std::string     travelPointName;
	if (!JavaLibrary::convert (javaStringParam_travelPointName, travelPointName))
	{
		DEBUG_WARNING (true, ("addPlanetTravelPoint (): DB failed to convert parameter 2 (travelPointName) to string"));
		return JNI_FALSE;
	}

	//-- get (and verify) location
	std::string sceneId;
	Vector position_w;
	if (!ScriptConversion::convertWorld (jobject_location, position_w, sceneId))
	{
		DEBUG_WARNING (true, ("addPlanetTravelPoint (): DB failed to convert parameter 3 (location) to sceneId, and position_w"));
		return JNI_FALSE;
	}

	//-- make sure the planet name matches the location name
//	if (sceneId != planetName)
//	{
//		DEBUG_WARNING (true, ("addPlanetTravelPoint (): DB planet %s does not match scene id %s in travel point location %s", planetName.c_str (), sceneId.c_str (), travelPointName.c_str ()));
//		return FALSE;
//	}

	//-- add to the planet
	planetObject->addTravelPoint (travelPointName, position_w, cost, jboolean_interplanetary == JNI_TRUE, static_cast<TravelPoint::TravelPointType>(type));

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlanetNamespace::removePlanetTravelPoint (JNIEnv* const /*env*/, jobject /*self*/, jstring jstring_planetName, jstring jstring_travelPointName)
{
	//-- get the planet object
	JavaStringParam javaStringParam_planetName (jstring_planetName);
	std::string     planetName;
	if (!JavaLibrary::convert (javaStringParam_planetName, planetName))
	{
		DEBUG_WARNING (true, ("removePlanetTravelPoint (): DB failed to convert parameter 1 (planetName) to string"));
		return JNI_FALSE;
	}

	PlanetObject* const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		DEBUG_WARNING (true, ("removePlanetTravelPoint (): DB planet %s does not exist", planetName.c_str ()));
		return JNI_FALSE;
	}

	//-- get the name of the travel point
	JavaStringParam javaStringParam_travelPointName (jstring_travelPointName);
	std::string     travelPointName;
	if (!JavaLibrary::convert (javaStringParam_travelPointName, travelPointName))
	{
		DEBUG_WARNING (true, ("removePlanetTravelPoint (): DB failed to convert parameter 2 (travelPointName) to string"));
		return JNI_FALSE;
	}

	//-- remove from the planet
	planetObject->removeTravelPoint (travelPointName);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsPlanetNamespace::getPlanetTravelPoints (JNIEnv* env, jobject /*self*/, jstring jstring_planetName)
{
	//-- get the planet object
	JavaStringParam javaStringParam_planetName (jstring_planetName);
	std::string     planetName;
	if (!JavaLibrary::convert (javaStringParam_planetName, planetName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPoints (): DB failed to convert parameter 1 (planetName) to string"));
		return JNI_FALSE;
	}

	PlanetObject* const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPoints (): DB planet %s does not exist", planetName.c_str ()));
		return JNI_FALSE;
	}

	//-- create list of travel points
	std::vector<std::string> travelPointNameList;
	planetObject->getTravelPointNameList (travelPointNameList);

	//-- convert to a string array
	LocalObjectArrayRefPtr jobjectArray_travelPointNameList = createNewObjectArray(travelPointNameList.size (), JavaLibrary::getClsString());

	uint i;
	for (i = 0; i < travelPointNameList.size (); ++i)
	{
		JavaString jstring_travelPointName (Unicode::narrowToWide (travelPointNameList [i]));
		setObjectArrayElement (*jobjectArray_travelPointNameList, i, jstring_travelPointName);
	}

	return jobjectArray_travelPointNameList->getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsPlanetNamespace::getPlanetTravelPointGcwContestedRegion (JNIEnv* env, jobject self, jstring jstring_planetName, jstring jstring_travelPointName)
{
	//-- get the planet object
	JavaStringParam javaStringParam_planetName (jstring_planetName);
	std::string     planetName;
	if (!JavaLibrary::convert (javaStringParam_planetName, planetName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointGcwContestedRegion (): DB failed to convert parameter 1 (planetName) to string"));
		return 0;
	}

	PlanetObject* const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointGcwContestedRegion (): DB planet %s does not exist", planetName.c_str ()));
		return 0;
	}

	//-- get the name of the travel point
	JavaStringParam javaStringParam_travelPointName (jstring_travelPointName);
	std::string     travelPointName;
	if (!JavaLibrary::convert (javaStringParam_travelPointName, travelPointName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointGcwContestedRegion (): DB failed to convert parameter 2 (travelPointName) to string"));
		return 0;
	}

	const TravelPoint* const travelPoint = planetObject->getTravelPoint (travelPointName);
	if (!travelPoint)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointGcwContestedRegion (): DB travel point %s does not exist on planet %s", travelPointName.c_str (), planetName.c_str ()));
		return 0;
	}

	if (travelPoint->getGcwContestedRegion().empty())
	{
		return 0;
	}

	return JavaString(travelPoint->getGcwContestedRegion().c_str()).getReturnValue();
}

// ----------------------------------------------------------------------

jobject JNICALL ScriptMethodsPlanetNamespace::getPlanetTravelPointLocation (JNIEnv* const /*env*/, jobject /*self*/, jstring jstring_planetName, jstring jstring_travelPointName)
{
	//-- get the planet object
	JavaStringParam javaStringParam_planetName (jstring_planetName);
	std::string     planetName;
	if (!JavaLibrary::convert (javaStringParam_planetName, planetName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointLocation (): DB failed to convert parameter 1 (planetName) to string"));
		return 0;
	}

	PlanetObject* const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointLocation (): DB planet %s does not exist", planetName.c_str ()));
		return 0;
	}

	//-- get the name of the travel point
	JavaStringParam javaStringParam_travelPointName (jstring_travelPointName);
	std::string     travelPointName;
	if (!JavaLibrary::convert (javaStringParam_travelPointName, travelPointName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointLocation (): DB failed to convert parameter 2 (travelPointName) to string"));
		return 0;
	}

	const TravelPoint* const travelPoint = planetObject->getTravelPoint (travelPointName);
	if (!travelPoint)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointLocation (): DB travel point %s does not exist on planet %s", travelPointName.c_str (), planetName.c_str ()));
		return 0;
	}

	//-- transform into location
	const Vector& position_w = travelPoint->getPosition_w ();

	LocalRefPtr location;
	if (!ScriptConversion::convert (position_w, NetworkId (), location))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointLocation (): PB could not convert travel point %s to location (scene %s, position <%1.2f, %1.2f, %1.2f>)", travelPointName.c_str (), planetName.c_str (), position_w.x, position_w.y, position_w.z));
		return 0;
	}

	return location->getReturnValue();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPlanetNamespace::getPlanetTravelPointCost (JNIEnv* const /*env*/, jobject /*self*/, jstring jstring_planetName, jstring jstring_travelPointName)
{
	//-- get the planet object
	JavaStringParam javaStringParam_planetName (jstring_planetName);
	std::string     planetName;
	if (!JavaLibrary::convert (javaStringParam_planetName, planetName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointCost (): DB failed to convert parameter 1 (planetName) to string"));
		return 0;
	}

	PlanetObject* const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointCost (): DB planet %s does not exist", planetName.c_str ()));
		return 0;
	}

	//-- get the name of the travel point
	JavaStringParam javaStringParam_travelPointName (jstring_travelPointName);
	std::string     travelPointName;
	if (!JavaLibrary::convert (javaStringParam_travelPointName, travelPointName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointCost (): DB failed to convert parameter 2 (travelPointName) to string"));
		return 0;
	}

	const TravelPoint* const travelPoint = planetObject->getTravelPoint (travelPointName);
	if (!travelPoint)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointCost (): DB travel point %s does not exist on planet %s", travelPointName.c_str (), planetName.c_str ()));
		return 0;
	}

	return travelPoint->getCost ();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlanetNamespace::getPlanetTravelPointInterplanetary (JNIEnv* const /*env*/, jobject /*self*/, jstring jstring_planetName, jstring jstring_travelPointName)
{
	//-- get the planet object
	JavaStringParam javaStringParam_planetName (jstring_planetName);
	std::string     planetName;
	if (!JavaLibrary::convert (javaStringParam_planetName, planetName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointInterplanetary (): DB failed to convert parameter 1 (planetName) to string"));
		return 0;
	}

	PlanetObject* const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointInterplanetary (): DB planet %s does not exist", planetName.c_str ()));
		return 0;
	}

	//-- get the name of the travel point
	JavaStringParam javaStringParam_travelPointName (jstring_travelPointName);
	std::string     travelPointName;
	if (!JavaLibrary::convert (javaStringParam_travelPointName, travelPointName))
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointInterplanetary (): DB failed to convert parameter 2 (travelPointName) to string"));
		return 0;
	}

	const TravelPoint* const travelPoint = planetObject->getTravelPoint (travelPointName);
	if (!travelPoint)
	{
		DEBUG_WARNING (true, ("getPlanetTravelPointInterplanetary (): DB travel point %s does not exist on planet %s", travelPointName.c_str (), planetName.c_str ()));
		return 0;
	}

	return travelPoint->getInterplanetary ();
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsPlanetNamespace::getPlanetTravelCost (JNIEnv* const /*env*/, jobject /*self*/, jstring jstring_planetName1, jstring jstring_planetName2)
{
	//-- get planet 1 name
	JavaStringParam javaStringParam_planetName1 (jstring_planetName1);
	std::string     planetName1;
	if (!JavaLibrary::convert (javaStringParam_planetName1, planetName1))
	{
		DEBUG_WARNING (true, ("getPlanetTravelCost (): DB failed to convert parameter 1 (planetName1) to string"));
		return 0;
	}

	//-- get planet 2 name
	JavaStringParam javaStringParam_planetName2 (jstring_planetName2);
	std::string     planetName2;
	if (!JavaLibrary::convert (javaStringParam_planetName2, planetName2))
	{
		DEBUG_WARNING (true, ("getPlanetTravelCost (): DB failed to convert parameter 2 (planetName2) to string"));
		return 0;
	}

	//-- find and return the cost
	int cost;
	if (!TravelManager::getPlanetSingleHopCost (planetName1, planetName2, cost))
	{
		DEBUG_WARNING (true, ("getPlanetTravelCost (): DB could not get the cost between planet %s and planet %s", planetName1.c_str (), planetName2.c_str ()));
		return 0;
	}

	return cost;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlanetNamespace::enterClientTicketPurchaseMode (JNIEnv* /*env*/, jobject /*self*/, jlong jobject_player, jstring jstring_planetName, jstring jstring_travelPointName, jboolean instantTravel)
{
	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_WARNING (true, ("enterClientTicketPurchaseMode (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client* const client = player->getClient ();
	if (!client)
	{
		DEBUG_WARNING (true, ("enterClientTicketPurchaseMode (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- make sure we have a valid planet name
	JavaStringParam javaStringParam_planetName (jstring_planetName);
	std::string     planetName;
	if (!JavaLibrary::convert (javaStringParam_planetName, planetName))
	{
		DEBUG_WARNING (true, ("enterClientTicketPurchaseMode (): DB failed to convert parameter 2 (planetName) to string"));
		return JNI_FALSE;
	}

	//-- make sure we have a valid travel point name
	JavaStringParam javaStringParam_travelPointName (jstring_travelPointName);
	std::string     travelPointName;
	if (!JavaLibrary::convert (javaStringParam_travelPointName, travelPointName))
	{
		DEBUG_WARNING (true, ("enterClientTicketPurchaseMode (): DB failed to convert parameter 3 (travelPointName) to string"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	EnterTicketPurchaseModeMessage const msg(planetName, travelPointName, instantTravel == JNI_TRUE);
	client->send(msg, true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsPlanetNamespace::isAreaTooFullForTravel (JNIEnv* const /*env*/, jobject /*self*/, jstring planetName_j, jint x, jint z)
{
	std::string planetName;
	JavaStringParam planetName_jsp (planetName_j);
	if (!JavaLibrary::convert (planetName_jsp, planetName))
	{
		WARNING(true,("isAreaTooFullForTravel could not convert the planet name to string"));
		return JNI_TRUE; // invalid locations are always too full
	}
	int const population = ServerUniverse::getInstance().getPopulationAtLocation(planetName, x, z);
	if (population > ConfigServerGame::getMaxPopulationForNewbieTravel())
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsPlanetNamespace::getBuildoutAreaName( JNIEnv *const /*env*/, jobject /*self*/, const jfloat x, const jfloat z, const jstring sceneId_j )
{
	std::string sceneId;
	JavaStringParam sceneId_jsp (sceneId_j);
	if (!JavaLibrary::convert (sceneId_jsp, sceneId))
	{
		WARNING(true,("getBuildoutAreaName could not convert the scene id to string"));
		return 0;
	}

	const BuildoutArea * const area = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(sceneId, x, z, false, false);

	return area ? JavaString( area->areaName ).getReturnValue() : 0;
}

// ----------------------------------------------------------------------

jfloatArray JNICALL ScriptMethodsPlanetNamespace::getBuildoutAreaSizeAndCenter(JNIEnv *const /*env*/, jobject /*self*/, const jfloat x, const jfloat z, const jstring sceneId_j, const jboolean ignoreInternal, const jboolean allowComposite)
{
	std::string sceneId;
	JavaStringParam sceneId_jsp (sceneId_j);
	if (!JavaLibrary::convert (sceneId_jsp, sceneId))
	{
		WARNING(true,("getBuildoutAreaSizeAndCenter could not convert the scene id to string"));
		return 0;
	}

	const BuildoutArea * const area = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(sceneId.c_str(), x, z, (ignoreInternal == JNI_TRUE));
	if (!area)
		return 0;

	LocalFloatArrayRefPtr valueArray = createNewFloatArray(static_cast<jsize>(4));
	if (valueArray == LocalFloatArrayRef::cms_nullPtr)
	{
		return 0;
	}

	Vector2d areaSize = area->getSize(allowComposite == JNI_TRUE);
	
	jfloat value = static_cast<jfloat>(areaSize.x);
	setFloatArrayRegion(*valueArray, 0, 1, &value);

	value = static_cast<jfloat>(areaSize.y);
	setFloatArrayRegion(*valueArray, 1, 1, &value);

	value = static_cast<jfloat>(area->getXCenterPoint(allowComposite == JNI_TRUE));
	setFloatArrayRegion(*valueArray, 2, 1, &value);

	value = static_cast<jfloat>(area->getZCenterPoint(allowComposite == JNI_TRUE));
	setFloatArrayRegion(*valueArray, 3, 1, &value);

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsPlanetNamespace::requestSameServer(JNIEnv *env, jobject self, jlong object1, jlong object2)
{
	ServerObject const * serverObject1 = 0;
	if (!JavaLibrary::getObject(object1, serverObject1))
	{
		WARNING(true, ("requestSameServer - could not find object %s", NetworkId(object1).getValueString().c_str()));
		return;
	}

	ServerObject const * serverObject2 = 0;
	if (!JavaLibrary::getObject(object2, serverObject2))
	{
		WARNING(true, ("requestSameServer - could not find object %s", NetworkId(object2).getValueString().c_str()));
		return;
	}

	GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
		"RequestSameServer",
		std::make_pair(serverObject1->getNetworkId(), serverObject2->getNetworkId()));

	GameServer::getInstance().sendToPlanetServer(rssMessage);
}

// ======================================================================
