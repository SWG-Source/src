// ======================================================================
//
// PlayerCreatureController.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlayerCreatureController.h"

#include "UnicodeUtils.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/Chat.h"
#include "serverGame/Client.h"
#include "serverGame/CommandQueue.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/MissionObject.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerBuffBuilderManager.h"
#include "serverGame/ServerImageDesignerManager.h"
#include "serverGame/ServerSecureTrade.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedGame/SharedBuffBuilderManager.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/AIDebuggingMessages.h"
#include "sharedNetworkMessages/BuffBuilderChangeMessage.h"
#include "sharedNetworkMessages/DroidCommandProgrammingMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"
#include "sharedNetworkMessages/MessageQueueChangeRoleIconChoice.h"
#include "sharedNetworkMessages/MessageQueueCraftCustomization.h"
#include "sharedNetworkMessages/MessageQueueCraftEmptySlot.h"
#include "sharedNetworkMessages/MessageQueueCraftFillSlot.h"
#include "sharedNetworkMessages/MessageQueueCraftRequestSession.h"
#include "sharedNetworkMessages/MessageQueueCraftSelectSchematic.h"
#include "sharedNetworkMessages/MessageQueueCreateSaga.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsChangeRequest.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueGeneric.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequestData.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionListRequest.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueueObjectMenuRequest.h"
#include "sharedNetworkMessages/MessageQueueSelectCurrentWorkingSkill.h"
#include "sharedNetworkMessages/MessageQueueSelectProfessionTemplate.h"
#include "sharedNetworkMessages/MessageQueueSpaceMiningSellResource.h"
#include "sharedNetworkMessages/MessageQueueTeleportAck.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/ServerTimeMessage.h"
#include "sharedNetworkMessages/ServerWeatherMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Portal.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/ValueTypeBool.h"
#include "sharedUtility/ValueDictionary.h"
#include "sharedUtility/ValueTypeSignedInt.h"
#include "sharedUtility/ValueTypeObjId.h"
#include "sharedUtility/ValueTypeString.h"

#include <limits>

// ======================================================================

namespace PlayerCreatureControllerNamespace
{
	std::string const minigameResultTargetName("table");
	std::string const minigameResultDictionaryName("result");

	int syncStampLongDeltaTime(uint32 stamp1, uint32 stamp2)
	{
		uint32 const delta = stamp1 - stamp2;
		if (delta > 0x7fffffff)
			return static_cast<int>(0xffffffff-delta);
		return static_cast<int>(delta);
	}

// ----------------------------------------------------------------------

	CellObject *getCellObject(NetworkId const &id)
	{
		if (id != NetworkId::cms_invalid)
		{
			Object * const o = NetworkIdManager::getObjectById(id);
			if (o)
			{
				ServerObject * const so = o->asServerObject();
				if (so)
					return so->asCellObject();
			}
		}
		return 0;
	}

// ----------------------------------------------------------------------

	CellProperty const *getCellProperty(NetworkId const &id)
	{
		if (id == NetworkId::cms_invalid)
			return CellProperty::getWorldCellProperty();
		Object * const cellObj = NetworkIdManager::getObjectById(id);
		if (!cellObj)
			return 0;
		return cellObj->getCellProperty();
	}

	bool isMissionTerminal(const ServerObject& obj)
	{
		if (GameObjectTypes::isTypeOf (obj.getGameObjectType(), SharedObjectTemplate::GOT_terminal_mission) ||
			(obj.getScriptObject() && obj.getScriptObject()->hasScript("systems.missions.base.mission_terminal")))
		{
			return true;
		}
		return false;
	}

	bool isCreaturePassenger(CreatureObject const & creature)
	{
		if(!creature.getState(States::RidingMount))
		{
			return false;
		}

		CreatureObject const * const mount = creature.getMountedCreature();

		if(!mount)
		{
			LOG("mounts-bug", ("PlayerCreatureControllerNamespace::isCreaturePassenger(): server id=[%d],object id=[%s] creature has state RidingMount but getMountedCreature() returns nullptr.", static_cast<int>(GameServer::getInstance().getProcessId()), creature.getNetworkId().getValueString().c_str()));
			return false;
		}

		CreatureObject const * const primaryRider = mount->getPrimaryMountingRider();

		if(!primaryRider)
		{
			LOG("mounts-bug", ("PlayerCreatureControllerNamespace::isCreaturePassenger(): server id=[%d],object id=[%s] creature has state RidingMount but mount->getPrimaryMountingRider() returns nullptr.", static_cast<int>(GameServer::getInstance().getProcessId()), creature.getNetworkId().getValueString().c_str()));
			return false;
		}

		return (primaryRider != &creature);
	}
}
// ======================================================================

using namespace PlayerCreatureControllerNamespace;

// ======================================================================

PlayerCreatureController::PlayerCreatureController(CreatureObject *newOwner) :
	CreatureController(newOwner),
	m_teleportIds(),
	m_lastVerifiedMove(),
	m_lastTimeSynchronization(0),
	m_lastWeatherIndex(0),
	m_lastWeatherWindVelocity_w (),
	m_speedMaximum(0.f, ConfigServerGame::getMoveSpeedCheckFrequencyMs(), ConfigServerGame::getMoveSlowdownFudgeTimeMs()),
	m_movingTimeout(2.0f),
	m_lastMoveSyncStamp(0),
	m_lastSpeed(0),
	m_lastSpeedCheckFailureTime(0),
	m_speedCheckConsecutiveFailureCount(0),
	m_resendSpeedMax (false)
{
	preventMovementUpdates();
}

// ----------------------------------------------------------------------

PlayerCreatureController::~PlayerCreatureController()
{
}

// ----------------------------------------------------------------------

PlayerObject *PlayerCreatureController::getPlayerObjectInternal(CreatureObject const * creature)
{
	static SlotId const playerSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("ghost"));

	if (creature)
	{
		SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*creature);
	
		if (container)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			Object * const o = container->getObjectInSlot(playerSlot, tmp).getObject();
			if (o)
			{
				ServerObject * const so = o->asServerObject();
				if (so)
					return so->asPlayerObject();
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

PlayerObject *PlayerCreatureController::getPlayerObject(CreatureObject * creature)
{
	return getPlayerObjectInternal(creature);
}

// ----------------------------------------------------------------------

PlayerObject const * PlayerCreatureController::getPlayerObject(CreatureObject const * creature)
{
	return getPlayerObjectInternal(creature);
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::isGod() const
{
	//-- Get the creature owner.
	CreatureObject const *const creature = NON_NULL(getCreature());

	//-- Get the client for the owner.
	Client const *const client = creature->getClient();
	if (!client)
		return false;

	//-- Find out if this client is in god mode.
	return client->isGod();
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::isTeleporting() const
{
	return !m_teleportIds.empty();
}

// ----------------------------------------------------------------------

void PlayerCreatureController::preventMovementUpdates()
{
	m_teleportIds.clear();
	IGNORE_RETURN(m_teleportIds.insert(-1));
}

// ----------------------------------------------------------------------

void PlayerCreatureController::resyncMovementUpdates()
{
	ServerObject * const netObject = NON_NULL(getServerOwner());
	Client * const client = netObject->getClient();
	FATAL(!client || !netObject->isAuthoritative(), ("Tried to resync movement updates either while nonauth or without a client!"));
	m_teleportIds.clear();
	// Get a sequenceId, and negate it, signalling that we aren't actually updating a transform, just have movement locked out until
	// we're resynced.  Also assure we don't use -1, as it is used for locking out updates while we're missing either authority or a client.
	int sequenceId = -1;
	while (sequenceId == -1)
		sequenceId = -static_cast<int>(netObject->getAndIncrementMoveSequenceNumber());
	IGNORE_RETURN(m_teleportIds.insert(sequenceId));
	appendMessage(
		CM_netUpdateTransform,
		0.0f,
		new MessageQueueDataTransform(0, sequenceId, Transform(), 0.f, 0.f, false),
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

void PlayerCreatureController::teleport(Transform const &goal, ServerObject *goalObj)
{
	ServerObject * const netObject = NON_NULL(getServerOwner());
	Client * const client = netObject->getClient();
	if (client)
	{
		int sequenceId = netObject->getAndIncrementMoveSequenceNumber();
		IGNORE_RETURN(m_teleportIds.insert(sequenceId));
		if (goalObj)
		{
			appendMessage(
				CM_netUpdateTransformWithParent,
				0.0f,
				new MessageQueueDataTransformWithParent(0, sequenceId, goalObj->getNetworkId(), goal, 0.f, 0.f, false),
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
		else
		{
			appendMessage(
				CM_netUpdateTransform,
				0.0f,
				new MessageQueueDataTransform(0, sequenceId, goal, 0.f, 0.f, false),
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
	setGoal(goal, goalObj, true);
	m_lastVerifiedMove.clear();
}

// ----------------------------------------------------------------------

void PlayerCreatureController::setAuthoritative(bool newAuthoritative)
{
	CreatureController::setAuthoritative(newAuthoritative);

	if (!newAuthoritative)
		preventMovementUpdates();

	//-- Ensure movement validation doesn't keep around any leftover state
	//   if this was previously a proxy, and before that was authoritative.
	m_lastVerifiedMove.clear();
}

// ----------------------------------------------------------------------

void PlayerCreatureController::onClientReady()
{
	// This means we are both authoritative and have a client now.  We can start accepting client transforms.
	resyncMovementUpdates();

	// Control our ship if we have one
	CreatureObject * const creature = NON_NULL(getCreature());
	
	ShipObject * const shipObject = creature->getPilotedShip();
	if (shipObject)
		NON_NULL(creature->getClient())->addControlledObject(*shipObject);

	// Fix up link-dead status
	PlayerObject * const player = getPlayerObject(creature);
	if (player)
		player->setLinkDead(false);
}

// ----------------------------------------------------------------------

void PlayerCreatureController::onClientLost()
{
	// Note: this function can be called when the owner has been partially
	// destructed, so we need to handle that case.

	CreatureObject * const creature = NON_NULL(getServerOwner())->asCreatureObject();

	if (creature && creature->isAuthoritative())
	{
		preventMovementUpdates();

		// Fix up link-dead status
		PlayerObject * const player = getPlayerObject(creature);
		if (player)
			player->setLinkDead(true);
	}
}

// ----------------------------------------------------------------------

void PlayerCreatureController::handleTeleportAck(MessageQueueTeleportAck const &msg)
{
	IGNORE_RETURN(m_teleportIds.erase(msg.getSequenceId()));
}

// ----------------------------------------------------------------------

void PlayerCreatureController::logMoveFailed(char const *reason)
{
	CreatureObject const * const creature = NON_NULL(getCreature());
	PlayerObject const * const player = getPlayerObject(creature);

	unsigned int const stationId = player ? static_cast<unsigned int>(player->getStationId()) : 0;

	LOG(
		"movement",
		(
			"move fail - object %s stationId %u - %s",
			creature ? creature->getNetworkId().getValueString().c_str() : "<nullptr>",
			stationId,
			reason));
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::isLocationValid(Vector const &position_w, CellObject const *cell) const
{
	// don't allow people too close to the origin
	if (position_w.withinEpsilon(Vector::zero, 0.1f))
		return false;

	TerrainObject const *const terrainObject = TerrainObject::getConstInstance();
	if (nullptr != terrainObject && (nullptr == cell || cell->getCellProperty()->isWorldCell()))
	{
		if (!terrainObject->isPassableForceChunkCreation(position_w))
			return false;
	}

	if (ConfigServerGame::getMoveCheckDestinationCollision())
	{
		// Translate the player's local collision sphere to the world-space test 
		// position and see if there's anything there. If so, the location isn't valid.

		CollisionProperty const * const collision = getOwner()->getCollisionProperty();

		if (!collision)
			return false;

		Sphere localSphere = collision->getBoundingSphere_l();

		Sphere testSphere(position_w + localSphere.getCenter(), localSphere.getRadius());

		if (CollisionWorld::query(testSphere, nullptr)) 
			return false;
	}
	return true;
}

// ----------------------------------------------------------------------

/*
 * checkValidMove returns whether the move was valid, but also calls
 * handleInvalidMove in those cases where warping back to the last
 * valid spot is the desired behavior.
 */
bool PlayerCreatureController::checkValidMove(MoveSnapshot const &m, float const reportedSpeed)
{
	uint32 const currentClientSyncStamp = m.getSyncStamp();
	CreatureObject * const creature = NON_NULL(getCreature());

	// update the velocity in the serverController
	if (creature != nullptr)
	{
		Vector moveDistance = m.getPosition_w() - creature->getPosition_w();
		moveDistance.y = 0.0f;

		int moveTimeDiffMs = syncStampLongDeltaTime(m_lastMoveSyncStamp, currentClientSyncStamp);

		setCurrentVelocity(moveDistance / (moveTimeDiffMs / 1000.0f));
		//DEBUG_REPORT_LOG(true, ("distance: %1.2f %1.2f time: %1.2f\n", moveDistance.x, moveDistance.y, moveTimeDiffMs / 1000.f));

		m_lastMoveSyncStamp = currentClientSyncStamp;
	}

	// gods can move however they like
	if (isGod())
	{
		// can't move into world cell in space
		if (ServerWorld::isSpaceScene() && !m.getCell())
			return false;

		m_lastVerifiedMove = m;
		return true;
	}

	// only check if checking is turned on	
	if (!ConfigServerGame::getMoveValidationEnabled())
		return true;

	if (!m.isValid())
		return handleInvalidMove("invalid destination");

	if (creature == nullptr)
		return handleInvalidMove("creature is nullptr");
	
	if (!m.isAllowed(*creature))
		return handleInvalidMove("not allowed in dest cell");

	// check to see whether it's possible to exist at the destination
	if (!isLocationValid(m.getPosition_w(), m.getCell()))
		return handleInvalidMove("tried to move to an illegal location");

	// check to see whether it's possible to get from the current pob to the destination pob
	{
		Object const *sourcePob = ContainerInterface::getTopmostContainer(*creature);
		PortalProperty const *sourcePortalProperty = sourcePob ? sourcePob->getPortalProperty() : 0;
		CellObject const * const destCell = m.getCell();
		PortalProperty const *destPortalProperty = destCell ? ContainerInterface::getContainedByObject(*destCell)->getPortalProperty() : 0;
		if (sourcePortalProperty != destPortalProperty)
		{
			// Moving between pobs.  This is only valid if one of these is nullptr, since pobs only connect to the world
			if (sourcePortalProperty && destPortalProperty)
				return handleInvalidMove("tried to move from one pob to another without passing through the world cell");
			if (sourcePortalProperty && !sourcePortalProperty->hasPassablePortalToParentCell())
				return handleInvalidMove("tried to move into the world cell from a pob with no exits");
			if (destPortalProperty && !destPortalProperty->hasPassablePortalToParentCell())
				return handleInvalidMove("tried to move from the world cell into a pob with no entrance");
		}
	}

	Vector distanceToMoveSnapshot = m.getPosition_w() - creature->getPosition_w();
	distanceToMoveSnapshot.y = 0.0f;

	if (distanceToMoveSnapshot.magnitudeSquared () > sqr(ConfigServerGame::getMoveMaxDistance()) && !isCreaturePassenger(*creature))
		return handleInvalidMove("tried to move too far");

	// fail move validation if the inventory is overloaded
	ServerObject *const inventoryObject = creature->getInventory();
	if (inventoryObject)
	{
		VolumeContainer *const inventoryContainer = ContainerInterface::getVolumeContainer(*inventoryObject);
		if ((inventoryContainer) && (inventoryContainer->getCurrentVolume() >= (inventoryContainer->getTotalVolume() + ConfigServerGame::getMoveValidationMaxInventoryOverload())))
		{
			Chat::sendSystemMessage(*creature, StringId("system_msg", "move_fail_inventory_overloaded"), Unicode::emptyString);

			char buffer[128];
			snprintf(buffer, sizeof(buffer)-1, "full inventory %d/%d", inventoryContainer->getCurrentVolume(), inventoryContainer->getTotalVolume());
			buffer[sizeof(buffer)-1] = '\0';

			return handleInvalidMove(buffer);
		}		
	}

	Client * const client = creature->getClient();
	if (!client)
		return handleInvalidMove("Creature's client is nullptr");
	
	uint32 const currentServerSyncStamp = client->getServerSyncStampLong();
	
	// tolerance to account for variations
	float const maxSpeed = m_speedMaximum.getRecentMaximum(currentServerSyncStamp) * ConfigServerGame::getMoveSpeedTolerance();

	if(creature->isDead() || creature->isIncapacitated())
		return handleInvalidMove("Player is dead, or incapped");

	bool doSpeedCheck = true;

	if (!m_lastVerifiedMove.isValid())
	{
		// We don't have a valid last verified move, so assume our current
		// location is valid, but don't do a speed check since we have no idea
		// when this move occurred.  
		m_lastVerifiedMove = MoveSnapshot(
			creature->getPosition_p(),
			safe_cast<CellObject *>(creature->getAttachedTo()),
			currentServerSyncStamp);
		doSpeedCheck = false;
	}

	if (doSpeedCheck)
	{
		uint32 const previousClientSyncStamp = m_lastVerifiedMove.getSyncStamp();
		int timeDiffMs = syncStampLongDeltaTime(previousClientSyncStamp, currentClientSyncStamp);

		bool const sitting = creature->getPosture() == Postures::Sitting;
		// This sitting animation can actually move the character and we want to make
		// sure that these small movements are actually verified and that
		// m_lastVerifiedMove will actually be updated while these small
		// movements occur.  Otherwise we'll eventually generate an invalid
		// location when the client updates itself at the 300 second mark
		// due to the location differences from the sit.
		if (!sitting)
		{
			if (timeDiffMs < ConfigServerGame::getMoveSpeedCheckFrequencyMs())
			{
				// assume we're ok to move, but don't update the last verified
				return true;
			}
		}

		// check for bogus timestamps, making sure to handle timestamp rollover
		if (ConfigServerGame::getMoveValidationCheckForPastTimestamp())
		{
			// new timestamp from client must be greater than previous timestamp from client (i.e. cannot travel back in time)
			if (currentClientSyncStamp < previousClientSyncStamp)
			{
				uint32 const rolloverFudgeFactor = static_cast<uint32>(100000); // 100 seconds

				// pretty sure that timestamp did not rollover
				if ((previousClientSyncStamp > rolloverFudgeFactor) && ((std::numeric_limits<uint32>::max() - rolloverFudgeFactor) > previousClientSyncStamp))
				{
					int const timeDiffMs = syncStampLongDeltaTime(previousClientSyncStamp, currentClientSyncStamp);

					// only fail movement if the difference is beyond a certain threshold,
					// to account for network latency, UDP unreliability, clock drift, etc.
					if (timeDiffMs > ConfigServerGame::getMoveValidationPastTimestampThresholdMs())
					{
						char buf[256];
						snprintf(buf, sizeof(buf)-1, "out of order timestamps (%dms), oss=%lu, css=%lu (%lud:%luh:%lum:%lus), ssc=%lu",
							timeDiffMs,
							previousClientSyncStamp,
							currentServerSyncStamp,
							(currentServerSyncStamp / 1000) / (60 * 60 * 24),
							((currentServerSyncStamp / 1000) % (60 * 60 * 24)) / (60 * 60),
							((currentServerSyncStamp / 1000) % (60 * 60)) / 60,
							(currentServerSyncStamp / 1000) % 60,
							currentClientSyncStamp);
						buf[sizeof(buf)-1] = '\0';

						return handleInvalidMove(buf);
					}
				}
			}
		}

		// check for bogus timestamps, making sure to handle timestamp rollover
		if (ConfigServerGame::getMoveValidationCheckForFutureTimestamp())
		{
			// new timestamp from client must be less than the current server timestamp (i.e. cannot travel into the future)
			if (currentClientSyncStamp > currentServerSyncStamp)
			{
				uint32 const rolloverFudgeFactor = static_cast<uint32>(100000); // 100 seconds

				// pretty sure that timestamp did not rollover
				if ((currentServerSyncStamp > rolloverFudgeFactor) && ((std::numeric_limits<uint32>::max() - rolloverFudgeFactor) > currentServerSyncStamp))
				{
					int const timeDiffMs = syncStampLongDeltaTime(currentClientSyncStamp, currentServerSyncStamp);

					// only fail movement if the difference is beyond a certain threshold,
					// to account for network latency, UDP unreliability, clock drift, etc.
					if (timeDiffMs > ConfigServerGame::getMoveValidationFutureTimestampThresholdMs())
					{
						char buf[256];
						snprintf(buf, sizeof(buf)-1, "timestamp from the future (%dms), oss=%lu, css=%lu (%lud:%luh:%lum:%lus), ssc=%lu",
							timeDiffMs,
							previousClientSyncStamp,
							currentServerSyncStamp,
							(currentServerSyncStamp / 1000) / (60 * 60 * 24),
							((currentServerSyncStamp / 1000) % (60 * 60 * 24)) / (60 * 60),
							((currentServerSyncStamp / 1000) % (60 * 60)) / 60,
							(currentServerSyncStamp / 1000) % 60,
							currentClientSyncStamp);
						buf[sizeof(buf)-1] = '\0';

						return handleInvalidMove(buf);
					}
				}
			}
		}

		// make sure the client and server time hasn't drifted too far apart
		if (ConfigServerGame::getMoveValidationCheckForTimestampDrift())
		{
			int const clientServerTimeDiff = syncStampLongDeltaTime(currentClientSyncStamp, currentServerSyncStamp);
			if (clientServerTimeDiff > ConfigServerGame::getMoveValidationTimestampDriftThresholdMs())
			{
				char buf[256];
				snprintf(buf, sizeof(buf)-1, "timestamp drift (%dms), oss=%lu, css=%lu (%lud:%luh:%lum:%lus), ssc=%lu",
					clientServerTimeDiff,
					previousClientSyncStamp,
					currentServerSyncStamp,
					(currentServerSyncStamp / 1000) / (60 * 60 * 24),
					((currentServerSyncStamp / 1000) % (60 * 60 * 24)) / (60 * 60),
					((currentServerSyncStamp / 1000) % (60 * 60)) / 60,
					(currentServerSyncStamp / 1000) % 60,
					currentClientSyncStamp);
				buf[sizeof(buf)-1] = '\0';

				return handleInvalidMove(buf);
			}
		}

		// check if moving from last verified location to goal location exceeds max speed
		// we should never be dealing with moves that are shorter than 1 ms
		if (timeDiffMs < 1)
			timeDiffMs = 1;
		// We enforce a maximum check distance of twice the check interval, to disallow 'saving up' movement
		// and warping all at once.  This still allows for a substantial amount of that, unfortunately.
		if (timeDiffMs >= ConfigServerGame::getMoveSpeedCheckFrequencyMs() * 2)
			timeDiffMs = ConfigServerGame::getMoveSpeedCheckFrequencyMs() * 2;

		// add in some padding for the case that we're sitting.  This will allow
		// a little more flexibility while playing the sitting animation which
		// will update the character's position on the client.  Because the
		// character is moving at such a low speed it becomes very easy to cause
		// the conditions to generate an invalid move.
		float const maxDistSqr = sitting ? sqr((maxSpeed*timeDiffMs/1000.0f) + 0.1f) : sqr(maxSpeed*timeDiffMs/1000.0f);

		// use only xz for speed
		Vector const oldV = m_lastVerifiedMove.getPosition_w();
		Vector const newV = m.getPosition_w();
		float distSqr = sqr(newV.x-oldV.x) + sqr(newV.z-oldV.z);

		if (distSqr > maxDistSqr)
		{
			time_t const previousSpeedCheckFailureTime = m_lastSpeedCheckFailureTime;

			// Our player has moved too far. Now we check if his move is just outrageous. If so, we are going to skip the leeway count and immediate correct him.
			// The math is the exact code we used above - we're just using a different scalar for the time value.
			float const maxSpeedPerUpdateLimit = m_speedMaximum.getRecentMaximum(currentServerSyncStamp) * ConfigServerGame::getMoveMaxDistanceLeewayPerUpdate();
			float const maxDistSqrPerUpdate = sitting ? sqr((maxSpeedPerUpdateLimit*timeDiffMs/1000.0f) + 0.1f) : sqr(maxSpeedPerUpdateLimit*timeDiffMs/1000.0f);

			if(distSqr > maxDistSqrPerUpdate)
			{

				Vector const serverLoc = creature->getPosition_w();
				char buf[512];
				snprintf(buf, sizeof(buf)-1, "too fast (dist=%g, maxDistLower=%g, maxDistUpper=%g, legalSpeed=%g, maxSpeedLower=%g, maxSpeedUpper=%g, reportedSpeed=%g, clientTimeDiff=%d, serverTimeDiff=%d, clientServerTimeDiff=%d, failureInterval=%lds, failureCount=%d, lastVerifiedLoc=(%g,%g,%g,%lu), reportedLoc=(%g,%g,%g,%lu), serverLoc=(%g,%g,%g,%lu))",
					sqrt(distSqr),
					sqrt(maxDistSqr),
					sqrt(maxDistSqrPerUpdate),
					m_speedMaximum.getRecentMaximum(currentServerSyncStamp),
					maxSpeed,
					maxSpeedPerUpdateLimit,
					reportedSpeed,
					timeDiffMs,
					syncStampLongDeltaTime(previousClientSyncStamp, currentServerSyncStamp),
					syncStampLongDeltaTime(currentClientSyncStamp, currentServerSyncStamp),
					(m_lastSpeedCheckFailureTime - previousSpeedCheckFailureTime),
					m_speedCheckConsecutiveFailureCount,
					oldV.x, oldV.y, oldV.z, previousClientSyncStamp,
					newV.x, newV.y, newV.z, currentClientSyncStamp,
					serverLoc.x, serverLoc.y, serverLoc.z, currentServerSyncStamp);
				buf[sizeof(buf)-1] = '\0';
				return handleInvalidMove(buf);
			}

			
			m_lastSpeedCheckFailureTime = ::time(nullptr);
			++m_speedCheckConsecutiveFailureCount;

			// if this is the first validation failure "in a while", let it pass, because it may be a false positive
			if ((previousSpeedCheckFailureTime + ConfigServerGame::getMoveSpeedCheckFailureLeewaySeconds()) >= m_lastSpeedCheckFailureTime)
			{
				if (m_speedCheckConsecutiveFailureCount >= ConfigServerGame::getMoveSpeedCheckFailureLeewayCount())
				{

					Vector const serverLoc = creature->getPosition_w();
					char buf[512];
					snprintf(buf, sizeof(buf)-1, "too fast (dist=%g, maxDistLower=%g, maxDistUpper=%g, legalSpeed=%g, maxSpeedLower=%g, maxSpeedUpper=%g, reportedSpeed=%g, clientTimeDiff=%d, serverTimeDiff=%d, clientServerTimeDiff=%d, failureInterval=%lds, failureCount=%d, lastVerifiedLoc=(%g,%g,%g,%lu), reportedLoc=(%g,%g,%g,%lu), serverLoc=(%g,%g,%g,%lu))",
						sqrt(distSqr),
						sqrt(maxDistSqr),
						sqrt(maxDistSqrPerUpdate),
						m_speedMaximum.getRecentMaximum(currentServerSyncStamp),
						maxSpeed,
						maxSpeedPerUpdateLimit,
						reportedSpeed,
						timeDiffMs,
						syncStampLongDeltaTime(previousClientSyncStamp, currentServerSyncStamp),
						syncStampLongDeltaTime(currentClientSyncStamp, currentServerSyncStamp),
						(m_lastSpeedCheckFailureTime - previousSpeedCheckFailureTime),
						m_speedCheckConsecutiveFailureCount,
						oldV.x, oldV.y, oldV.z, previousClientSyncStamp,
						newV.x, newV.y, newV.z, currentClientSyncStamp,
						serverLoc.x, serverLoc.y, serverLoc.z, currentServerSyncStamp);
					buf[sizeof(buf)-1] = '\0';
					return handleInvalidMove(buf);
				}
			}
			else
			{
				m_speedCheckConsecutiveFailureCount = 0;
			}
		}
		else
		{
			m_speedCheckConsecutiveFailureCount = 0;
		}
	}

	// set last verified location
	m_lastVerifiedMove = m;
	return true;
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::handleInvalidMove(char const *failReason)
{
	if (isGod())
	{
		//-- Don't move gods back but do log the reason.
		logMoveFailed(failReason);
		return true;
	}
	
	CreatureObject * const creature = NON_NULL(getCreature());

	// if we are the passenger on a mount then the mount's location, not our last
	// good location should be the destination of any warps.
	{
		bool const isPassenger = isCreaturePassenger(*creature);
		CreatureObject * const mountCreature = creature->getMountedCreature();
		if(isPassenger && mountCreature)
		{
			m_lastVerifiedMove = MoveSnapshot(
				mountCreature->getPosition_p(),
				safe_cast<CellObject *>(mountCreature->getAttachedTo()),
				getCurSyncStamp());
		}
	// if we don't have a last verified move, set one from the current location
		else if (!m_lastVerifiedMove.isValid())
	{
		m_lastVerifiedMove = MoveSnapshot(
			creature->getPosition_p(),
			safe_cast<CellObject *>(creature->getAttachedTo()),
			getCurSyncStamp());
	}
	}

	// if the last valid location doesn't reject us, warp to it
	CellObject * const cell = m_lastVerifiedMove.getCell();
	if (cell)
	{
		if (cell->isAllowed(*creature))
		{
			// Previously in a cell, and still allowed in, so send them back to the
			// old location, but at the new orientation.
			Transform t(cell->getTransform_o2w().rotateTranslate_p2l(creature->getTransform_o2w()));
			t.setPosition_p(m_lastVerifiedMove.getPosition_p());
			teleport(t, cell);
		}
		else
		{
			// previously in a cell, but no longer allowed, so expel from the building of that cell
			cell->getOwnerBuilding()->expelObject(*creature);
		}
	}
	else if (isLocationValid(m_lastVerifiedMove.getPosition_w(), m_lastVerifiedMove.getCell()))
	{
		Transform objectToWorld(creature->getTransform_o2w());
		objectToWorld.setPosition_p(m_lastVerifiedMove.getPosition_w());
		teleport(objectToWorld, 0);
	}
	else
	{
		// we can't come up with a place we're allowed to be, so let them move
		m_lastVerifiedMove.clear();
		return true;
	}

	logMoveFailed(failReason);
	return false;
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::handleMove(Vector const &position_w, uint32 syncStamp, float const reportedSpeed)
{
	if (isTeleporting() || getCreature()->getInvulnerabilityTimer() > 0.f)
		return false;

	return checkValidMove(MoveSnapshot(position_w, 0, syncStamp), reportedSpeed);
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::handleMove(Vector const &position_p, NetworkId const &cellObjectId, uint32 syncStamp, float const reportedSpeed)
{
	if (isTeleporting() || getCreature()->getInvulnerabilityTimer() > 0.f)
		return false;

	CellObject * const cell = getCellObject(cellObjectId);
	if (cell)
		return checkValidMove(MoveSnapshot(position_p, cell, syncStamp), reportedSpeed);

	//-- Build a description of what's wrong.
	char buffer[512];
	snprintf(buffer, sizeof(buffer) - 1, "player specified non-existent cell with id=[%s].", cellObjectId.getValueString().c_str());
	buffer[sizeof(buffer) - 1] = '\0';

	//-- Log the invalid move.
	return handleInvalidMove(buffer);
}

// ----------------------------------------------------------------------

void PlayerCreatureController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	CreatureObject * const owner = NON_NULL(getCreature());
	PlayerObject * const playerOwner = getPlayerObject(owner);

	switch (message)
	{
	case CM_teleportAck:
		{
			const MessageQueueTeleportAck *const msg = dynamic_cast<const MessageQueueTeleportAck *>(data);
			if (msg)
				handleTeleportAck(*msg);
		}
		break;
	case CM_netUpdateTransform:
		{
			// player creatures cannot move if not in the world (pilots, gunners)
			if (!owner->isInWorld())
				break;

			const MessageQueueDataTransform *const msg = dynamic_cast<const MessageQueueDataTransform *>(data);
			if (msg)
			{
				bool const fromClient = (flags&GameControllerMessageFlags::SOURCE_REMOTE_CLIENT) ? true : false;
				if ((!owner->isAuthoritative()) ^ fromClient)
				{
					if (flags & GameControllerMessageFlags::RELIABLE)
						m_sendReliableTransformThisFrame = true;

					if (   !fromClient
					    || handleMove(msg->getTransform().getPosition_p(), msg->getSyncStamp(), msg->getSpeed()))
					{
						DEBUG_REPORT_LOG(ConfigServerGame::getDebugMovement(), ("nut (%g,%g,%g) %s\n",
							msg->getTransform().getPosition_p().x,
							msg->getTransform().getPosition_p().y,
							msg->getTransform().getPosition_p().z,
							fromClient ? "client" : "server"));
						CreatureController::handleMessage(message, value, data, flags);
					}
				}

				Vector p1(msg->getTransform().getPosition_p());
				Vector p2(owner->getPosition_p());
				if (p1.magnitudeBetweenSquared(p2) > 0.0001f)
				{
					if ( fromClient )
					{
						playerMovedAndNeedsToCancelWarmup( *owner );
					}
					m_movingTimeout = 2.0f;
				}
				m_lastSpeed = msg->getSpeed();
			}
		}
		break;
	case CM_netUpdateTransformWithParent:
		{
			// player creatures cannot move if not in the world (pilots, gunners)
			if (!owner->isInWorld())
				break;

			const MessageQueueDataTransformWithParent *const msg = dynamic_cast<const MessageQueueDataTransformWithParent *>(data);
			if (msg)
			{
				bool const fromClient = (flags&GameControllerMessageFlags::SOURCE_REMOTE_CLIENT) ? true : false;
				if ((!owner->isAuthoritative()) ^ fromClient)
				{
					if (flags & GameControllerMessageFlags::RELIABLE)
						m_sendReliableTransformThisFrame = true;

					if (   !fromClient
					    || handleMove(msg->getTransform().getPosition_p(), msg->getParent(), msg->getSyncStamp(), msg->getSpeed()))
					{
						DEBUG_REPORT_LOG(ConfigServerGame::getDebugMovement(), ("nutwp (%g,%g,%g) %s %s\n",
							msg->getTransform().getPosition_p().x,
							msg->getTransform().getPosition_p().y,
							msg->getTransform().getPosition_p().z,
							msg->getParent().getValueString().c_str(),
							fromClient ? "client" : "server"));
						CreatureController::handleMessage(message, value, data, flags);
					}
				}

				if (msg->getTransform().getPosition_p() != owner->getPosition_p())
				{
					if ( fromClient )
					{
						playerMovedAndNeedsToCancelWarmup( *owner );
					}
					m_movingTimeout = 2.0f;
				}
			}
		}
		break;
	case CM_objectMenuRequest:
		{
			MessageQueueObjectMenuRequest const * const msg = safe_cast<MessageQueueObjectMenuRequest const *>(data);
			if (msg)
				handleObjectMenuRequest(msg);
		}
		break;
	case CM_missionAbort:
		{
			MessageQueueNetworkId const * const msg = safe_cast<MessageQueueNetworkId const *>(data);
			MissionObject * const missionObject = dynamic_cast<MissionObject *>(NetworkIdManager::getObjectById(msg->getNetworkId()));
			if (missionObject)
			{
				MessageQueueNetworkId * const rsp = new MessageQueueNetworkId(msg->getNetworkId());
				missionObject->abortMission();
				appendMessage(static_cast<int>(CM_missionAbort), 0.0f, rsp, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
		break;
	case CM_missionListRequest:
		{
			// get mission listing from the mission board
			if (owner->isPlayerControlled())
			{
				MessageQueueMissionListRequest const * const msg = dynamic_cast<MessageQueueMissionListRequest const *>(data);
				
				if (msg)
				{
					// get the mission terminal
					ServerObject * const terminal = ServerWorld::findObjectByNetworkId(msg->getTerminalId());
					Container::ContainerErrorCode tmp = Container::CEC_Success;

					//hack since some mission terminals are placed sort of inside pillars, they fail LOS checks.  So allow
					//players to use mission terminals they can't see.  This is ok, because we don't care about permissions
					//which are the only things checked in canManipulate after the LOS check.
					if (   terminal
					    && isMissionTerminal(*terminal)
					    && (   owner->canManipulateObject(*terminal, false, false, false, 12.0f, tmp)
					        || tmp == Container::CEC_CantSee))
					{
						owner->setMissionBoardUISequenceId(msg->getSequenceId());
						owner->addToMissionRequestQueue(terminal->getNetworkId());
					}
				}
			}
		}
		break;
	case CM_missionAcceptRequest:
		{
			MessageQueueMissionGenericRequest const * const msg = dynamic_cast<MessageQueueMissionGenericRequest const *>(data);
			if (msg)
			{
				MessageQueueMissionGenericResponse * const rsp = new MessageQueueMissionGenericResponse(msg->getMissionObjectId(), false, msg->getSequenceId ());
				MissionObject * const missionObject = safe_cast<MissionObject *>(ServerWorld::findObjectByNetworkId(msg->getMissionObjectId()));
				ServerObject * const missionTerminal = ServerWorld::findObjectByNetworkId(msg->getTerminalId());
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				if (   missionObject
				    && missionTerminal
				    && isMissionTerminal(*missionTerminal)
				    && (   owner->canManipulateObject(*missionTerminal, false, false, false, 12.0f, tmp)
				        || tmp == Container::CEC_CantSee))
				{
					missionObject->grantMissionTo(owner->getNetworkId());
					rsp->setSuccess(true);
				}
				
				appendMessage(static_cast<int>(CM_missionAcceptResponse), 0.0f, rsp, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
		break;
	case CM_fillSchematicSlotMessage:
		{
			MessageQueueCraftFillSlot const * const msg = NON_NULL (dynamic_cast<MessageQueueCraftFillSlot const *>(data));
			if (owner->isAuthoritative())
			{
				if (getSecureTrade())
					getSecureTrade()->cancelTrade(*owner);
				Crafting::CraftingError result = playerOwner->fillSlot(msg->getSlot(), msg->getOption(), msg->getIngredient());
				MessageQueueGenericIntResponse * const response = new MessageQueueGenericIntResponse(message, result, msg->getSequenceId());
				appendMessage(CM_craftingResult, 0.0f, response, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
		break;
	case CM_emptySchematicSlotMessage:
		{
			MessageQueueCraftEmptySlot const * const msg = NON_NULL(dynamic_cast<MessageQueueCraftEmptySlot const *>(data));
			if (owner->isAuthoritative())
			{
				if (getSecureTrade())
					getSecureTrade()->cancelTrade(*owner);
				Crafting::CraftingError result = playerOwner->emptySlot(msg->getSlot(), msg->getTargetContainer());
				MessageQueueGenericIntResponse * const response = new MessageQueueGenericIntResponse(message, result, msg->getSequenceId());
				appendMessage(CM_craftingResult, 0.0f, response, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
		break;
	case CM_changeRoleIconChoice:
		{
			MessageQueueChangeRoleIconChoice const * const msg = NON_NULL(dynamic_cast<MessageQueueChangeRoleIconChoice const *>(data));
			if (owner->isAuthoritative() && playerOwner)
			{
				playerOwner->setRoleIconChoice(msg->getRoleIconChoice());
			}
		}
		break;
	case CM_experimentMessage:
		{
			MessageQueueCraftExperiment const * const msg = NON_NULL(dynamic_cast<MessageQueueCraftExperiment const *>(data));
			if (owner->isAuthoritative())
			{
				if (getSecureTrade())
					getSecureTrade()->cancelTrade(*owner);
				int coreLevel = msg->getCoreLevel();
				Crafting::CraftingResult result = playerOwner->experiment(msg->getExperiments(), msg->getTotalPoints(), coreLevel);
				MessageQueueGenericIntResponse * const response = new MessageQueueGenericIntResponse(message, result, msg->getSequenceId());
				appendMessage(CM_experimentResult, 0.0f, response, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
		break;
	case CM_setCustomizationData:
		{
			MessageQueueCraftCustomization const * const msg = NON_NULL(dynamic_cast<MessageQueueCraftCustomization const *>(data));
			if (owner->isAuthoritative())
			{
				if (getSecureTrade())
					getSecureTrade()->cancelTrade(*owner);
				int const result = playerOwner->setCustomizationData(msg->getName(), msg->getAppearance(), msg->getCustomization(), msg->getItemCount());
				MessageQueueGenericIntResponse * const response = new MessageQueueGenericIntResponse(message, result, 0);
				appendMessage(CM_craftingResult, 0.0f, response, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
		break;
	case CM_setMaxForcePower:
		{
			MessageQueueGenericValueType<int> const * const msg = safe_cast<MessageQueueGenericValueType<int> const *>(data);
			// set the object value and send a message to all it's proxies
			if (msg)
				playerOwner->setMaxForcePower(msg->getValue());
			DEBUG_WARNING(!msg, ("PlayerCreatureController (%s:%s) received a CM_setMaxForcePower message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
		}
		break;
	case CM_setForcePower:
		{
			MessageQueueGenericValueType<int> const * const msg = safe_cast<MessageQueueGenericValueType<int> const *>(data);
			// set the object value and send a message to all it's proxies
			if (msg)
				playerOwner->setForcePower(msg->getValue());
			DEBUG_WARNING(!msg, ("PlayerCreatureController (%s:%s) received a CM_setForcePower message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
		}
		break;
	case CM_setForceRegenRate:
		{
			MessageQueueGenericValueType<float> const * const msg = safe_cast<MessageQueueGenericValueType<float> const *>(data);
			// set the object value and send a message to all it's proxies
			if (msg)
				playerOwner->setForcePowerRegenRate(msg->getValue());
			DEBUG_WARNING(!msg, ("PlayerCreatureController (%s:%s) received a CM_setForceRegen message but could not retrieve data from the message supplied", owner->getObjectTemplateName(), owner->getNetworkId().getValueString().c_str()));
		}
		break;

	case CM_imageDesignerChange:
		{
			ImageDesignChangeMessage const * const inMsg = safe_cast<ImageDesignChangeMessage const *>(data);
			if (inMsg)
			{
				NetworkId const & myId = owner->getNetworkId();
				NetworkId const & designerId = inMsg->getDesignerId();
				NetworkId const & recipientId = inMsg->getRecipientId();
				Object const * const designerObj = NetworkIdManager::getObjectById(designerId);
				ServerObject const * const designerServer = designerObj ? designerObj->asServerObject() : nullptr;
				CreatureObject const * const designer = designerServer ? designerServer->asCreatureObject() : nullptr;
				Object * const recipientObj = NetworkIdManager::getObjectById(recipientId);
				ServerObject * const recipientServer = recipientObj ? recipientObj->asServerObject() : nullptr;
				CreatureObject * const recipient = recipientServer ? recipientServer->asCreatureObject() : nullptr;
				if(designer && recipient)
				{
					//designer-sent message, either data to sync to the recipient, or committed data to check and apply
					if(myId == designerId)
					{
						if(!inMsg->getAccepted())
						{
							//designer hasn't accepted yet, send change to the client so they can see it before agreeing
							Object * const recipientObject = NetworkIdManager::getObjectById(inMsg->getRecipientId());
							Controller * const recipientController = recipientObject ? recipientObject->getController() : nullptr;
							if(recipientController)
							{
								ImageDesignChangeMessage * outMsg = new ImageDesignChangeMessage();

								*outMsg = *inMsg;
								recipientController->appendMessage(CM_imageDesignerChange, 0, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
							}
						}
						else
						{
							if(designer != recipient)
							{
								//check and apply this data once the recipient also accepts
								Object * const recipientObject = NetworkIdManager::getObjectById(inMsg->getRecipientId());
								Controller * const recipientController = recipientObject ? recipientObject->getController() : nullptr;
								if(recipientController)
								{
									ImageDesignChangeMessage * outMsg = new ImageDesignChangeMessage();

									*outMsg = *inMsg;
									recipientController->appendMessage(CM_imageDesignerChange, 0, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
								}
							}
						}
					}
					//recipient-sent message, either sending the amount of money to pay, or accepted data to check and apply
					if(myId == recipientId)
					{
						if(!inMsg->getAccepted())
						{
							//recipient hasn't accepted yet, send the data to the designer to update the recipient-sent amount of money
							//designer hasn't accepted yet, send change to the client so they can see it before agreeing
							Object * const designerObject = NetworkIdManager::getObjectById(inMsg->getDesignerId());
							Controller * const designerController = designerObject ? designerObject->getController() : nullptr;
							if(designerController)
							{
								ImageDesignChangeMessage * outMsg = new ImageDesignChangeMessage();

								*outMsg = *inMsg;
								outMsg->setOrigin(ImageDesignChangeMessage::O_SERVER);
								designerController->appendMessage(CM_imageDesignerChange, 0, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
							}
						}
						else
						{
							//check and apply this data once the designer also accepts
							SharedImageDesignerManager::Session session(*inMsg);

							//get the server-stored session, so we can get the real starting time
							SharedImageDesignerManager::Session serverSession;
							bool const result = SharedImageDesignerManager::getSession(session.designerId, serverSession);
							if(result)
								session.startingTime = serverSession.startingTime;
							//ensure we validate against the server's skill mod data
							SharedImageDesignerManager::SkillMods const skillMods = ServerImageDesignerManager::getSkillModsForDesigner(session.designerId);
							session.bodySkillMod = skillMods.bodySkillMod;
							session.faceSkillMod = skillMods.faceSkillMod;
							session.markingsSkillMod = skillMods.markingsSkillMod;
							session.hairSkillMod = skillMods.hairSkillMod;
							SharedImageDesignerManager::updateSession(session);

							std::string const recipientSpeciesGender = CustomizationManager::getServerSpeciesGender(*recipient);
							CustomizationData * const customizationData = recipient->fetchCustomizationData();
							ServerObject * const hair = recipient->getHair();
							TangibleObject * const tangibleHair = hair ? hair->asTangibleObject() : nullptr;
							CustomizationData * customizationDataHair = nullptr;
							if(tangibleHair)
								customizationDataHair = tangibleHair->fetchCustomizationData();
							if(customizationData)
							{
								if(SharedImageDesignerManager::isSessionValid(session, skillMods, recipientSpeciesGender, customizationData, customizationDataHair))
								{
									time_t const timeLeft = SharedImageDesignerManager::getTimeRemaining(session.designerId);
									if(timeLeft <= 0)
									{
										ServerImageDesignerManager::sendSessionToScriptForValidation(session);
									}
									else
									{
										Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_not_enough_time_passed, Unicode::emptyString);
										if(designer != recipient)
											Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_not_enough_time_passed, Unicode::emptyString);
										ServerImageDesignerManager::cancelSession(designer->getNetworkId(), recipient->getNetworkId());
									}
								}
								else
								{
									Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_session_not_valid, Unicode::emptyString);
									if(designer != recipient)
										Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_session_not_valid, Unicode::emptyString);
									ServerImageDesignerManager::cancelSession(designer->getNetworkId(), recipient->getNetworkId());
								}
								customizationData->release();
							}
							else
							{
								ServerImageDesignerManager::cancelSession(designer->getNetworkId(), recipient->getNetworkId());
							}
						}
					}
				}
			}
		}
		break;

	case CM_imageDesignerCancel:
		{
			ImageDesignChangeMessage const * const inMsg = safe_cast<ImageDesignChangeMessage const *>(data);
			if (inMsg)
			{
				//cancel the session on the server
				SharedImageDesignerManager::Session session;
				bool const result = SharedImageDesignerManager::getSession(inMsg->getDesignerId(), session);
				if(result)
				{
					//if the recipient canceled the session, tell the designer
					Object const * const designerObject = NetworkIdManager::getObjectById(session.designerId);
					ServerObject const * const designer = designerObject ? designerObject->asServerObject() : nullptr;
					if(designer && (owner->getNetworkId() != session.designerId))
						Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_canceled_by_recip, Unicode::emptyString);

					ServerImageDesignerManager::cancelSession(inMsg->getDesignerId(), inMsg->getRecipientId());
				}
			}
		}
		break;

	case CM_buffBuilderChange:
		{
			BuffBuilderChangeMessage const * const inMsg = safe_cast<BuffBuilderChangeMessage const *>(data);
			if (inMsg)
			{
				NetworkId const & myId = owner->getNetworkId();
				NetworkId const & bufferId = inMsg->getBufferId();
				NetworkId const & recipientId = inMsg->getRecipientId();
				Object const * const bufferObj = NetworkIdManager::getObjectById(bufferId);
				ServerObject const * const bufferServer = bufferObj ? bufferObj->asServerObject() : nullptr;
				CreatureObject const * const buffer = bufferServer ? bufferServer->asCreatureObject() : nullptr;
				Object * const recipientObj = NetworkIdManager::getObjectById(recipientId);
				ServerObject * const recipientServer = recipientObj ? recipientObj->asServerObject() : nullptr;
				CreatureObject * const recipient = recipientServer ? recipientServer->asCreatureObject() : nullptr;
				if(buffer && recipient)
				{
					//buffer-sent message, either data to sync to the recipient, or committed data to check and apply
					if(myId == bufferId)
					{
						if(!inMsg->getAccepted())
						{
							//buffer hasn't accepted yet, send change to the client so they can see it before agreeing
							Object * const recipientObject = NetworkIdManager::getObjectById(inMsg->getRecipientId());
							Controller * const recipientController = recipientObject ? recipientObject->getController() : nullptr;
							if(recipientController)
							{
								BuffBuilderChangeMessage * outMsg = new BuffBuilderChangeMessage();

								*outMsg = *inMsg;
								recipientController->appendMessage(CM_buffBuilderChange, 0, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
							}
						}
						else
						{
							if(buffer != recipient)
							{
								//check and apply this data once the recipient also accepts
								Object * const recipientObject = NetworkIdManager::getObjectById(inMsg->getRecipientId());
								Controller * const recipientController = recipientObject ? recipientObject->getController() : nullptr;
								if(recipientController)
								{
									BuffBuilderChangeMessage * outMsg = new BuffBuilderChangeMessage();
									*outMsg = *inMsg;
									recipientController->appendMessage(CM_buffBuilderChange, 0, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
								}
							}
						}
					}
					//recipient-sent message, either sending the amount of money to pay, or accepted data to check and apply
					if(myId == recipientId)
					{
						if(!inMsg->getAccepted())
						{
							//recipient hasn't accepted yet, send the data to the buffer to update the recipient-sent amount of money
							//buffer hasn't accepted yet, send change to the client so they can see it before agreeing
							Object * const bufferObject = NetworkIdManager::getObjectById(inMsg->getBufferId());
							Controller * const bufferController = bufferObject ? bufferObject->getController() : nullptr;
							if(bufferController)
							{
								BuffBuilderChangeMessage * outMsg = new BuffBuilderChangeMessage();

								*outMsg = *inMsg;
								outMsg->setOrigin(BuffBuilderChangeMessage::O_SERVER);
								bufferController->appendMessage(CM_buffBuilderChange, 0, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
							}
						}
						else
						{
							//check and apply this data once the buffer also accepts
							SharedBuffBuilderManager::Session session(*inMsg);

							//get the server-stored session, so we can get the real starting time
							SharedBuffBuilderManager::Session serverSession;
							bool const result = SharedBuffBuilderManager::getSession(session.bufferId, serverSession);
							if(result)
							{
								session.startingTime = serverSession.startingTime;
							}
							
							SharedBuffBuilderManager::updateSession(session);

							if(SharedBuffBuilderManager::isSessionValid(session))
							{
								time_t const timeLeft = SharedBuffBuilderManager::getTimeRemaining(session.bufferId);
								if(timeLeft <= 0)
								{
									ServerBuffBuilderManager::sendSessionToScriptForValidation(session);
								}
								else
								{
									// JU_TODO: change to a buff builder specific message
									Chat::sendSystemMessage(*buffer, SharedStringIds::imagedesigner_not_enough_time_passed, Unicode::emptyString);
									if(buffer != recipient)
									{
										// JU_TODO: change to a buff builder specific message
										Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_not_enough_time_passed, Unicode::emptyString);
									}
									ServerBuffBuilderManager::cancelSession(buffer->getNetworkId(), recipient->getNetworkId());
								}
							}
							else
							{
								// JU_TODO: change to a buff builder specific message
								Chat::sendSystemMessage(*buffer, SharedStringIds::imagedesigner_session_not_valid, Unicode::emptyString);
								if(buffer != recipient)
								{
									// JU_TODO: change to a buff builder specific message
									Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_session_not_valid, Unicode::emptyString);
								}
								ServerBuffBuilderManager::cancelSession(buffer->getNetworkId(), recipient->getNetworkId());
							}	
						}
					}
				}
			}
		}
		break;

	case CM_buffBuilderCancel:
		{
			BuffBuilderChangeMessage const * const inMsg = safe_cast<BuffBuilderChangeMessage const *>(data);
			if (inMsg)
			{
				//cancel the session on the server
				SharedBuffBuilderManager::Session session;
				bool const result = SharedBuffBuilderManager::getSession(inMsg->getBufferId(), session);
				if(result)
				{
					//if the recipient canceled the session, tell the designer
					Object const * const bufferObject = NetworkIdManager::getObjectById(session.bufferId);
					ServerObject const * const buffer = bufferObject ? bufferObject->asServerObject() : nullptr;
					
					if(buffer && (owner->getNetworkId() != session.bufferId))
					{
						Chat::sendSystemMessage(*buffer, SharedStringIds::buffbuilder_canceled_by_recip, Unicode::emptyString);
					}

					ServerBuffBuilderManager::cancelSession(inMsg->getBufferId(), inMsg->getRecipientId());
				}
			}
		}
		break;

	case CM_spaceTerminalRequest:
		{
			CreatureObject const * const owner = NON_NULL(getCreature());
			if(owner)
			{
				const MessageQueueGenericValueType<NetworkId> * const msg = dynamic_cast<MessageQueueGenericValueType<NetworkId> const *>(data);
				if(msg)
				{
					Object const * const terminalO = NetworkIdManager::getObjectById(msg->getValue());
					ServerObject const * const terminal = terminalO ? terminalO->asServerObject() : nullptr;
					if (terminal)
					{
						Client const * const client = owner->getClient();
						if (!client)
						{
							Chat::sendSystemMessage(*owner, Unicode::narrowToWide("(unlocalized) Could not get the player for finding ship parking data, unexpected behavior may occur."), Unicode::emptyString);
							return;
						}
						typedef std::vector<std::pair<NetworkId, std::string > > Payload;
						Payload outData;
						Object const * const buildingO = ContainerInterface::getTopmostContainer(*terminal);
						ServerObject const * const building = buildingO ? buildingO->asServerObject() : nullptr;
						if (building) {
							DynamicVariableList const & buildingObjVars = building->getObjVars();
							std::string terminalParkingLocation;
							bool const result = buildingObjVars.getItem("travel.point_name", terminalParkingLocation);

							if (result)
								outData.push_back(std::make_pair(terminal->getNetworkId(), terminalParkingLocation));
							else
								Chat::sendSystemMessage(*owner, Unicode::narrowToWide("(unlocalized) This terminal is not registered with the planet, unexpected behavior may occur."), Unicode::emptyString);

							std::vector<NetworkId> ships;
							owner->getAllShipsInDatapad(ships);
							for (std::vector<NetworkId>::const_iterator i = ships.begin(); i != ships.end(); ++i) {
								Object const * const shipO = NetworkIdManager::getObjectById(*i);
								ServerObject const * const shipSO = shipO ? shipO->asServerObject() : nullptr;
								ShipObject const * const ship = shipSO ? shipSO->asShipObject() : nullptr;
								if (ship) {
									ContainedByProperty const * const contained = ship->getContainedByProperty();
									Object const * const containerO = contained ? contained->getContainedBy() : nullptr;
									ServerObject const * const container = containerO ? containerO->asServerObject() : nullptr;
									if(container) {
										DynamicVariableList const & shipControlDeviceObjVars = container->getObjVars();
										std::string shipParkingLocation;
										IGNORE_RETURN(shipControlDeviceObjVars.getItem("strParkingLocation", shipParkingLocation));
										//push the data in, even if the parking location is empty (empty is acceptable, and used for newly-created ships)
										outData.push_back(std::make_pair(ship->getNetworkId(), shipParkingLocation));
									}
								}
							}
						}
						else {
							Chat::sendSystemMessage(*owner, Unicode::narrowToWide("(unlocalized) Could not find building that space terminal is in, cannot resolve parking information."), Unicode::emptyString);
						}
						MessageQueueGenericValueType<Payload> * const msg = new MessageQueueGenericValueType<Payload>(outData);
						appendMessage(static_cast<int>(CM_spaceTerminalResponse), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
					}
					else
					{
						Chat::sendSystemMessage(*owner, Unicode::narrowToWide("(unlocalized) Bad space terminal received, cannot resolve parking information."), Unicode::emptyString);
					}
				}
				else
				{
					Chat::sendSystemMessage(*owner, Unicode::narrowToWide("(unlocalized) Unexpected message, cannot resolve parking information."), Unicode::emptyString);
				}
			}
		}
		break;

	case CM_droidCommandProgramming:
		{
			DroidCommandProgrammingMessage const * const inMsg = safe_cast<DroidCommandProgrammingMessage const *>(data);
			if (inMsg)
			{
				CreatureObject * const owner = NON_NULL(getCreature());
				if(owner)
				{
					GameScriptObject * const scriptObject = owner->getScriptObject();
					if (scriptObject)
					{
						std::vector<std::string> const & commands = inMsg->getCommands();
						std::vector<NetworkId> const & chipsToAdd = inMsg->getChipsToAdd();
						std::vector<NetworkId> const & chipsToRemove = inMsg->getChipsToRemove();
						NetworkId const & droidControlDevice = inMsg->getDroidControlDevice();

						ScriptParams params;
						params.addParam(droidControlDevice);
						std::vector<char const *> commandStrs;
						for(std::vector<std::string>::const_iterator i = commands.begin(); i != commands.end(); ++i)
						{
							commandStrs.push_back(i->c_str());
						}
						params.addParam(commandStrs);
						params.addParam(chipsToAdd);
						params.addParam(chipsToRemove);
						IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_PROGRAM_DROID_COMMANDS, params));
					}
				}
			}
		}
		break;

	case CM_inviteOtherGroupMembersToLaunchIntoSpace:
		{
			CreatureObject const * const owner = NON_NULL(getCreature());
			if(owner != 0)
			{
				MessageQueueGenericValueType<NetworkId> const * const msg = safe_cast<MessageQueueGenericValueType<NetworkId> const *>(data);
				if (msg != 0)
				{
					NetworkId const & playerToAsk = msg->getValue();

					Object * const playerToAskObject = NetworkIdManager::getObjectById(playerToAsk);
					Controller * const playerToAskController = (playerToAskObject != 0) ? playerToAskObject->getController() : 0;

					if (playerToAskController != 0)
					{
						MessageQueueGenericValueType<NetworkId> * data = new MessageQueueGenericValueType<NetworkId>(owner->getNetworkId());

						playerToAskController->appendMessage(CM_askGroupMemberToLaunchIntoSpace,
														0.0f,
														data,
														GameControllerMessageFlags::SEND |
														GameControllerMessageFlags::RELIABLE |
														GameControllerMessageFlags::DEST_AUTH_CLIENT);
					}
				}
			}
		}
		break;

	case CM_groupMemberInvitationToLaunchIntoSpaceResponse:
		{
			NetworkId const & acceptingPlayer = NON_NULL(getCreature())->getNetworkId();

			typedef std::pair<NetworkId, bool> Payload;

			MessageQueueGenericValueType<Payload> const * const msg = safe_cast<MessageQueueGenericValueType<Payload> const *>(data);
			if (msg != 0)
			{
				Payload const incomingPayload = msg->getValue();

				Object * const invitingPlayerObject = NetworkIdManager::getObjectById(incomingPayload.first);
				Controller * const invitingPlayerController = (invitingPlayerObject != 0) ? invitingPlayerObject->getController() : 0;

				if (invitingPlayerController != 0)
				{
					Payload outgoingPayload;
					outgoingPayload.first = acceptingPlayer;
					outgoingPayload.second = incomingPayload.second;

					MessageQueueGenericValueType<Payload> * data = new MessageQueueGenericValueType<Payload>(outgoingPayload);

					invitingPlayerController->appendMessage(CM_relayGroupMemberInvitationToLaunchAnswer,
															0.0f,
															data,
															GameControllerMessageFlags::SEND |
															GameControllerMessageFlags::RELIABLE |
															GameControllerMessageFlags::DEST_AUTH_CLIENT);
				}
			}
		}
		break;

	case CM_groupLotteryWindowCloseResults:
		{
			// forward to the mob/container so that the loot script
			// trigger can be called on the corpse
			typedef std::pair<NetworkId, std::vector<NetworkId> > Payload;

			MessageQueueGenericValueType<Payload> const * const msg = safe_cast<MessageQueueGenericValueType<Payload> const *>(data);
			if (msg != 0)
			{
				Payload const & incomingPayload = msg->getValue();

				typedef std::vector<NetworkId> Ids;

				NetworkId const & containerId = incomingPayload.first;
				Ids const & selectedIds = incomingPayload.second;

				ServerObject * const container = ServerWorld::findObjectByNetworkId(containerId);
				Object * const containerOwner = (container != 0) ? ContainerInterface::getContainedByObject(*container) : 0;
				Controller * const controller = (containerOwner != 0) ? containerOwner->getController() : 0;

				if (controller != 0)
				{
					Payload payload;
					payload.first = NON_NULL(getCreature())->getNetworkId();
					payload.second = selectedIds;

					typedef MessageQueueGenericValueType<Payload> Message;
					Message * const message = new Message(payload);

					controller->appendMessage(
						CM_groupLotteryWindowCloseResults,
						0.0f,
						message,
						GameControllerMessageFlags::SEND |
						GameControllerMessageFlags::RELIABLE |
						GameControllerMessageFlags::DEST_AUTH_SERVER);
				}
				else
				{
					Chat::sendSystemMessage(*owner, StringId("group", "too_far_away_for_lottery"), Unicode::emptyString);
				}
			}
		}
		break;

	case CM_cyberneticsChangeRequest:
		{
			MessageQueueCyberneticsChangeRequest const * const msg = dynamic_cast<MessageQueueCyberneticsChangeRequest const *>(data);
			NOT_NULL(msg);
			if (msg != nullptr)
			{
				CreatureObject * const owner = NON_NULL(getCreature());
				if(owner)
				{
					NetworkId const & npcId = msg->getTarget();
					Object * const o = NetworkIdManager::getObjectById(npcId);
					ServerObject * const so = o ? o->asServerObject() : nullptr;
					if(so)
					{
						Controller * const npcController = so->getController();
						if(npcController)
						{
							MessageQueueCyberneticsChangeRequest * const msgToNPC = new MessageQueueCyberneticsChangeRequest(msg->getChangeType(), msg->getCyberneticPiece(), owner->getNetworkId());
							npcController->appendMessage(
								CM_cyberneticsChangeRequestToNPC,
								0.0f,
								msgToNPC,
								GameControllerMessageFlags::SEND |
								GameControllerMessageFlags::RELIABLE |
								GameControllerMessageFlags::DEST_AUTH_SERVER);
						}
					}
				}
			}
		}
		break;

	case CM_spaceMiningSaleSellResource:
		{
			MessageQueueSpaceMiningSellResource const * const msg = dynamic_cast<MessageQueueSpaceMiningSellResource const *>(data);
			if (msg != nullptr)
			{
				ServerObject * const spaceStation = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg->m_spaceStationId));
				if (nullptr == spaceStation)
				{
					WARNING(true, ("PlayerCreatureController CM_spaceMiningSaleSellResource invalid station [%s]", msg->m_spaceStationId.getValueString().c_str()));
				}
				else
				{
					GameScriptObject * const scriptObject = spaceStation->getScriptObject();
					if (scriptObject)
					{
						ScriptParams params;
						params.addParam(getOwner()->getNetworkId());
						params.addParam(msg->m_shipId);
						params.addParam(msg->m_spaceStationId);
						params.addParam(msg->m_resourceId);
						params.addParam(msg->m_amount);
						scriptObject->trigAllScripts(Scripting::TRIG_SPACE_MINING_SELL_RESOURCE, params);
					}
				}
			}
		}
		break;

	case CM_setCurrentWorkingSkill:
		{
			MessageQueueSelectCurrentWorkingSkill const * const msg = NON_NULL(dynamic_cast<MessageQueueSelectCurrentWorkingSkill const *>(data));
			if (owner->isAuthoritative() && playerOwner)
			{
				playerOwner->setWorkingSkill(msg->getCurrentWorkingSkill(), true);
			}
		}
		break;

	case CM_setProfessionTemplate:
		{
			MessageQueueSelectProfessionTemplate const * const msg = NON_NULL(dynamic_cast<MessageQueueSelectProfessionTemplate const *>(data));
			if (owner->isAuthoritative() && playerOwner)
			{
				playerOwner->setSkillTemplate(msg->getProfessionTemplate(), true);
			}
		}
		break;

	case CM_clientMinigameOpen:
		{
			typedef ValueDictionary Payload;
			MessageQueueGenericValueType<Payload> const * const msg = dynamic_cast<MessageQueueGenericValueType<Payload> const *>(data);
			if(msg)
			{
				CreatureObject * creature = getCreature();
				if(creature)
				{
					creature->clientMinigameOpen(msg->getValue());
				}
			}
		}
		break;

	case CM_clientMinigameClose:
		{
			typedef ValueDictionary Payload;
			MessageQueueGenericValueType<Payload> const * const msg = dynamic_cast<MessageQueueGenericValueType<Payload> const *>(data);
			if(msg)
			{
				CreatureObject * creature = getCreature();
				if(creature)
				{
					creature->clientMinigameClose(msg->getValue());
				}
			}
		}
		break;

	case CM_clientMinigameResult:
		{
			typedef ValueDictionary Payload;
			MessageQueueGenericValueType<Payload> const * const msg = dynamic_cast<MessageQueueGenericValueType<Payload> const *>(data);
			if(msg)
			{
				//extract the target object
				ValueDictionary const & data = msg->getValue();

				DictionaryValueMap const & valueMap = data.get();

				DictionaryValueMap::const_iterator itr = valueMap.find(minigameResultTargetName);

				if(itr != valueMap.end() && itr->second != nullptr)
				{
					if(itr->second->getType() == ValueTypeObjId::ms_type)
					{
						ValueTypeObjId * container = static_cast<ValueTypeObjId*>(itr->second);
						NetworkId tableOid = container->getValue();
						
						if(tableOid.isValid())
						{
							ScriptParams params;
							params.addParam(data, minigameResultDictionaryName);

							ScriptDictionaryPtr dictionary;
							GameScriptObject::makeScriptDictionary(params, dictionary);
							if (dictionary.get() != nullptr)
							{
								dictionary->serialize();
								MessageToQueue::getInstance().sendMessageToJava(tableOid, 
									"clientMinigameResult", dictionary->getSerializedData(), 0, false);
							}
						}
					}
				}
			}
		}
		break;

	case CM_createSaga:
		{
			MessageQueueCreateSaga const * const msg = dynamic_cast<MessageQueueCreateSaga const *>(data);
			if(msg)
			{
				CreatureObject * const owner = NON_NULL(getCreature());
				if(owner)
				{
					GameScriptObject * const scriptObject = owner->getScriptObject();
					if (scriptObject)
					{
						ValueDictionary paramsDict;
						ScriptParams params;
						
						paramsDict.insert("questName", ValueTypeString(msg->getQuestName()));
						paramsDict.insert("questDescription", ValueTypeString(msg->getQuestDescription()));
						paramsDict.insert("totalTasks", ValueTypeSignedInt(msg->getTotalTasks()));
						paramsDict.insert("rewards", ValueTypeString(msg->getRewards()));
						paramsDict.insert("recipe", ValueTypeBool(msg->isRecipe()));
						paramsDict.insert("oldRecipeObject", ValueTypeObjId(msg->getRecipeOID()));
						paramsDict.insert("share", ValueTypeSignedInt(msg->getShareAmount()));

						const std::vector<MessageQueueCreateSaga::TaskInfo> & tasks = msg->getTasks();
						for(unsigned int i = 0; i < tasks.size(); ++i)
						{
							char task[32];
							memset(task, 0, 32);
							sprintf(task, "%d", i);

							
							std::string paramString;

							char taskType[64];
							memset(taskType, 0, 64);
							sprintf(taskType, "%d",tasks[i].numParameters);
							paramString.append(taskType);

							for(int j = 0; j < tasks[i].numParameters; ++j)
							{
								paramString.append("~", 1);
								paramString.append(tasks[i].parameters[j].c_str());
							}

							paramsDict.insert(task, ValueTypeString(paramString));
						}
						params.addParam(paramsDict, "taskDictionary");
						ScriptDictionaryPtr dictionary;
						GameScriptObject::makeScriptDictionary(params, dictionary);
						if (dictionary.get() != nullptr)
						{
							dictionary->serialize();
							MessageToQueue::getInstance().sendMessageToJava(owner->getNetworkId(), 
								"OnCreateSaga", dictionary->getSerializedData(), 0, false);
						}
					}
				}
				
			}
		}
		break;

	default:
		CreatureController::handleMessage(message, value, data, flags);
		break;
	}
}

// ----------------------------------------------------------------------

float PlayerCreatureController::realAlter(float time)
{
	float alterResult = CreatureController::realAlter(time);

	CreatureObject const * const owner = NON_NULL(getCreature());
	Client * const client = owner->getClient();
	
	if (m_lastTimeSynchronization + 30 < ServerClock::getInstance().getGameTimeSeconds())
	{
		m_lastTimeSynchronization = ServerClock::getInstance().getGameTimeSeconds();

		if (client)
		{
			ServerTimeMessage timeSync(static_cast<int64>(ServerClock::getInstance().getGameTimeSeconds()));
			client->send(timeSync, true);

			// @todo: this should no be called every alter
			int weather;
			float windVelocityX, windVelocityY, windVelocityZ;
			PlanetObject const * const planet = ServerUniverse::getInstance().getCurrentPlanet();
			if (planet)
			{
				planet->getWeather(weather, windVelocityX, windVelocityY, windVelocityZ);
				if (m_lastWeatherIndex != weather)
				{
					m_lastWeatherIndex            = weather;
					m_lastWeatherWindVelocity_w.x = windVelocityX;
					m_lastWeatherWindVelocity_w.y = windVelocityY;
					m_lastWeatherWindVelocity_w.z = windVelocityZ;

					client->send(ServerWeatherMessage(m_lastWeatherIndex, m_lastWeatherWindVelocity_w), true);
				}
			}
		}
	}

#ifdef _DEBUG
	if (client && ConfigServerGame::getSendPlayerTransform())
	{
		Transform const & transform = owner->getTransform_o2p();

		CellProperty const * const cell = owner->getParentCell();

		NetworkId id = (cell && (cell != CellProperty::getWorldCellProperty())) ? cell->getOwner().getNetworkId() : NetworkId();

		DebugTransformMessage message(transform, id);

		client->send(message, true);
	}
#endif

	updateMaxMoveSpeed();

	updateMovementType(time);

	if(m_resendSpeedMax && client)
	{
		GenericValueTypeMessage<float> const msg("fca11a62d23041008a4f0df36aa7dca6", m_speedMaximum.getLastSetValue());
		client->send(msg, true);
		m_resendSpeedMax = false;
	}

	return alterResult;
}

// ----------------------------------------------------------------------

void PlayerCreatureController::updateMovementType ( float time )
{
	CreatureObject * const owner = NON_NULL(getCreature());

	if (!owner->isAuthoritative())
		return;

	m_movingTimeout -= time;

	if (m_movingTimeout <= 0.0f)
	{
		owner->setMovementStationary();
		m_movingTimeout = 0.0f;
	}
	else
	{
		owner->setMovementRun();
	}
}

// ----------------------------------------------------------------------

void PlayerCreatureController::onMoveFailed()
{
	CreatureObject * const owner = NON_NULL(getCreature());
	if (owner->isAuthoritative() && owner->isInWorld())
		teleport(owner->getTransform_o2p(), dynamic_cast<ServerObject *>(&(owner->getParentCell()->getOwner())));
}

// ----------------------------------------------------------------------

void PlayerCreatureController::updateHibernate()
{
	// do nothing, only ai update their hibernate state
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::shouldHibernate() const
{
	return false;
}

// ----------------------------------------------------------------------

void PlayerCreatureController::updateMaxMoveSpeed()
{
	CreatureObject * const owner = NON_NULL(getCreature());

	//-- If owner is riding a mount/vehicle, use the mount/vehicle's movement rate;
	//   otherwise, use the owner's movement rate..
	CreatureObject *const mountForOwner        = owner->getMountedCreature();
	CreatureObject *const movementSourceObject = mountForOwner ? mountForOwner : owner;
	
	float walkSpeed = movementSourceObject->getWalkSpeed();
	float runSpeed = movementSourceObject->getRunSpeed();
	float speed = runSpeed > 0.0f ? runSpeed : walkSpeed;

	if (speed != m_speedMaximum.getLastSetValue()) //lint !e777 // yep, testing for equality is in fact what we want.
	{
		m_speedMaximum.setValue(speed, getCurSyncStamp());
		GenericValueTypeMessage<float> const msg("fca11a62d23041008a4f0df36aa7dca6", m_speedMaximum.getLastSetValue());
		Client const * const client = owner->getClient();
		if (client)
			return client->send(msg, true);	// Tell the Client about it's new speed according to the server.
		else
			m_resendSpeedMax = true; // Try to send our new speed as soon as our client is ready.
	}
}

// ----------------------------------------------------------------------


uint32 PlayerCreatureController::getCurSyncStamp() const
{
	CreatureObject const * const creature = NON_NULL(getCreature());
	Client const * const client = creature->getClient();
	if (client)
		return client->getServerSyncStampLong();
	return 0;
}

// ----------------------------------------------------------------------

void PlayerCreatureController::handleObjectMenuRequest(MessageQueueObjectMenuRequest const *msg)
{
	CreatureObject * const creatureOwner = NON_NULL(getCreature());
	if (msg && creatureOwner->isAuthoritative())
	{
		ServerObject * const target = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg->getTargetId()));
		if (target)
		{
			if (!target->isAuthoritative())
			{
				GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
					"RequestSameServer",
					std::make_pair(
						ContainerInterface::getTopmostContainer(*creatureOwner)->getNetworkId(),
						ContainerInterface::getTopmostContainer(*target)->getNetworkId()));
				GameServer::getInstance().sendToPlanetServer(rssMessage);
			}
			else
			{
				GameScriptObject * const scriptObject = target->getScriptObject();
				if (scriptObject)
				{
					static const int move_type           = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE");
					static const int move_forward_type   = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE_FORWARD");
					static const int move_back_type      = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE_BACK");
					static const int move_left_type      = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE_LEFT");
					static const int move_right_type     = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE_RIGHT");
					static const int move_up_type        = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE_UP");
					static const int move_down_type      = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE_DOWN");
					static const int move_copy_location_type = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE_COPY_LOCATION");
					static const int move_copy_height_type = RadialMenuManager::getMenuTypeByName ("ITEM_MOVE_COPY_HEIGHT");
					static const int rotate_type         = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE");
					static const int rotate_left_type    = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_LEFT");
					static const int rotate_right_type   = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_RIGHT");
					static const int rotate_forward_type = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_FORWARD");
					static const int rotate_backward_type = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_BACKWARD");
					static const int rotate_clockwise_type = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_CLOCKWISE");
					static const int rotate_counterclockwise_type = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_COUNTERCLOCKWISE");
					static const int rotate_random_type  = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_RANDOM");
					static const int rotate_random_yaw_type = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_RANDOM_YAW");
					static const int rotate_random_pitch_type = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_RANDOM_PITCH");
					static const int rotate_random_roll_type = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_RANDOM_ROLL");
					static const int rotate_reset_type   = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_RESET");
					static const int rotate_copy_type    = RadialMenuManager::getMenuTypeByName ("ITEM_ROTATE_COPY");

					// Holocron check.
					Object const * const containedByObj = ContainerInterface::getContainedByObject(*target);
					if(containedByObj && containedByObj->asServerObject() && ( containedByObj->asServerObject()->getGameObjectType() == SharedObjectTemplate::GOT_chronicles_quest_holocron || containedByObj->asServerObject()->getGameObjectType() == SharedObjectTemplate::GOT_chronicles_quest_holocron_recipe ) )
						return; // No menus for Holocron contained items.

					bool addedRotateMenuItems = false;
					std::string positiveRotateDegree;
					std::string negativeRotateDegree;

					RadialMenuManager::DataVector dv(msg->getData());

					//-- see if we need the pickup, move, or rotate options
					//-- these are governed by building permisisons if the object is in a building
					//-- if the object is not in a building, it can probably be picked up.
					bool blockedByNoTrade = false;
					bool showPickUpMenu = false;
					if (objectMenuRequestCanManipulateObject(*creatureOwner, *target, blockedByNoTrade, showPickUpMenu))
					{
						if (showPickUpMenu)
						{
							static const int pickup_type = RadialMenuManager::getMenuTypeByName ("ITEM_PICKUP");
							if (!RadialMenuManager::addRootMenu (dv, pickup_type, Unicode::emptyString, false))
								WARNING (true, ("ServerController unable to apply pickup menu [%d] to menu for %s", pickup_type, target->getNetworkId ().getValueString ().c_str ()));
						}

						if (!target->isInWorldCell())
						{
							const int move_id = RadialMenuManager::addRootMenu (dv, move_type, Unicode::emptyString, true);
								
							if (!move_id)
								WARNING (true, ("ServerController unable to apply move menu [%d] to menu for %s", move_type, target->getNetworkId ().getValueString ().c_str ()));
							else
							{
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, move_id, move_forward_type, Unicode::emptyString, false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, move_id, move_back_type,    Unicode::emptyString, false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, move_id, move_left_type,    Unicode::emptyString, false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, move_id, move_right_type,   Unicode::emptyString, false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, move_id, move_up_type,      Unicode::emptyString, false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, move_id, move_down_type,    Unicode::emptyString, false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, move_id, move_copy_location_type, Unicode::emptyString, false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, move_id, move_copy_height_type, Unicode::emptyString, false));
							}

							const int rotate_id = RadialMenuManager::addRootMenu (dv, rotate_type, Unicode::emptyString, true);
								
							if (!rotate_id)
								WARNING (true, ("ServerController unable to apply rotate menu [%d] to menu for %s", rotate_type, target->getNetworkId ().getValueString ().c_str ()));
							else
							{
								int rotateDegrees = 90;
								if (creatureOwner->getClient())
									rotateDegrees = abs(creatureOwner->getClient()->getFurnitureRotationDegree());

								positiveRotateDegree = FormattedString<32>().sprintf(" +%d", rotateDegrees);
								negativeRotateDegree = FormattedString<32>().sprintf(" -%d", rotateDegrees);

								addedRotateMenuItems = true;
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_left_type, Unicode::narrowToWide(std::string("@ui_radial:item_rotate_yaw") + positiveRotateDegree), false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_right_type, Unicode::narrowToWide(std::string("@ui_radial:item_rotate_yaw") + negativeRotateDegree), false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_random_yaw_type, Unicode::emptyString, false));

								PlayerObject const * const playerObject = getPlayerObject(creatureOwner);
								if ((playerObject && playerObject->hasCompletedCollection("force_shui_tier_02")) || (creatureOwner->getClient() && creatureOwner->getClient()->isGod()))
								{
									IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_forward_type, Unicode::narrowToWide(std::string("@ui_radial:item_rotate_pitch") + positiveRotateDegree), false));
									IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_backward_type, Unicode::narrowToWide(std::string("@ui_radial:item_rotate_pitch") + negativeRotateDegree), false));
									IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_random_pitch_type, Unicode::emptyString, false));
									IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_clockwise_type, Unicode::narrowToWide(std::string("@ui_radial:item_rotate_roll") + positiveRotateDegree), false));
									IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_counterclockwise_type, Unicode::narrowToWide(std::string("@ui_radial:item_rotate_roll") + negativeRotateDegree), false));
									IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_random_roll_type, Unicode::emptyString, false));
									IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_random_type, Unicode::emptyString, false));
								}
								
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_reset_type, Unicode::emptyString, false));
								IGNORE_RETURN(RadialMenuManager::addSubMenu(dv, rotate_id, rotate_copy_type, Unicode::emptyString, false));
							}
						}
					}

					ScriptParams params;
					params.addParam(creatureOwner->getNetworkId());
					params.addParam(dv);
					IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_OBJECT_MENU_REQUEST, params));

					RadialMenuManager::DataVector resultMenuInfo (params.getObjectMenuRequestDataArrayParam(1));
					RadialMenuManager::DataVector::iterator it;
					for(it = resultMenuInfo.begin(); it != resultMenuInfo.end(); ++it)
					{
						ObjectMenuRequestData & requestData = *it;						
						if(!requestData.m_labelId.getTable().empty())
						{
							requestData.m_label = Unicode::narrowToWide (std::string ("@") + requestData.m_labelId.getCanonicalRepresentation ());
						}

						// calling into script causes the customized label that
						// we set above to get blown away, so we need to reset it
						else if (addedRotateMenuItems && requestData.m_label.empty())
						{
							if (requestData.m_menuItemType == rotate_left_type)
								requestData.m_label = Unicode::narrowToWide(std::string("@ui_radial:item_rotate_yaw") + positiveRotateDegree);
							else if (requestData.m_menuItemType == rotate_right_type)
								requestData.m_label = Unicode::narrowToWide(std::string("@ui_radial:item_rotate_yaw") + negativeRotateDegree);
							else if (requestData.m_menuItemType == rotate_forward_type)
								requestData.m_label = Unicode::narrowToWide(std::string("@ui_radial:item_rotate_pitch") + positiveRotateDegree);
							else if (requestData.m_menuItemType == rotate_backward_type)
								requestData.m_label = Unicode::narrowToWide(std::string("@ui_radial:item_rotate_pitch") + negativeRotateDegree);
							else if (requestData.m_menuItemType == rotate_clockwise_type)
								requestData.m_label = Unicode::narrowToWide(std::string("@ui_radial:item_rotate_roll") + positiveRotateDegree);
							else if (requestData.m_menuItemType == rotate_counterclockwise_type)
								requestData.m_label = Unicode::narrowToWide(std::string("@ui_radial:item_rotate_roll") + negativeRotateDegree);
						}
					}

					appendMessage(
						CM_objectMenuResponse,
						0.0f,
						new MessageQueueObjectMenuRequest(target->getNetworkId(), creatureOwner->getNetworkId(), resultMenuInfo, msg->m_sequence),
						GameControllerMessageFlags::SEND|GameControllerMessageFlags::RELIABLE|GameControllerMessageFlags::DEST_AUTH_CLIENT);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::objectMenuRequestCanManipulateObject(CreatureObject const & player, ServerObject const & item, bool & blockedByNoTrade, bool & showPickUpMenu)
{
	blockedByNoTrade = false;
	showPickUpMenu = false;
	Container::ContainerErrorCode code = Container::CEC_Success;

	// call canManipulateObject telling it to skip the no trade check;
	// we'll do the no trade check separately if canManipulateObject passes
	bool allowedByGodMode = false;
	if (item.isInWorld() && player.canManipulateObject(item, true, true, true, 100.0f, code, true, &allowedByGodMode))
	{
		// check for no trade
		blockedByNoTrade = false;
		showPickUpMenu = true;
		if (!allowedByGodMode && item.markedNoTrade())
		{
			blockedByNoTrade = true;
			showPickUpMenu = false;

			// the item must be owned by this creature
			NetworkId ownerId;
			if (item.asIntangibleObject())
			{
				ServerObject const * const containedBy = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(item));
				ownerId = containedBy ? containedBy->getOwnerId() : NetworkId::cms_invalid;
			}
			else
			{
				ownerId = item.getOwnerId();
			}

			if (ownerId == player.getNetworkId())
			{
				blockedByNoTrade = false;
				showPickUpMenu = true;
			}

			// show the pick up menu item if the no trade item is a "shared" no trade
			// item, and is immediately contained in a structure cell, and is being
			// accessed by a character on the same account as the current owner of the item
			if (!showPickUpMenu && ownerId.isValid() && item.markedNoTradeShared(true))
			{
				ServerObject const * const containedBy = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(item));
				if (containedBy && containedBy->asCellObject())
				{
					PlayerObject const * const playerObject = getPlayerObject(&player);
					if (playerObject && (playerObject->getStationId() == NameManager::getInstance().getPlayerStationId(ownerId)))
						showPickUpMenu = true;
				}
			}
		}

		// creatures cannot be picked up, with special exception made for NPC vendor below
		if (item.asCreatureObject())
			showPickUpMenu = false;

		// vendors have special pickup rules
		if (item.isVendor())
		{
			// the only vendors that can be picked up are
			// uninitialized vendor, but only by the vendor owner
			showPickUpMenu = false;
			if (!item.getObjVars().hasItem("vendor_initialized"))
			{
				NetworkId vendorOwner;
				if (item.getObjVars().getItem("vendor_owner", vendorOwner) && (vendorOwner == player.getNetworkId()))
					showPickUpMenu = true;
			}
		}

		return true;
	}

	return false;
}

// ======================================================================

PlayerCreatureController::MoveSnapshot::MoveSnapshot() :
	m_inited(false),
	m_cell(),
	m_position_p(),
	m_position_w(),
	m_syncStamp(0)
{
}

// ----------------------------------------------------------------------

PlayerCreatureController::MoveSnapshot::MoveSnapshot(Vector const &position_p, CellObject *cell, uint32 syncStamp) :
	m_inited(true),
	m_cell(cell),
	m_position_p(position_p),
	m_position_w(cell ? cell->getTransform_o2w().rotateTranslate_l2p(position_p) : position_p),
	m_syncStamp(syncStamp)
{
}

// ----------------------------------------------------------------------

PlayerCreatureController::MoveSnapshot::MoveSnapshot(MoveSnapshot const &rhs) :
	m_inited(rhs.m_inited),
	m_cell(rhs.m_cell),
	m_position_p(rhs.m_position_p),
	m_position_w(rhs.m_position_w),
	m_syncStamp(rhs.m_syncStamp)
{
}

// ----------------------------------------------------------------------

PlayerCreatureController::MoveSnapshot &PlayerCreatureController::MoveSnapshot::operator=(MoveSnapshot const &rhs)
{
	if (&rhs != this)
	{
		m_inited = rhs.m_inited;
		m_cell = rhs.m_cell;
		m_position_p = rhs.m_position_p;
		m_position_w = rhs.m_position_w;
		m_syncStamp = rhs.m_syncStamp;
	}
	return *this;
}

// ----------------------------------------------------------------------

void PlayerCreatureController::MoveSnapshot::clear()
{
	m_inited = false;
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::MoveSnapshot::isValid() const
{
	return m_inited;
}

// ----------------------------------------------------------------------

bool PlayerCreatureController::MoveSnapshot::isAllowed(CreatureObject const &who) const
{
	CellObject const * const cell = m_cell.getPointer();
	return !cell || cell->isAllowed(who);
}

// ----------------------------------------------------------------------

CellObject *PlayerCreatureController::MoveSnapshot::getCell()
{
	return m_cell.getPointer();
} //lint !e1762 // logically nonconst

// ----------------------------------------------------------------------

CellObject const *PlayerCreatureController::MoveSnapshot::getCell() const
{
	return m_cell.getPointer();
}

// ----------------------------------------------------------------------

Vector const &PlayerCreatureController::MoveSnapshot::getPosition_p() const
{
	return m_position_p;
}

// ----------------------------------------------------------------------

Vector const PlayerCreatureController::MoveSnapshot::getPosition_w() const
{
	// if cell can be resolved, return world position relative to current position of cell
	CellObject const * const cell = m_cell.getPointer();
	if (cell)
		return cell->getTransform_o2w().rotateTranslate_l2p(m_position_p);

	// if no cell, or no longer able to resolve cell, use original world position
	return m_position_w;
}

// ----------------------------------------------------------------------

uint32 PlayerCreatureController::MoveSnapshot::getSyncStamp() const
{
	return m_syncStamp;
}

//----------------------------------------------------------------------

PlayerCreatureController * PlayerCreatureController::asPlayerCreatureController()
{
	return this;
}

//----------------------------------------------------------------------

PlayerCreatureController const * PlayerCreatureController::asPlayerCreatureController() const
{
	return this;
}

//----------------------------------------------------------------------

void PlayerCreatureController::playerMovedAndNeedsToCancelWarmup( CreatureObject &owner )
{
	CommandQueue * const queue = owner.getCommandQueue();

	if ( queue->getCurrentCommandState() == CommandQueue::State_Warmup )
	{
		queue->cancelCurrentCommand();
	}
}

//----------------------------------------------------------------------

float PlayerCreatureController::getLastSpeed() const
{
	return m_lastSpeed;
}

// ======================================================================

