//========================================================================
//
// ScriptMethodsHyperspace.cpp - implements script methods dealing with hyperspace
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "sharedFoundation/MessageQueue.h"
#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipController.h"
#include "serverGame/ShipObject.h"
#include "sharedGame/HyperspaceManager.h"
#include "sharedGame/SharedHyperspaceStringIds.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/CachedNetworkId.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"

#include "UnicodeUtils.h"

//========================================================================
// constants
//========================================================================

namespace ScriptMethodsHyperspaceNamespace
{
	float const cs_hyperspaceDelayTime = 6.0f;
	float const cs_hyperspaceInvulnerableTime = cs_hyperspaceDelayTime + 10.0f;
	float const cs_hyperspaceMinDistance = 500.f;

	bool install();
	void sendUnlockPlayerShipInputOnClient(ShipObject * shipObject);

	void       JNICALL hyperspacePlayerToLocation(JNIEnv *env, jobject self, jlong player, jstring scene, jfloat x_w, jfloat y_w, jfloat z_w, jlong cell, jfloat x_p, jfloat y_p, jfloat z_p, jstring callback, jboolean forceLoadScreen);
	void       JNICALL hyperspacePlayerToLocationCellName(JNIEnv *env, jobject self, jlong player, jstring scene, jfloat x_w, jfloat y_w, jfloat z_w, jlong building, jstring cell, jfloat x_p, jfloat y_p, jfloat z_p, jstring callback, jboolean forceLoadScreen);
	void       JNICALL hyperspacePlayerToHyperspacePoint(JNIEnv *env, jobject self, jlong player, jstring hyperspacePoint, jstring callback, jboolean forceLoadScreen, jboolean ignoreTooFull);
	jboolean   JNICALL isHyperspacePointOverpopulated(JNIEnv *env, jobject self, jstring jhyperspacePoint);
	void       JNICALL hyperspacePrepareShipOnClient(JNIEnv *env, jobject self, jlong player, jstring jhyperspacePoint);
	void       JNICALL hyperspaceRestoreShipOnClientFromAbortedHyperspace(JNIEnv *env, jobject self, jlong player);
	jstring    JNICALL getSceneForHyperspacePoint(JNIEnv *env, jobject self, jstring hyperspacePoint);
}

void ScriptMethodsHyperspaceNamespace::sendUnlockPlayerShipInputOnClient(ShipObject * const shipObject)
{
	CreatureObject * const pilotCreature = (shipObject != 0) ? shipObject->getPilot() : 0;
	Controller * const pilotController = (pilotCreature != 0) ? pilotCreature->getController() : 0;

	if (pilotController != 0)
	{
		pilotController->appendMessage(
			CM_unlockPlayerShipInputOnClient,
			0.0f,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

using namespace ScriptMethodsHyperspaceNamespace;

//========================================================================
// install
//========================================================================

bool ScriptMethodsHyperspaceNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsHyperspaceNamespace::c)}
	JF("_hyperspacePlayerToLocation",                   "(JLjava/lang/String;FFFJFFFLjava/lang/String;Z)V", hyperspacePlayerToLocation),
	JF("_hyperspacePlayerToLocation",                   "(JLjava/lang/String;FFFJLjava/lang/String;FFFLjava/lang/String;Z)V", hyperspacePlayerToLocationCellName),
	JF("_hyperspacePlayerToHyperspacePoint",            "(JLjava/lang/String;Ljava/lang/String;ZZ)V", hyperspacePlayerToHyperspacePoint),
	JF("isHyperspacePointOverpopulated",               "(Ljava/lang/String;)Z", isHyperspacePointOverpopulated),
	JF("_hyperspacePrepareShipOnClient",                "(JLjava/lang/String;)V", hyperspacePrepareShipOnClient),
	JF("_hyperspaceRestoreShipOnClientFromAbortedHyperspace","(J)V", hyperspaceRestoreShipOnClientFromAbortedHyperspace),
	JF("getSceneForHyperspacePoint",                   "(Ljava/lang/String;)Ljava/lang/String;", getSceneForHyperspacePoint),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI action state callback methods
//========================================================================

void JNICALL ScriptMethodsHyperspaceNamespace::hyperspacePlayerToLocation(JNIEnv *env, jobject self, jlong player, jstring scene, jfloat x_w, jfloat y_w, jfloat z_w, jlong cell, jfloat x_p, jfloat y_p, jfloat z_p, jstring callback, jboolean forceLoadScreen)
{
	ServerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - could not get player"));
		return;
	}

	JavaStringParam localScene(scene);
	std::string sceneName;
	if (!JavaLibrary::convert(localScene, sceneName))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - could not get scene name"));
		return;
	}
	std::string callbackName;

	if (callback)
	{
		JavaStringParam localCallback(callback);
		if (!JavaLibrary::convert(localCallback, callbackName))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - could not get callback"));
			return;
		}
	}

	ShipObject * const shipObject = ShipObject::getContainingShipObject(playerObject);

	if (!shipObject)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - Hyperspacing is only allowed for ShipObjects %s", playerObject->getDebugInformation().c_str()));
		return;
	}

	// Make sure the ship is not docking

	Controller const * const controller = (shipObject != nullptr) ? shipObject->getController() : nullptr;
	ShipController const * const shipController = (controller != nullptr) ? controller->asShipController() : nullptr;

	if (   (shipController != nullptr)
		&& shipController->isDocking())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - Hyperspacing is not allowed on ships that are currently docking: %s", shipObject->getDebugInformation().c_str()));
		return;
	}

	shipObject->setInvulnerabilityTimer(cs_hyperspaceInvulnerableTime);

	NetworkId cellId(cell);
	GameServer::getInstance().requestSceneWarpDelayed(
		CachedNetworkId(*shipObject),
		sceneName,
		Vector(x_w, y_w, z_w),
		cellId,
		Vector(x_p, y_p, z_p),
		cs_hyperspaceDelayTime,
		callbackName.c_str(),
		forceLoadScreen);
}

//------------------------------------------------------------------------------------------------

void JNICALL ScriptMethodsHyperspaceNamespace::hyperspacePlayerToLocationCellName(JNIEnv *env, jobject self, jlong player, jstring scene, jfloat x_w, jfloat y_w, jfloat z_w, jlong building, jstring cell, jfloat x_p, jfloat y_p, jfloat z_p, jstring callback, jboolean forceLoadScreen)
{
	ServerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - could not get player"));
		return;
	}

	JavaStringParam localScene(scene);
	std::string sceneName;
	if (!JavaLibrary::convert(localScene, sceneName))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - could not get scene name"));
		return;
	}
	std::string callbackName;

	if (callback)
	{
		JavaStringParam localCallback(callback);
		if (!JavaLibrary::convert(localCallback, callbackName))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - could not get callback"));
			return;
		}
	}

	JavaStringParam localCell(cell);
	std::string cellName;
	if (!JavaLibrary::convert(localCell, cellName))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - could not get call name"));
		return;
	}

	NetworkId buildingId(building);
	if (buildingId == NetworkId::cms_invalid)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - could not get building id"));
		return;
	}

	
	ShipObject * const shipObject = ShipObject::getContainingShipObject(playerObject);

	if (!shipObject)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - Hyperspacing is only allowed for ShipObjects %s", playerObject->getDebugInformation().c_str()));
		return;
	}

	// Make sure the ship is not docking

	Controller const * const controller = (shipObject != nullptr) ? shipObject->getController() : nullptr;
	ShipController const * const shipController = (controller != nullptr) ? controller->asShipController() : nullptr;

	if (   (shipController != nullptr)
		&& shipController->isDocking())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - Hyperspacing is not allowed on ships that are currently docking: %s", shipObject->getDebugInformation().c_str()));
		return;
	}

	shipObject->setInvulnerabilityTimer(cs_hyperspaceInvulnerableTime);

	GameServer::getInstance().requestSceneWarpDelayed(
		CachedNetworkId(*shipObject),
		sceneName,
		Vector(x_w, y_w, z_w),
		buildingId,
		cellName,
		Vector(x_p, y_p, z_p),
		cs_hyperspaceDelayTime,
		callbackName.c_str(),
		forceLoadScreen);
}

//------------------------------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsHyperspaceNamespace::isHyperspacePointOverpopulated(JNIEnv *env, jobject self, jstring jhyperspacePoint)
{
	JavaStringParam localHyperspacePoint(jhyperspacePoint);
	std::string hyperspacePoint;
	if (!JavaLibrary::convert(localHyperspacePoint, hyperspacePoint))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("isHyperspacePointAvailable - could not get hyperspace point name"));
		return true;
	}

	if(HyperspaceManager::isValidHyperspacePoint(hyperspacePoint))
	{
		HyperspaceManager::HyperspaceLocation location;
		bool const result = HyperspaceManager::getHyperspacePoint(hyperspacePoint, location);

		if (result)
		{
			int const population = ServerUniverse::getInstance().getPopulationAtLocation(location.sceneName, static_cast<int>(location.location.x), static_cast<int>(location.location.z));
			return (population > ConfigServerGame::getMaxPopulationForNewbieTravel());
		}
	}
	return true; // invalid points are always over-populated
}

//------------------------------------------------------------------------------------------------

void JNICALL ScriptMethodsHyperspaceNamespace::hyperspacePlayerToHyperspacePoint(JNIEnv *env, jobject self, jlong player, jstring jhyperspacePoint, jstring callback, jboolean forceLoadScreen, jboolean ignoreTooFull)
{
	ServerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - could not get player"));
		return;
	}

	CreatureObject * const playerCreature = playerObject ? playerObject->asCreatureObject() : nullptr;
	if(!playerCreature)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - player is not a creature"));
		return;
	}

	JavaStringParam localHyperspacePoint(jhyperspacePoint);
	std::string hyperspacePoint;
	if (!JavaLibrary::convert(localHyperspacePoint, hyperspacePoint))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - could not get hyperspace point name"));
		return;
	}

	std::string callbackName;

	if (callback)
	{
		JavaStringParam localCallback(callback);
		if (!JavaLibrary::convert(localCallback, callbackName))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - could not get callback"));
			return;
		}
	}

	ShipObject * const shipObject = ShipObject::getContainingShipObject(playerObject);

	if (!shipObject)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - Hyperspacing is only allowed for ShipObjects %s", playerObject->getDebugInformation().c_str()));
		return;
	}

	// Make sure the ship is not docking

	Controller const * const controller = (shipObject != nullptr) ? shipObject->getController() : nullptr;
	ShipController const * const shipController = (controller != nullptr) ? controller->asShipController() : nullptr;

	if (   (shipController != nullptr)
		&& shipController->isDocking())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - Hyperspacing is not allowed on ships that are currently docking: %s", shipObject->getDebugInformation().c_str()));
		return;
	}

	if(HyperspaceManager::isValidHyperspacePoint(hyperspacePoint))
	{
		// we first check if this hyperspace point is the home location - if so, call back into script to finish handling it. 
		if (hyperspacePoint == HyperspaceManager::getHomeLocationHyperspacePointName())
		{
			ScriptParams params;
			IGNORE_RETURN(playerCreature->getScriptObject()->trigAllScripts(Scripting::TRIG_HYPERSPACE_TO_HOME_LOCATION, params));
			return;
		}
	
		HyperspaceManager::HyperspaceLocation location;
		bool const result = HyperspaceManager::getHyperspacePoint(hyperspacePoint, location);
		location.location += Vector::randomUnit() * ConfigServerGame::getHyperspaceRandomOffsetRange();
		if(result)
		{
			if(!location.requiredCommand.empty())
			{
				if(!playerCreature->hasCommand(location.requiredCommand))
				{
					Chat::sendSystemMessage(*playerCreature, SharedHyperspaceStringIds::not_cleared_for_point, Unicode::emptyString);

					// We need to unlock the pilot's controls here since we didn't end up hyperspacing
					sendUnlockPlayerShipInputOnClient(shipObject);
					return;
				}
			}

			if (   location.sceneName == ServerWorld::getSceneId()
			    && location.location.magnitudeBetweenSquared(playerCreature->findPosition_w()) < sqr(cs_hyperspaceMinDistance))
			{
				Chat::sendSystemMessage(*playerCreature, SharedHyperspaceStringIds::already_at_point, Unicode::emptyString);

				// We need to unlock the pilot's controls here since we didn't end up hyperspacing
				sendUnlockPlayerShipInputOnClient(shipObject);
				return;
			}

			if (!ignoreTooFull)
			{
				int const population = ServerUniverse::getInstance().getPopulationAtLocation(location.sceneName, static_cast<int>(location.location.x), static_cast<int>(location.location.z));
				if (population > ConfigServerGame::getMaxPopulationForNewbieTravel())
				{
					Chat::sendSystemMessage(*playerCreature, SharedHyperspaceStringIds::zone_too_full, Unicode::emptyString);

					// We need to unlock the pilot's controls here since we didn't end up hyperspacing
					sendUnlockPlayerShipInputOnClient(shipObject);
					return;
				}
			}

			shipObject->setInvulnerabilityTimer(cs_hyperspaceInvulnerableTime);

			GameServer::getInstance().requestSceneWarpDelayed(
				CachedNetworkId(*shipObject),
				location.sceneName,
				Vector(location.location.x, location.location.y, location.location.z),
				NetworkId::cms_invalid,
				Vector(location.location.z, location.location.y, location.location.z),
				cs_hyperspaceDelayTime,
				callbackName.c_str(),
				forceLoadScreen);
		}
	}
}

//------------------------------------------------------------------------------------------------

void JNICALL ScriptMethodsHyperspaceNamespace::hyperspacePrepareShipOnClient(JNIEnv *env, jobject self, jlong player, jstring jhyperspacePoint)
{
	ServerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - could not get player"));
		return;
	}

	CreatureObject * const playerCreature = playerObject ? playerObject->asCreatureObject() : nullptr;
	if(playerCreature == 0)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - player is not a creature"));
		return;
	}

	ShipObject * const shipObject = ShipObject::getContainingShipObject(playerObject);

	if (shipObject == 0)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - Hyperspacing is only allowed for ShipObjects %s", playerObject->getDebugInformation().c_str()));
		return;
	}

	JavaStringParam localHyperspacePoint(jhyperspacePoint);
	std::string hyperspacePoint;
	if (!JavaLibrary::convert(localHyperspacePoint, hyperspacePoint))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - could not get hyperspace point name"));
		return;
	}


	if(HyperspaceManager::isValidHyperspacePoint(hyperspacePoint))
	{
		HyperspaceManager::HyperspaceLocation location;

		if (HyperspaceManager::getHyperspacePoint(hyperspacePoint, location))
		{
			Controller const * const controller = (shipObject != nullptr) ? shipObject->getController() : nullptr;
			ShipController const * const shipController = (controller != nullptr) ? controller->asShipController() : nullptr;

			if (shipController != 0)
			{
				// Make sure the ship is not docking
				if (shipController->isDocking())
				{
					JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - Hyperspacing is not allowed on ships that are currently docking: %s", shipObject->getDebugInformation().c_str()));
					return;
				}

				CreatureObject * const pilotCreature = shipObject->getPilot();
				Controller * const pilotController = (pilotCreature != 0) ? pilotCreature->getController() : 0;

				if (pilotController != 0)
				{
					MessageQueueGenericValueType<std::pair<std::string, Vector> > * data = new MessageQueueGenericValueType<std::pair<std::string, Vector> >(std::make_pair(location.sceneName, location.location));

					pilotController->appendMessage(CM_hyperspaceOrientShipToPointAndLockPlayerInput,
													0.0f,
													data,
													GameControllerMessageFlags::SEND |
													GameControllerMessageFlags::RELIABLE |
													GameControllerMessageFlags::DEST_AUTH_CLIENT);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------

void JNICALL ScriptMethodsHyperspaceNamespace::hyperspaceRestoreShipOnClientFromAbortedHyperspace(JNIEnv *env, jobject self, jlong player)
{
	ServerObject *playerObject = 0;
	if (!JavaLibrary::getObject(player, playerObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - could not get player"));
		return;
	}

	CreatureObject * const playerCreature = playerObject ? playerObject->asCreatureObject() : nullptr;
	if(playerCreature == 0)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToHyperspacePoint - player is not a creature"));
		return;
	}

	ShipObject * const shipObject = ShipObject::getContainingShipObject(playerObject);

	if (shipObject == 0)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("hyperspacePlayerToLocation - Hyperspacing is only allowed for ShipObjects %s", playerObject->getDebugInformation().c_str()));
		return;
	}

	Controller const * const controller = (shipObject != nullptr) ? shipObject->getController() : nullptr;
	ShipController const * const shipController = (controller != nullptr) ? controller->asShipController() : nullptr;

	if (shipController != 0)
	{
		// If the ship is docking, then unlocking the controls is not the correct thing to do.
		if (shipController->isDocking())
			return;

		CreatureObject * const pilotCreature = shipObject->getPilot();
		Controller * const pilotController = (pilotCreature != 0) ? pilotCreature->getController() : 0;

		if (pilotController != 0)
		{
			pilotController->appendMessage(CM_unlockPlayerShipInputOnClient,
											0.0f,
											GameControllerMessageFlags::SEND |
											GameControllerMessageFlags::RELIABLE |
											GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
}

//------------------------------------------------------------------------------------------------

jstring JNICALL ScriptMethodsHyperspaceNamespace::getSceneForHyperspacePoint(JNIEnv *env, jobject self, jstring jhyperspacePoint)
{
	JavaStringParam localHyperspacePoint(jhyperspacePoint);
	std::string hyperspacePoint;
	if (!JavaLibrary::convert(localHyperspacePoint, hyperspacePoint))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("getSceneForHyperspacePoint - could not get hyperspace point name"));
		return nullptr;
	}

	if(HyperspaceManager::isValidHyperspacePoint(hyperspacePoint))
	{
		HyperspaceManager::HyperspaceLocation location;
		bool const result = HyperspaceManager::getHyperspacePoint(hyperspacePoint, location);

		if (result)
		{
			JavaString str(location.sceneName);
			return str.getReturnValue();
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------------------------
