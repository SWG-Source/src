// ======================================================================
//
// ScriptMethodsNewbieTutorial.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/NewbieTutorial.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/NewbieTutorialEnableHudElement.h"
#include "sharedNetworkMessages/NewbieTutorialEnableInterfaceElement.h"
#include "sharedNetworkMessages/NewbieTutorialHighlightUIElement.h"
#include "sharedNetworkMessages/NewbieTutorialSetToolbarElement.h"
#include "sharedNetworkMessages/NewbieTutorialRequest.h"
#include "sharedObject/Controller.h"
#include "sharedUtility/StartingLocationData.h"
#include "sharedUtility/StartingLocationManager.h"

#include <map>

using namespace JNIWrappersNamespace;


// ======================================================================

namespace ScriptMethodsNewbieTutorialNamespace
{
	void sendMessageToPlayer (Object & player, MessageQueue::Data * msg, int messageType)
	{
		Controller * const controller = player.getController ();

		if (controller)
		{
			controller->appendMessage(
				messageType,
				0.0f,
				msg,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_CLIENT
				);
		}
		else
		{
			WARNING (true, ("newbieTutorialSendStartingLocationsToPlayer: bad controller"));
			delete msg;
		}
	}

	bool install();

	jboolean     JNICALL newbieTutorialRequest (JNIEnv* env, jobject self, jlong player, jstring request);
	jboolean     JNICALL newbieTutorialEnableHudElement (JNIEnv* env, jobject self, jlong player, jstring name, jboolean enable, jfloat blinkTime);
	jboolean     JNICALL newbieTutorialEnableInterfaceElement (JNIEnv* env, jobject self, jlong player, jstring name, jboolean enable);
	jboolean     JNICALL newbieTutorialSetToolbarElement (JNIEnv* env, jobject self, jlong player, jint slot, jstring commandName);
	jboolean     JNICALL newbieTutorialSetToolbarElementObject (JNIEnv* env, jobject self, jlong player, jint slot, jlong object);
	jboolean     JNICALL newbieTutorialHighlightUIElement (JNIEnv* env, jobject self, jlong player, jstring widgetPath, jfloat time);
	void         JNICALL newbieTutorialSendStartingLocationsToPlayer (JNIEnv* env, jobject self, jlong player, jobjectArray locations);
	void         JNICALL newbieTutorialSendStartingLocationSelectionResult (JNIEnv* env, jobject self, jlong player, jstring name, jboolean result);
	jobjectArray JNICALL getStartingLocations(JNIEnv* env, jobject self);
	jboolean     JNICALL isStartingLocationAvailable(JNIEnv* env, jobject self, jstring name);
	jobject      JNICALL getStartingLocationInfo(JNIEnv* env, jobject self, jstring name);
	jboolean     JNICALL isInTutorialArea(JNIEnv* env, jobject self, jlong object);
}

using namespace ScriptMethodsNewbieTutorialNamespace;

//========================================================================
// install
//========================================================================

bool ScriptMethodsNewbieTutorialNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsNewbieTutorialNamespace::c)}
	JF("_newbieTutorialRequest",                             "(JLjava/lang/String;)Z",                   newbieTutorialRequest),
	JF("_newbieTutorialEnableHudElement",                    "(JLjava/lang/String;ZF)Z",                 newbieTutorialEnableHudElement),
	JF("_newbieTutorialEnableInterfaceElement",              "(JLjava/lang/String;Z)Z",                  newbieTutorialEnableInterfaceElement),
	JF("_newbieTutorialSetToolbarElement",                   "(JILjava/lang/String;)Z",                  newbieTutorialSetToolbarElement),
	JF("_newbieTutorialSetToolbarElement",                   "(JIJ)Z",                                   newbieTutorialSetToolbarElementObject),
	JF("_newbieTutorialHighlightUIElement",                  "(JLjava/lang/String;F)Z",                  newbieTutorialHighlightUIElement),
	JF("_newbieTutorialSendStartingLocationsToPlayer",       "(J[Ljava/lang/String;)V",                  newbieTutorialSendStartingLocationsToPlayer),
	JF("_newbieTutorialSendStartingLocationSelectionResult", "(JLjava/lang/String;Z)V",                  newbieTutorialSendStartingLocationSelectionResult),
	JF("getStartingLocations",                               "()[Ljava/lang/String;",                    getStartingLocations),
	JF("isStartingLocationAvailable",                        "(Ljava/lang/String;)Z",                    isStartingLocationAvailable),
	JF("getStartingLocationInfo",                            "(Ljava/lang/String;)Lscript/location;",    getStartingLocationInfo),
	JF("_isInTutorialArea",                                  "(J)Z",                                     isInTutorialArea),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNewbieTutorialNamespace::newbieTutorialRequest (JNIEnv* /*env*/, jobject /*self*/, jlong jobject_player, jstring jstring_request)
{
	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialRequest (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client* const client = player->getClient ();
	if (!client)
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialRequest (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- get the request string
	JavaStringParam javaStringParam_request (jstring_request);
	std::string     request;
	if (!JavaLibrary::convert (javaStringParam_request, request))
	{
		DEBUG_WARNING (true, ("newbieTutorialRequest (): DB failed to convert parameter 2 (request) to string"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	NewbieTutorialRequest const msg(request);
	client->send(msg, true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNewbieTutorialNamespace::newbieTutorialEnableHudElement (JNIEnv* env, jobject self, jlong jobject_player, jstring jstring_name, jboolean jboolean_enable, jfloat jfloat_blinkTime)
{
	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialEnableHudElement (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client* const client = player->getClient ();
	if (!client)
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialEnableHudElement (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- get the request string
	JavaStringParam javaStringParam_name (jstring_name);
	std::string     name;
	if (!JavaLibrary::convert (javaStringParam_name, name))
	{
		DEBUG_WARNING (true, ("newbieTutorialEnableHudElement (): DB failed to convert parameter 2 (name) to string"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	NewbieTutorialEnableHudElement const msg(name, jboolean_enable == JNI_TRUE, jfloat_blinkTime);
	client->send(msg, true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNewbieTutorialNamespace::newbieTutorialEnableInterfaceElement (JNIEnv* env, jobject self, jlong jobject_player, jstring jstring_name, jboolean jboolean_enable)
{
	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialEnableInterfaceElement (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client* const client = player->getClient ();
	if (!client)
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialEnableInterfaceElement (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- get the request string
	JavaStringParam javaStringParam_name (jstring_name);
	std::string     name;
	if (!JavaLibrary::convert (javaStringParam_name, name))
	{
		DEBUG_WARNING (true, ("newbieTutorialEnableInterfaceElement (): DB failed to convert parameter 2 (name) to string"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	NewbieTutorialEnableInterfaceElement const msg(name, jboolean_enable == JNI_TRUE);
	client->send(msg, true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNewbieTutorialNamespace::newbieTutorialSetToolbarElement (JNIEnv* env, jobject self, jlong jobject_player, jint slot, jstring jstring_commandName)
{
	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSetToolbarElement (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client* const client = player->getClient ();
	if (!client)
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSetToolbarElement (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- get the command name string
	JavaStringParam javaStringParam_commandName (jstring_commandName);
	std::string     commandName;
	if (!JavaLibrary::convert (javaStringParam_commandName, commandName))
	{
		DEBUG_WARNING (true, ("newbieTutorialSetToolbarElement (): DB failed to convert parameter 3 (commandName) to string"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	NewbieTutorialSetToolbarElement const msg(slot, commandName);
	client->send(msg, true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNewbieTutorialNamespace::newbieTutorialSetToolbarElementObject (JNIEnv* env, jobject self, jlong jobject_player, jint slot, jlong object)
{
	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSetToolbarElement (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client* const client = player->getClient ();
	if (!client)
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSetToolbarElement (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	NewbieTutorialSetToolbarElement const msg(slot, object);
	client->send(msg, true);

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNewbieTutorialNamespace::newbieTutorialHighlightUIElement (JNIEnv* env, jobject self, jlong jobject_player, jstring jstring_widgetPath, jfloat time)
{
	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSetToolbarElement (): DB could not find player object"));
		return JNI_FALSE;
	}

	//-- get the client associated with the player
	Client* const client = player->getClient ();
	if (!client)
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSetToolbarElement (): PB could get the player's client object"));
		return JNI_FALSE;
	}

	//-- get the widget path string
	JavaStringParam javaStringParam_widgetPath (jstring_widgetPath);
	std::string     widgetPath;
	if (!JavaLibrary::convert (javaStringParam_widgetPath, widgetPath))
	{
		DEBUG_WARNING (true, ("newbieTutorialSetToolbarElement (): DB failed to convert parameter 3 (widgetPath) to string"));
		return JNI_FALSE;
	}

	//-- send the message to the client
	NewbieTutorialHighlightUIElement const msg(widgetPath, time);
	client->send(msg, true);

	return JNI_TRUE;
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsNewbieTutorialNamespace::newbieTutorialSendStartingLocationSelectionResult (JNIEnv* env, jobject self, jlong jobject_player, jstring jstring_name, jboolean result)
{
	UNREF (self);

	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSendStartingLocationSelectionResult (): DB could not find player object"));
		return;
	}

	std::string name;
	if (!JavaLibrary::convert(JavaStringParam (jstring_name), name))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSendStartingLocationSelectionResult (): DB bad string"));
		return;
	}

	typedef std::pair<std::string, bool> Payload;
	typedef MessageQueueGenericValueType<Payload> MessageType;
	MessageType * const message = new MessageType (Payload (name, (bool)result));
	sendMessageToPlayer (*player, message, CM_startingLocationSelectionResult);
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsNewbieTutorialNamespace::newbieTutorialSendStartingLocationsToPlayer (JNIEnv* env, jobject self, jlong jobject_player, jobjectArray locations)
{
	UNREF (self);

	//-- get the player object
	ServerObject* player = 0;
	if (!JavaLibrary::getObject (jobject_player, player))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("newbieTutorialSendStartingLocationsToPlayer (): DB could not find player object"));
		return;
	}

	StartingLocationManager::StartingLocationVector ldv;

	if (locations)
	{
		std::string name;

		const jsize count = env->GetArrayLength (locations);
		for (jsize i = 0; i < count; ++i)
		{
			const LocalRefPtr loc = getObjectArrayElement (LocalObjectArrayRefParam(locations), i);
			const jstring jstr  = static_cast<const jstring>(loc->getValue());

			name.clear ();

			if (JavaLibrary::convert(JavaStringParam (jstr), name))
			{
				const StartingLocationData * const data = StartingLocationManager::findLocationByName (name);
				if (data)
					ldv.push_back (*data);
				else
					WARNING (true, ("newbieTutorialSendStartingLocationsToPlayer: request for bogus location [%s] index [%d/", name.c_str (), i));
			}
		}
	}
	else
	{
		//-- copy vector
		ldv = StartingLocationManager::getLocations ();
	}

	//-- Sort the locations by population
	typedef std::multimap<int,const StartingLocationData*> PopulationSort;
	PopulationSort populations;

	for (StartingLocationManager::StartingLocationVector::const_iterator it = ldv.begin (); it != ldv.end (); ++it)
	{
		const StartingLocationData & data = *it;
		int population = ServerUniverse::getInstance().getPopulationAtLocation(data.planet, static_cast<int>(data.x), static_cast<int>(data.z));
		DEBUG_REPORT_LOG(true,("Starting locations:  %s (%i,%i): population %i\n",data.planet.c_str(),static_cast<int>(data.x), static_cast<int>(data.z),population));
		populations.insert(std::pair<int,const StartingLocationData*>(population,&data));
	}

	//-- Build the list for the client in order.  Mark them unavailable when we've reached the minimum and the population is too high.
	typedef std::pair<StartingLocationData, bool> LocationStatus;
	typedef std::vector<LocationStatus>        LocationStatusVector;
	LocationStatusVector clientLocations;
	clientLocations.reserve (ldv.size ());

	int locationCount=0;
	bool available=true;
	for (PopulationSort::iterator i=populations.begin(); i!=populations.end(); ++i)
	{
		if (available && (locationCount++ >= ConfigServerGame::getMinNewbieTravelLocations()) && (i->first > ConfigServerGame::getMaxPopulationForNewbieTravel()))
			available=false;
		clientLocations.push_back (LocationStatus (*i->second, available));
	}

	//--Send the list to the client
	typedef MessageQueueGenericValueType<LocationStatusVector> MessageType;
	MessageType * const message = new MessageType (clientLocations);
	sendMessageToPlayer (*player, message, CM_startingLocations);
}

//----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsNewbieTutorialNamespace::getStartingLocations  (JNIEnv* env, jobject self)
{
	UNREF (env);
	UNREF (self);

	const StartingLocationManager::StartingLocationVector & ldv = StartingLocationManager::getLocations ();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(ldv.size (), JavaLibrary::getClsString());
	size_t i = 0;
	for (StartingLocationManager::StartingLocationVector::const_iterator it = ldv.begin (); it != ldv.end (); ++it, ++i)
	{
		const StartingLocationData & data = *it;
		JavaString jval(data.name);
		setObjectArrayElement(*valueArray, i, jval);
	}

	return valueArray->getReturnValue();
}

//----------------------------------------------------------------------

jobject     JNICALL ScriptMethodsNewbieTutorialNamespace::getStartingLocationInfo  (JNIEnv* env, jobject self, jstring jName)
{
	UNREF (self);

	if (!jName)
	{
		WARNING (true, ("getStartingLocationInfo nullptr name"));
		return JNI_FALSE;
	}

	std::string name;
	if (!JavaLibrary::convert(JavaStringParam (jName), name))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("getStartingLocationInfo (): DB bad namestring"));
		return JNI_FALSE;
	}

	const StartingLocationData * const data = StartingLocationManager::findLocationByName (name);
	if (!data)
	{
		WARNING (true, ("getStartingLocationInfo: request for bogus location [%s]", name.c_str ()));
		return JNI_FALSE;
	}

	const NetworkId cellId (data->cellId);

	Vector loc;
	loc.x = data->x;
	loc.y = data->y;
	loc.z = data->z;

	LocalRefPtr jLocation;
	if (ScriptConversion::convert (loc, data->planet, cellId, jLocation))
		return jLocation->getReturnValue();

	return 0;
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNewbieTutorialNamespace::isStartingLocationAvailable (JNIEnv* env, jobject self, jstring jstring_name)
{
	std::string name;
	if (!JavaLibrary::convert(JavaStringParam (jstring_name), name))
	{
		DEBUG_REPORT_LOG_PRINT (true, ("isStartingLocationAvailable (): DB bad string"));
		return JNI_FALSE;
	}

	const StartingLocationData * const data = StartingLocationManager::findLocationByName (name);
	if (!data)
	{
		WARNING (true, ("isStartingLocationAvailable: request for bogus location [%s]", name.c_str ()));
		return JNI_FALSE;
	}

	// check if the scene is supposed to be running
	if (!GameServer::getInstance().isPlanetEnabledForCluster(data->planet))
		return JNI_FALSE;

	// check if the destination is too full
	int const population = ServerUniverse::getInstance().getPopulationAtLocation(data->planet, static_cast<int>(data->x), static_cast<int>(data->z));
	if (population > ConfigServerGame::getMaxPopulationForNewbieTravel())
		return JNI_FALSE;

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNewbieTutorialNamespace::isInTutorialArea(JNIEnv *env, jobject self, jlong object)
{
	const ServerObject *serverObject = 0;
	if (!JavaLibrary::getObject(object, serverObject))
		return false;
	
	return NewbieTutorial::isInTutorialArea(serverObject);
}

// ======================================================================

