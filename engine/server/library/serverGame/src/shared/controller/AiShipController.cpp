// ======================================================================
//
// AiShipController.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipController.h"

#include "serverGame/AiPilotManager.h"
#include "serverGame/AiShipAttackTargetList.h"
#include "serverGame/AiShipBehaviorAttackBomber.h"
#include "serverGame/AiShipBehaviorAttackCapitalShip.h"
#include "serverGame/AiShipBehaviorAttackFighter.h"
#include "serverGame/AiShipBehaviorDock.h"
#include "serverGame/AiShipBehaviorFollow.h"
#include "serverGame/AiShipBehaviorIdle.h"
#include "serverGame/AiShipBehaviorTrack.h"
#include "serverGame/AiShipBehaviorWaypoint.h"
#include "serverGame/AiShipPilotData.h"
#include "serverGame/AiShipTurretTargetingSystem.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GroupObject.h"
#include "serverGame/MissileManager.h"
#include "serverGame/Pvp.h"
#include "serverGame/AiServerShipObjectInterface.h"
#include "serverGame/ShipAiEnemySearchManager.h"
#include "serverGame/ShipAiReactionManager.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceAttackSquad.h"
#include "serverGame/SpacePath.h"
#include "serverGame/SpacePathManager.h"
#include "serverGame/SpaceSquad.h"
#include "serverGame/SpaceSquadManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/SpaceAvoidanceManager.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedGame/AiDebugString.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"

#include "UnicodeUtils.h"

#include <map>
#include <vector>

// ======================================================================
//
// AiShipControllerNamespace
//
// ======================================================================

namespace AiShipControllerNamespace
{
	typedef std::set<Client *> ObserverList;
	typedef std::vector<ServerObject *> VisibilityList;
	typedef std::vector<NetworkId> EnemyList;

	bool s_installed = false;
	VisibilityList s_visibilityList;
	EnemyList s_enemyList;
	bool s_attackingEnabled = true;
	float const s_goalStopDistance = 4.0f;
	float const s_slowdownThrottlePositionMin = 0.6f;
	bool s_spaceAiClientDebugEnabled = false;
	float s_slowDownTurnRadiusGain = 0.5f;
	float s_throttleTurnRadiusGain = 0.8f;
	float const s_playerGroupDamageTakenPercent = 0.1f;
	float const s_squadDamageTakenPercent = 0.05f;
	float const s_guardSquadDamageTakenPercent = 1.2f;
}

using namespace AiShipControllerNamespace;

// ======================================================================
//
// AiShipController
//
// ======================================================================

// ----------------------------------------------------------------------

void AiShipController::install()
{
	DEBUG_FATAL(s_installed, ("Already installed."));

	AiShipPilotData::install();
	AiPilotManager::install();

	s_spaceAiClientDebugEnabled = ConfigServerGame::isAiClientDebugEnabled();

	s_installed = true;
}

// ----------------------------------------------------------------------

void AiShipController::setClientDebugEnabled(bool const enabled)
{
	s_spaceAiClientDebugEnabled = enabled;
}

// ----------------------------------------------------------------------

bool AiShipController::isClientDebugEnabled()
{
	return s_spaceAiClientDebugEnabled;
}

// ----------------------------------------------------------------------

AiShipController * AiShipController::getAiShipController(NetworkId const & unit)
{
	Object * const object = NetworkIdManager::getObjectById(unit);

	return (object != nullptr) ? AiShipController::asAiShipController(object->getController()) : nullptr;
}

// ----------------------------------------------------------------------

AiShipController * AiShipController::asAiShipController(Controller * const controller)
{
	ShipController * const shipController = (controller != nullptr) ? controller->asShipController() : nullptr;
	AiShipController * const aiShipController = (shipController != nullptr) ? shipController->asAiShipController() : nullptr;

	return aiShipController;
}

// ----------------------------------------------------------------------

AiShipController const * AiShipController::asAiShipController(Controller const * const controller)
{
	ShipController const * const shipController = (controller != nullptr) ? controller->asShipController() : nullptr;
	AiShipController const * const aiShipController = (shipController != nullptr) ? shipController->asAiShipController() : nullptr;

	return aiShipController;
}

// ----------------------------------------------------------------------

AiShipController::AiShipController(ShipObject * const owner) :
	ShipController(owner),
	m_pilotData(&AiShipPilotData::getDefaultPilotData()),
	m_pendingNonAttackBehavior(nullptr),
	m_nonAttackBehavior(nullptr),
	m_pendingAttackBehavior(nullptr),
	m_attackBehavior(nullptr),
	m_shipName(),
	m_shipClass(ShipAiReactionManager::SC_invalid),
	m_requestedSlowDown(false),
	m_squad(nullptr),
	m_attackSquad(nullptr),
	m_formationPosition_l(),
	m_attackFormationPosition_l(),
	m_path(nullptr),
	m_currentPathIndex(0),
	m_aggroRadius(200.0f),
	m_countermeasureState(CS_none),
	m_reactToMissileTimer(new Timer),
#ifdef _DEBUG
	m_aiDebugStringCrc(0),
#endif // _DEBUG
	m_moveToGoalPosition_w(),
	m_lastAttackUpdate(-1),
	m_pilotManagerInfo(new AiPilotManager),
	m_attackOrders(AO_attackFreely),
	m_exclusiveAggroSet(new CachedNetworkIdSet)
{
}

// ----------------------------------------------------------------------

AiShipController::~AiShipController()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::~AiShipController() owner(%s) DESTROYING UNIT", getOwner()->getDebugInformation().c_str()));

	// Remove this unit from its squad

	if (m_squad != nullptr)
	{
		// The owner of the squad is SpaceSquadManager

		getSquad().removeUnit(getOwner()->getNetworkId());
		m_squad = nullptr;
	}

	// Remove this unit from its attack squad

	if (m_attackSquad != nullptr)
	{
		// The owner of the attack squad is SpaceSquad

		m_attackSquad->removeUnit(getOwner()->getNetworkId());
		m_attackSquad = nullptr;
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::~AiShipController() owner(%s) ERROR: Why was this unit not in an attack squad?", getOwner()->getDebugInformation().c_str()));
	}

	// Remove path here.
	SpacePathManager::release(m_path, getOwner());

	m_path = nullptr;
	m_pilotData = nullptr;

	delete m_nonAttackBehavior;
	m_nonAttackBehavior = nullptr;
	delete m_pendingNonAttackBehavior;
	m_pendingNonAttackBehavior = nullptr;
	delete m_pendingAttackBehavior;
	m_pendingAttackBehavior = nullptr;
	delete m_attackBehavior;
	m_attackBehavior = nullptr;
	delete m_reactToMissileTimer;
	delete m_pilotManagerInfo;
	delete m_exclusiveAggroSet;
}

// ----------------------------------------------------------------------
void AiShipController::endBaselines()
{
	DEBUG_FATAL((m_squad != nullptr), ("m_squad should be nullptr"));
	m_squad = SpaceSquadManager::createSquad();

	getSquad().addUnit(getOwner()->getNetworkId());

	// Always default the ship to the idle non-attacking behavior

	idle();
}

// ----------------------------------------------------------------------

float AiShipController::realAlter(float const elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipController::realAlter");

#ifdef _DEBUG

	AiDebugString * aiDebugString = nullptr;
	if (s_spaceAiClientDebugEnabled
		&& !getShipOwner()->getObservers().empty())
	{
		aiDebugString = new AiDebugString;
	}

#endif // _DEBUG

	// We have to have a pending behavior because while in a behavior a trigger can get called which can then kill that behavior, so we have to wait until the next frame to switch behaviors so they don't stomp each other from triggers.

	if (m_pendingNonAttackBehavior != nullptr)
	{
		delete m_nonAttackBehavior;
		m_nonAttackBehavior = m_pendingNonAttackBehavior;
		m_pendingNonAttackBehavior = nullptr;
	}

	if (m_pendingDockingBehavior != nullptr)
	{
		delete m_dockingBehavior;
		m_dockingBehavior = m_pendingDockingBehavior;
		m_pendingDockingBehavior = nullptr;
	}

	if ((m_dockingBehavior != nullptr)
		&& (m_dockingBehavior->isDockFinished()))
	{
		delete m_dockingBehavior;
		m_dockingBehavior = nullptr;
	}

	m_yawPosition = 0.f;
	m_pitchPosition = 0.f;
	m_rollPosition = 0.f;
	setThrottle(0.0f);

	ShipObject * const shipOwner = NON_NULL(getShipOwner());

	PROFILER_AUTO_BLOCK_DEFINE("behaviors");
	if (m_attackBehavior != nullptr) // This was added for ships that are contructed for AI but not fully contructed proper using the space_mobile.tab
	{
		if (isAttacking()) // Capital ships never go into attack mode as such, always follow their non-attacking behavior (although they may fire turrets as they go)
		{
			PROFILER_AUTO_BLOCK_DEFINE("attack behaviors");

			// See if we need to send out the enter combat trigger

			if (m_lastAttackUpdate != (Os::getNumberOfUpdates() - 1))
			{
				triggerEnterCombat(getPrimaryAttackTarget());
			}

			m_lastAttackUpdate = Os::getNumberOfUpdates();

			//-- Open wings

			if (shipOwner->hasWings() && !shipOwner->wingsOpened())
			{
				shipOwner->openWings();
			}

			if (m_dockingBehavior != nullptr)
			{
				m_dockingBehavior->unDock();
			}

			if (isAttackSquadLeader()
				|| !getAttackSquad().isInFormation())
			{
				m_attackBehavior->alter(elapsedTime);

#ifdef _DEBUG
				if (aiDebugString != nullptr)
				{
					m_attackBehavior->addDebug(*aiDebugString);
				}
#endif // _DEBUG
			}
			else
			{
				// Follow the attack squad leader

				Object * const leaderObject = getAttackSquad().getLeader().getObject();
				ShipController * const leaderShipController = leaderObject->getController()->asShipController();
				AiShipController * const leaderAiShipController = (leaderShipController != nullptr) ? leaderShipController->asAiShipController() : nullptr;

				if (leaderAiShipController != nullptr)
				{
					Transform transform(leaderObject->getTransform_o2w());
					transform.setPosition_p(leaderAiShipController->getMoveToGoalPosition_w());

					Vector const & goalPosition_w = Formation::getPosition_w(transform, getAttackFormationPosition_l());
					float const throttle = calculateThrottleToPosition_w(goalPosition_w, getLargestTurnRadius());

					moveTo(goalPosition_w, throttle, elapsedTime);
				}
				else
				{
					LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::realAlter() ERROR: shipOwner(%s) Trying to follow a non-existent attack squad leader.", getOwner()->getNetworkId().getValueString().c_str()));
				}
			}

			// If the attack behavior changes, switch it immediately after alter.  Don't wait for the next frame, because we don't want to lose state changes
			// that might happen during this frame
			if (m_pendingAttackBehavior)
			{
				delete m_attackBehavior;
				m_attackBehavior = m_pendingAttackBehavior;
				m_pendingAttackBehavior = nullptr;
			}
		}
		else if (isBeingDocked())
		{
			// Hold the AI ship while it is being docked

			setThrottle(0.0f);
		}
		else if (m_dockingBehavior != nullptr)
		{
			m_dockingBehavior->alter(elapsedTime);

#ifdef _DEBUG
			if (aiDebugString != nullptr)
			{
				m_dockingBehavior->addDebug(*aiDebugString);
			}
#endif // _DEBUG
		}
		else if (m_nonAttackBehavior != nullptr)
		{
			PROFILER_AUTO_BLOCK_DEFINE("non-attack behaviors");

			//-- Close wings

			if (shipOwner->hasWings() && shipOwner->wingsOpened())
			{
				shipOwner->closeWings();
			}

			// Check for enemies periodically

			if (!m_enemyCheckQueued
				&& shouldCheckForEnemies())
			{
				m_enemyCheckQueued = true;
				ShipAiEnemySearchManager::add(*shipOwner);
			}

#ifdef _DEBUG
			if (!s_attackingEnabled)
			{
				m_attackTargetList->clear();
			}
#endif // _DEBUG

			if (isSquadLeader())
			{
				m_nonAttackBehavior->alter(elapsedTime);

#ifdef _DEBUG
				if (aiDebugString != nullptr)
				{
					m_nonAttackBehavior->addDebug(*aiDebugString);
				}
#endif // _DEBUG
			}
			else
			{
				// Follow the squad leader

				Object * const squadLeader = getSquad().getLeader().getObject();

				if (squadLeader != nullptr)
				{
					Vector goalPosition_w(Formation::getPosition_w(squadLeader->getTransform_o2w(), getFormationPosition_l()));

					// Determine if we need to request the unit we are following to slow down

					if (getOwner()->getPosition_w().magnitudeBetweenSquared(goalPosition_w) > sqr(getLargestTurnRadius() * s_slowDownTurnRadiusGain))
					{
						ShipController * const followedUnitShipController = squadLeader->getController()->asShipController();
						AiShipController * const followedUnitAiShipController = (followedUnitShipController != nullptr) ? followedUnitShipController->asAiShipController() : nullptr;

						if (followedUnitAiShipController != nullptr)
						{
							followedUnitAiShipController->requestSlowDown();
						}
						else
						{
							LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::realAlter() ERROR: shipOwner(%s) The followed unit does not have an AiShipController.", getOwner()->getNetworkId().getValueString().c_str()));
						}
					}

					float const throttle = calculateThrottleToPosition_w(goalPosition_w, getLargestTurnRadius() * s_throttleTurnRadiusGain);

					if (throttle <= 0.0f)
					{
						//-- The vehicle is at its destination position, so face the same direction as the squad leader

						moveTo(squadLeader->rotateTranslate_o2w(Vector(0.0f, 0.0f, 10000.0f)), 0.0f, elapsedTime);
					}
					else
					{
						//-- The vehicle is trying to get to its goal position

						moveTo(goalPosition_w, throttle, elapsedTime);
					}

#ifdef _DEBUG
					if (aiDebugString != nullptr)
					{
						m_nonAttackBehavior->addDebug(*aiDebugString);
					}
#endif // _DEBUG
				}
				else
				{
					LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::realAlter() ERROR: shipOwner(%s) Trying to follow a non-existent squad leader.", getOwner()->getNetworkId().getValueString().c_str()));
				}
			}
		}
		else
		{
			DEBUG_WARNING(true, ("debug_ai: There should never be a nullptr non-attacking behavior %s", getOwner()->getDebugInformation().c_str()));
		}
	}

	// If this ship is the squad leader and it can't move, pick a new squad leader

	if (isSquadLeader())
	{
		PROFILER_AUTO_BLOCK_DEFINE("check squadLeader");
		if (isAttacking()
			|| !hasFunctionalEngines())
		{
			getSquad().assignNewLeader();
		}
	}

	// If this ship is the attack squad leader and it can't move, pick a new attack squad leader

	if (isAttackSquadLeader())
	{
		PROFILER_AUTO_BLOCK_DEFINE("check attackSquadLeader");
		if (!hasFunctionalEngines())
		{
			getAttackSquad().assignNewLeader();
		}
	}

	//-- Manage countermeasures
	{
		PROFILER_AUTO_BLOCK_DEFINE("Manage countermeasures");
		switch (m_countermeasureState)
		{
		case CS_reacting:
			if (m_reactToMissileTimer->updateZero(elapsedTime))
				m_countermeasureState = CS_launching;
			break;

		case CS_launching:
			if (MissileManager::getInstance().isTargetedByMissile(getOwner()->getNetworkId()))
			{
				for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
				{
					PROFILER_AUTO_BLOCK_DEFINE("countermeasures missile loop");
					if (NON_NULL(getShipOwner())->isCountermeasure(weaponIndex))
						getShipOwner()->fireShotNonTurretServer(weaponIndex, NetworkId::cms_invalid, ShipChassisSlotType::SCST_invalid);
				}
			}
			else
				m_countermeasureState = CS_none;
			break;

		case CS_none:
		default:
			break;
		}
	}

	//-- Update flight model
	{
		PROFILER_AUTO_BLOCK_DEFINE("update dynamics model");

		bool const modifySpeed = isAttacking();
		AiServerShipObjectInterface const aiServerShipObjectInterface(shipOwner, *m_pilotData, modifySpeed);
		float const throttle = getThrottle();

		m_shipDynamicsModel->model(elapsedTime, m_yawPosition, m_pitchPosition, m_rollPosition, throttle, aiServerShipObjectInterface);
	}

	//-- Update the server position based on the model
	{
		PROFILER_AUTO_BLOCK_DEFINE("set transform");
		shipOwner->setTransform_o2p(m_shipDynamicsModel->getTransform());
	}

#ifdef _DEBUG
	if ((m_attackBehavior != nullptr)
		&& (aiDebugString != nullptr))
	{
		PROFILER_AUTO_BLOCK_DEFINE("sendDebugAiToClients");
		sendDebugAiToClients(*aiDebugString);
		delete aiDebugString;
	}
#endif // _DEBUG

	//-- Chain back up to parent
	float alterResult = ShipController::realAlter(elapsedTime);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

void AiShipController::moveTo(Vector const & position_w, float const throttle, float const deltaTime)
{
	bool slowDown = false;

	// If we are not following a unit, see if we need to slow down for someone

	if ((m_nonAttackBehavior != nullptr)
		&& (m_nonAttackBehavior->getBehaviorType() != ASBT_follow)
		&& m_requestedSlowDown
		&& !isAttacking())
	{
		slowDown = true;
	}
	else
	{
		setThrottle(throttle);
	}

	// If we need to slow down for a following unit, then lets set the new throttle

	if (slowDown)
	{
		m_requestedSlowDown = false;
		static float throttleGain = 1.0f;
		setThrottle(clamp(s_slowdownThrottlePositionMin, getThrottle() - (getThrottle() * deltaTime * throttleGain), 1.0f));
	}

	// See if we need to avoid an obstacle

	Vector resultAvoidancePosition_w;
	Vector const & shipVelocity = getShipOwner()->getCurrentVelocity_p();

	if (SpaceAvoidanceManager::getAvoidancePosition(*getOwner(), shipVelocity, position_w, resultAvoidancePosition_w))
	{
		IGNORE_RETURN(face(resultAvoidancePosition_w, deltaTime));
	}
	else
	{
		// See if we need to avoid another squad member

		Vector const squadAvoidanceVector(getSquad().getAvoidanceVector(*getShipOwner()));

		if (squadAvoidanceVector != Vector::zero)
		{
			Vector const squadAvoidancePosition_w(getOwnerPosition_w() + squadAvoidanceVector);
			IGNORE_RETURN(face(squadAvoidancePosition_w, deltaTime));
		}
		else
		{
			IGNORE_RETURN(face(position_w, deltaTime));
		}
	}

	m_moveToGoalPosition_w = position_w;
}

// ----------------------------------------------------------------------

bool AiShipController::addDamageTaken(NetworkId const & attackingUnit, float const damage, bool const verifyAttacker)
{
	return ShipController::addDamageTaken(attackingUnit, damage, verifyAttacker);
}

// ----------------------------------------------------------------------

bool AiShipController::addDamageTaken(NetworkId const & attackingUnit, float const damage, bool const verifyAttacker, bool const notifySquad, bool const checkPlayerAttacker)
{
	bool const result = addDamageTaken(attackingUnit, damage, verifyAttacker);

	if (result)
	{
		// If this is a player attacking, take damage from all the group members

		if (checkPlayerAttacker)
		{
			ShipObject const * const attackingShipObject = ShipObject::asShipObject(NetworkIdManager::getObjectById(attackingUnit));

			if (attackingShipObject != nullptr)
			{
				CreatureObject const * const attackingPilotCreatureObject = attackingShipObject->getPilot();

				if ((attackingPilotCreatureObject != nullptr)
					&& attackingPilotCreatureObject->isPlayerControlled())
				{
					GroupObject * const groupObject = attackingPilotCreatureObject->getGroup();

					if (groupObject != nullptr)
					{
						GroupObject::GroupMemberVector const & groupMembers = groupObject->getGroupMembers();
						GroupObject::GroupMemberVector::const_iterator iterGroupMembers = groupMembers.begin();

						for (; iterGroupMembers != groupMembers.end(); ++iterGroupMembers)
						{
							GroupObject::GroupMember const & groupMember = (*iterGroupMembers);
							NetworkId const & groupMemberPilotNetworkId = groupMember.first;
							CreatureObject const * const groupMemberPilotCreatureObject = CreatureObject::asCreatureObject(NetworkIdManager::getObjectById(groupMemberPilotNetworkId));

							if (groupMemberPilotCreatureObject != nullptr)
							{
								ShipObject const * const groupMemberShipObject = groupMemberPilotCreatureObject->getPilotedShip();

								if (groupMemberShipObject != nullptr)
								{
									if (groupMemberShipObject != attackingShipObject)
									{
										bool const checkPlayerAttacker = false;

										IGNORE_RETURN(addDamageTaken(groupMemberShipObject->getNetworkId(), damage * s_playerGroupDamageTakenPercent, verifyAttacker, notifySquad, checkPlayerAttacker));
									}
								}
							}
							else
							{
								DEBUG_WARNING(true, ("debug_ai: AiShipController::addDamageTaken() owner(%s) Why does this group member (%s) not resolve to a CreatureObject?", getOwner()->getDebugInformation().c_str(), groupMemberPilotNetworkId.getValueString().c_str()));
							}
						}
					}
				}
			}
		}

		if (notifySquad)
		{
			// Let the squad know a member is taking damage

			SpaceSquad & squad = getSquad();

			squad.addDamageTaken(attackingUnit, damage * s_squadDamageTakenPercent);

			// Tell all the units guarding me that I am being attacked

			SpaceSquad::SpaceSquadList & guardedByList = squad.getGuardedByList();
			SpaceSquad::SpaceSquadList::iterator iterGuardedByList = guardedByList.begin();

			for (; iterGuardedByList != guardedByList.end(); ++iterGuardedByList)
			{
				SpaceSquad * const guardSquad = NON_NULL(*iterGuardedByList);

				guardSquad->setGuarding(true);
				guardSquad->addDamageTaken(attackingUnit, damage * s_guardSquadDamageTakenPercent);
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

void AiShipController::setAttackOrders(AiShipController::AttackOrders const attackOrders)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::setAttackOrders() owner(%s) attackOrders(%s)", getOwner()->getNetworkId().getValueString().c_str(), AiShipController::getAttackOrdersString(attackOrders)));

	m_attackOrders = attackOrders;

	getSquad().assignAttackSquad(*this);
}

// ----------------------------------------------------------------------

AiShipController::AttackOrders AiShipController::getAttackOrders() const
{
	return m_attackOrders;
}

// ----------------------------------------------------------------------

void AiShipController::setLeashRadius(float const radius)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::setLeashRadius() owner(%s) radius(%.2f)", getOwner()->getNetworkId().getValueString().c_str(), radius));

	NON_NULL(m_attackBehavior)->setLeashRadius(radius);
}

// ----------------------------------------------------------------------

void AiShipController::follow(NetworkId const & followedUnit, Vector const & direction_l, float const distance)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::follow() owner(%s) followedUnit(%s) direction_o(%.1f, %.1f, %.1f) offset(%.1f)", getOwner()->getNetworkId().getValueString().c_str(), followedUnit.getValueString().c_str(), direction_l.x, direction_l.y, direction_l.z, distance));

	SpacePathManager::release(m_path, getOwner());
	m_path = nullptr;

	float appearanceRadius = 0.0f;
	Object const * const followedObject = NetworkIdManager::getObjectById(followedUnit);

	if (followedObject != nullptr)
	{
		//-- This needs to be improved to cast a ray from this position back towards the ship and get the actual collision position

		CollisionProperty const * const ownerCollisionProperty = getOwner()->getCollisionProperty();
		CollisionProperty const * const followedObjectCollisionProperty = followedObject->getCollisionProperty();

		if (ownerCollisionProperty != nullptr)
		{
			appearanceRadius += ownerCollisionProperty->getBoundingSphere_l().getRadius();
		}

		if (followedObjectCollisionProperty != nullptr)
		{
			appearanceRadius += followedObjectCollisionProperty->getBoundingSphere_l().getRadius();
		}
	}

	setFormationPosition_l(direction_l * (distance + appearanceRadius));

	delete m_pendingNonAttackBehavior;
	m_pendingNonAttackBehavior = new AiShipBehaviorFollow(*this, followedUnit);

	triggerBehaviorChanged(ASBT_follow);
}

// ----------------------------------------------------------------------

int AiShipController::getBehaviorType() const
{
	AiShipBehaviorType result = ASBT_idle;

	if (m_nonAttackBehavior != nullptr)
	{
		result = m_nonAttackBehavior->getBehaviorType();
	}

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::getBehaviorType() owner(%s) behavior(%s)", getOwner()->getNetworkId().getValueString().c_str(), AiShipBehaviorBase::getBehaviorString(static_cast<AiShipBehaviorType>(result))));

	return static_cast<int>(result);
}

// ----------------------------------------------------------------------

void AiShipController::idle()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::idle() owner(%s)", getOwner()->getNetworkId().getValueString().c_str()));

	SpacePathManager::release(m_path, getOwner());
	m_path = nullptr;

	delete m_pendingNonAttackBehavior;
	m_pendingNonAttackBehavior = new AiShipBehaviorIdle(*this);;

	triggerBehaviorChanged(ASBT_idle);
}

// ----------------------------------------------------------------------

void AiShipController::track(Object const & target)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::track() owner(%s) target(%s)", getOwner()->getNetworkId().getValueString().c_str(), target.getNetworkId().getValueString().c_str()));

	SpacePathManager::release(m_path, getOwner());
	m_path = nullptr;

	delete m_pendingNonAttackBehavior;
	m_pendingNonAttackBehavior = new AiShipBehaviorTrack(*this, target);

	triggerBehaviorChanged(ASBT_track);
}

// ----------------------------------------------------------------------

void AiShipController::moveTo(SpacePath * const path)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::moveTo() owner(%s) path(0x%p)", getOwner()->getNetworkId().getValueString().c_str(), path));

	if (path != m_path)
	{
		SpacePathManager::release(m_path, getOwner());
		m_path = SpacePathManager::fetch(path, getOwner(), getShipRadius());
		setCurrentPathIndex(0);
	}

	delete m_pendingNonAttackBehavior;
	m_pendingNonAttackBehavior = new AiShipBehaviorWaypoint(*this, false);

	triggerBehaviorChanged(ASBT_moveTo);
}

// ----------------------------------------------------------------------

void AiShipController::addPatrolPath(SpacePath * const path)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::addPatrolPath() unit(%s) path(0x%p)", getOwner()->getNetworkId().getValueString().c_str(), path));

	if (path != m_path)
	{
		SpacePathManager::release(m_path, getOwner());
		m_path = SpacePathManager::fetch(path, getOwner(), getShipRadius());
	}

	// If we are already patrolling, don't change the behavior

	delete m_pendingNonAttackBehavior;
	m_pendingNonAttackBehavior = new AiShipBehaviorWaypoint(*this, true);

	triggerBehaviorChanged(ASBT_patrol);
}

// ----------------------------------------------------------------------

void AiShipController::clearPatrolPath()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::clearPatrolPath() owner(%s)", getOwner()->getNetworkId().getValueString().c_str()));

	if (m_path != nullptr)
	{
		m_path->clear();
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::clearPatrolPath() Unable to find path(0x%p)", m_path));
	}

	setCurrentPathIndex(0);
}

// ----------------------------------------------------------------------

void AiShipController::requestSlowDown()
{
	m_requestedSlowDown = true;
}

// ----------------------------------------------------------------------

void AiShipController::triggerBehaviorChanged(AiShipBehaviorType const newBehavior)
{
	// Only send the trigger if the new behavior is different from the old behavior

	AiShipBehaviorType const oldBehavior = (m_nonAttackBehavior != nullptr) ? m_nonAttackBehavior->getBehaviorType() : ASBT_idle;

	if (oldBehavior != newBehavior)
	{
		Object * const object = getOwner();
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
		GameScriptObject * const gameScriptObject = (serverObject != nullptr) ? serverObject->getScriptObject() : nullptr;

		if (gameScriptObject != nullptr)
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::triggerBehaviorChanged() unit(%s) old(%s) new(%s)", getOwner()->getNetworkId().getValueString().c_str(), AiShipBehaviorBase::getBehaviorString(oldBehavior), AiShipBehaviorBase::getBehaviorString(newBehavior)));

			ScriptParams scriptParams;
			scriptParams.addParam(static_cast<int>(newBehavior));
			scriptParams.addParam(static_cast<int>(oldBehavior));
			IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_UNIT_BEHAVIOR_CHANGED, scriptParams));
		}
		else
		{
			WARNING(true, ("Unable to get the ScriptObject for this object."));
		}
	}
}

// ----------------------------------------------------------------------

void AiShipController::triggerEnterCombat(NetworkId const & attackTarget)
{
	Object * const object = getOwner();
	ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
	GameScriptObject * const gameScriptObject = (serverObject != nullptr) ? serverObject->getScriptObject() : nullptr;

	if (gameScriptObject != nullptr)
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::triggerEnterCombat() unit(%s) attackTarget(%s)", getOwner()->getNetworkId().getValueString().c_str(), attackTarget.getValueString().c_str()));

		ScriptParams scriptParams;
		scriptParams.addParam(attackTarget);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SPACE_UNIT_ENTER_COMBAT, scriptParams));
	}
	else
	{
		WARNING(true, ("Unable to get the ScriptObject for this object."));
	}
}

// ----------------------------------------------------------------------

PersistentCrcString const & AiShipController::getShipName() const
{
	return m_shipName;
}

// ----------------------------------------------------------------------

void AiShipController::setPilotType(std::string const & pilotType)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::setPilotType() unit(%s) pilotType(%s)", getOwner()->getNetworkId().getValueString().c_str(), pilotType.c_str()));

	ShipObject * const shipObject = getShipOwner();

	if (pilotType.empty())
	{
		DEBUG_WARNING(true, ("AiShipController::setPilotType empty pilotType for ship [%s]",
			shipObject ? shipObject->getDebugInformation().c_str() : "NONE"));
		return;
	}

	m_pilotData = &AiShipPilotData::getPilotData(pilotType);
	AiPilotManager::getPilotData(*m_pilotData, *m_pilotManagerInfo);

	// Make sure the ship name is set
	if (shipObject != nullptr)
	{
		std::string shipName;

		if (shipObject->getObjVars().getItem("ship.shipName", shipName))
		{
			m_shipName.set(shipName.c_str(), false);
			ShipAiReactionManager::setShipSpaceMobileType(*shipObject, m_shipName);
		}
		else
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::setPilotType() Unable to find ship.shipName objvar for ai %s", getOwner()->getNetworkId().getValueString().c_str()));
		}
	}

	// Create the attack behavior based on the ship class

	delete m_attackBehavior;
	m_attackBehavior = nullptr;

	m_shipClass = ShipAiReactionManager::getShipClass(m_shipName);

	switch (m_shipClass)
	{
	case ShipAiReactionManager::SC_invalid:
	default: { FATAL(true, ("Invalid ship name(%s)", m_shipName.getString())); }
	case ShipAiReactionManager::SC_fighter: { m_attackBehavior = new AiShipBehaviorAttackFighter(*this); } break;
	case ShipAiReactionManager::SC_bomber: { m_attackBehavior = new AiShipBehaviorAttackBomber(*this); } break;
	case ShipAiReactionManager::SC_capitalShip: { m_attackBehavior = new AiShipBehaviorAttackCapitalShip(*this); } break;
	case ShipAiReactionManager::SC_transport: { m_attackBehavior = new AiShipBehaviorAttackFighter(*this); } break;
	}

	// Create the turreting target system, if appropriate

	if (shipObject && shipObject->hasTurrets())
		addTurretTargetingSystem(new AiShipTurretTargetingSystem(*this));

	// Set the default aggro distance

	setAggroRadius(m_pilotData->m_aggroRadius);

	FATAL((m_attackBehavior == nullptr), ("The attack behavior can not be nullptr."));
}

// ----------------------------------------------------------------------

CachedNetworkId const & AiShipController::getPrimaryAttackTarget() const
{
	if (isAttackSquadLeader())
	{
		return getAttackTargetList().getPrimaryTarget();
	}

	return getAttackSquad().getPrimaryAttackTarget();
}

// ----------------------------------------------------------------------

ShipObject const * AiShipController::getPrimaryAttackTargetShipObject() const
{
	Object const * const targetObject = getPrimaryAttackTarget().getObject();
	ServerObject const * const targetServerObject = (targetObject != nullptr) ? targetObject->asServerObject() : nullptr;
	ShipObject const * const targetShipObject = (targetServerObject != nullptr) ? targetServerObject->asShipObject() : nullptr;

	return targetShipObject;
}

// ----------------------------------------------------------------------

bool AiShipController::isAttacking() const
{
	bool result = false;

	if (hasFunctionalEngines())
	{
		bool const attackSquadLeader = isAttackSquadLeader();

		if (attackSquadLeader)
		{
			float const weaponAttackRange = getAttackSquad().getWeaponAttackRange();
			bool const hasWeapons = (weaponAttackRange > 0.0f);
			bool const shouldAttack = (getAttackOrders() != AiShipController::AO_holdFire);
			bool const hasTarget = !getAttackTargetList().isEmpty();
			bool const capitalShip = getShipOwner()->isCapitalShip();

			result = (hasWeapons && shouldAttack && hasTarget && s_attackingEnabled && !capitalShip);
		}
		else
		{
			result = getAttackSquad().isAttacking();
		}
	}

	return result;
}

// ----------------------------------------------------------------------

void AiShipController::setAttackingEnabled(bool const enabled)
{
	s_attackingEnabled = enabled;
}

// ----------------------------------------------------------------------

Vector AiShipController::getOwnerPosition_w() const
{
	return getOwner()->getPosition_w();
}

// ----------------------------------------------------------------------

Vector AiShipController::getOwnerObjectFrameK_w() const
{
	return getOwner()->getObjectFrameK_w();
}

// ----------------------------------------------------------------------

void AiShipController::setSquad(SpaceSquad * const squad)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::setSquad() unit(%s) squadId(%d) objTemplate(%s)", getOwner()->getNetworkId().getValueString().c_str(), NON_NULL(squad)->getId(), getOwner()->getObjectTemplateName()));

	// Make sure we are not putting the unit in it's current squad

	if (m_squad != NON_NULL(squad))
	{
		// Remove the unit from its previous squad

		if ((m_squad != nullptr)
			&& !m_squad->isEmpty())
		{
			m_squad->removeUnit(getOwner()->getNetworkId());
		}

		// Set the new squad

		m_squad = squad;
	}
}

// ----------------------------------------------------------------------

SpaceSquad & AiShipController::getSquad()
{
	return *NON_NULL(m_squad);
}

// ----------------------------------------------------------------------

SpaceSquad const & AiShipController::getSquad() const
{
	return *NON_NULL(m_squad);
}

// ----------------------------------------------------------------------

void AiShipController::setFormationPosition_l(Vector const & position_l)
{
	m_formationPosition_l = position_l;
}

// ----------------------------------------------------------------------

Vector const & AiShipController::getFormationPosition_l() const
{
	return m_formationPosition_l;
}

// ----------------------------------------------------------------------

void AiShipController::setAttackFormationPosition_l(Vector const & position_l)
{
	m_attackFormationPosition_l = position_l;
}

// ----------------------------------------------------------------------

Vector const & AiShipController::getAttackFormationPosition_l() const
{
	return m_attackFormationPosition_l;
}

// ----------------------------------------------------------------------

bool AiShipController::isSquadLeader() const
{
	return (getSquad().getLeader() == getOwner()->getNetworkId());
}

// ----------------------------------------------------------------------

void AiShipController::setAttackSquad(SpaceAttackSquad * const attackSquad)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::setAttackSquad() unit(%s) attackSquad(%d)", getOwner()->getNetworkId().getValueString().c_str(), NON_NULL(attackSquad)->getId()));

	// Make sure we are not putting the unit in it's current attack squad

	if (m_attackSquad != NON_NULL(attackSquad))
	{
		// Remove the unit from its pervious attack squad

		if (m_attackSquad != nullptr)
		{
			m_attackSquad->removeUnit(getOwner()->getNetworkId());
		}

		// Set the new attack squad

		m_attackSquad = attackSquad;
	}
}

// ----------------------------------------------------------------------

SpaceAttackSquad & AiShipController::getAttackSquad()
{
	return *NON_NULL(m_attackSquad);
}

// ----------------------------------------------------------------------

SpaceAttackSquad const & AiShipController::getAttackSquad() const
{
	return *NON_NULL(m_attackSquad);
}

// ----------------------------------------------------------------------

bool AiShipController::isAttackSquadLeader() const
{
	return (getAttackSquad().getLeader() == getOwner()->getNetworkId());
}

// ----------------------------------------------------------------------

float AiShipController::getShipRadius() const
{
	float result = 1.0f;
	ShipObject const * const shipObject = getShipOwner();
	CollisionProperty const * const shipCollision = shipObject->getCollisionProperty();

	if (shipCollision != nullptr)
	{
		result = shipCollision->getBoundingSphere_l().getRadius();
	}

	return result;
}

// ----------------------------------------------------------------------

SpacePath * AiShipController::getPath()
{
	return m_path;
}

// ----------------------------------------------------------------------

bool AiShipController::shouldCheckForEnemies() const
{
	bool result = false;

	if (s_attackingEnabled)
	{
		bool const capitalShip = getShipOwner()->isCapitalShip();

		if (capitalShip)
		{
			bool const turrets = getShipOwner()->hasTurrets();

			if (turrets)
			{
				result = true;
			}
		}
		else
		{
			bool const wantsToAttack = (getAttackOrders() != AiShipController::AO_holdFire);

			if (wantsToAttack)
			{
				bool const attackSquadLeader = isAttackSquadLeader();

				if (attackSquadLeader)
				{
					bool const attacking = isAttacking();

					if (!attacking)
					{
						bool const functionalEngines = hasFunctionalEngines();
						float const weaponAttackRange = getAttackSquad().getWeaponAttackRange();
						bool const hasWeapons = (weaponAttackRange > 0.0f);

						if (functionalEngines && hasWeapons)
						{
							result = true;
						}
						else
						{
							bool const turrets = getShipOwner()->hasTurrets();

							if (!functionalEngines && turrets)
							{
								result = true;
							}
						}
					}
				}
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

void AiShipController::setCurrentPathIndex(unsigned int const index)
{
	//LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::setCurrentPathIndex() unit(%s) pathIndex(%u) pathSize(%u)", getOwner()->getNetworkId().getValueString().c_str(), index, (m_path != nullptr) ? m_path->getTransformList().size() : 0));

	if ((m_path != nullptr)
		&& !m_path->isEmpty())
	{
		m_currentPathIndex = index;
	}
	else
	{
		m_currentPathIndex = 0;
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled() && (m_currentPathIndex > 0), "debug_ai", ("AiShipController::setCurrentPathIndex() Setting a non-zero path index(%d) on a nullptr or empty path", index));
	}
}

// ----------------------------------------------------------------------

unsigned int AiShipController::getCurrentPathIndex() const
{
	return m_currentPathIndex;
}

// ----------------------------------------------------------------------

float AiShipController::calculateThrottleToPosition_w(Vector const & position_w, float const slowDownDistance) const
{
	float result = 0.0f;

	Object const * const object = getOwner();

	if (object != nullptr)
	{
		float const distanceToGoalSquared = object->getPosition_w().magnitudeBetweenSquared(position_w);

		if ((distanceToGoalSquared < sqr(slowDownDistance))
			&& (slowDownDistance > 0.0f))
		{
			static float scale = 0.5f;
			float const distanceToGoal = sqrt(distanceToGoalSquared);
			result = clamp(0.0f, (distanceToGoal / slowDownDistance) * scale, 1.0f);

			if ((result < 0.5f)
				&& (distanceToGoal < getGoalStopDistance()))
			{
				result = 0.0f;
			}
		}
		else
		{
			result = 1.0f;
		}
	}

	return result;
}

// ----------------------------------------------------------------------

float AiShipController::getGoalStopDistance()
{
	return s_goalStopDistance;
}

// ----------------------------------------------------------------------

void AiShipController::dock(ShipObject & dockTarget, float const secondsAtDock)
{
	ShipController::dock(dockTarget, secondsAtDock);

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::dock() owner(%s) dockTarget(%s) secondsAtDock(%.2f)", getOwner()->getNetworkId().getValueString().c_str(), dockTarget.getNetworkId().getValueString().c_str(), secondsAtDock));

	// If we are the squad leader, assign a new unit as the squad leader so we can go dock without everyone in the squad following

	if (isSquadLeader())
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::dock() owner(%s) Requesting a new squad leader.", getOwner()->getNetworkId().getValueString().c_str()));

		getSquad().assignNewLeader();
	}
}

// ----------------------------------------------------------------------

std::string const & AiShipController::getPilotType() const
{
	return NON_NULL(m_pilotData)->m_name;
}

// ----------------------------------------------------------------------

float AiShipController::getPilotSkill() const
{
	return NON_NULL(m_pilotData)->m_skill;
}

// ----------------------------------------------------------------------

float AiShipController::getPilotAggression() const
{
	return NON_NULL(m_pilotData)->m_aggression;
}

// ----------------------------------------------------------------------

/**
 * Switch the current attack behavior to a fighter behavior.  Called if
 * a bomber has to attack fighters, for example.
 */
void AiShipController::switchToFighterAttack()
{
	DEBUG_FATAL(dynamic_cast<AiShipBehaviorAttackFighter*>(m_attackBehavior), ("Programmer bug:  called switchToFighterAttack() when already using a fighter attack"));
	delete m_pendingAttackBehavior; // in case we tried to switch twice
	m_pendingAttackBehavior = new AiShipBehaviorAttackFighter(*NON_NULL(m_attackBehavior));
}

// ----------------------------------------------------------------------

/**
 * Switch the current attack behavior to a fighter behavior.  Called if
 * a fighter has to make bombing runs on a capital ship, for example.
 */
void AiShipController::switchToBomberAttack()
{
	DEBUG_FATAL(dynamic_cast<AiShipBehaviorAttackBomber*>(m_attackBehavior), ("Programmer bug:  called switchToBomberAttack() when already using a bomber attack"));
	delete m_pendingAttackBehavior; // in case we tried to switch twice
	m_pendingAttackBehavior = new AiShipBehaviorAttackBomber(*NON_NULL(m_attackBehavior));
}

// ----------------------------------------------------------------------

bool AiShipController::removeAttackTarget(NetworkId const & unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::removeAttackTarget() owner(%s) unit(%s)", getOwner()->getNetworkId().getValueString().c_str(), unit.getValueString().c_str()));
	DEBUG_WARNING((unit == NetworkId::cms_invalid), ("debug_ai: owner(%s) ERROR: Trying to remove a nullptr unit(%s) from the attack target list.", getOwner()->getNetworkId().getValueString().c_str(), unit.getValueString().c_str()));

	return ShipController::removeAttackTarget(unit);
}

// ----------------------------------------------------------------------

/**
 * Start launch countermeasures if appropriate.
 *
 * If this becomes any more complicated, we should probably make a
 * CountermeasureSystem that works like TurretTargetingSystem, to avoid
 * putting too much "behavior" logic into AiShipController.
 */
void AiShipController::onTargetedByMissile(int const /*missileId*/)
{
	// if not already launching countermeasures, start reacting after a little time has passed
	if (m_countermeasureState == CS_none)
	{
		AiShipPilotData const * const pilotData = NON_NULL(getPilotData());
		m_reactToMissileTimer->setExpireTime(Random::randomReal(0.0f, pilotData->m_countermeasureReactionTime) + Random::randomReal(0.0f, pilotData->m_countermeasureReactionTime)); // react after a random interval that averages to m_countermeasureReactionTime.  Use two die rolls to give more of a bell-shaped distribution
		m_reactToMissileTimer->reset();
		m_countermeasureState = CS_reacting;
	}
}

// ----------------------------------------------------------------------

float AiShipController::getAggroRadius() const
{
	return m_aggroRadius;
}

// ----------------------------------------------------------------------

void AiShipController::setAggroRadius(float const aggroRadius)
{
	TerrainObject const * const terrain = TerrainObject::getInstance();
	DEBUG_FATAL(!terrain, ("Unable to get a terrain object"));

	float const mapSideLength = terrain->getMapWidthInMeters();
	IGNORE_RETURN(WithinRangeInclusiveInclusive(0.00001f, aggroRadius, mapSideLength));

	m_aggroRadius = aggroRadius;
}

// ----------------------------------------------------------------------

AiShipController * AiShipController::asAiShipController()
{
	return this;
}

// ----------------------------------------------------------------------

AiShipController const * AiShipController::asAiShipController() const
{
	return this;
}

// ----------------------------------------------------------------------

Vector const & AiShipController::getMoveToGoalPosition_w() const
{
	return m_moveToGoalPosition_w;
}

// ----------------------------------------------------------------------

void AiShipController::onAttackTargetListEmpty()
{
	// If my whole squad has stopped fighting, then we can mark all
	// squads guarding me as not needing to gaurd my squad anymore.

	SpaceSquad::SpaceSquadList const & guardedByList = getSquad().getGuardedByList();

	if (!guardedByList.empty()
		&& getSquad().isAttackTargetListEmpty())
	{
		SpaceSquad::SpaceSquadList::const_iterator iterGuardedByList = guardedByList.begin();

		for (; iterGuardedByList != guardedByList.end(); ++iterGuardedByList)
		{
			SpaceSquad * const spaceSquad = NON_NULL(*iterGuardedByList);

			spaceSquad->setGuarding(false);
		}
	}
}

#ifdef _DEBUG
// ----------------------------------------------------------------------

void AiShipController::sendDebugAiToClients(AiDebugString & aiDebugString)
{
	NOT_NULL(m_attackBehavior);
	ShipObject * const shipOwner = getShipOwner();
	const ObserverList & observerList = shipOwner->getObservers();

	if (!observerList.empty())
	{
		FormattedString<512> formattedString;

		//if (!isAttackSquadLeader())
		//{
		//	if (getSquad().hasTarget())
		//	{
		//		// Attack formation position
		//
		//		aiDebugString.addLineToPosition(Formation::getPosition_w(getAttackSquad().getLeader().getObject()->getTransform_o2w(), getAttackFormationPosition_l()), PackedRgb::solidMagenta);
		//	}
		//	else
		//	{
		//		// Squad formation position
		//
		//		aiDebugString.addLineToPosition(Formation::getPosition_w(getSquad().getLeader().getObject()->getTransform_o2w(), getFormationPosition_l()), PackedRgb::solidMagenta);
		//	}
		//}

		//{
		//	// Guarding info
		//
		//	std::string guardString("gb: ");
		//	SpaceSquad::SpaceSquadList::const_iterator iterGuardedByList = getSquad().getGuardedByList()->begin();
		//
		//	for (; iterGuardedByList != getSquad().getGuardedByList()->end(); ++iterGuardedByList)
		//	{
		//		guardString += formattedString.sprintf("%d, ", (*iterGuardedByList)->getId());
		//	}
		//
		//	if (getSquad().getGuardTarget())
		//	{
		//		guardString += formattedString.sprintf("gt: %d\n", getSquad().getGuardTarget()->getId());
		//	}
		//	else
		//	{
		//		guardString += "gt: 0\n";
		//	}
		//
		//	aiDebugString.addText(guardString, PackedRgb::solidYellow);
		//}

		// Are my engines functional?

		if (!shipOwner->isComponentFunctional(ShipChassisSlotType::SCST_engine))
		{
			aiDebugString.addText("NON-FUNCTIONAL ENGINE\n", PackedRgb::solidMagenta);
		}
		else if (shipOwner->getShipActualSpeedMaximum() <= 0.0f)
		{
			aiDebugString.addText("ZERO MAX SPEED ENGINE\n", PackedRgb::solidMagenta);
		}

		// Am I currently in the process of guarding another squad

		if (getSquad().isGuarding())
		{
			aiDebugString.addText("GUARDING\n", PackedRgb::solidWhite);
		}

		// Am I auto aggro immune?

		if (getShipOwner()->isAutoAggroImmune())
		{
			aiDebugString.addText("AUTO AGGRO IMMUNE\n", PackedRgb::solidWhite);
		}

		// Am I damage aggro immune?

		if (getShipOwner()->isDamageAggroImmune())
		{
			aiDebugString.addText("DAMAGE AGGRO IMMUNE\n", PackedRgb::solidWhite);
		}

		// Am I being docked?

		if (isBeingDocked())
		{
			aiDebugString.addText("BEING DOCKED\n", PackedRgb::solidCyan);
		}

		// Am I the squad leader?
		{
			if (isSquadLeader())
			{
				aiDebugString.addText("Squad Leader\n", PackedRgb::solidYellow);
			}
		}

		// Am I the attack squad leader?
		{
			if (isAttackSquadLeader())
			{
				aiDebugString.addText("Attack Squad Leader\n", PackedRgb::solidRed);
			}
		}

		// Tier, ship name, and ship class
		{
			aiDebugString.addText(formattedString.sprintf("%s %s (%s)\n", getPilotType().c_str(), m_shipName.getString(), ShipAiReactionManager::getShipClassString(m_shipClass)), PackedRgb::solidWhite);
		}

		// Squad and Attack squad
		{
			aiDebugString.addText(formattedString.sprintf("Squad(%d:%d) Attack Squad(%d:%d)\n", getSquad().getId(), getSquad().getUnitCount(), getAttackSquad().getId(), getAttackSquad().getUnitCount()));
		}

		// Attack orders, aggro distance and leash distance
		{
			bool const includeMissiles = true;
			float const attackRange = shipOwner->getApproximateAttackRange(includeMissiles);

			if (attackRange > 0.0f)
			{
				bool const checkForEnemies = shouldCheckForEnemies();
				float const leashRadius = m_attackBehavior->getLeashRadius();

				aiDebugString.addText(formattedString.sprintf("%s agg(%.0f) lsh(%.0f) %s\n", AiShipController::getAttackOrdersString(getAttackOrders()), m_aggroRadius, leashRadius, (m_attackBehavior == nullptr) ? " [NO ATTACK BEHAVIOR]" : (checkForEnemies ? " [LFE]" : "")));
			}
		}

		// Turrets
		{
			if (shipOwner->hasTurrets())
			{
				float const turretMissChance = getTurretMissChance() * 100.0f;
				float const turretMissAngleDegrees = convertRadiansToDegrees(getTurretMissAngle());

				aiDebugString.addText(formattedString.sprintf("turret miss(%.0f%% @ %.0f)\n", turretMissChance, turretMissAngleDegrees));
			}
		}

		//int ammoCurrent = 0;
		//int ammoMax = 0;
		//
		//for (int chassisSlot = ShipChassisSlotType::SCST_weapon_first; chassisSlot < ShipChassisSlotType::SCST_weapon_last; ++chassisSlot)
		//{
		//	int const weaponIndex = ShipChassisSlotType::getWeaponIndex(chassisSlot);
		//
		//	if (parentShipObject->isMissile(weaponIndex))
		//	{
		//		ammoCurrent += parentShipObject->getWeaponAmmoCurrent(static_cast<int>(chassisSlot));
		//		ammoMax += parentShipObject->getWeaponAmmoMaximum(static_cast<int>(chassisSlot));
		//	}
		//}
		//
		//weaponString += FormattedString<256>().sprintf("Missiles(%d/%d)", ammoCurrent, ammoMax);

		//aiDebugString.addText(formattedString.sprintf("\nlpos(%.0f,%.0f,%.0f) gpos(%.0f,%.0f,%.0f)", m_leashAnchorPosition_w.x, m_leashAnchorPosition_w.y, m_leashAnchorPosition_w.z, m_moveToGoalPosition_w.x, m_moveToGoalPosition_w.y, m_moveToGoalPosition_w.z));

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
				if ((characterObject != nullptr)
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

// ----------------------------------------------------------------------

bool AiShipController::debug_forceFighterAttackManeuver(int const maneuverType)
{
	bool success = false;

	AiShipBehaviorAttackFighter * const aiFighterBehavior = dynamic_cast<AiShipBehaviorAttackFighter *>(m_attackBehavior);
	if (aiFighterBehavior)
	{
		aiFighterBehavior->debug_forceManeuver(maneuverType);
		success = true;
	}

	return success;
}

#endif // _DEBUG

// ----------------------------------------------------------------------
void AiShipController::clampPositionToZone(Vector & position_w) const
{
	TerrainObject const * const terrain = NON_NULL(TerrainObject::getInstance());
	float const mapSideHalfWidth = terrain->getMapWidthInMeters() * 0.50f;
	float const turnRadius = getLargestTurnRadius();
	float const min = -(mapSideHalfWidth - (turnRadius * 2.0f));
	float const max = +(mapSideHalfWidth - (turnRadius * 2.0f));

	position_w.x = clamp(min, position_w.x, max);
	position_w.y = clamp(min, position_w.y, max);
	position_w.z = clamp(min, position_w.z, max);
}

// ----------------------------------------------------------------------

char const * AiShipController::getAttackOrdersString(AttackOrders const attackOrders)
{
	switch (attackOrders)
	{
	case AO_holdFire: { return "HOLD FIRE"; }
	case AO_returnFire: { return "RETURN FIRE"; }
	case AO_attackFreely: { return "ATTACK FREELY"; }
	case AO_count:
	default: break;
	}

	return "INVALID ATTACK ORDER";
}

// ----------------------------------------------------------------------

bool AiShipController::hasFunctionalEngines() const
{
	return getShipOwner()->isComponentFunctional(ShipChassisSlotType::SCST_engine);
}

// ----------------------------------------------------------------------

void AiShipController::addExclusiveAggro(NetworkId const & unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::addExclusiveAggro() owner(%s) unit(%s) m_exclusiveAggroSet->size(%u+1)", getOwner()->getNetworkId().getValueString().c_str(), unit.getValueString().c_str(), m_exclusiveAggroSet->size()));
#ifdef _DEBUG
	{
		// Make sure this is a player

		Object * const unitObject = NetworkIdManager::getObjectById(unit);
		ServerObject * const unitServerObject = (unitObject != nullptr) ? unitObject->asServerObject() : nullptr;
		CreatureObject * const unitCreatureObject = (unitServerObject != nullptr) ? unitServerObject->asCreatureObject() : nullptr;

		if (!unitCreatureObject
			|| !unitCreatureObject->isPlayerControlled())
		{
			DEBUG_WARNING(true, ("debug_ai: AiShipController::addExclusiveAggro() owner(%s) ERROR: The unit(%s) must be a player.", getOwner()->getDebugInformation().c_str(), unit.getValueString().c_str()));
			return;
		}
	}

#endif // _DEBUG
	bool verifyTargetList = false;

	if (m_exclusiveAggroSet->empty())
	{
		verifyTargetList = true;
	}

	IGNORE_RETURN(m_exclusiveAggroSet->insert(CachedNetworkId(unit)));

	if (verifyTargetList)
	{
		// Make sure only exclusive aggros are in the target list

		m_attackTargetList->verify();
	}
}

// ----------------------------------------------------------------------

void AiShipController::removeExclusiveAggro(NetworkId const & unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiShipController::removeExclusiveAggro() owner(%s) unit(%s) m_exclusiveAggroSet->size(%u-1)", getOwner()->getNetworkId().getValueString().c_str(), unit.getValueString().c_str(), m_exclusiveAggroSet->size()));

	IGNORE_RETURN(m_exclusiveAggroSet->erase(CachedNetworkId(unit)));
}

// ----------------------------------------------------------------------

bool AiShipController::isExclusiveAggro(CreatureObject const & pilot) const
{
	bool result = false;
	CachedNetworkIdSet::const_iterator iterExclusiveAggroSet = m_exclusiveAggroSet->begin();

	for (; iterExclusiveAggroSet != m_exclusiveAggroSet->end(); ++iterExclusiveAggroSet)
	{
		CachedNetworkId const & aggroCachedNetworkId = (*iterExclusiveAggroSet);

		if (pilot.getNetworkId() == aggroCachedNetworkId)
		{
			// Found a matching player

			result = true;
			break;
		}

		// Check the group members of this player

		CreatureObject const * const aggroCreatureObject = CreatureObject::asCreatureObject(aggroCachedNetworkId.getObject());

		if (aggroCreatureObject != nullptr)
		{
			GroupObject * const groupObject = aggroCreatureObject->getGroup();

			if (groupObject != nullptr)
			{
				GroupObject::GroupMemberVector const & groupMembers = groupObject->getGroupMembers();
				GroupObject::GroupMemberVector::const_iterator iterGroupMembers = groupMembers.begin();

				for (; iterGroupMembers != groupMembers.end(); ++iterGroupMembers)
				{
					GroupObject::GroupMember const & groupMember = (*iterGroupMembers);
					NetworkId const & memberNetworkId = groupMember.first;

					if (pilot.getNetworkId() == memberNetworkId)
					{
						// Found a matching group member

						result = true;
						break;
					}
				}
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipController::hasExclusiveAggros() const
{
	return !m_exclusiveAggroSet->empty();
}

// ----------------------------------------------------------------------
bool AiShipController::isValidTarget(ShipObject const & unit) const
{
	bool result = false;

	CreatureObject const * const pilotCreatureObject = unit.getPilot();

	if (pilotCreatureObject != nullptr)
	{
		if (pilotCreatureObject->isPlayerControlled())
		{
			bool const exclusiveAggros = hasExclusiveAggros();

			if (exclusiveAggros)
			{
				bool const validExclusiveAggro = isExclusiveAggro(*pilotCreatureObject);

				if (validExclusiveAggro)
				{
					result = true;
				}
			}
			else
			{
				result = true;
			}
		}
		else
		{
			result = true;
		}
	}
	else
	{
		result = true;
	}

	return result;
}

// ----------------------------------------------------------------------
ShipAiReactionManager::ShipClass AiShipController::getShipClass() const
{
	return m_shipClass;
}

// ----------------------------------------------------------------------
float AiShipController::getTurretMissChance() const
{
	return m_pilotData->m_turretMissChance;
}

// ----------------------------------------------------------------------
float AiShipController::getTurretMissAngle() const
{
	return m_pilotData->m_turretMissAngle;
}

// ======================================================================