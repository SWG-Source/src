// ======================================================================
//
// AiCreatureController.cpp
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiCreatureController.h"

#include "serverGame/AiMovementBase.h"
#include "serverGame/AiMovementSwarm.h"
#include "serverGame/AiMovementFace.h"
#include "serverGame/AiMovementFlee.h"
#include "serverGame/AiMovementFollow.h"
#include "serverGame/AiMovementIdle.h"
#include "serverGame/AiMovementLoiter.h"
#include "serverGame/AiMovementMove.h"
#include "serverGame/AiMovementPathFollow.h"
#include "serverGame/AiMovementPatrol.h"
#include "serverGame/AiMovementWander.h"
#include "serverGame/AiMovementWanderInterior.h"
#include "serverGame/AiCreatureCombatProfile.h"
#include "serverGame/AiCreatureData.h"
#include "serverGame/AiLogManager.h"
#include "serverGame/CellObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/NameManager.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/WeaponObject.h"
#include "serverNetworkMessages/AiMovementMessage.h"
#include "serverNetworkMessages/AiCreatureStateMessage.h"
#include "serverPathfinding/CityPathGraphManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverScript/ScriptFunctionTable.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/AiDebugString.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedUtility/Location.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/AIDebuggingMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/Location.h"
#include "UnicodeUtils.h"

#include <algorithm>

using namespace Scripting;

// ======================================================================
//
// AiCreatureControllerNamespace
//
// ======================================================================

namespace AiCreatureControllerNamespace
{
	bool s_installed = false;
	typedef std::set<Client *> ObserverList;

	PersistentCrcString * s_defaultCreatureName = nullptr;

	void remove();
	Location getLocation(ServerObject const & serverObject);
}

using namespace AiCreatureControllerNamespace;

//-----------------------------------------------------------------------
void AiCreatureControllerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed."));

	delete s_defaultCreatureName;
	s_defaultCreatureName = nullptr;

	s_installed = false;
}

//-----------------------------------------------------------------------
Location AiCreatureControllerNamespace::getLocation(ServerObject const & serverObject)
{
	CellProperty const * const cellProperty = serverObject.getParentCell();
	Object const * const cellObject = (cellProperty != nullptr) ? &cellProperty->getOwner() : nullptr;
	Vector const & positionRelativeToCellOrWorld = (cellObject != nullptr) ? serverObject.getPosition_c() : serverObject.getPosition_w();
	NetworkId const & networkIdForCellOrWorld = (cellObject != nullptr) ? cellObject->getNetworkId() : NetworkId::cms_invalid;

	return Location(positionRelativeToCellOrWorld, networkIdForCellOrWorld, Location::getCrcBySceneName(serverObject.getSceneId()));
}

// ======================================================================
//
// AiCreatureController
//
// ======================================================================

//-----------------------------------------------------------------------
AICreatureController::AICreatureController(CreatureObject * newOwner)
 : CreatureController(newOwner)
 , m_movement()
 , m_pendingMovement()
 , m_suspendedMovement()
 , m_lastStartPosition(newOwner->getPosition_p())
 , m_lastEndPosition(newOwner->getPosition_p())
 , m_sleepTimer(0.0f)
 , m_inPathfindingRegion(false)
 , m_stuckCounter(0)
 , m_authority(false)
 , m_speed(0.0f)
 , m_aiCreatureData(&AiCreatureData::getDefaultCreatureData())
 , m_primaryWeaponActions()
 , m_secondaryWeaponActions()
 , m_running(false)
 , m_homeLocation()
 , m_primaryWeapon()
 , m_secondaryWeapon()
 , m_frozen(false)
 , m_combatStartLocation()
 , m_retreating(false)
 , m_retreatingStartTime(0)
 , m_logging(false)
 , m_creatureName()
 , m_hibernationDelay(0)
 , m_hibernationTimer(0)
#ifdef _DEBUG
 , m_aiDebugStringCrc(0)
#endif // _DEBUG
 , m_invalidTarget(false)
{
	m_creatureName.setSourceObject(this);
}

//-----------------------------------------------------------------------
AICreatureController::~AICreatureController()
{
	m_movement          = AiMovementBaseNullPtr;
	m_pendingMovement   = AiMovementBaseNullPtr;
	m_suspendedMovement = AiMovementBaseNullPtr;

	Object * const owner = getOwner();

	if (owner != nullptr && owner->isAuthoritative() && !getHibernate() && m_hibernationDelay.get() > 0 && m_hibernationTimer.get() > 0)
		ObjectTracker::removeDelayedHibernatingAI();

	if ((owner != nullptr) && AiLogManager::isLogging(owner->getNetworkId()))
	{
		AiLogManager::setLogging(owner->getNetworkId(), false);
	}
}

//-----------------------------------------------------------------------

void AICreatureController::install()
{
	DEBUG_FATAL(s_installed, ("Already installed."));

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", (""));
	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("-------- AI DEBUG - NEW SESSION --------"));
	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", (""));

	AiCreatureCombatProfile::install();
	AiCreatureData::install();

	s_defaultCreatureName = new PersistentCrcString("NonCombatAI", false);

	ExitChain::add(AiCreatureControllerNamespace::remove, "AiCreatureControllerNamespace::remove");
	s_installed = true;
}

//-----------------------------------------------------------------------

void AICreatureController::CreatureNameChangedCallback::modified(AICreatureController & target, std::string const & /*oldValue*/, std::string const & newValue, bool /*local*/) const
{
	target.onCreatureNameChanged(newValue);
}

//-----------------------------------------------------------------------

void AICreatureController::handleMessage (int message, float value, const MessageQueue::Data* const data, uint32 flags)
{
	CreatureObject * const owner = static_cast<CreatureObject*>(getOwner());
	if (owner == nullptr)
	{
		DEBUG_FATAL(true, ("Owner is nullptr in AiCreatureController::handleMessage\n"));
		return;
	}

	switch (message)
	{
		case CM_aiSetMovement:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiSetMovement) owner(%s)", getDebugInformation().c_str()));

				AiMovementMessage const * const msg = safe_cast<const AiMovementMessage *>(data);

				if (msg != nullptr)
				{
					if (owner->isAuthoritative())
					{
						DEBUG_WARNING(true, ("AICreatureController::handleMessage() owner(%s) Received CM_aiSetMovement(%d) for an authoritative object. Only proxied objects should receive this controller message.", getDebugInformation().c_str(), static_cast<int>(msg->getMovementType())));
					}
					else
					{
						if (msg->getMovement() != AiMovementBaseNullPtr)
						{
							changeMovement(msg->getMovement());
						}
						else
						{
							changeMovement(AiMovementBaseNullPtr);
						}
					}
				}
			}
			break;
		case CM_aiSetMovementRun:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiSetMovementRun) owner(%s)", getDebugInformation().c_str()));
				setMovementRun();
			}
			break;
		case CM_aiSetMovementWalk:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiSetMovementWalk) owner(%s)", getDebugInformation().c_str()));
				setMovementWalk();
			}
			break;
		case CM_aiSetCreatureName:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiSetCreatureName) owner(%s)", getDebugInformation().c_str()));
				MessageQueueGenericValueType<std::string> const * const msg = safe_cast<MessageQueueGenericValueType<std::string> const *>(data);

				if (msg != nullptr)
				{
					setCreatureName(msg->getValue());
				}
			}
			break;
		case CM_aiSetHomeLocation:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiSetHomeLocation) owner(%s)", getDebugInformation().c_str()));
				MessageQueueGenericValueType<Location> const * const msg = safe_cast<MessageQueueGenericValueType<Location> const *>(data);

				if (msg != nullptr)
				{
					setHomeLocation(msg->getValue());
				}
			}
			break;
		case CM_aiSetFrozen:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiSetFrozen) owner(%s)", getDebugInformation().c_str()));
				MessageQueueGenericValueType<bool> const * const msg = safe_cast<MessageQueueGenericValueType<bool> const *>(data);

				if (msg != nullptr)
				{
					setFrozen(msg->getValue());
				}
			}
			break;
		case CM_aiSetRetreating:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiSetRetreating) owner(%s)", getDebugInformation().c_str()));
				MessageQueueGenericValueType<bool> const * const msg = safe_cast<MessageQueueGenericValueType<bool> const *>(data);

				if (msg != nullptr)
				{
					setRetreating(msg->getValue());
				}
			}
			break;
		case CM_aiSetLogging:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiSetLogging) owner(%s)", getDebugInformation().c_str()));
				MessageQueueGenericValueType<bool> const * const msg = safe_cast<MessageQueueGenericValueType<bool> const *>(data);

				if (msg != nullptr)
				{
					setLogging(msg->getValue());
				}
			}
			break;
		case CM_aiMarkCombatStartLocation:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiMarkCombatStartLocation) owner(%s)", getDebugInformation().c_str()));
				markCombatStartLocation();
			}
			break;
		case CM_aiEquipPrimaryWeapon:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiEquipPrimaryWeapon) owner(%s)", getDebugInformation().c_str()));
				equipPrimaryWeapon();
			}
			break;
		case CM_aiEquipSecondaryWeapon:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiEquipSecondaryWeapon) owner(%s)", getDebugInformation().c_str()));
				equipSecondaryWeapon();
			}
			break;
		case CM_aiUnEquipWeapons:
			{
				LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::handleMessage(CM_aiUnEquipWeapons) owner(%s)", getDebugInformation().c_str()));
				unEquipWeapons();
			}
			break;
		case CM_setHibernationDelay:
			{
				MessageQueueGenericValueType<float> const * const msg = safe_cast<MessageQueueGenericValueType<float> const *>(data);
				if (msg != nullptr)
					setHibernationDelay(msg->getValue());
			}
			break;
		default:
			CreatureController::handleMessage(message, value, data, flags);
			break;
	}
}

//-----------------------------------------------------------------------
void AICreatureController::setSpeed(float const speed)
{
	m_speed = speed;
}

//-----------------------------------------------------------------------
float AICreatureController::getSpeed() const
{
	return m_speed;
}

// ----------------------------------------------------------------------

bool AICreatureController::isRunning ( void ) const
{
	float runSpeed = getCreature()->getRunSpeed();
	float walkSpeed = getCreature()->getWalkSpeed();

	// This is a hack to make swimming always go at a constant speed.

	if (m_running.get())
	{
		return (runSpeed != 0);
	}
	else
	{
		return (walkSpeed == 0);
	}
}

// ----------------------------------------------------------------------

float AICreatureController::getAcceleration ( void ) const
{
	float base;

	if (isRunning())
	{
		base = getCreature()->getAcceleration(SharedCreatureObjectTemplate::MT_run);
	}
	else
	{
		base = getCreature()->getAcceleration(SharedCreatureObjectTemplate::MT_walk);
	}

	float scale = getCreature()->getAccelScale();
	float modifier = getCreature()->getAccelPercent();

	return base * scale * modifier;
}

// ----------------------------------------------------------------------

void AICreatureController::moveTowards( CellProperty const * cell, Vector const & position, float time )
{
	turnToward(cell, position);

	float const stepDistance = getSpeed() * time;
	IGNORE_RETURN(moveAlongGround( cell, position, stepDistance ));
}

// ----------------------------------------------------------------------

float AICreatureController::getDistanceToLocationSquared(CellProperty const * cell, Vector const & point) const
{
	Vector localPoint = CollisionUtils::transformToCell( cell, point, getCreatureCell() );
	Vector creaturePos = getCreaturePosition_p();

	return creaturePos.magnitudeBetweenSquared(localPoint);
}

// ----------------------------------------------------------------------

bool AICreatureController::reachedPoint ( CellProperty const * cell, Vector const & point, float radius ) const
{
	float const distanceSquared = getDistanceToLocationSquared(cell, point);

	// We are giving a little tolerance here for floating point error

	return (distanceSquared <= (std::max(0.0f, sqr(radius)) + 0.01f));
}

// ----------------------------------------------------------------------

void AICreatureController::updateMovementType ( void )
{
	CreatureObject * const creature = getCreature();

	Vector newHeading = creature->getPosition_w() - m_lastStartPosition;

	float speed = getSpeed();

	if (newHeading.normalize() && (speed > 0))
	{
		setCurrentVelocity(newHeading * speed);

		if (creature->isAuthoritative())
		{
			if (isRunning())
				creature->setMovementRun();
			else
				creature->setMovementWalk();
		}
	}
	else
	{
		setCurrentVelocity(Vector::zero);
		if (creature->isAuthoritative())
			creature->setMovementStationary();
	}
}


// ----------------------------------------------------------------------

float AICreatureController::realAlter(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiCreatureController::realAlter");

	//DEBUG_REPORT_LOG(true, ("%u\n", Os::getProcessId()));

	float alterResult = CreatureController::realAlter(time);
	CreatureObject * const creatureOwner = getCreature();

#ifdef _DEBUG
	AiDebugString * aiDebugString = nullptr;
	if (!creatureOwner->getObservers().empty())
	{
		aiDebugString = new AiDebugString;
	}
#endif // _DEBUG

	// tick down our hibernation timer
	if (m_hibernationTimer.get() > 0 && creatureOwner->isAuthoritative())
	{
		if (shouldHibernate())
		{
//			DEBUG_REPORT_LOG(true, ("[aitest] Hibernation timer countdown, timer = %f, time = %f\n", m_hibernationTimer.get(), time));
			if (m_hibernationTimer.get() <= m_hibernationDelay.get())
				m_hibernationTimer = std::max(m_hibernationTimer.get() - time, 0.0f);
			else
			{
				// we want to skip the first frame, due to coming out of hibernation
				m_hibernationTimer = m_hibernationDelay.get();
			}
			if (m_hibernationTimer.get() == 0)
			{
				if (!getHibernate())
					ObjectTracker::removeDelayedHibernatingAI();
				updateHibernate();
			}
		}
		else
		{
			// we are counting down, but we don't need to hibernate anyway; stop the count
			m_hibernationTimer = -1.0f;
			ObjectTracker::removeDelayedHibernatingAI();
		}
	}

	// creatures not in the world and hibernating creatures should go no further
	if (!creatureOwner->isInWorld() || getHibernate())
	{
#ifdef _DEBUG
		if (aiDebugString != nullptr)
		{
			sendDebugAiToClients(*aiDebugString);
			delete aiDebugString;
		}
#endif // _DEBUG

//		DEBUG_REPORT_LOG(true, ("[aitest] Hibernating %s from alter\n", creatureOwner->getNetworkId().getValueString().c_str()));
		return alterResult;
	}
	
	// check floating
	{
		CollisionProperty const * const collision = creatureOwner->getCollisionProperty();
		Footprint const * const foot = (collision != nullptr) ? collision->getFootprint() : nullptr;
		bool floating = (foot != nullptr) ? foot->isFloating() : false;

		if (floating)
		{
			return alterResult;
		}
	}

	// check sleeping
	if (m_sleepTimer > 0.0f)
	{
		m_sleepTimer -= time;
//		DEBUG_REPORT_LOG(true, ("[aitest] Sleeping %s for %f from alter\n", creatureOwner->getNetworkId().getValueString().c_str(), m_sleepTimer));
		AlterResult::incorporateLongAlterResult(alterResult, m_sleepTimer);
		return alterResult;
	}

	// change behaviors
	if (getPendingMovementType() != AMT_invalid)
	{
		PROFILER_AUTO_BLOCK_DEFINE("AiCreatureController::realAlter: getPendingMovementType() != AMT_invalid");

		// Update our inPathfindingRegion flag whenever the behavior changes
		CityPathGraph const * graph = CityPathGraphManager::getCityGraphFor(creatureOwner);
		m_inPathfindingRegion = (graph != nullptr);
		applyMovementChange();
	}

	// alter behaviors
	Vector newStartPosition = getCreature()->getPosition_w();

	if (   !creatureOwner->isDead()
	    && creatureOwner->isAuthoritative())
	{
		PROFILER_AUTO_BLOCK_DEFINE("AiCreatureController::realAlter: !creatureOwner->isDead() && creatureOwner->isAuthoritative()");

		// This is to help catch and prevent bugs with retreating until all the kinks are worked out
		{
			if (isRetreating())
			{
				PROFILER_AUTO_BLOCK_DEFINE("AiCreatureController::realAlter: isRetreating()");

				int const retreatingTime = static_cast<int>(Os::getRealSystemTime() - m_retreatingStartTime.get());
				int const maxRetreatTime = 80;

				if (retreatingTime > maxRetreatTime)
				{
					AiLocation location(m_combatStartLocation.get().getCell(), m_combatStartLocation.get().getCoordinates(), 0.0f);
					warpTo(location.getCell(), location.getPosition_p());

					setRetreating(false);

					DEBUG_WARNING(true, ("AICreatureController::realAlter() owner(%s) inCombat(%s) Retreating has lasted(%d) > %d seconds. Auto-releasing retreat.", getDebugInformation().c_str(), getCreature()->isInCombat() ? "yes" : "no", retreatingTime, maxRetreatTime));
				}
			}
		}

		// Retreating - See if the hate list timer needs to be reset
		{
			if (!creatureOwner->isHateListEmpty())
			{
				PROFILER_AUTO_BLOCK_DEFINE("AiCreatureController::realAlter: !creatureOwner->isHateListEmpty()");

				bool resetHateTimer = false;
				CachedNetworkId const & hateTarget = creatureOwner->getHateTarget();
				CreatureObject * const hateTargetCreatureObject = CreatureObject::asCreatureObject(hateTarget.getObject());
				CreatureController const * const hateTargetCreatureController = (hateTargetCreatureObject != nullptr) ? CreatureController::asCreatureController(hateTargetCreatureObject->getController()) : nullptr;

				if (   (hateTargetCreatureObject != nullptr)
					&& (hateTargetCreatureController != nullptr))
				{
					float const hateTargetMovementSpeedSquared = hateTargetCreatureController->getCurrentVelocity().magnitudeSquared();
					float const hateTargetWalkSpeedSquared = sqr(hateTargetCreatureObject->getWalkSpeed());

					if (hateTargetMovementSpeedSquared <= hateTargetWalkSpeedSquared)
					{
						resetHateTimer = true;
					}
					else
					{
						Object * const combatStartLocationCell = NetworkIdManager::getObjectById(m_combatStartLocation.get().getCell());
						Vector const & combatStartPosition_c = m_combatStartLocation.get().getCoordinates();
						Vector const & combatStartPosition_w = (combatStartLocationCell != nullptr) ? combatStartLocationCell->rotateTranslate_o2w(combatStartPosition_c) : combatStartPosition_c;
						float const distanceToCombatStartLocationSquared = creatureOwner->getPosition_w().magnitudeBetweenSquared(combatStartPosition_w);
						float const aggroRadius = getAggroRadius();

						if (distanceToCombatStartLocationSquared <= sqr(aggroRadius * 1.2f))
						{
							resetHateTimer = true;
						}
					}

					if (resetHateTimer)
					{
						creatureOwner->resetHateTimer();
						hateTargetCreatureObject->resetHateTimer();
					}
				}
				else if (hateTarget.getObject() != nullptr)
				{
					// AI don't need to lose interest in stationary AI

					creatureOwner->resetHateTimer();
				}

				// Fix up a bad hate list / no available target state.
				if(!hateTarget.isValid())
				{
					if(m_invalidTarget)
					{
						creatureOwner->clearHateList();
						m_invalidTarget = false;
					}
					else
						m_invalidTarget = true; // give the system 1 frame to try and aquire a new valid target.
				}
				else
					m_invalidTarget = false;
			}
		}

		if (m_movement != AiMovementBaseNullPtr)
		{
			PROFILER_AUTO_BLOCK_DEFINE("AiCreatureController::realAlter: m_movement != AiMovementBaseNullPtr");

			PerformanceTimer timer;
			timer.start();

			// it's somehow possible that m_movement will get deleted during its
			// alter, so increment its ref count to prevent that
			{
				AiMovementBasePtr temp(m_movement);

				if (!m_frozen.get())
				{
					m_movement->alter(time);
				}

#ifdef _DEBUG
				if (aiDebugString != nullptr)
				{
					m_movement->addDebug(*aiDebugString);
				}
#endif // _DEBUG
				temp = AiMovementBaseNullPtr;
			}

			timer.stop();

			float msec = timer.getElapsedTime() * 1000.0f;

			// AI movements should hopefully never take 30 milliseconds

			if (   (m_movement != AiMovementBaseNullPtr)
#ifdef _DEBUG
				&& (msec >= 60.0f))
#else
				&& (msec >= 30.0f))
#endif // _DEBUG
			{
				std::string warningString;
				m_movement->getDebugInfo(warningString);

				DEBUG_WARNING(true, (FormattedString<4096>().sprintf("AICreatureController::realAlter() owner(%s) movement(%s) alter took %.0fms. Dumping movement info - %s\n", getDebugInformation().c_str(), AiMovementBase::getMovementString(m_movement->getType()), msec, warningString.c_str())));
			}
		}
	}

	m_lastStartPosition = newStartPosition;
	m_lastEndPosition = creatureOwner->getPosition_w();

	updateMovementType();

#ifdef _DEBUG
	if (aiDebugString != nullptr)
	{
		sendDebugAiToClients(*aiDebugString);
		delete aiDebugString;
	}
#endif // _DEBUG

	// done
	return alterResult;
}

//----------------------------------------------------------------------
void AICreatureController::changeMovement(AiMovementBasePtr newMovement)
{
	LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::changeMovement() movement(%s)", (newMovement != AiMovementBaseNullPtr) ? AiMovementBase::getMovementString(newMovement->getType()) : "nullptr"));

	if (getOwner()->isAuthoritative())
	{
		if (m_pendingMovement != newMovement)
		{
			m_pendingMovement = newMovement;
			m_sleepTimer = 0.0f;

			getOwner()->scheduleForAlter();
		}
	}
	else
	{
		m_movement = newMovement;
		m_pendingMovement = AiMovementBaseNullPtr;
	}
}

//-----------------------------------------------------------------------

/**
 * Stops the current movement type of the creature, allowing it to be resumed at a 
 * later time. 
 * 
 * NOTE: Only one movement can be suspended at a time. If this function 
 * is called while a previous movement is suspended, it will fail.
 *
 * @return true if the movement has been suspended, false if not.
 */
bool AICreatureController::suspendMovement()
{
	bool result = true;
	if (m_suspendedMovement == AiMovementBaseNullPtr)
	{
		m_suspendedMovement = m_movement;
		stop();
	}
	else if (m_movement != AiMovementBaseNullPtr)
	{
		result = false;
		DEBUG_WARNING(true, ("AICreatureController::suspendMovement called for creature %s when there is already a suspended movement type %d", 
			getOwner()->getNetworkId().getValueString().c_str(), static_cast<int>(m_movement->getType())));
	}
	return result;
}

//-----------------------------------------------------------------------

/**
 * Resumes a previously suspended movement.
 *
 * @return true if the movement was resumed, false if there was no previous movement.
 */
bool AICreatureController::resumeMovement()
{
	bool result = false;
	if (m_suspendedMovement != AiMovementBaseNullPtr)
	{
		result = true;
		m_pendingMovement = m_suspendedMovement;
		m_suspendedMovement = AiMovementBaseNullPtr;
	}
	return result;
}

//-----------------------------------------------------------------------

bool AICreatureController::hasSuspendedMovement()
{
	return m_suspendedMovement != AiMovementBaseNullPtr;
}

//-----------------------------------------------------------------------
void AICreatureController::applyMovementChange()
{
	if (m_movement != m_pendingMovement)
	{
		m_movement = m_pendingMovement;

		if (getOwner()->isAuthoritative())
		{
			if (m_movement != AiMovementBaseNullPtr)
			{
				sendMessageToProxyServer(CM_aiSetMovement, new AiMovementMessage(getOwner()->getNetworkId(), m_movement));
				(safe_cast<ServerObject*>(getOwner()))->setDefaultAlterTime(m_movement->getDefaultAlterTime());
			}
			else
			{
				sendMessageToProxyServer(CM_aiSetMovement, new AiMovementMessage(getOwner()->getNetworkId()));
				(safe_cast<ServerObject*>(getOwner()))->setDefaultAlterTime(AlterResult::cms_keepNoAlter);
			}
			// note: we need to do this now or updateHibernate will fail
			m_pendingMovement = AiMovementBaseNullPtr;
			updateHibernate();
		}
	}

	m_pendingMovement = AiMovementBaseNullPtr;
}

//-----------------------------------------------------------------------

bool AICreatureController::hasPendingMovement() const
{
	return m_pendingMovement != AiMovementBaseNullPtr;
}

//-----------------------------------------------------------------------

void AICreatureController::getBehaviorDebugInfo (std::string & outString) const
{
	if (m_movement != AiMovementBaseNullPtr)
	{
		m_movement->getDebugInfo(outString);
   	}
	else
	{
		outString += "No Behavior\n";
	}
}

// ======================================================================
// Wander
// ======================================================================

//-----------------------------------------------------------------------
bool AICreatureController::wander( float minDistance, float maxDistance, float minAngle, float maxAngle, float minDelay, float maxDelay)
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::wander() owner(%s) minDistance(%.2f) maxDistance(%.2f) minAngle(%.2f) maxAngle(%.2f) minDelay(%.2f) maxDelay(%.2f) Trying to wander while retreating, failing request.", getDebugInformation().c_str(), minDistance, maxDistance, minAngle, maxAngle, minDelay, maxDelay));
		return false;
	}

	LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::wander() owner(%s) minDistance(%.2f) maxDistance(%.2f) minAngle(%.2f) maxAngle(%.2f) minDelay(%.2f) maxDelay(%.2f)", getDebugInformation().c_str(), minDistance, maxDistance, minAngle, maxAngle, minDelay, maxDelay));

	if((minDistance == 0) && (maxDistance == 0))
	{
		return false;
	}

	if (getCreature()->getParentCell() != CellProperty::getWorldCellProperty())
	{
		AiMovementBasePtr movement(new AiMovementWanderInterior(this));
		changeMovement(movement);
	}
	else
	{
		AiMovementBasePtr movement(new AiMovementWander(this, minDistance, maxDistance, minAngle, maxAngle, minDelay, maxDelay));
		changeMovement(movement);
	}

	return true; //lint !e429 // custodial pointer 'behavior' has not been freed or returned // okay: ownership transfers via changeMovement().
}

// ======================================================================
// Loiter
// ======================================================================

//-----------------------------------------------------------------------
void AICreatureController::loiter(CellProperty const * homeCell, Vector const & home_p, float const minDistance, float const maxDistance, float const minDelay, float const maxDelay)
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::loiter() owner(%s) cell(%s) home(%.0f, %.0f, %.0f) distance[%.2f...%.2f] delay[%.0f...%.0f] Trying to loiter while retreating, failing request.", getDebugInformation().c_str(), ((homeCell != nullptr) ? homeCell->getCellName() : "nullptr"), home_p.x, home_p.y, home_p.z, minDistance, maxDistance, minDelay, maxDelay));
		return;
	}

	LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::loiter() owner(%s) cell(%s) home(%.0f, %.0f, %.0f) distance[%.2f...%.2f] delay[%.0f...%.0f]", getDebugInformation().c_str(), ((homeCell != nullptr) ? homeCell->getCellName() : "nullptr"), home_p.x, home_p.y, home_p.z, minDistance, maxDistance, minDelay, maxDelay));

	AiMovementBasePtr movement(new AiMovementLoiter(this, homeCell, home_p, minDistance, maxDistance, minDelay, maxDelay));

	changeMovement(movement);
}

//-----------------------------------------------------------------------
void AICreatureController::loiter(NetworkId const & homeId, float const minDistance, float const maxDistance, float const minDelay, float const maxDelay)
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::loiter() owner(%s) home(%s) distance[%.2f...%.2f] delay[%.0f...%.0f] Trying to loiter while retreating, failing request.", getDebugInformation().c_str(), homeId.getValueString().c_str(), minDistance, maxDistance, minDelay, maxDelay));
		return;
	}

	LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::loiter() owner(%s) home(%s) distance[%.2f...%.2f] delay[%.0f...%.0f]", getDebugInformation().c_str(), homeId.getValueString().c_str(), minDistance, maxDistance, minDelay, maxDelay));

	ServerObject const * const home = ServerWorld::findObjectByNetworkId(homeId);
	AiMovementBasePtr movement(new AiMovementLoiter(this,home,minDistance,maxDistance,minDelay,maxDelay));

	changeMovement(movement);
}

//-----------------------------------------------------------------------

void AICreatureController::moveTo(CellProperty const * cell, Vector const & target_p, float const radius)
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::moveTo() owner(%s) cell(%s) position(%.0f, %.0f, %.0f) radius(%.2f) Trying to moveTo while retreating, failing request.", getDebugInformation().c_str(), ((cell != nullptr) ? cell->getCellName() : "nullptr"), target_p.x, target_p.y, target_p.z, radius));
		return;
	}

	bool duplicateMovement = false;

	if (getMovementType() == AMT_move)
	{
		AiMovementMove * const aiMovementMove = (m_movement != AiMovementBaseNullPtr) ? m_movement->asAiMovementMove() : nullptr;

		if (aiMovementMove != nullptr)
		{
			AiLocation const & target = aiMovementMove->getTarget();

			if (   (target.getCell() == cell)
			    && (target.getPosition_p() == target_p)
			    && (target.getRadius() == radius))
			{
				duplicateMovement = true;
			}
		}
	}

	if (!duplicateMovement)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::moveTo() owner(%s) cell(%s) position(%.0f, %.0f, %.0f) radius(%.2f)", getDebugInformation().c_str(), ((cell != nullptr) ? cell->getCellName() : "nullptr"), target_p.x, target_p.y, target_p.z, radius));

		AiMovementBasePtr movement(new AiMovementMove(this, cell, target_p, radius));

		changeMovement(movement);
	}
}

//-----------------------------------------------------------------------

void AICreatureController::moveTo(Unicode::String const & targetName)
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::moveTo() owner(%s) target(%s) Trying to moveTo while retreating, failing request.", getDebugInformation().c_str(), Unicode::wideToNarrow(targetName).c_str()));
		return;
	}

	bool duplicateMovement = false;

	if (getMovementType() == AMT_move)
	{
		AiMovementMove * const aiMovementMove = (m_movement != AiMovementBaseNullPtr) ? m_movement->asAiMovementMove() : nullptr;

		if (aiMovementMove != nullptr)
		{
			if (aiMovementMove->getTargetName() == targetName)
			{
				duplicateMovement = true;
			}
		}
	}

	if (!duplicateMovement)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::moveTo() owner(%s) targetName(%s)", getDebugInformation().c_str(), Unicode::wideToNarrow(targetName).c_str()));

		AiMovementBasePtr behavior(new AiMovementMove(this, targetName));

		changeMovement(behavior);
	}
}

//-----------------------------------------------------------------------

void AICreatureController::patrol( std::vector<Location> const & locations, bool random, bool flip, bool repeat, int startPoint)
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::patrol() owner(%s) Trying to patrol while retreating, failing request.", getDebugInformation().c_str()));
		return;
	}

	bool duplicateMovement = false;

	if (getMovementType() == AMT_patrol)
	{
		AiMovementPatrol * const aiMovementPatrol = (m_movement != AiMovementBaseNullPtr) ? m_movement->asAiMovementPatrol() : nullptr;
		if (aiMovementPatrol != nullptr)
		{
//			if ()
//			{
//				duplicateMovement = true;
//			}
		}
	}

	if (!duplicateMovement)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::patrol() owner(%s)", getDebugInformation().c_str()));

		AiMovementBasePtr movement(new AiMovementPatrol(this, locations, random, flip, repeat, startPoint));

		changeMovement(movement);
	}
}

//-----------------------------------------------------------------------

void AICreatureController::patrol( std::vector<Unicode::String> const & locations, bool random, bool flip, bool repeat, int startPoint)
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::patrol() owner(%s) Trying to patrol while retreating, failing request.", getDebugInformation().c_str()));
		return;
	}

	bool duplicateMovement = false;

	if (getMovementType() == AMT_patrol)
	{
		AiMovementPatrol * const aiMovementPatrol = (m_movement != AiMovementBaseNullPtr) ? m_movement->asAiMovementPatrol() : nullptr;
		if (aiMovementPatrol != nullptr)
		{
//			if ()
//			{
//				duplicateMovement = true;
//			}
		}
	}

	if (!duplicateMovement)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::patrol() owner(%s)", getDebugInformation().c_str()));

		AiMovementBasePtr movement(new AiMovementPatrol(this, locations, random, flip, repeat, startPoint));

		changeMovement(movement);
	}
}

//-----------------------------------------------------------------------

void AICreatureController::stop()
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::stop() owner(%s) Trying to stop while retreating, failing request.", getDebugInformation().c_str()));
		return;
	}

	if (getMovementType() != AMT_idle)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::stop() owner(%s)", getDebugInformation().c_str()));

		AiMovementBasePtr behavior(new AiMovementIdle(this));

		changeMovement(behavior);
	}
}

//-----------------------------------------------------------------------
bool AICreatureController::faceTo(CellProperty const * targetCell, Vector const & target_p)
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::faceTo() owner(%s) cell(%s) position(%.0f, %.0f, %.0f) Trying to faceTo while retreating, failing faceTo request.", getDebugInformation().c_str(), ((targetCell != nullptr) ? targetCell->getCellName() : "nullptr"), target_p.x, target_p.y, target_p.z));
		return false;
	}

	bool duplicateMovement = false;

	if (getMovementType() == AMT_face)
	{
		AiMovementFace * const aiMovementFace = (m_movement != AiMovementBaseNullPtr) ? m_movement->asAiMovementFace() : nullptr;

		if (aiMovementFace != nullptr)
		{
			AiLocation const & target = aiMovementFace->getTarget();

			if (   (target.getCell() == targetCell)
			    && (target.getPosition_p() == target_p))
			{
				duplicateMovement = true;
			}
		}
	}

	if (!duplicateMovement)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::faceTo() owner(%s) cell(%s) position(%.0f, %.0f, %.0f)", getDebugInformation().c_str(), ((targetCell != nullptr) ? targetCell->getCellName() : "nullptr"), target_p.x, target_p.y, target_p.z));

		AiMovementBasePtr behavior(new AiMovementFace(this, targetCell, target_p));

		changeMovement(behavior);
	}

	return true; //lint !e429 // custodial pointer 'behavior' has not been freed or returned // okay: ownership transfers via changeMovement().
}

//-----------------------------------------------------------------------
bool AICreatureController::faceTo( NetworkId const & targetId )
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::faceTo() owner(%s) Trying to faceTo the target(%s) while retreating, failing request.", getDebugInformation().c_str(), targetId.getValueString().c_str()));
		return false;
	}

	ServerObject const * target = ServerWorld::findObjectByNetworkId(targetId);

	if (!target)
	{
		DEBUG_WARNING(true, ("AICreatureController::faceTo() owner(%s) Unable to resolve target(%s) to a ServerObject", getDebugInformation().c_str(), targetId.getValueString().c_str()));
		return false;
	}

	bool duplicateMovement = false;

	if (getMovementType() == AMT_face)
	{
		AiMovementFace * const aiMovementFace = (m_movement != AiMovementBaseNullPtr) ? m_movement->asAiMovementFace() : nullptr;

		if (aiMovementFace != nullptr)
		{
			AiLocation const & target = aiMovementFace->getTarget();

			if (target.getObjectId() == targetId)
			{
				duplicateMovement = true;
			}
		}
	}

	if (!duplicateMovement)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::faceTo() owner(%s) target(%s)", getDebugInformation().c_str(), targetId.getValueString().c_str()));

		AiMovementBasePtr behavior(new AiMovementFace(this, target));
		changeMovement(behavior);
	}

	return true; //lint !e429 // custodial pointer 'behavior' has not been freed or returned // okay: ownership transfers via changeMovement().
}

//----------------------------------------------------------------------
bool AICreatureController::follow( NetworkId const & targetId, float minDistance, float maxDistance )
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::follow() owner(%s) distance[%.2f...%.2f] Trying to follow the target(%s) while retreating, failing request.", getDebugInformation().c_str(), minDistance, maxDistance, targetId.getValueString().c_str()));
		return false;
	}

	ServerObject const * target = ServerWorld::findObjectByNetworkId(targetId);

	if (!target)
	{
		DEBUG_WARNING(true, ("AICreatureController::follow() owner(%s) distance[%.2f...%.2f] Unable to resolve target(%s) to a ServerObject", getDebugInformation().c_str(), minDistance, maxDistance, targetId.getValueString().c_str()));
		return false;
	}

	bool duplicateMovement = false;

	if (getMovementType() == AMT_follow)
	{
		AiMovementFollow * const aiMovementFollow = (m_movement != AiMovementBaseNullPtr) ? m_movement->asAiMovementFollow() : nullptr;

		if (aiMovementFollow != nullptr)
		{
			AiLocation const & target = aiMovementFollow->getTarget();

			if (   (targetId == target.getObjectId())
			    && (WithinEpsilonInclusive(minDistance, aiMovementFollow->getMinDistance(), 0.001f))
			    && (WithinEpsilonInclusive(maxDistance, aiMovementFollow->getMaxDistance(), 0.001f)))
			{
				duplicateMovement = true;
			}
		}
	}

	if (!duplicateMovement)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::follow() owner(%s) target(%s) distance[%.2f...%.2f]", getDebugInformation().c_str(), targetId.getValueString().c_str(), minDistance, maxDistance));

		AiMovementBasePtr behavior(new AiMovementFollow(this, target, minDistance, maxDistance));

		changeMovement(behavior);
	}

	return true; //lint !e429 // custodial pointer 'behavior' has not been freed or returned // okay: ownership transfers via changeMovement().
}

//----------------------------------------------------------------------
bool AICreatureController::follow( NetworkId const & targetId, Vector const & offset )
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::follow() owner(%s) offset(%.0f, %.0f, %.0f) Trying to follow the target(%s) while retreating, failing request.", getDebugInformation().c_str(), offset.x, offset.y, offset.z, targetId.getValueString().c_str()));
		return false;
	}

	ServerObject const * target = ServerWorld::findObjectByNetworkId(targetId);

	if (!target)
	{
		DEBUG_WARNING(true, ("AICreatureController::follow() owner(%s) offset(%.0f, %.0f, %.0f) Unable to resolve target(%s) to a ServerObject", getDebugInformation().c_str(), offset.x, offset.y, offset.z, targetId.getValueString().c_str()));
		return false;
	}

	bool duplicateMovement = false;

	if (getMovementType() == AMT_follow)
	{
		AiMovementFollow * const aiMovementFollow = (m_movement != AiMovementBaseNullPtr) ? m_movement->asAiMovementFollow() : nullptr;

		if (aiMovementFollow != nullptr)
		{
			AiLocation const & offsetTarget = aiMovementFollow->getOffsetTarget();

			if (   (targetId == offsetTarget.getObjectId())
			    && (offset == offsetTarget.getOffset()))
			{
				duplicateMovement = true;
			}
		}
	}

	if (!duplicateMovement)
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::follow() owner(%s) target(%s) offset(%.0f, %.0f, %.0f)", getDebugInformation().c_str(), targetId.getValueString().c_str(), offset.x, offset.y, offset.z));

		bool relativeOffset = true;
		float minDistance = 0.0f;
		float maxDistance = 0.0f;

		AiMovementBasePtr behavior(new AiMovementFollow(this, target, offset, relativeOffset, minDistance, maxDistance));

		changeMovement(behavior);
	}

	return true; //lint !e429 // custodial pointer 'behavior' has not been freed or returned // okay: ownership transfers via changeMovement().
}

// ----------------------------------------------------------------------
bool AICreatureController::swarm( NetworkId const & targetId )
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::swarm() owner(%s) Trying to flee the target(%s) while retreating, failing request.", getDebugInformation().c_str(), targetId.getValueString().c_str()));
		return false;
	}

	LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::swarm() owner(%s) target(%s)", getDebugInformation().c_str(), targetId.getValueString().c_str()));

	if(!targetId) return false;

	ServerObject const * target = ServerWorld::findObjectByNetworkId(targetId);

	if(!target) return false;

	// ----------
	// Check if we're already swarming the specified object.
	if (getMovementType() == AMT_swarm)
	{
		AiMovementSwarm *swarmBehavior = safe_cast<AiMovementSwarm*>(m_movement.get());
		if (swarmBehavior && (swarmBehavior->getTarget().getObjectId() == targetId))
		{
			// We're already swarming this object, keep swarming it with the same behavior.
			return true;
		}
	}

	AiMovementBasePtr behavior(new AiMovementSwarm(this,target));

	changeMovement(behavior);

	return true; //lint !e429 // custodial pointer 'behavior' has not been freed or returned // okay: ownership transfers via changeMovement().
}

// ----------------------------------------------------------------------
bool AICreatureController::swarm( NetworkId const & targetId, float offset )
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::swarm() owner(%s) offset(%.2f) Trying to flee the target(%s) while retreating, failing request.", getDebugInformation().c_str(), offset, targetId.getValueString().c_str()));
		return false;
	}

	LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::swarm() owner(%s) target(%s) offset(%.2f)", getDebugInformation().c_str(), targetId.getValueString().c_str(), offset));

	if(!targetId) return false;

	ServerObject const * target = ServerWorld::findObjectByNetworkId(targetId);

	if(!target) return false;

	AiMovementBasePtr behavior(new AiMovementSwarm(this, target, offset));

	changeMovement(behavior);

	return true; //lint !e429 // custodial pointer 'behavior' has not been freed or returned // okay: ownership transfers via changeMovement().
}

// ----------------------------------------------------------------------
bool AICreatureController::flee( NetworkId const & targetId, float minDistance, float maxDistance )
{
	if (isRetreating())
	{
		DEBUG_WARNING(true, ("AICreatureController::flee() owner(%s) distance[%.2f...%.2f] Trying to flee the target(%s) while retreating, failing request.", getDebugInformation().c_str(), minDistance, maxDistance, targetId.getValueString().c_str(), targetId.getValueString().c_str()));
		return false;
	}

	LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::flee() owner(%s) target(%s) distance[%.2f...%.2f]", getDebugInformation().c_str(), targetId.getValueString().c_str(), minDistance, maxDistance));

	if(getCreatureCell() != CellProperty::getWorldCellProperty())
	{
		AiMovementBasePtr behavior(new AiMovementIdle(this));

		changeMovement(behavior);

		return false;
	}

	if(!targetId) return false;

	ServerObject const * target = ServerWorld::findObjectByNetworkId(targetId);

	if(!target) return false;

	// ----------

	AiMovementBasePtr behavior(new AiMovementFlee(this, target, minDistance, maxDistance));

	changeMovement(behavior);

	return true; //lint !e429 // custodial pointer 'behavior' has not been freed or returned // okay: ownership transfers via changeMovement().
}

// ----------------------------------------------------------------------
CreatureObject const * AICreatureController::getCreature ( void ) const
{
	return safe_cast< CreatureObject const * >( getOwner() );
}

CreatureObject * AICreatureController::getCreature ( void )
{
	return safe_cast< CreatureObject * >( getOwner() );
}

CellProperty const * AICreatureController::getCreatureCell ( void  ) const
{
	return getOwner()->getParentCell();
}

Vector AICreatureController::getCreaturePosition_p ( void ) const
{
	return getCreature()->getPosition_p();
}

Vector AICreatureController::getCreaturePosition_w ( void ) const
{
	return getCreature()->getPosition_w();
}

float AICreatureController::getCreatureRadius ( void ) const
{
	return getCreature()->getRadius();
}

// ----------------------------------------------------------------------

void AICreatureController::moveCreature( CellProperty const * newCell, Vector const & inNewPosition, bool testMove )
{
	if(testMove)
	{
		CanMoveResult result = CollisionWorld::canMove(getCreature(),inNewPosition,0.05f,false,true,false);

		if((result == CMR_MoveOK) || (result == CMR_HitObstacle))
		{
			m_stuckCounter = 0;
		}
		else
		{
			m_stuckCounter++;

			return;
		}
	}
	else
	{
		m_stuckCounter = 0;
	}

	Vector newPosition = inNewPosition;

	CellProperty * currentCell = getOwner()->getParentCell();

	CellObject * newCellObject = nullptr;

	if( (newCell != nullptr) && (newCell != CellProperty::getWorldCellProperty()) )
	{
		newCellObject = const_cast<CellObject*>(safe_cast<CellObject const *>(&newCell->getOwner()));
	}

	Vector oldPosition = getCreaturePosition_p();

	// ----------

	if(currentCell == newCell)
	{
		float dummy;

		Vector offset(0.0f,1.0f,0.0f);

		CellProperty const * destCell = getCreatureCell()->getDestinationCell(oldPosition+offset,newPosition+offset,dummy);

		if((destCell != nullptr) && (destCell != newCell))
		{
			newPosition = CollisionUtils::transformToCell(newCell,newPosition,destCell);
			newCell = destCell;

			newCellObject = nullptr;

			if(newCell != CellProperty::getWorldCellProperty())
			{
				newCellObject = const_cast<CellObject*>(safe_cast<CellObject const *>(&newCell->getOwner()));
			}
		}
	}

	if (currentCell == newCell)
	{
		Transform objectToCell = getOwner()->getTransform_o2p();
		objectToCell.setPosition_p(newPosition);
		setGoal(objectToCell, newCellObject);
	}
	else
	{
		Transform objectToWorld = getOwner()->getTransform_o2w();

		Transform cellToWorld = newCellObject ? newCellObject->getTransform_o2w() : Transform::identity;

		Transform worldToCell;

		worldToCell.invert(cellToWorld);

		Transform objectToCell;
		objectToCell.multiply(worldToCell, objectToWorld);

		objectToCell.setPosition_p(newPosition);

		setGoal(objectToCell, newCellObject);
	}
}

//----------------------------------------------------------------------

bool stepTowards(Vector const & startPosition, Vector const & goalPosition, float stepDistance, Vector & newPosition)
{
	bool reachedGoal = true;
	Vector directionToGoal = Vector(goalPosition.x, 0.0f, goalPosition.z) - Vector(startPosition.x, 0.0f, startPosition.z);
	float const distanceToGoalSquared = directionToGoal.magnitudeSquared();

	if (distanceToGoalSquared <= sqr(stepDistance)) 
	{
		// We have overshot our goal, so just snap to the goal position

		newPosition = goalPosition;
	}
	else
	{
		// We are not to our goal yet so step towards it

		reachedGoal = false;

		// Create a unit vector towards the goal

		directionToGoal.normalize();
		
		// Move along the unit vector the requested distance

		directionToGoal *= stepDistance;

		newPosition = startPosition + directionToGoal;
	}

	return reachedGoal;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

bool AICreatureController::moveAlongGround( CellProperty const * targetCell, Vector const & target, float stepDistance)
{
	if (stepDistance <= 0)
		return true;

	CellProperty const * creatureCell = getOwner()->getParentCell();
	Vector localTarget = CollisionUtils::transformToCell( targetCell, target, creatureCell );
	Vector creaturePos = getOwner()->getPosition_p();
	Vector stepGoal;

	bool reachedGoal = stepTowards(creaturePos,localTarget,stepDistance,stepGoal);


	// For warping to work properly even when we are not in a pathfinding region
	// we still need to warp if we are in a behavior like follow or flee.
	// Not warping in these cases was causing ai to get stuck in Tansarii and would 
	// also cause problems anywhere there is not a CityPathGraphManager
	AiMovementType curMoveType = AMT_invalid;
	if (m_movement != AiMovementBaseNullPtr)
	{
		curMoveType = m_movement->getType();
	}

	bool testCollisions = m_inPathfindingRegion || curMoveType == AMT_follow || curMoveType == AMT_flee;

	moveCreature(creatureCell,stepGoal,testCollisions);

	return reachedGoal;
}

//----------------------------------------------------------------------

void AICreatureController::turnToward( CellProperty const * targetCell, Vector const & target )
{
	CellProperty const * creatureCell = getOwner()->getParentCell();

	Vector localTarget = CollisionUtils::transformToCell( targetCell, target, creatureCell );

	Vector result = getOwner()->rotateTranslate_p2o(localTarget);

	float angle = result.theta();

	getOwner()->yaw_o(angle);
}

//----------------------------------------------------------------------

float AICreatureController::getGroundDistance( Vector const & A, Vector const & B )
{
	float dx = A.x - B.x;
	float dz = A.z - B.z;

	return sqrt(dx*dx+dz*dz);
}

//--------------------------------------------------------------------
void AICreatureController::setMovementRun()
{
	if (getOwner()->isAuthoritative())
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::setMovementRun() owner(%s)", getDebugInformation().c_str()));

		m_running = true;
	}
	else
	{
		sendMessageToAuthServer(CM_aiSetMovementRun);
	}
}

//--------------------------------------------------------------------
void AICreatureController::setMovementWalk()
{
	if (getOwner()->isAuthoritative())
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::setMovementWalk() owner(%s)", getDebugInformation().c_str()));

		m_running = false;
	}
	else
	{
		sendMessageToAuthServer(CM_aiSetMovementWalk);
	}
}

// ----------------------------------------------------------------------

void AICreatureController::warpTo( CellProperty const * newCell, Vector const & newPosition )
{
	DEBUG_WARNING((!newCell || newCell == CellProperty::getWorldCellProperty()) && newPosition == Vector::zero, ("AICreatureController::warpTo tried to send a creature to the origin."));

	moveCreature(newCell,newPosition,false);

	CollisionWorld::objectWarped(getOwner());

	m_lastStartPosition = newPosition;
	m_lastEndPosition = newPosition;
}

// ----------------------------------------------------------------------

void AICreatureController::sleep ( float sleepTime )
{
	m_sleepTimer = sleepTime;
}

// ----------------------------------------------------------------------

bool AICreatureController::shouldHibernate ( void ) const
{
	if (m_movement != AiMovementBaseNullPtr && !m_movement->getHibernateOk())
		return false;

	if (m_pendingMovement != AiMovementBaseNullPtr)
		return false;

	if (isRetreating())
	{
		return false;
	}

	return CreatureController::shouldHibernate();
}

// ----------------------------------------------------------------------

void AICreatureController::setHibernate(bool hibernate)
{
	bool oldHibernate = getHibernate();
	float oldHibernationTimer = m_hibernationTimer.get();
	if (!oldHibernate && hibernate && m_hibernationDelay.get() > 0 && oldHibernationTimer != 0)
	{
		// check if we want to start our hibernation timer
		if (oldHibernationTimer < 0)
		{
			m_hibernationTimer = m_hibernationDelay.get();
		}
	}
	else
	{
		CreatureController::setHibernate(hibernate);
		bool newHibernate = getHibernate();

		// if we're coming out of hibernation, reset our alter time
		if (!newHibernate && oldHibernate)
		{
			// flag the hibernate timer as ready to use
			if (m_hibernationDelay.get() > 0)
				m_hibernationTimer = -1.0f;
			if (m_movement != AiMovementBaseNullPtr)
				(safe_cast<ServerObject*>(getOwner()))->setDefaultAlterTime(m_movement->getDefaultAlterTime());
			else
				(safe_cast<ServerObject*>(getOwner()))->setDefaultAlterTime(AlterResult::cms_keepNoAlter);
		}
	}
	if (hibernate && !oldHibernate && !getHibernate() && oldHibernationTimer <= 0 && m_hibernationTimer.get() > 0)
		ObjectTracker::addDelayedHibernatingAI();
}

// ----------------------------------------------------------------------

void AICreatureController::setHibernationDelay(float delay)
{
	if (getOwner()->isAuthoritative())
	{
		if (delay < 0)
			delay = 0;
		m_hibernationDelay = delay;
		float oldTimer = m_hibernationTimer.get();
		if (delay > 0)
		{
			if (!getHibernate() && oldTimer == 0)
			{
				// flag us as ready to delay the next time we want to hibernate
				m_hibernationTimer = -1.0f;
			}
			else
			{
				// we need to set the hibernation timer > than the delay to flag skipping the first alter, which will be a large number
				delay += 1.0f;
				m_hibernationTimer = delay;
			}
		}
		else
		{
			if (!getHibernate() && oldTimer > 0)
				ObjectTracker::removeDelayedHibernatingAI();
			m_hibernationTimer = 0;
		}
		// see if we need to change our hibernate state
		if (oldTimer > 0 && delay == 0)
			updateHibernate();
		else if (delay > 0 && getHibernate())
		{
			setHibernate(false);
			updateHibernate();
			// updateHibernate() can modify m_hibernationTimer, so we have to set it again
			m_hibernationTimer = delay;
		}
	}
	else
	{
		sendMessageToAuthServer(CM_setHibernationDelay, new MessageQueueGenericValueType<float>(delay));
	}
}

// ----------------------------------------------------------------------

int AICreatureController::getStuckCounter ( void ) const
{
	return m_stuckCounter;
}

// ----------------------------------------------------------------------

void AICreatureController::setAuthority ( bool newAuthority )
{
	LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::setAuthority() authority(%s) inCombat(%s) owner(%s) ", newAuthority ? "yes" : "no", getCreature()->isInCombat() ? "yes" : "no", getDebugInformation().c_str()));

	// if we are being made authoritative, refresh our ai behavior
	if (newAuthority && !m_authority && m_movement != AiMovementBaseNullPtr)
	{
		m_movement->refresh();
	}
	m_authority = newAuthority;
}

// ----------------------------------------------------------------------

void AICreatureController::forwardServerObjectSpecificBaselines() const
{
	AiCreatureStateMessage aiCreatureStateMessage;
	aiCreatureStateMessage.m_networkId = getOwner()->getNetworkId();
	aiCreatureStateMessage.m_movement = AiMovementMessage(getOwner()->getNetworkId(), m_movement);

	GenericValueTypeMessage<AiCreatureStateMessage> const genericMessage("AiCreatureStateMessage", aiCreatureStateMessage);
	ServerMessageForwarding::send(genericMessage);
}

// ----------------------------------------------------------------------

typedef std::map< int, std::string > TriggerNameLookup;

char const * const getStringForTrigger( int trigger )
{
	static TriggerNameLookup lookup;
	static bool initialized = false;

	if(!initialized)
	{
#define add(A) lookup[static_cast<int>(A)] = #A

		add( TRIG_WANDER_MOVING );
		add( TRIG_WANDER_WAYPOINT );
		add( TRIG_WANDER_WAITING );
		add( TRIG_WANDER_PATH_NOT_FOUND );
		add( TRIG_LOITER_MOVING );
		add( TRIG_LOITER_WAYPOINT );
		add( TRIG_LOITER_WAITING );
		add( TRIG_FOLLOW_TARGET_LOST );
		add( TRIG_FOLLOW_WAITING );
		add( TRIG_FOLLOW_MOVING );
		add( TRIG_FOLLOW_PATH_NOT_FOUND );
		add( TRIG_FLEE_TARGET_LOST );
		add( TRIG_FLEE_WAYPOINT );
		add( TRIG_FLEE_PATH_NOT_FOUND );
		add( TRIG_MOVE_PATH_COMPLETE );
		add( TRIG_MOVE_MOVING );
		add( TRIG_MOVE_PATH_NOT_FOUND );
		add( TRIG_MADE_AUTHORITATIVE );

#undef add

		initialized = true;
	}

	return lookup[trigger].c_str();
}

void AICreatureController::triggerScriptsSimple( int intId )
{
	TrigId id = static_cast<TrigId>(intId);

	//LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::triggerScriptsSimple : %s\n", getStringForTrigger(intId)));

	ScriptParams params;
	IGNORE_RETURN(getServerOwner()->getScriptObject()->trigAllScripts(id, params));
}

// ----------------------------------------------------------------------

void AICreatureController::triggerScriptsWithTarget( int intId, NetworkId const & targetId )
{
	//LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::triggerScriptsWithTarget : %s\n", getStringForTrigger(intId)));

	TrigId id = static_cast<TrigId>(intId);

	ScriptParams params;
	params.addParam(targetId);
	IGNORE_RETURN(getServerOwner()->getScriptObject()->trigAllScripts(id, params));
}

// ----------------------------------------------------------------------

void AICreatureController::triggerScriptsWithParam ( int intId, float & io_param )
{
	//LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::triggerScriptsWithParam: %s, %f\n", getStringForTrigger(intId),io_param));

	TrigId id = static_cast<TrigId>(intId);

	ScriptParams params;
	params.addParam(io_param);
	IGNORE_RETURN(getServerOwner()->getScriptObject()->trigAllScripts(id, params));
	io_param = params.getFloatParam(0);
}

// ----------------------------------------------------------------------

AICreatureController * AICreatureController::getAiCreatureController(NetworkId const & networkId)
{
	Object * const object = NetworkIdManager::getObjectById(networkId);

	return (object != nullptr) ? AICreatureController::asAiCreatureController(object->getController()) : nullptr;
}

// ----------------------------------------------------------------------

AICreatureController * AICreatureController::asAiCreatureController(Controller * controller)
{
	CreatureController * const creatureController = (controller != nullptr) ? controller->asCreatureController() : nullptr;
	AICreatureController * const aiCreatureController = (creatureController != nullptr) ? creatureController->asAiCreatureController() : nullptr;

	return aiCreatureController;
}

// ----------------------------------------------------------------------

AICreatureController const * AICreatureController::asAiCreatureController(Controller const * controller)
{
	CreatureController const * const creatureController = (controller != nullptr) ? controller->asCreatureController() : nullptr;
	AICreatureController const * const aiCreatureController = (creatureController != nullptr) ? creatureController->asAiCreatureController() : nullptr;

	return aiCreatureController;
}

//----------------------------------------------------------------------

AICreatureController * AICreatureController::asAiCreatureController()
{
	return this;
}

//----------------------------------------------------------------------

AICreatureController const * AICreatureController::asAiCreatureController() const
{
	return this;
}

//----------------------------------------------------------------------

float AICreatureController::getAggroRadius() const
{
	float result = ConfigServerGame::getAiBaseAggroRadius();

	if (m_aiCreatureData->m_aggressive > 0.0f)
	{
		result = m_aiCreatureData->m_aggressive;
	}

	return clamp(0.0f, result, ConfigServerGame::getAiMaxAggroRadius());
}

// ----------------------------------------------------------------------

float AICreatureController::getRespectRadius(NetworkId const & target) const
{
	float const aggroRadius = getAggroRadius();
	float result = aggroRadius;

	CreatureObject const * respectCreatureObject = CreatureObject::getCreatureObject(target);

	if (respectCreatureObject != nullptr)
	{
		// If the target has a master, then use the master's level for the respect calculation

		NetworkId const & masterId = respectCreatureObject->getMasterId();

		if (masterId != NetworkId::cms_invalid)
		{
			respectCreatureObject = CreatureObject::getCreatureObject(masterId);
		}

		// Respect is only towards players

		if (   (respectCreatureObject != nullptr)
		    && respectCreatureObject->isPlayerControlled())
		{
			CreatureObject const * const creatureOwner = getCreature();
			int const ownerLevel = creatureOwner->getLevel();
			int const targetLevel = respectCreatureObject->getLevel();
			float const levelDifference = static_cast<float>(targetLevel) - static_cast<float>(ownerLevel);
			float const respectPercent = clamp(0.04f, (1.0f - (levelDifference / 20.0f)), 1.20f);

			result = respectPercent * aggroRadius;
		}
		else
		{
			DEBUG_WARNING(true, ("AICreatureController::getRespectRadius() owner(%s) master(%s) The owner has a master who is not a player. Why does this happen?", getDebugInformation().c_str(), (respectCreatureObject != nullptr) ? respectCreatureObject->getDebugInformation().c_str() : masterId.getValueString().c_str()));
		}
	}

	return result;
}

// ----------------------------------------------------------------------

float AICreatureController::getAssistRadius() const
{
	float assistRadius = ConfigServerGame::getAiAssistRadius();

	if (m_aiCreatureData->m_assist > 0.0f)
	{
		assistRadius = m_aiCreatureData->m_assist;
	}

	return clamp(0.0f, assistRadius, ConfigServerGame::getAiMaxAggroRadius());
}

#ifdef _DEBUG
// ----------------------------------------------------------------------

void AICreatureController::sendDebugAiToClients(AiDebugString & aiDebugString)
{
	ServerObject * const serverOwner = getServerOwner();
	const ObserverList & observerList = serverOwner->getObservers();

	if (serverOwner->isAuthoritative() && !observerList.empty())
	{
		CreatureObject * const creatureOwner = serverOwner->asCreatureObject();
		FormattedString<512> fs;

		// Display the name and level
		{
			aiDebugString.addText(fs.sprintf("%d %s\n", creatureOwner->getLevel(), (getCreatureName().isEmpty() ? "nullptr" : getCreatureName().getString())), PackedRgb::solidWhite);
		}

		// Display the look at target
		//{
		//	aiDebugString.addText(fs.sprintf("lookAtTarget(%s)\n", creatureOwner->getLookAtTarget().getValueString().c_str()), PackedRgb::solidWhite);
		//}

		// Display the trigger volumes
		//{
		//	aiDebugString.addText("Trigger Volumes\n", PackedRgb::solidWhite);
		//	ServerObject::TriggerVolumeMap::const_iterator iterTriggerVolumes = creatureOwner->getTriggerVolumeMap().begin();
		//	int index = 1;
		//
		//	for (; iterTriggerVolumes != creatureOwner->getTriggerVolumeMap().end(); ++iterTriggerVolumes)
		//	{
		//		aiDebugString.addText(fs.sprintf("%i %s\n", index, iterTriggerVolumes->first.c_str()), PackedRgb::solidWhite);
		//		++index;
		//	}
		//}

		if (creatureOwner->isInvulnerable())
		{
			aiDebugString.addText("INVULNERABLE\n", PackedRgb::solidMagenta);
		}

		if (isFrozen())
		{
			aiDebugString.addText("FROZEN\n", PackedRgb::solidCyan);
		}

		if (creatureOwner->isDead())
		{
			aiDebugString.addText("DEAD\n", PackedRgb::solidRed);
		}
		else
		{
			if (isRetreating())
			{
				int const retreatingTime = static_cast<int>(Os::getRealSystemTime() - m_retreatingStartTime.get());

				aiDebugString.addText(fs.sprintf("RETREATING %d\n", retreatingTime), PackedRgb::solidMagenta);
			}

			// Show the floorId
			{
				Floor const * const floor = CollisionWorld::getFloorStandingOn(*creatureOwner);

				aiDebugString.addText(fs.sprintf("floor (%s)\n", (floor == nullptr) ? "none" : floor->getId().getValueString().c_str()), PackedRgb::solidWhite);
			}

			// Display the AI movement speed
			{
				float const movementPercent = (creatureOwner->getMovementPercent() * 100.0f);

				aiDebugString.addText(fs.sprintf("mv (%.0f%%:%.1f)%s%s%s\n", movementPercent, getCurrentVelocity().magnitude(), (creatureOwner->isStopped() ? " stopped" : ""), (creatureOwner->isWalking() ? " walking" : ""), (creatureOwner->isRunning() ? " running" : "")), PackedRgb::solidWhite);
				aiDebugString.addText(fs.sprintf("walk %.1f run %.1f\n", creatureOwner->getWalkSpeed(), creatureOwner->getRunSpeed()), PackedRgb::solidWhite);
			}

			Vector const & ownerPosition_w = serverOwner->getPosition_w();

			if (creatureOwner->isInCombat())
			{
				// Primary Weapon
				{
					ServerObject const * const primaryServerObject = ServerObject::getServerObject(getPrimaryWeapon());
					WeaponObject const * const primaryWeaponObject = (primaryServerObject != nullptr) ? primaryServerObject->asWeaponObject() : nullptr;

					if (primaryWeaponObject != nullptr)
					{
						aiDebugString.addText(fs.sprintf("%s pri(%s) [%.0f...%.0f] sp(%s)\n", (usingPrimaryWeapon() ? "->" : ""), FileNameUtils::get(primaryWeaponObject->getObjectTemplateName(), FileNameUtils::fileName).c_str(), primaryWeaponObject->getMinRange(), primaryWeaponObject->getMaxRange(), m_aiCreatureData->m_primarySpecials.isEmpty() ? "none" : m_aiCreatureData->m_primarySpecials.getString()), PackedRgb::solidWhite);
					}
					else
					{
						aiDebugString.addText(fs.sprintf("pri(nullptr:ERROR)\n"), PackedRgb::solidWhite);
					}

					//if (usingPrimaryWeapon())
					//{
						//float const ownerRadius = creatureOwner->getRadius();
						//float const minRange = primaryWeaponObject->getMinRange() + ownerRadius;
						//float const maxRange = primaryWeaponObject->getMaxRange() + ownerRadius;
						//
						//if (minRange > ownerRadius)
						//{
						//	aiDebugString.addCircleAtObjectOffset(creatureOwner->getNetworkId(), Vector::zero, minRange, PackedRgb::solidMagenta);
						//}
						//
						//if (maxRange > ownerRadius)
						//{
						//	aiDebugString.addCircleAtObjectOffset(creatureOwner->getNetworkId(), Vector::zero, maxRange, PackedRgb::solidRed);
						//}
					//}
				}

				// Secondary Weapon
				{
					ServerObject const * const secondaryServerObject = ServerObject::getServerObject(getSecondaryWeapon());
					WeaponObject const * const secondaryWeaponObject = (secondaryServerObject != nullptr) ? secondaryServerObject->asWeaponObject() : nullptr;

					if (secondaryWeaponObject != nullptr)
					{
						aiDebugString.addText(fs.sprintf("%s sec (%s) [%.0f...%.0f] sp(%s)\n", (usingSecondaryWeapon() ? "->" : ""), FileNameUtils::get(secondaryWeaponObject->getObjectTemplateName(), FileNameUtils::fileName).c_str(), secondaryWeaponObject->getMinRange(), secondaryWeaponObject->getMaxRange(), m_aiCreatureData->m_secondarySpecials.isEmpty() ? "none" : m_aiCreatureData->m_secondarySpecials.getString()), PackedRgb::solidWhite);
					}
					else
					{
						aiDebugString.addText(fs.sprintf("sec (NONE)\n"), PackedRgb::solidWhite);
					}

					//if (usingSecondaryWeapon())
					//{
						//float const ownerRadius = creatureOwner->getRadius();
						//float const minRange = secondaryWeaponObject->getMinRange() + ownerRadius;
						//float const maxRange = secondaryWeaponObject->getMaxRange() + ownerRadius;
						//
						//if (minRange > ownerRadius)
						//{
						//	aiDebugString.addCircleAtObjectOffset(creatureOwner->getNetworkId(), Vector::zero, minRange, PackedRgb::solidMagenta);
						//}
						//
						//if (maxRange > ownerRadius)
						//{
						//	aiDebugString.addCircleAtObjectOffset(creatureOwner->getNetworkId(), Vector::zero, maxRange, PackedRgb::solidRed);
						//}
					//}
				}

				// Display the leash length
				{
					aiDebugString.addLineToPosition(CellProperty::getPosition_w(m_combatStartLocation.get()), PackedRgb::solidGreen);
					//aiDebugString.addCircle(m_combatStartLocation.get().getCoordinates(), getLeashRadius(), PackedRgb::solidGreen);

					//float const leashLength = m_combatStartLocation.get().getCoordinates().magnitudeBetween(ownerPosition_w);
					//float const leashRemaining = (floorf(getLeashRadius() - leashLength) / 2.0f) * 2.0f;
					//aiDebugString.addText(fs.sprintf("leashRemaining(%.0fm)\n", leashRemaining), PackedRgb::solidCyan);
				}

				{
					aiDebugString.addText(fs.sprintf("combat duration (%d)\n", creatureOwner->getCombatDuration()), PackedRgb::solidWhite);
				}

				// Display the AI hate list
				{
					//aiDebugString.addText(fs.sprintf("in combat: %s\n", (creatureOwner->isInCombat() ? "YES" : "NO")), PackedRgb::solidWhite);

					HateList::SortedList hateList;
					creatureOwner->getSortedHateList(hateList);

					if (hateList.empty())
					{
						aiDebugString.addText("* NO HATE TARGETS *\n", PackedRgb::solidYellow);
					}
					else
					{
						if (creatureOwner->isHateListAutoExpireTargetEnabled())
						{
							aiDebugString.addText(fs.sprintf("* HATE LIST * %d\n", creatureOwner->getHateListAutoExpireTargetDuration() - creatureOwner->getTimeSinceLastHateListUpdate()), PackedRgb::solidRed);
						}
						else
						{
							aiDebugString.addText("* HATE LIST * no expire\n", PackedRgb::solidRed);
						}

						HateList::SortedList::const_iterator iterHateList = hateList.begin();

						for (; iterHateList != hateList.end(); ++iterHateList)
						{
							CachedNetworkId const & hateTarget = iterHateList->first;
							TangibleObject const * const hateTargetTangibleObject = TangibleObject::asTangibleObject(hateTarget.getObject());
							float const hate = iterHateList->second;
							std::string hateTargetName;
							if (hateTargetTangibleObject != nullptr)
							{
								hateTargetName = Unicode::wideToNarrow(hateTargetTangibleObject->getEncodedObjectName()).c_str();
							}
							else
							{
								hateTargetName = "nullptr";
							}

							aiDebugString.addText(fs.sprintf("%s:%s(%.1f)\n", hateTargetName.c_str(), hateTarget.getValueString().c_str(), hate), (iterHateList == hateList.begin()) ? PackedRgb::solidGreen : PackedRgb::solidRed);
						}
					}
				}
			}
			else
			{
				if (isAggressive())
				{
					aiDebugString.addText(fs.sprintf("Aggro %.0fm\n", getAggroRadius()), PackedRgb::solidRed);
					aiDebugString.addCircleAtObjectOffset(creatureOwner->getNetworkId(), Vector::zero, getAggroRadius(), PackedRgb::solidRed);
				}

				if (isAssist())
				{
					aiDebugString.addText(fs.sprintf("Assist %.0fm\n", getAssistRadius()), PackedRgb::solidOrange);
					aiDebugString.addCircleAtObjectOffset(creatureOwner->getNetworkId(), Vector::zero, getAssistRadius(), PackedRgb::solidOrange);
				}

				//if (isStalker())
				//{
				//	aiDebugString.addText("S", PackedRgb::solidYellow);
				//}

				if (isKiller())
				{
					aiDebugString.addText("DeathBlow", PackedRgb::solidYellow);
				}
			}

			// Line to server position

			aiDebugString.addLineToPosition(ownerPosition_w, PackedRgb::solidYellow);
		}

		std::string finalString;
		uint32 const crc = aiDebugString.toString(finalString);

		if (m_aiDebugStringCrc != crc)
		{
			m_aiDebugStringCrc = crc;

			ObserverList::const_iterator iterObserverList = observerList.begin();

			for (; iterObserverList != observerList.end(); ++iterObserverList)
			{
				Client * const client = NON_NULL(*iterObserverList);
				ServerObject * const characterObject = client->getCharacterObject();

				// Only send to the client if the objvar is set

				int temp = 0;
				if (   (characterObject != nullptr)
				    && characterObject->getObjVars().getItem("ai_debug_string", temp))
				{
					if (temp > 0)
					{
						bool const reliable = true;
						GenericValueTypeMessage<std::pair<NetworkId, std::string> > message("AiDebugString", std::make_pair(getOwner()->getNetworkId(), finalString));

						client->send(message, reliable);
					}
				}
			}
		}
	}
}
#endif // _DEBUG

// ----------------------------------------------------------------------
void AICreatureController::setHomeLocation(Location const & location)
{
	if (getOwner()->isAuthoritative())
	{
		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::setHomeLocation() owner(%s) sceneId(%u:%s) cell(%s) coordinates(%.2f, %.2f, %.2f)", getDebugInformation().c_str(), location.getSceneIdCrc(), (location.getSceneId() != nullptr) ? location.getSceneId() : "nullptr", location.getCell().getValueString().c_str(), location.getCoordinates().x, location.getCoordinates().y, location.getCoordinates().z));

		m_homeLocation = location;
	}
	else
	{
		sendMessageToAuthServer(CM_aiSetHomeLocation, new MessageQueueGenericValueType<Location>(location));
	}
}

// ----------------------------------------------------------------------
Location const & AICreatureController::getHomeLocation() const
{
	return m_homeLocation.get();
}

// ----------------------------------------------------------------------
void AICreatureController::markCombatStartLocation()
{
	CreatureObject const * const creatureOwner = getCreature();

	if (creatureOwner->isAuthoritative())
	{
		m_combatStartLocation = getLocation(*creatureOwner);

		LOGC(AiLogManager::isLogging(creatureOwner->getNetworkId()), "debug_ai", ("AICreatureController::markCombatStartLocation() owner(%s) sceneId(%u:%s) cell(%s) coordinates(%.2f, %.2f, %.2f)", getDebugInformation().c_str(), m_combatStartLocation.get().getSceneIdCrc(), (m_combatStartLocation.get().getSceneId() != nullptr) ? m_combatStartLocation.get().getSceneId() : "nullptr", m_combatStartLocation.get().getCell().getValueString().c_str(), m_combatStartLocation.get().getCoordinates().x, m_combatStartLocation.get().getCoordinates().y, m_combatStartLocation.get().getCoordinates().z));

		m_primaryWeaponActions.reset();
		m_secondaryWeaponActions.reset();
	}
	else
	{
		sendMessageToAuthServer(CM_aiMarkCombatStartLocation);
	}
}

// ----------------------------------------------------------------------

Location const & AICreatureController::getCombatStartLocation() const
{
	return m_combatStartLocation.get();
}

// ----------------------------------------------------------------------

float AICreatureController::getLeashRadius()
{
	return ConfigServerGame::getAiLeashRadius();
}

// ----------------------------------------------------------------------

void AICreatureController::setCreatureName(std::string const & creatureName)
{
	if (getCreature()->isAuthoritative())
	{
		m_creatureName = creatureName;
	}
	else
	{
		sendMessageToAuthServer(CM_aiSetCreatureName, new MessageQueueGenericValueType<std::string>(creatureName));
	}
}

// ----------------------------------------------------------------------

void AICreatureController::onCreatureNameChanged(std::string const & creatureName)
{
	CreatureObject * const creatureOwner = getCreature();

	// creatureName should be the creature name as defined in creatures_datatable.xml

	m_aiCreatureData = &AiCreatureData::getCreatureData(TemporaryCrcString(creatureName.c_str(), false));

	if (creatureOwner && creatureOwner->isAuthoritative())
	{
		setPrimaryWeapon(m_aiCreatureData->m_primaryWeapon);
		setSecondaryWeapon(m_aiCreatureData->m_secondaryWeapon);
		creatureOwner->setDifficulty(m_aiCreatureData->m_difficulty);
	}

	AiCreatureCombatProfile const * const primaryWeaponCombatProfile = AiCreatureCombatProfile::getCombatProfile(m_aiCreatureData->m_primarySpecials);

	if (primaryWeaponCombatProfile != nullptr)
	{
		m_primaryWeaponActions.setCombatProfile(*creatureOwner, *primaryWeaponCombatProfile);
	}
	
	AiCreatureCombatProfile const * const secondaryWeaponCombatProfile = AiCreatureCombatProfile::getCombatProfile(m_aiCreatureData->m_secondarySpecials);

	if (secondaryWeaponCombatProfile != nullptr)
	{
		m_secondaryWeaponActions.setCombatProfile(*creatureOwner, *secondaryWeaponCombatProfile);
	}

	LOGC(AiLogManager::isLogging(creatureOwner->getNetworkId()), "debug_ai", ("AICreatureController::setCreatureName() owner(%s) creatureName(%s) level(%d) primaryWeapon(%s:%s) secondaryWeapon(%s:%s)", getDebugInformation().c_str(), creatureName.c_str(), creatureOwner->getLevel(), m_primaryWeapon.get().getValueString().c_str(), m_aiCreatureData->m_primaryWeapon.getString(), m_secondaryWeapon.get().getValueString().c_str(), m_aiCreatureData->m_secondaryWeapon.getString()));
}

// ----------------------------------------------------------------------

void AICreatureController::setPrimaryWeapon(CrcString const & objectTemplate)
{
	bool const required = true;
	NetworkId const newPrimaryWeapon = createWeapon("setPrimaryWeapon", objectTemplate, required);

	if (newPrimaryWeapon == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("AICreatureController::setPrimaryWeapon() owner(%s) Unable to create the requested primary weapon(%s)", getDebugInformation().c_str(), objectTemplate.getString()));
	}
	else
	{
		bool const primaryWeaponEquipped = usingPrimaryWeapon();

		destroyPrimaryWeapon();

		m_primaryWeapon = newPrimaryWeapon;

		if (primaryWeaponEquipped)
		{
			equipPrimaryWeapon();
		}
	}
}

// ----------------------------------------------------------------------

void AICreatureController::destroyPrimaryWeapon()
{
	WeaponObject * const primaryWeapon = WeaponObject::getWeaponObject(getPrimaryWeapon());

	if (primaryWeapon != nullptr)
	{
		unEquipWeapons();
		primaryWeapon->permanentlyDestroy(DeleteReasons::God);
	}
}

// ----------------------------------------------------------------------

void AICreatureController::setSecondaryWeapon(CrcString const & objectTemplate)
{
	bool const required = false;
	NetworkId const newSecondaryWeapon = createWeapon("setSecondaryWeapon", objectTemplate, required);

	if (newSecondaryWeapon != NetworkId::cms_invalid)
	{
		bool const secondaryWeaponEquipped = usingSecondaryWeapon();

		destroySecondaryWeapon();

		m_secondaryWeapon = newSecondaryWeapon;

		if (secondaryWeaponEquipped)
		{
			equipSecondaryWeapon();
		}
	}
}

// ----------------------------------------------------------------------

void AICreatureController::destroySecondaryWeapon()
{
	WeaponObject * const secondaryWeapon = WeaponObject::getWeaponObject(getSecondaryWeapon());

	if (secondaryWeapon != nullptr)
	{
		unEquipWeapons();
		secondaryWeapon->permanentlyDestroy(DeleteReasons::God);
	}
}

// ----------------------------------------------------------------------

PersistentCrcString const & AICreatureController::getCreatureName() const
{
	if (m_aiCreatureData->m_name == nullptr)
	{
		return *s_defaultCreatureName;
	}

	return *m_aiCreatureData->m_name;
}

// ----------------------------------------------------------------------

NetworkId const & AICreatureController::getPrimaryWeapon() const
{
	return m_primaryWeapon.get();
}

// ----------------------------------------------------------------------

NetworkId const & AICreatureController::getSecondaryWeapon() const
{
	return m_secondaryWeapon.get();
}

//-----------------------------------------------------------------------
NetworkId AICreatureController::createWeapon(char const * const functionName, CrcString const & weaponName, bool const required)
{
	NetworkId result;
	CreatureObject * const creatureOwner = getCreature();
	ServerObject * const inventory = creatureOwner->getInventory();

	if (inventory == nullptr)
	{
		DEBUG_WARNING(true, ("AICreatureController::%s() Unable to find inventory for (%s) to place a weapon(%s)", functionName, getDebugInformation().c_str(), weaponName.getString()));
	}
	else
	{
		if (weaponName == TemporaryCrcString("none", false))
		{
			if (required)
			{
				DEBUG_WARNING(required, ("debug_ai: AICreatureController::%s() A required weapon is specified as ""none"" for (%s), defaulting to ""unarmed""", functionName, getDebugInformation().c_str()));
				result = getUnarmedWeapon();
			}
			else
			{
				result = NetworkId::cms_invalid;
			}
		}
		else
		{
			result = getUnarmedWeapon();

			if (weaponName != TemporaryCrcString("unarmed", false))
			{
				std::string const weaponTemplateName(AiCreatureData::getWeaponTemplateName(weaponName));

				if (weaponTemplateName != "unarmed")
				{
					ConstCharCrcString const weaponCrcName(ObjectTemplateList::lookUp(weaponTemplateName.c_str()));

					if (weaponCrcName.getCrc() == 0)
					{
						DEBUG_WARNING(true, ("AICreatureController::%s() Unable to resolve a weapon(%s) crc for (%s)", functionName, weaponTemplateName.c_str(), getDebugInformation().c_str()));
					}
					else
					{
						bool const persisted = false;
						ServerObject * const newObject = ServerWorld::createNewObject(weaponCrcName.getCrc(), *inventory, persisted);

						if (newObject != nullptr)
						{
							result = newObject->getNetworkId();
						}
						else
						{
							DEBUG_WARNING(true, ("AICreatureController::%s() Unable to create a WeaponObject(%s) for (%s)", functionName, weaponTemplateName.c_str(), getDebugInformation().c_str()));
						}
					}
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------
NetworkId AICreatureController::getUnarmedWeapon()
{
	NetworkId result;
	CreatureObject * const creatureOwner = getCreature();
	WeaponObject * const defaultWeapon = creatureOwner->getDefaultWeapon();

	if (defaultWeapon != nullptr)
	{
		result = defaultWeapon->getNetworkId();
	}
	else
	{
		DEBUG_WARNING(true, ("AICreatureController::getUnarmedWeapon() Unable to find a default weapon for (%s)", getDebugInformation().c_str()));
	}

	return result;
}

//-----------------------------------------------------------------------
void AICreatureController::equipPrimaryWeapon()
{
	CreatureObject * const creatureOwner = getCreature();

	if (creatureOwner->isAuthoritative())
	{
		if (!isCombatAi())
		{
			DEBUG_WARNING(true, ("AICreatureController::equipPrimaryWeapon() owner(%s) A non-combat AI is trying to equip a weapon", getDebugInformation().c_str()));
			return;
		}

		if (!usingPrimaryWeapon())
		{
			ServerObject * const primaryWeaponServerObject = ServerObject::getServerObject(getPrimaryWeapon());
			WeaponObject * const primaryWeaponObject = (primaryWeaponServerObject != nullptr) ? primaryWeaponServerObject->asWeaponObject() : nullptr;

			if (primaryWeaponObject != nullptr)
			{
				unEquipWeapons();

				CreatureObject * const creatureOwner = getCreature();

				if (primaryWeaponObject == creatureOwner->getDefaultWeapon())
				{
					creatureOwner->setCurrentWeapon(*primaryWeaponObject);
				}
				else
				{
					Container::ContainerErrorCode errorCode;

					if (ContainerInterface::transferItemToGeneralContainer(*creatureOwner, *primaryWeaponServerObject, nullptr, errorCode))
					{
						LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::equipPrimaryWeapon() owner(%s) primaryWeapon(%s)\n", getDebugInformation().c_str(), primaryWeaponServerObject->getDebugInformation().c_str()));

						creatureOwner->setCurrentWeapon(*primaryWeaponObject);

						GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(creatureOwner);

						if (gameScriptObject != nullptr)
						{
							ScriptParams scriptParams;
							scriptParams.addParam(primaryWeaponObject->getNetworkId());
							IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_AI_PRIMARY_WEAPON_EQUIPPED, scriptParams));
						}
					}
					else
					{
						DEBUG_WARNING(true, ("AICreatureController::equipPrimaryWeapon() owner(%s) Transfer of primaryWeapon(%s) to the visible equipped slot failed(%d).", getDebugInformation().c_str(), primaryWeaponServerObject->getDebugInformation().c_str(), errorCode));
					}
				}
			}
			else
			{
				DEBUG_WARNING(true, ("AICreatureController::equipPrimaryWeapon() owner(%s) Unable to resolve primaryWeapon(%s:%s) to a WeaponObject.", getDebugInformation().c_str(), m_primaryWeapon.get().getValueString().c_str(), m_aiCreatureData->m_primaryWeapon.getString()));
			}
		}
	}
	else
	{
		sendMessageToAuthServer(CM_aiEquipPrimaryWeapon);
	}
}

//-----------------------------------------------------------------------
void AICreatureController::equipSecondaryWeapon()
{
	CreatureObject * const creatureOwner = getCreature();

	if (creatureOwner->isAuthoritative())
	{
		if (!isCombatAi())
		{
			DEBUG_WARNING(true, ("AICreatureController::equipSecondaryWeapon() owner(%s) A non-combat AI is trying to equip a weapon", getDebugInformation().c_str()));
			return;
		}

		if (!usingSecondaryWeapon())
		{
			ServerObject * const secondaryWeaponServerObject = ServerObject::getServerObject(getSecondaryWeapon());
			WeaponObject * const secondaryWeaponObject = (secondaryWeaponServerObject != nullptr) ? secondaryWeaponServerObject->asWeaponObject() : nullptr;

			if (secondaryWeaponObject != nullptr)
			{
				unEquipWeapons();

				CreatureObject * const creatureOwner = getCreature();

				if (secondaryWeaponObject == creatureOwner->getDefaultWeapon())
				{
					creatureOwner->setCurrentWeapon(*secondaryWeaponObject);
				}
				else
				{
					Container::ContainerErrorCode errorCode;

					if (ContainerInterface::transferItemToGeneralContainer(*creatureOwner, *secondaryWeaponServerObject, nullptr, errorCode))
					{
						LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::equipSecondaryWeapon() owner(%s) secondaryWeapon(%s) errorCode(%d)\n", getDebugInformation().c_str(), secondaryWeaponServerObject->getDebugInformation().c_str()));

						creatureOwner->setCurrentWeapon(*secondaryWeaponObject);

						GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(creatureOwner);

						if (gameScriptObject != nullptr)
						{
							ScriptParams scriptParams;
							scriptParams.addParam(secondaryWeaponObject->getNetworkId());
							IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_AI_SECONDARY_WEAPON_EQUIPPED, scriptParams));
						}
					}
					else
					{
						DEBUG_WARNING(true, ("AICreatureController::equipSecondaryWeapon() owner(%s) Transfer of secondaryWeapon(%s) to the visible equipped slot failed(%d).", getDebugInformation().c_str(), secondaryWeaponServerObject->getDebugInformation().c_str(), errorCode));
					}
				}
			}
			else
			{
				DEBUG_WARNING(true, ("AICreatureController::equipSecondaryWeapon() owner(%s) Unable to resolve secondaryWeapon(%s:%s) to a WeaponObject.", getDebugInformation().c_str(), m_secondaryWeapon.get().getValueString().c_str(), m_aiCreatureData->m_secondaryWeapon.getString()));
			}
		}
	}
	else
	{
		sendMessageToAuthServer(CM_aiEquipSecondaryWeapon);
	}
}

//-----------------------------------------------------------------------
void AICreatureController::unEquipWeapons()
{
	CreatureObject * const creatureOwner = getCreature();

	if (creatureOwner->isAuthoritative())
	{
		if (!isCombatAi())
		{
			DEBUG_WARNING(true, ("AICreatureController::unEquipWeapons() owner(%s) A non-combat AI is trying to unequip a weapon", getDebugInformation().c_str()));
			return;
		}

		LOGC(AiLogManager::isLogging(getOwner()->getNetworkId()), "debug_ai", ("AICreatureController::unEquipWeapons() owner(%s)\n", getDebugInformation().c_str()));

		CreatureObject * const creatureOwner = getCreature();
		WeaponObject * const currentWeaponObject = creatureOwner->getCurrentWeapon();
		WeaponObject * const defaultWeapon = creatureOwner->getDefaultWeapon();

		if (currentWeaponObject != defaultWeapon)
		{
			// Put the current weapon back in the inventory
			{
				ServerObject * const inventory = creatureOwner->getInventory();

				if (inventory != nullptr)
				{
					Container::ContainerErrorCode error;

					if (!ContainerInterface::transferItemToVolumeContainer(*inventory, *currentWeaponObject, creatureOwner, error))
					{
						DEBUG_WARNING(true, ("AICreatureController::unEquipWeapons() owner(%s) Transfer of the currentWeapon(%s) to the inventory failed(%d).", getDebugInformation().c_str(), currentWeaponObject->getDebugInformation().c_str(), error));
					}
				}
				else
				{
					DEBUG_WARNING(true, ("AICreatureController::unEquipWeapons() owner(%s) Unable to find an inventory.", getDebugInformation().c_str()));
				}
			}

			// Equip the default weapon
			{
				if (defaultWeapon != nullptr)
				{
					creatureOwner->setCurrentWeapon(*defaultWeapon);
				}
				else
				{
					DEBUG_WARNING(true, ("AICreatureController::unEquipWeapons() owner(%s) nullptr default weapon", getDebugInformation().c_str()));
				}
			}
		}
	}
	else
	{
		sendMessageToAuthServer(CM_aiUnEquipWeapons);
	}
}

//-----------------------------------------------------------------------
bool AICreatureController::hasPrimaryWeapon() const
{
	bool const result = (getPrimaryWeapon() != NetworkId::cms_invalid);

	return result;
}

//-----------------------------------------------------------------------
bool AICreatureController::hasSecondaryWeapon() const
{
	bool const result = (getSecondaryWeapon() != NetworkId::cms_invalid);

	return result;
}

//-----------------------------------------------------------------------
bool AICreatureController::usingPrimaryWeapon()
{
	bool result = false;

	if (hasPrimaryWeapon())
	{
		WeaponObject const * const currentWeaponObject = getCreature()->getCurrentWeapon();

		if (currentWeaponObject != nullptr)
		{
			result = (getPrimaryWeapon() == currentWeaponObject->getNetworkId());
		}
	}

	return result;
}

//-----------------------------------------------------------------------
bool AICreatureController::usingSecondaryWeapon()
{
	bool result = false;

	if (hasSecondaryWeapon() && !usingPrimaryWeapon())
	{
		WeaponObject const * const currentWeaponObject = getCreature()->getCurrentWeapon();

		if (currentWeaponObject != nullptr)
		{
			result = (getSecondaryWeapon() == currentWeaponObject->getNetworkId());
		}
	}

	return result;
}

//-----------------------------------------------------------------------
void AICreatureController::setFrozen(bool const frozen)
{
	if (getOwner()->isAuthoritative())
	{
		m_frozen = frozen;
	}
	else
	{
		sendMessageToAuthServer(CM_aiSetFrozen, new MessageQueueGenericValueType<bool>(frozen));
	}
}

//-----------------------------------------------------------------------
bool AICreatureController::isFrozen() const
{
	return m_frozen.get();
}

//-----------------------------------------------------------------------
float AICreatureController::getMovementSpeedPercent() const
{
	return m_aiCreatureData->m_movementSpeedPercent;
}

//-----------------------------------------------------------------------
void AICreatureController::addServerNpAutoDeltaVariables(Archive::AutoDeltaByteStream & stream)
{
	stream.addVariable(m_running);
	stream.addVariable(m_homeLocation);
	stream.addVariable(m_primaryWeapon);
	stream.addVariable(m_secondaryWeapon);
	stream.addVariable(m_frozen);
	stream.addVariable(m_combatStartLocation);
	stream.addVariable(m_retreating);
	stream.addVariable(m_retreatingStartTime);
	stream.addVariable(m_logging);
	stream.addVariable(m_creatureName);
	stream.addVariable(m_hibernationDelay);
	stream.addVariable(m_hibernationTimer);

	m_primaryWeaponActions.addServerNpAutoDeltaVariables(stream);
	m_secondaryWeaponActions.addServerNpAutoDeltaVariables(stream);
}

//-----------------------------------------------------------------------
bool AICreatureController::isAggressive() const
{
	CreatureObject const * const creatureOwner = getCreature();

	return (!creatureOwner->isInvulnerable() && (m_aiCreatureData->m_aggressive > 0.0f));
}

//-----------------------------------------------------------------------
bool AICreatureController::isAssist() const
{
	CreatureObject const * const creatureOwner = getCreature();

	return (!creatureOwner->isInvulnerable() && (m_aiCreatureData->m_assist > 0.0f));
}

//-----------------------------------------------------------------------
bool AICreatureController::isStalker() const
{
	CreatureObject const * const creatureOwner = getCreature();

	return (!creatureOwner->isInvulnerable() && m_aiCreatureData->m_stalker);
}

//-----------------------------------------------------------------------
bool AICreatureController::isKiller() const
{
	CreatureObject const * const creatureOwner = getCreature();

	return (!creatureOwner->isInvulnerable() && (m_aiCreatureData->m_deathBlow == AiCreatureData::DB_instant));
}

//-----------------------------------------------------------------------
void AICreatureController::setRetreating(bool const retreating)
{
	CreatureObject * const creatureOwner = getCreature();

	if (creatureOwner->isAuthoritative())
	{
		bool const retreatStart = !m_retreating.get() && retreating;
		bool const retreatComplete = m_retreating.get() && !retreating;

		if (retreatStart)
		{
			// Start the AI towards its anchor position

			NetworkId const & cellId = m_combatStartLocation.get().getCell();
			Vector const & position_c = m_combatStartLocation.get().getCoordinates();

			if (cellId != NetworkId::cms_invalid)
			{
				CellObject const * const cellObject = CellObject::getCellObject(cellId);

				if (cellObject != nullptr)
				{
					CellProperty const * const cellProperty = cellObject->getCellProperty();

					moveTo(cellProperty, position_c);
				}
				else
				{
					DEBUG_WARNING(true, ("AICreatureController::setRetreating() ai(%s) Unable to resolve the cellId(%s) to a CellObject", creatureOwner->getDebugInformation().c_str(), cellId.getValueString().c_str()));
				}
			}
			else
			{
				moveTo(CellProperty::getWorldCellProperty(), position_c);
			}
		}

		m_retreating = retreating;

		if (retreatStart)
		{
			m_retreatingStartTime = Os::getRealSystemTime();

			creatureOwner->clearHateList();

			triggerScriptsSimple(TRIG_AI_RETREAT_START);
		}
		else if (retreatComplete)
		{
			triggerScriptsSimple(TRIG_AI_RETREAT_COMPLETE);
		}
	}
	else
	{
		sendMessageToAuthServer(CM_aiSetRetreating, new MessageQueueGenericValueType<bool>(retreating));
	}
}

//-----------------------------------------------------------------------
void AICreatureController::setLogging(bool const logging)
{
	CreatureObject * const creatureOwner = getCreature();

	if (creatureOwner->isAuthoritative())
	{
		m_logging = logging;

		AiLogManager::setLogging(creatureOwner->getNetworkId(), logging);
	}
	else
	{
		sendMessageToAuthServer(CM_aiSetLogging, new MessageQueueGenericValueType<bool>(logging));
	}
}

//-----------------------------------------------------------------------
bool AICreatureController::isLogging() const
{
	return m_logging.get();
}

//-----------------------------------------------------------------------
bool AICreatureController::isRetreating() const
{
	return m_retreating.get();
}

//-----------------------------------------------------------------------
bool AICreatureController::isCombatAi() const
{
	return (m_aiCreatureData != &AiCreatureData::getDefaultCreatureData());
}

// ----------------------------------------------------------------------

std::string AICreatureController::getDebugInformation() const
{
	return FormattedString<1024>().sprintf("creatureName=%s %s", getCreatureName().getString(), getOwner()->getDebugInformation().c_str());
}

//-----------------------------------------------------------------------
void AICreatureController::sendMessageToAuthServer(GameControllerMessage const gameControllerMessage)
{
	int const flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER;

	appendMessage(gameControllerMessage, 0, flags);
}

//-----------------------------------------------------------------------
void AICreatureController::sendMessageToAuthServer(GameControllerMessage const gameControllerMessage, MessageQueue::Data * messageQueue)
{
	int const flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER;

	appendMessage(gameControllerMessage, 0, messageQueue, flags);
}

//-----------------------------------------------------------------------
void AICreatureController::sendMessageToProxyServer(GameControllerMessage const gameControllerMessage, MessageQueue::Data * messageQueue)
{
	int const flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_PROXY_SERVER;

	appendMessage(gameControllerMessage, 0, messageQueue, flags);
}

//-----------------------------------------------------------------------
AiMovementType AICreatureController::getMovementType() const
{
	AiMovementType result = AMT_invalid;

	if (m_movement != AiMovementBaseNullPtr)
	{
		result = m_movement->getType();
	}

	return result;
}

//-----------------------------------------------------------------------
AiMovementType AICreatureController::getPendingMovementType() const
{
	AiMovementType result = AMT_invalid;

	if (m_pendingMovement != AiMovementBaseNullPtr)
	{
		result = m_pendingMovement->getType();
	}

	return result;
}

//-----------------------------------------------------------------------
PersistentCrcString const & AICreatureController::getCombatAction()
{
	if (usingPrimaryWeapon())
	{
		return m_primaryWeaponActions.getCombatAction();
	}
	else if (usingSecondaryWeapon())
	{
		return m_secondaryWeaponActions.getCombatAction();
	}

	return PersistentCrcString::empty;
}

//-----------------------------------------------------------------------
time_t AICreatureController::getKnockDownRecoveryTime() const
{
	AiCreatureCombatProfile const * const combatProfile = AiCreatureCombatProfile::getCombatProfile(m_aiCreatureData->m_primarySpecials);

	return (combatProfile != nullptr) ? combatProfile->m_knockDownRecoveryTime : 0;
}
//-----------------------------------------------------------------------
std::string const AICreatureController::getCombatActionsString()
{
	std::string result;
	FormattedString<4096> fs;

	result += fs.sprintf("knockDownRecoveryTime: %u\n", static_cast<unsigned int>(getKnockDownRecoveryTime()));

	// Primary Weapon
	{
		WeaponObject const * const primaryWeaponObject = WeaponObject::getWeaponObject(getPrimaryWeapon());

		if (primaryWeaponObject != nullptr)
		{
			result += fs.sprintf("PRIMARY WEAPON >>> %s range[%.0f...%.0f] %s\n", FileNameUtils::get(primaryWeaponObject->getObjectTemplateName(), FileNameUtils::fileName).c_str(), primaryWeaponObject->getMinRange(), primaryWeaponObject->getMaxRange(), usingPrimaryWeapon() ? "(active)" : "");
		}
		else
		{
			result += fs.sprintf("PRIMARY WEAPON >>> ERROR NO WEAPON\n");
		}

		AiCreatureCombatProfile const * const primaryWeaponCombatProfile = AiCreatureCombatProfile::getCombatProfile(m_aiCreatureData->m_primarySpecials);
		if (primaryWeaponCombatProfile != nullptr)
		{
			result += primaryWeaponCombatProfile->toString();
		}
		else
		{
			result += fs.sprintf(" no special attacks\n");
		}
	}

	// Secondary Weapon
	{
		WeaponObject const * const secondaryWeaponObject = WeaponObject::getWeaponObject(getSecondaryWeapon());

		if (secondaryWeaponObject != nullptr)
		{
			result += fs.sprintf("SECONDARY WEAPON >>> %s range[%.0f...%.0f] %s\n", FileNameUtils::get(secondaryWeaponObject->getObjectTemplateName(), FileNameUtils::fileName).c_str(), secondaryWeaponObject->getMinRange(), secondaryWeaponObject->getMaxRange(), usingSecondaryWeapon() ? "(active)" : "");
		}
		else
		{
			result += fs.sprintf("SECONDARY WEAPON >>> NONE\n");
		}

		AiCreatureCombatProfile const * const secondaryWeaponCombatProfile = AiCreatureCombatProfile::getCombatProfile(m_aiCreatureData->m_secondarySpecials);

		if (secondaryWeaponCombatProfile != nullptr)
		{
			result += secondaryWeaponCombatProfile->toString();
		}
		else
		{
			result += fs.sprintf(" no special attacks\n");
		}
	}

	return result;
}

// ======================================================================
