// ======================================================================
//
// PlayerShipController.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlayerShipController.h"

#include "serverGame/AiShipBehaviorDock.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerShipTurretTargetingSystem.h"
#include "serverGame/ServerShipObjectInterface.h"
#include "serverGame/ShipAiEnemySearchManager.h"
#include "serverGame/ShipClientUpdateTracker.h"
#include "serverGame/ShipObject.h"
#include "serverGame/TangibleObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedLog/Log.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueSpaceMiningSellResource.h"
#include "sharedNetworkMessages/MessageQueueTeleportAck.h"
#include "sharedNetworkMessages/ShipUpdateTransformCollisionMessage.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/NetworkIdManager.h"

#include <limits>
#include <map>

// ======================================================================
// PlayerShipControllerNamespace
// ======================================================================

namespace PlayerShipControllerNamespace
{
	bool ms_logHandleNetUpdateTransform;

	void remove();

	float const s_targetedByAiExpireTime = 20.0f;

	int syncStampLongDeltaTime(uint32 stamp1, uint32 stamp2)
	{
		uint32 const delta = stamp1 - stamp2;
		if (delta > 0x7fffffff)
			return static_cast<int>(0xffffffff-delta);
		return static_cast<int>(delta);
	}

}

using namespace PlayerShipControllerNamespace;

// ======================================================================

void PlayerShipController::install()
{
	DebugFlags::registerFlag(ms_logHandleNetUpdateTransform, "ServerGame/RemoteShipController", "logHandleNetUpdateTransform*");
	ExitChain::add(&remove, "PlayerShipController::remove");
}

// ----------------------------------------------------------------------

void PlayerShipControllerNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_logHandleNetUpdateTransform);
}

// ======================================================================
// PUBLIC PlayerShipController
// ======================================================================

PlayerShipController::PlayerShipController(ShipObject * const owner) :
	ShipController(owner),
	m_clientToServerLastSyncStamp(0),
	m_teleportIds(),
	m_shipAccelRate(owner->getShipActualAccelerationRate(), ConfigServerGame::getShipMoveCheckIntervalMs(), ConfigServerGame::getShipMoveCheckFudgeTimeMs()),
	m_shipDecelRate(owner->getShipActualDecelerationRate(), ConfigServerGame::getShipMoveCheckIntervalMs(), ConfigServerGame::getShipMoveCheckFudgeTimeMs()),
	m_shipYprMaximum(owner->getShipActualYprRateMaximum(), ConfigServerGame::getShipMoveCheckIntervalMs(), ConfigServerGame::getShipMoveCheckFudgeTimeMs()),
	m_shipSpeedMaximum(owner->getShipActualSpeedMaximum(), ConfigServerGame::getShipMoveCheckIntervalMs(), ConfigServerGame::getShipMoveCheckFudgeTimeMs()),
	m_lastVerifiedTransform(),
	m_lastVerifiedSpeed(0.f),
	m_lastVerifiedSyncStamp(0),
	m_gunnerWeaponIndexList(new GunnerWeaponIndexList),
	m_targetedByAiTimer(s_targetedByAiExpireTime)
{
	preventMovementUpdates();
}

// ----------------------------------------------------------------------

PlayerShipController::~PlayerShipController()
{
	delete m_gunnerWeaponIndexList;
}

// ----------------------------------------------------------------------

void PlayerShipController::endBaselines()
{
	if (getAuthoritative())
	{
		ShipObject * const ship = getShipOwner();
		ship->clearCondition(static_cast<int>(TangibleObject::C_docking));

		m_shipAccelRate.setValue(ship->getShipActualAccelerationRate(), 0);
		m_shipDecelRate.setValue(ship->getShipActualDecelerationRate(), 0);
		m_shipYprMaximum.setValue(ship->getShipActualYprRateMaximum(), 0);
		m_shipSpeedMaximum.setValue(ship->getShipActualSpeedMaximum(), 0);
	}

	m_yawPosition = 0.f;
	m_pitchPosition = 0.f;
	m_rollPosition = 0.f;
	m_throttlePosition = 0.f;

	ShipController::endBaselines();
}

// ----------------------------------------------------------------------

void PlayerShipController::setAuthoritative(bool newAuthoritative)
{
	ShipController::setAuthoritative(newAuthoritative);

	if (!newAuthoritative)
		preventMovementUpdates();
	else
	{
		ShipObject const * const ship = NON_NULL(getShipOwner());
		if (ship->getClient())
			resyncMovementUpdates();

		m_shipAccelRate.setValue(ship->getShipActualAccelerationRate(), 0);
		m_shipDecelRate.setValue(ship->getShipActualDecelerationRate(), 0);
		m_shipYprMaximum.setValue(ship->getShipActualYprRateMaximum(), 0);
		m_shipSpeedMaximum.setValue(ship->getShipActualSpeedMaximum(), 0);
	}
}

// ----------------------------------------------------------------------

void PlayerShipController::onClientReady()
{
	m_clientToServerLastSyncStamp = 0;
	if (getAuthoritative())
		resyncMovementUpdates();
}

// ----------------------------------------------------------------------

void PlayerShipController::onClientLost()
{
	m_yawPosition = 0.f;
	m_pitchPosition = 0.f;
	m_rollPosition = 0.f;
	m_throttlePosition = 0.f;
	preventMovementUpdates();
	unDock();
}

// ----------------------------------------------------------------------

void PlayerShipController::receiveTransform(ShipUpdateTransformMessage const & shipUpdateTransformMessage)
{
	if (isTeleporting())
		return;

	ShipObject * const owner = NON_NULL(getShipOwner());

	if (owner->hasCondition(static_cast<int>(TangibleObject::C_docking)))
		return;

	uint32 const syncStamp = shipUpdateTransformMessage.getSyncStampLong();
	if (   m_clientToServerLastSyncStamp == 0
	    || syncStampLongDeltaTime(m_clientToServerLastSyncStamp, syncStamp) > 0)
	{
		m_clientToServerLastSyncStamp = syncStamp;

		Transform const &transform = shipUpdateTransformMessage.getTransform();
		Vector const &velocity = shipUpdateTransformMessage.getVelocity();
		float const speed = velocity.magnitude();

		if (!checkValidMove(transform, velocity, speed, syncStamp))
			teleport(m_lastVerifiedTransform, 0);
		else
		{
			m_shipDynamicsModel->setTransform(transform);
			m_shipDynamicsModel->setVelocity(velocity);

			ServerShipObjectInterface const serverShipObjectInterface(owner);

			//-- Transform speeds into throttle positions and compute delta time
			m_throttlePosition = 0.f;
			m_yawPosition = 0.f;
			m_pitchPosition = 0.f;
			m_rollPosition = 0.f;
			float deltaTime = 0.f;

			Client const * const client = owner->getClient();
			if (client)
			{
				CreatureObject const * const pilot = safe_cast<CreatureObject const *>(client->getCharacterObject());
				if (pilot && !pilot->getState(States::Frozen))
				{
					if (serverShipObjectInterface.getMaximumSpeed() > 0.f)
						m_throttlePosition = clamp(0.f, speed / serverShipObjectInterface.getMaximumSpeed(), 1.f);
					if (serverShipObjectInterface.getMaximumYaw() > 0.f)
						m_yawPosition = clamp(-1.f, shipUpdateTransformMessage.getYawRate() / serverShipObjectInterface.getMaximumYaw(), 1.f);
					if (serverShipObjectInterface.getMaximumPitch() > 0.f)
						m_pitchPosition = clamp(-1.f, shipUpdateTransformMessage.getPitchRate() / serverShipObjectInterface.getMaximumPitch(), 1.f);
					if (serverShipObjectInterface.getMaximumRoll() > 0.f)
						m_rollPosition = clamp(-1.f, shipUpdateTransformMessage.getRollRate() / serverShipObjectInterface.getMaximumRoll(), 1.f);

					deltaTime = client->computeDeltaTimeInSeconds(syncStamp);
				}
			}

			//-- Advance by delta time
			m_shipDynamicsModel->model(deltaTime, m_yawPosition, m_pitchPosition, m_rollPosition, m_throttlePosition, serverShipObjectInterface);

			DEBUG_REPORT_LOG(ms_logHandleNetUpdateTransform, ("PlayerShipController::handleNetUpdateTransform[%s]: %1.2f\n", owner->getNetworkId().getValueString().c_str(), m_throttlePosition));
		}
	}
	else
		DEBUG_REPORT_LOG(true, ("PlayerShipController::handleNetUpdateTransform[%s]: disregarding older packet\n", owner->getNetworkId().getValueString().c_str()));
}

// ----------------------------------------------------------------------

void PlayerShipController::teleport(Transform const &goal, ServerObject *goalObj)
{
	ShipObject * const ship = NON_NULL(getShipOwner());
	Client * const client = ship->getClient();

	if (client && !goalObj)
	{
		int const sequenceId = static_cast<int>(ship->getAndIncrementMoveSequenceNumber());
		IGNORE_RETURN(m_teleportIds.insert(sequenceId));

		appendMessage(
			CM_netUpdateTransform,
			0.f,
			new MessageQueueDataTransform(0, sequenceId, goal, 0.f, 0.f, false),
			GameControllerMessageFlags::RELIABLE|GameControllerMessageFlags::SEND|GameControllerMessageFlags::DEST_AUTH_CLIENT);

		m_lastVerifiedTransform = goal;
		m_lastVerifiedSpeed = 0;
		m_lastVerifiedSyncStamp = getCurSyncStamp();
	}

	ShipController::teleport(goal, goalObj);
}

// ----------------------------------------------------------------------

void PlayerShipController::handleMessage(int const message, float const value, MessageQueue::Data const * const data, uint32 const flags)
{
	switch (message)
	{
	case CM_teleportAck:
		{
			MessageQueueTeleportAck const * const msg = dynamic_cast<MessageQueueTeleportAck const *>(data);
			if (msg)
				handleTeleportAck(msg->getSequenceId());
		}
		break;
	case CM_shipStopFiring:
		{
			ShipObject * const ship = NON_NULL(getShipOwner());

			typedef MessageQueueGenericValueType<int> MessageType;			
			MessageType const * const msg = dynamic_cast<MessageType const *>(data);

			if (msg)
			{
				int const weaponIndex = msg->getValue();
				ship->stopFiringWeapon(weaponIndex);
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

	default:
		ShipController::handleMessage(message, value, data, flags);
		break;
	}
}

// ----------------------------------------------------------------------

void PlayerShipController::onShipAccelerationRateChanged(float value)
{
	m_shipAccelRate.setValue(value, getCurSyncStamp());
}

// ----------------------------------------------------------------------

void PlayerShipController::onShipDecelerationRateChanged(float value)
{
	m_shipDecelRate.setValue(value, getCurSyncStamp());
}

// ----------------------------------------------------------------------

void PlayerShipController::onShipYprMaximumChanged(float value)
{
	m_shipYprMaximum.setValue(value, getCurSyncStamp());
}

// ----------------------------------------------------------------------

void PlayerShipController::onShipSpeedMaximumChanged(float value)
{
	m_shipSpeedMaximum.setValue(value, getCurSyncStamp());
}

// ----------------------------------------------------------------------

bool PlayerShipController::isTeleporting() const
{
	return !m_teleportIds.empty();
}

// ======================================================================
// PROTECTED PlayerShipController
// ======================================================================

float PlayerShipController::realAlter(float const elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("PlayerShipController::realAlter");

	ShipObject * const owner = getShipOwner();

	if (!owner || !owner->isInWorld())
		return ShipController::realAlter(elapsedTime);

	if (owner && owner->isInitialized())
	{
		if (m_pendingDockingBehavior != nullptr)
		{
			delete m_dockingBehavior;
			m_dockingBehavior = m_pendingDockingBehavior;
			m_pendingDockingBehavior = nullptr;

			getShipOwner()->setCondition(static_cast<int>(TangibleObject::C_docking));
			Client * const client = owner->getClient();

			if (client != nullptr)
			{
				ShipClientUpdateTracker::queueForUpdate(*client, *owner);
			}
			else
			{
				unDock();
			}
		}

		if (   (m_dockingBehavior != nullptr)
		    && m_dockingBehavior->isDockFinished())
		{
			delete m_dockingBehavior;
			m_dockingBehavior = nullptr;

			getShipOwner()->clearCondition(static_cast<int>(TangibleObject::C_docking));
		}

		if (m_dockingBehavior != nullptr)
		{
			//-- The docking behavior will calculate new values for m_yaw/m_pitch/m_roll/m_throttle[Position] implicitly through calling ShipController members
			m_dockingBehavior->alter(elapsedTime);
		}

		//-- Update flight model
		ServerShipObjectInterface const serverShipObjectInterface(owner);
		m_shipDynamicsModel->model(elapsedTime, m_yawPosition, m_pitchPosition, m_rollPosition, m_throttlePosition, serverShipObjectInterface);

		//-- Update the server position based on the model
		owner->setTransform_o2p(m_shipDynamicsModel->getTransform());

		// Request new enemies for any turrets

		if (   hasTurretTargetingSystem()
		    && !m_enemyCheckQueued)
		{
			m_enemyCheckQueued = true;
			ShipAiEnemySearchManager::add(*owner);
		}

		if (m_targetedByAiTimer.updateNoReset(elapsedTime))
		{
			owner->clearCondition(static_cast<int>(TangibleObject::C_spaceCombatMusic));
		}
	}

	//-- Chain back up to parent
	float alterResult = ShipController::realAlter(elapsedTime);

	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

void PlayerShipController::handleNetUpdateTransform(MessageQueueDataTransform const & /*message*/)
{
	//-- This message should be handled in handleMessage
	DEBUG_FATAL(true, ("PlayerShipController::handleNetUpdateTransform: not implemented (nor should it be)"));
}

// ----------------------------------------------------------------------

void PlayerShipController::handleNetUpdateTransformWithParent(MessageQueueDataTransformWithParent const & /*message*/)
{
	//-- This message should be handled in handleMessage
	DEBUG_FATAL(true, ("PlayerShipController::handleNetUpdateTransformWithParent: not implemented (nor should it be)"));
}

// ======================================================================
// PRIVATE PlayerShipController
// ======================================================================

void PlayerShipController::preventMovementUpdates()
{
	m_teleportIds.clear();
	IGNORE_RETURN(m_teleportIds.insert(-1));
}

// ----------------------------------------------------------------------

void PlayerShipController::handleTeleportAck(int sequenceId)
{
	IGNORE_RETURN(m_teleportIds.erase(sequenceId));
}

// ----------------------------------------------------------------------

void PlayerShipController::resyncMovementUpdates()
{
	// We resync updates by doing a fake teleport, giving a negative
	// sequence so the client does not actually do the movement.
	// This locks out movement updates until the teleport is acknowledged.

	ShipObject * const ship = NON_NULL(getShipOwner());
	Client * const client = ship->getClient();

	FATAL(!client || !ship->isAuthoritative(), ("Tried to resync ship movement updates either while nonauth or without a client!"));

	m_teleportIds.clear();

	int sequenceId = -1;
	while (sequenceId == -1)
		sequenceId = -static_cast<int>(ship->getAndIncrementMoveSequenceNumber());

	IGNORE_RETURN(m_teleportIds.insert(sequenceId));

	appendMessage(
		CM_netUpdateTransform,
		0.f,
		new MessageQueueDataTransform(0, sequenceId, Transform::identity, 0.f, 0.f, false),
		GameControllerMessageFlags::RELIABLE|GameControllerMessageFlags::SEND|GameControllerMessageFlags::DEST_AUTH_CLIENT);

	m_lastVerifiedTransform = m_shipDynamicsModel->getTransform();
	m_lastVerifiedSpeed = m_shipDynamicsModel->getSpeed();
	m_lastVerifiedSyncStamp = getCurSyncStamp();
}

// ----------------------------------------------------------------------

void PlayerShipController::experiencedCollision()
{
	ShipObject * const owner = getShipOwner();
	if (owner != 0)
	{
		Client* client = owner->getClient();

		if (client != 0)
		{
			ShipUpdateTransformCollisionMessage const collisionMessage(
				owner->getNetworkId(),
				getTransform(),
				getVelocity(),
				client->getServerSyncStampLong());
			client->send(collisionMessage, false);
		}
	}
}

// ----------------------------------------------------------------------

uint32 PlayerShipController::getCurSyncStamp() const
{
	Client const * const client = NON_NULL(getShipOwner())->getClient();
	return client ? client->getServerSyncStampLong() : 0;
}

// ----------------------------------------------------------------------

void PlayerShipController::logMoveFail(char const *reasonFmt, ...) const
{
	ShipObject const * const ship = NON_NULL(getShipOwner());
	Client const * const client = NON_NULL(ship->getClient());

	char reason[512];
	va_list ap;
	va_start(ap, reasonFmt);
	IGNORE_RETURN(_vsnprintf(reason, sizeof(reason)-1, reasonFmt, ap));
	reason[sizeof(reason)-1] = '\0';

	LOG(
		"shipmove",
		(
			"fail - ship %s creature %s stationId %u - %s",
			ship->getNetworkId().getValueString().c_str(),
			client->getCharacterObjectId().getValueString().c_str(),
			client->getStationId(),
			reason));
	va_end(ap);
}

// ----------------------------------------------------------------------

bool PlayerShipController::checkValidMove(Transform const &transform, Vector const &velocity, float speed, uint32 const currentClientSyncStamp)
{
	if (!ConfigServerGame::getShipMoveValidationEnabled())
		return true;

	ShipObject * const owner = NON_NULL(getShipOwner());
	Client const * const client = NON_NULL(owner->getClient());

	// gods can move however they like, and assume their moves are valid
	if (!client->isGod())
	{
		uint32 const currentServerSyncStamp = client->getServerSyncStampLong();
		uint32 const previousClientSyncStamp = m_lastVerifiedSyncStamp;
		int const timeDiffMs = syncStampLongDeltaTime(previousClientSyncStamp, currentClientSyncStamp);
		float const timeDiffSec = static_cast<float>(timeDiffMs)*0.001f;
		float const tolerance = ConfigServerGame::getShipMoveCheckTolerance();
		float const timeTolerance = ConfigServerGame::getShipMoveCheckTimeToleranceSeconds();

		// is our new speed valid?
		float const speedMaximum = m_shipSpeedMaximum.getRecentMaximum(currentServerSyncStamp);
		if (speed > speedMaximum*tolerance)
		{
			logMoveFail("too fast (speed=%g, speedMax=%g, timeDiff=%g, time %u)", speed, speedMaximum, timeDiffSec, currentClientSyncStamp);
			return false;
		}

		// Only do position/rotation validation periodically, since their timestamps
		// can be affected by latency variation.
		if (timeDiffMs < ConfigServerGame::getShipMoveCheckIntervalMs())
		{
			// assume we're ok to move, but don't update the last verified
			return true;
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
						logMoveFail("out of order timestamps (%dms), oss=%lu, css=%lu (%lud:%luh:%lum:%lus), ssc=%lu",
							timeDiffMs,
							previousClientSyncStamp,
							currentServerSyncStamp,
							(currentServerSyncStamp / 1000) / (60 * 60 * 24),
							((currentServerSyncStamp / 1000) % (60 * 60 * 24)) / (60 * 60),
							((currentServerSyncStamp / 1000) % (60 * 60)) / 60,
							(currentServerSyncStamp / 1000) % 60,
							currentClientSyncStamp);

						return false;
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
						logMoveFail("timestamp from the future (%dms), oss=%lu, css=%lu (%lud:%luh:%lum:%lus), ssc=%lu",
							timeDiffMs,
							previousClientSyncStamp,
							currentServerSyncStamp,
							(currentServerSyncStamp / 1000) / (60 * 60 * 24),
							((currentServerSyncStamp / 1000) % (60 * 60 * 24)) / (60 * 60),
							((currentServerSyncStamp / 1000) % (60 * 60)) / 60,
							(currentServerSyncStamp / 1000) % 60,
							currentClientSyncStamp);

						return false;
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
				logMoveFail("timestamp drift (%dms), oss=%lu, css=%lu (%lud:%luh:%lum:%lus), ssc=%lu",
					clientServerTimeDiff,
					previousClientSyncStamp,
					currentServerSyncStamp,
					(currentServerSyncStamp / 1000) / (60 * 60 * 24),
					((currentServerSyncStamp / 1000) % (60 * 60 * 24)) / (60 * 60),
					((currentServerSyncStamp / 1000) % (60 * 60)) / 60,
					(currentServerSyncStamp / 1000) % 60,
					currentClientSyncStamp);

				return false;
			}
		}

		// could we possibly have changed position enough?
		Vector const &lastPosition = m_lastVerifiedTransform.getPosition_p();
		Vector const &position = transform.getPosition_p();
		float const distanceSquared = lastPosition.magnitudeBetweenSquared(position);
		if (distanceSquared > sqr(speedMaximum*(timeDiffSec+timeTolerance)*tolerance)+0.2f)
		{
			logMoveFail(
				"too far (distSquared=%g, maxDistSquared=%g, timeDiff=%g from [%g,%g,%g] to [%g,%g,%g], time %u)",
				distanceSquared, sqr(speedMaximum*timeDiffSec), timeDiffSec,
				lastPosition.x, lastPosition.y, lastPosition.z,
				position.x, position.y, position.z, currentClientSyncStamp);
			return false;
		}

		// could we possibly have changed speeds enough?
		float const oldSpeed = m_lastVerifiedSpeed;
		if (speed > oldSpeed)
		{
			float const accel = (oldSpeed-speed)/(timeDiffSec+timeTolerance);
			float const maxAccel = m_shipAccelRate.getRecentMaximum(currentServerSyncStamp);
			if (accel > maxAccel*tolerance)
			{
				logMoveFail("too much acceleration (accel=%g, maxAccel=%g, time %u)", accel, maxAccel, currentClientSyncStamp);
				return false;
			}
		}
		else if (speed < oldSpeed)
		{
			float const decel = (speed-oldSpeed)/(timeDiffSec+timeTolerance);
			float const maxDecel = m_shipDecelRate.getRecentMaximum(currentServerSyncStamp);
			if (decel > maxDecel*tolerance)
			{
				logMoveFail("too much deceleration (decel=%g, maxDecel=%g, time %u)", decel, maxDecel, currentClientSyncStamp);
				return false;
			}
		}

		// could we possibly have changed orientation enough?
		float const rotation = acos(transform.getLocalFrameK_p().dot(m_lastVerifiedTransform.getLocalFrameK_p()))/(timeDiffSec+timeTolerance);
		float const maxRotation = m_shipYprMaximum.getRecentMaximum(currentServerSyncStamp);
		// Actual max rotation is roughly sqrt(2*sqr(maxRotation))+0.001f, to allow
		// for pitch/yaw/roll together being applied toward the same direction, and
		// adjust slightly for precision.
		if (sqr(rotation) > 2*sqr(maxRotation*tolerance)+0.001f)
		{
			logMoveFail("too much rotation (rotation=%g, maxRotation=%g, time %u)", rotation, maxRotation, currentClientSyncStamp);
			return false;
		}
	}

	UNREF(velocity);
	m_lastVerifiedTransform = transform;
	m_lastVerifiedSpeed = speed;
	m_lastVerifiedSyncStamp = currentClientSyncStamp;
	return true;
}

// ----------------------------------------------------------------------

PlayerShipController * PlayerShipController::asPlayerShipController()
{
	return this;
}

// ----------------------------------------------------------------------

PlayerShipController const * PlayerShipController::asPlayerShipController() const
{
	return this;
}

// ----------------------------------------------------------------------

bool PlayerShipController::shouldCheckForEnemies() const
{
	return (getShipOwner()->hasTurrets());
}

// ----------------------------------------------------------------------

void PlayerShipController::updateGunnerWeaponIndex(NetworkId const & player, int const gunnerWeaponIndex)
{
	GunnerWeaponIndexList::iterator iterGunnerWeaponIndexList = m_gunnerWeaponIndexList->find(player);

	if (iterGunnerWeaponIndexList != m_gunnerWeaponIndexList->end())
	{
		int const previousGunnerWeaponIndex = iterGunnerWeaponIndexList->second;
		iterGunnerWeaponIndexList->second = gunnerWeaponIndex;

		if (   (previousGunnerWeaponIndex >= 0)
			&& (gunnerWeaponIndex == -1))
		{
			// Gunner is exiting a gun

			setWeaponIndexPlayerControlled(previousGunnerWeaponIndex, false);
			m_gunnerWeaponIndexList->erase(iterGunnerWeaponIndexList);
		}
		else if (previousGunnerWeaponIndex != gunnerWeaponIndex)
		{
			// Gunner was previously in a gun and is moving to a new gun

			setWeaponIndexPlayerControlled(previousGunnerWeaponIndex, false);
			setWeaponIndexPlayerControlled(gunnerWeaponIndex, true);
		}
	}
	else
	{
		if (gunnerWeaponIndex >= 0)
		{
			// Gunner is entering a gun and was previously not in a gun

			setWeaponIndexPlayerControlled(gunnerWeaponIndex, true);
			IGNORE_RETURN(m_gunnerWeaponIndexList->insert(std::make_pair(player, gunnerWeaponIndex)));
		}
		else
		{
			// Gunner is not entering a gun and was not previously in a gun
		}
	}
}

// ----------------------------------------------------------------------

void PlayerShipController::setWeaponIndexPlayerControlled(int const weaponIndex, bool const playerControlled)
{
	DEBUG_FATAL((weaponIndex < 0), ("Invalid weaponIndex(%d)", weaponIndex));

	if (m_turretTargetingSystem != nullptr)
	{
		safe_cast<PlayerShipTurretTargetingSystem*>(m_turretTargetingSystem)->setWeaponIndexPlayerControlled(weaponIndex, playerControlled);
	}
}

// ----------------------------------------------------------------------

void PlayerShipController::addAiTargetingMe(NetworkId const & unit)
{
	ShipController::addAiTargetingMe(unit);

	if (m_targetedByAiTimer.isExpired())
	{
		getShipOwner()->setCondition(static_cast<int>(TangibleObject::C_spaceCombatMusic));
	}

	m_targetedByAiTimer.reset();
}

// ======================================================================
