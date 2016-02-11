// ======================================================================
//
// ShipController.cpp
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipController.h"

#include "serverGame/AiServerShipObjectInterface.h"
#include "serverGame/AiShipAttackTargetList.h"
#include "serverGame/AiShipBehaviorDock.h"
#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerShipTurretTargetingSystem.h"
#include "serverGame/ServerShipObjectInterface.h"
#include "serverGame/ShipObject.h"
#include "serverGame/ShipTurretTargetingSystem.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedLog/Log.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedObject/NetworkIdManager.h"

#include <set>

// ======================================================================
//
// ShipControllerNamespace
//
// ======================================================================

namespace ShipControllerNamespace
{
	typedef std::vector<CachedNetworkId> PurgeList;
	static PurgeList s_purgeList;
	time_t const s_maxTargetAge = 60 * 2;

	// If we are less than 5 degrees from the target orientation, don't roll.
	// This prevents "wobbling."
	float const s_fastOrientationDeltaRadians = convertDegreesToRadians(5.0f);
}

using namespace ShipControllerNamespace;

// ======================================================================
// PUBLIC ShipController
// ======================================================================

ShipController::ShipController(ShipObject * newOwner) :
	TangibleController(newOwner),
	m_shipDynamicsModel(new ShipDynamicsModel),
	m_yawPosition(0.0f),
	m_pitchPosition(0.0f),
	m_rollPosition(0.0f),
	m_throttlePosition(0.0f),
	m_pendingDockingBehavior(nullptr),
	m_dockingBehavior(nullptr),
	m_attackTargetList(new AiShipAttackTargetList(newOwner)),
	m_attackTargetDecayTimer(new Timer(static_cast<float>(s_maxTargetAge))),
	m_enemyCheckQueued(false),
	m_turretTargetingSystem(nullptr),
	m_dockedByList(new DockedByList),
	m_aiTargetingMeList(new CachedNetworkIdList)
{
	m_attackTargetDecayTimer->setElapsedTime(Random::randomReal() * m_attackTargetDecayTimer->getExpireTime());
}

// ----------------------------------------------------------------------

ShipController::~ShipController()
{
	// The turret targeting system must be deleted before clearAiTargetingMeList(), because
	// otherwise it will try to acquire new targets as the list is being cleared
	delete m_turretTargetingSystem;
	m_turretTargetingSystem = nullptr;

	clearAiTargetingMeList();

	delete m_shipDynamicsModel;
	delete m_dockedByList;
	delete m_aiTargetingMeList;
	delete m_pendingDockingBehavior;
	m_pendingDockingBehavior = nullptr;
	delete m_dockingBehavior;
	m_dockingBehavior = nullptr;
	delete m_attackTargetList;
	delete m_attackTargetDecayTimer;
}

// ----------------------------------------------------------------------

void ShipController::endBaselines()
{
	Object const * const owner = getOwner();
	if (owner)
	{
		m_shipDynamicsModel->setTransform(owner->getTransform_o2p());
		m_shipDynamicsModel->makeStationary();
	}

	TangibleController::endBaselines();
}

// ----------------------------------------------------------------------

void ShipController::teleport(Transform const &goal, ServerObject *goalObj)
{
	if (!goalObj)
		m_shipDynamicsModel->setTransform(goal);
	TangibleController::teleport(goal, goalObj);
}

// ----------------------------------------------------------------------

void ShipController::onAddedToWorld()
{
	Object const * const owner = getOwner();
	if (owner)
	{
		m_shipDynamicsModel->setTransform(owner->getTransform_o2p());
		m_shipDynamicsModel->makeStationary();
	}
}

// ----------------------------------------------------------------------

Transform const &ShipController::getTransform() const
{
	return m_shipDynamicsModel->getTransform();
}

// ----------------------------------------------------------------------

Vector const &ShipController::getVelocity() const
{
	return m_shipDynamicsModel->getVelocity();
}

// ----------------------------------------------------------------------

float ShipController::getSpeed() const
{
	return m_shipDynamicsModel->getSpeed();
}

// ----------------------------------------------------------------------

float ShipController::getYawRate() const
{
	return m_shipDynamicsModel->getYawRate();
}

// ----------------------------------------------------------------------

float ShipController::getPitchRate() const
{
	return m_shipDynamicsModel->getPitchRate();
}

// ----------------------------------------------------------------------

float ShipController::getRollRate() const
{
	return m_shipDynamicsModel->getRollRate();
}

// ----------------------------------------------------------------------

void ShipController::respondToCollision(Vector const & deltaToMove_p, Vector const & newReflection_p, Vector const & /* normalOfSurface_p */)
{
	// this is identical to the clients ShipController::respondToCollision.
	// the difference it that the server also must call the virtual method
	// ShipController::experiencedCollision at the end of execution
	Transform transform_p(m_shipDynamicsModel->getTransform());
	transform_p.move_p(deltaToMove_p);

	m_shipDynamicsModel->setTransform(transform_p);
	m_shipDynamicsModel->setVelocity(newReflection_p * m_shipDynamicsModel->getVelocity().magnitude());

	ShipObject * const owner = getShipOwner();
	NOT_NULL(owner);
	owner->setTransform_o2p(transform_p);

	experiencedCollision();
}

// ----------------------------------------------------------------------

float ShipController::realAlter(float elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("ShipController::realAlter");
	ShipObject const * const owner = getShipOwner();

	if (owner->isInWorld())
	{
		if (m_turretTargetingSystem)
		{
			m_turretTargetingSystem->alter(elapsedTime);
		}

		// Decay the attack targets
		if (m_attackTargetDecayTimer->updateZero(elapsedTime))
		{
			PROFILER_AUTO_BLOCK_DEFINE("attackTargetList");
			m_attackTargetList->purge(s_maxTargetAge);

			if (m_attackTargetList->isEmpty())
			{
				onAttackTargetListEmpty();
			}
		}
	}

	return TangibleController::realAlter(elapsedTime);
}

// ----------------------------------------------------------------------

void ShipController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	UNREF(flags);
	UNREF(value);
	ShipObject * const owner = getShipOwner();
	DEBUG_FATAL(!owner, ("Owner is nullptr in ShipController::handleMessage\n"));
	switch(message)
	{
	case CM_clientLookAtTarget:
		{
			const MessageQueueNetworkId * const msg = NON_NULL (dynamic_cast<const MessageQueueNetworkId *>(data));
			owner->setPilotLookAtTarget (msg->getNetworkId ());
		}
		break;

	case CM_clientLookAtTargetComponent:
		{
			const MessageQueueGenericValueType<int> * const msg = NON_NULL (dynamic_cast<const MessageQueueGenericValueType<int> *>(data));
			owner->setPilotLookAtTargetSlot (static_cast<ShipChassisSlotType::Type>(msg->getValue()));
		}
		break;

	default:
		ServerController::handleMessage(message, value, data, flags);
		break;
	}
}


// ----------------------------------------------------------------------

void ShipController::addTurretTargetingSystem(ShipTurretTargetingSystem * newSystem)
{
	DEBUG_FATAL(&newSystem->getShipController() != this,("Programmer bug:  called addTurretTargetingSystem with a ShipTurretTargetingSystem whose owner did not match the controller it tried to attach to."));
	removeTurretTargetingSystem();

	m_turretTargetingSystem = newSystem;
}

// ----------------------------------------------------------------------

void ShipController::removeTurretTargetingSystem()
{
	delete m_turretTargetingSystem;
	m_turretTargetingSystem = nullptr;
}

// ----------------------------------------------------------------------

bool ShipController::isBeingDocked() const
{
	return !m_dockedByList->empty();
}

// ----------------------------------------------------------------------

void ShipController::addDockedBy(Object const & unit)
{
	IGNORE_RETURN(m_dockedByList->insert(CachedNetworkId(unit)));

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipController::addDockedBy() owner(%s) dockedBy(%s) dockedByListSize(%u)", (getOwner() != nullptr) ? getOwner()->getNetworkId().getValueString().c_str() : "nullptr", unit.getNetworkId().getValueString().c_str(), m_dockedByList->size()));
}

// ----------------------------------------------------------------------

void ShipController::removeDockedBy(Object const & unit)
{
	DockedByList::iterator iterDockedByList = m_dockedByList->find(CachedNetworkId(unit));

	if (iterDockedByList != m_dockedByList->end())
	{
		m_dockedByList->erase(iterDockedByList);

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipController::removeDockedBy() owner(%s) dockedBy(%s) dockedByListSize(%u)", (getOwner() != nullptr) ? getOwner()->getNetworkId().getValueString().c_str() : "nullptr", unit.getNetworkId().getValueString().c_str(), m_dockedByList->size()));
	}
	else
	{
		DEBUG_WARNING(true, ("debug_ai: ERROR: Trying to remove a docking unit(%s) that is not actually in the process of docking.", unit.getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------

ShipController::DockedByList const & ShipController::getDockedByList() const
{
	return *m_dockedByList;
}

// ----------------------------------------------------------------------

void ShipController::addAiTargetingMe(NetworkId const & unit)
{
	//LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipController::addAiTargetingMe() owner(%s) unit(%s) m_aiTargetingMeList->size(%u+1)", getOwner()->getNetworkId().getValueString().c_str(), unit.getValueString().c_str(), m_aiTargetingMeList->size()));
	DEBUG_FATAL((NetworkIdManager::getObjectById(unit) == nullptr), ("ShipController::addAiTargetingMe() owner(%s) unit(%s) ERROR: The Ai who is targeting you is not a valid networkId", getOwner()->getNetworkId().getValueString().c_str(), unit.getValueString().c_str()));

	// Make sure this is a valid networkId of an alive object

	IGNORE_RETURN(m_aiTargetingMeList->insert(CachedNetworkId(unit)));
}

// ----------------------------------------------------------------------

void ShipController::removeAiTargetingMe(NetworkId const & unit)
{
	//LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipController::removeAiTargetingMe() owner(%s) unit(%s) m_aiTargetingMeList->size(%u-1)", getOwner()->getNetworkId().getValueString().c_str(), unit.getValueString().c_str(), m_aiTargetingMeList->size()));

	CachedNetworkIdList::iterator iterTargetByList = m_aiTargetingMeList->find(CachedNetworkId(unit));

	if (iterTargetByList != m_aiTargetingMeList->end())
	{
		m_aiTargetingMeList->erase(iterTargetByList);
	}
	else
	{
		// If you hit this fatal, then there is an removeAiTargetingMe() called without a matching addAiTargetingMe().

		DEBUG_FATAL(true, ("Unable to find unit."));
	}

	// If no one else is targeting me (player's only), stop combat music

	if (m_aiTargetingMeList->empty())
	{
		ShipObject * const shipOwner = getShipOwner();

		if (shipOwner != nullptr)
		{
			CreatureObject * const pilotOwner = CreatureObject::asCreatureObject(shipOwner->getPilot());

			if (   (pilotOwner != nullptr)
			    && pilotOwner->isPlayerControlled())
			{
				shipOwner->clearCondition(static_cast<int>(TangibleObject::C_spaceCombatMusic));

				//LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipController::removeAiTargetingMe() owner(%s) Clearing TangibleObject::C_spaceCombatMusic", getOwner()->getNetworkId().getValueString().c_str()));
			}
		}
	}
}

// ----------------------------------------------------------------------

ShipController::CachedNetworkIdList const & ShipController::getAiTargetingMeList() const
{
	return *m_aiTargetingMeList;
}

// ----------------------------------------------------------------------

void ShipController::setPosition_w(Vector const & position, bool const allowOrientation)
{
	Transform transform(m_shipDynamicsModel->getTransform());
	transform.setPosition_p(position);
	m_shipDynamicsModel->setTransform(transform);
	
	if (allowOrientation)
	{
		m_shipDynamicsModel->stop();
	}
	else
	{
		m_shipDynamicsModel->makeStationary();
	}
}

// ----------------------------------------------------------------------

bool ShipController::face(Vector const & goalPosition_w, float elapsedTime)
{
	//DEBUG_REPORT_LOG(true, ("ShipController::face() owner(%s)\n", getOwner()->getNetworkId().getValueString().c_str()));

	ShipObject * const shipObject = NON_NULL(getShipOwner());
	ServerShipObjectInterface const shipInterface(shipObject);
	Transform shipTransform = shipObject->getTransform_o2w();
	bool reachedYaw = false;
	bool reachedPitch = false;

	if (shipInterface.getMaximumYaw() > 0.f)
	{
		Vector const goal_o = shipTransform.rotateTranslate_p2l(goalPosition_w);
		float const goalYaw = goal_o.theta();
		float const goalYawRate = goalYaw / elapsedTime;
		float const maxYawRate = shipInterface.getMaximumYaw();

		if((goalYawRate >= -maxYawRate) && (goalYawRate <= maxYawRate))
			reachedYaw = true;

		float const actualYawRate = clamp(-maxYawRate, goalYawRate, maxYawRate);
		float const actualYaw = actualYawRate * elapsedTime;

		m_yawPosition = actualYawRate / maxYawRate;

		shipTransform.yaw_l(actualYaw);
	}

	if (shipInterface.getMaximumPitch() > 0.f)
	{
		Vector const goal_o = shipTransform.rotateTranslate_p2l(goalPosition_w);
		float const goalPitch = goal_o.phi();
		float const goalPitchRate  = goalPitch / elapsedTime;
		float const maxPitchRate = shipInterface.getMaximumPitch();

		if((goalPitchRate >= -maxPitchRate) && (goalPitchRate <= maxPitchRate))
			reachedPitch = true;

		float const actualPitchRate = clamp(-maxPitchRate, goalPitchRate, maxPitchRate);
		float const actualPitch = actualPitchRate * elapsedTime;

		m_pitchPosition = actualPitchRate / maxPitchRate;

		shipTransform.pitch_l(actualPitch);
	}

	// If enabled, rotate on the "fast" axis of the ship.
	bool isRollSet = false;

	if (AiServerShipObjectInterface::isFastAxisEnabled()) 
	{
		bool useFastAxis = true;
		AiShipController const * const aiShipController = asAiShipController();

		if (aiShipController != nullptr)
		{
			if (   (aiShipController->getShipClass() == ShipAiReactionManager::SC_capitalShip)
				|| (aiShipController->getShipClass() == ShipAiReactionManager::SC_transport)
				|| !aiShipController->isAttacking())
			{
				useFastAxis = false;
			}
		}

		if (useFastAxis)
		{
			Vector desiredOrientation(m_pitchPosition, m_yawPosition, 0.0f);
			Vector fastOrientation(shipInterface.getMaximumPitch(), shipInterface.getMaximumYaw(), 0.0f);
			
			// Validate the normalized vectors and rotate along fast axis iff the vectors differ significantly.
			if (desiredOrientation.normalize() && fastOrientation.normalize() && ((desiredOrientation.dot(fastOrientation) + s_fastOrientationDeltaRadians) < 1.0f))
			{
				Vector const & targetOrientation = desiredOrientation.cross(fastOrientation);
				m_rollPosition = clamp(-1.0f, targetOrientation.z, 1.0f);
				isRollSet = true;
			}
		}
	}

	// If the roll wasn't set, use the original roll calculation.
	if (!isRollSet && shipInterface.getMaximumRoll() > 0.f)
	{
		Vector const goal_o = shipTransform.rotateTranslate_p2l(goalPosition_w);
		float goalRoll = 0.f;
		float const d = sqrt(sqr(goal_o.x) + sqr(goal_o.y));
		float const phi = atan2(d,goal_o.z);

		if(phi > convertDegreesToRadians(10.f))
		{
			goalRoll = -atan2(goal_o.x,goal_o.y);
		}
		else
		{
			// if we're not trying to align with the target, try to align with the Y axis,
			// but only if we're not heading nearly straight up or down

			Vector const upPoint_w = shipTransform.getPosition_p() + Vector::unitY;
			Vector const upPoint_o = shipTransform.rotateTranslate_p2l(upPoint_w);
			float d2 = sqrt(sqr(upPoint_o.x) + sqr(upPoint_o.y));
			float phi2 = atan2(d2,std::abs(upPoint_o.z));

			if(phi2 > convertDegreesToRadians(10.f))
			{
				goalRoll = -atan2(upPoint_o.x,upPoint_o.y);
			}
		}

		float const goalRollRate = goalRoll / elapsedTime;
		float const maxRollRate = shipInterface.getMaximumRoll();
		float const actualRollRate = clamp(-maxRollRate, goalRollRate, maxRollRate);

		//float const actualRoll = actualRollRate * elapsedTime;

		m_rollPosition = actualRollRate / maxRollRate;
	}

	return reachedYaw && reachedPitch;
}

// ----------------------------------------------------------------------

bool ShipController::face(Vector const & goalPosition_w, Vector const & up_w, float const elapsedTime)
{
	//DEBUG_REPORT_LOG(true, ("ShipController::face(up_w) owner(%s)\n", getOwner()->getNetworkId().getValueString().c_str()));

	ShipObject * const shipObject = NON_NULL(getShipOwner());
	ServerShipObjectInterface const shipInterface(shipObject);
	Transform shipTransform(shipObject->getTransform_o2w());

	// Yaw

	bool reachedYaw = false;
	{
		if (shipInterface.getMaximumYaw() > 0.f)
		{
			Vector const goal_o = shipTransform.rotateTranslate_p2l(goalPosition_w);
			float const goalYaw = goal_o.theta();
			float const goalYawRate = goalYaw / elapsedTime;
			float const maxYawRate = shipInterface.getMaximumYaw();

			if((goalYawRate >= -maxYawRate) && (goalYawRate <= maxYawRate))
				reachedYaw = true;

			float const actualYawRate = clamp(-maxYawRate, goalYawRate, maxYawRate);
			float const actualYaw = actualYawRate * elapsedTime;

			m_yawPosition = actualYawRate / maxYawRate;

			shipTransform.yaw_l(actualYaw);
		}
	}

	// Pitch

	bool reachedPitch = false;
	{
		if (shipInterface.getMaximumPitch() > 0.f)
		{
			Vector const goal_o = shipTransform.rotateTranslate_p2l(goalPosition_w);
			float const goalPitch = goal_o.phi();
			float const goalPitchRate  = goalPitch / elapsedTime;
			float const maxPitchRate = shipInterface.getMaximumPitch();

			if((goalPitchRate >= -maxPitchRate) && (goalPitchRate <= maxPitchRate))
				reachedPitch = true;

			float const actualPitchRate = clamp(-maxPitchRate, goalPitchRate, maxPitchRate);
			float const actualPitch = actualPitchRate * elapsedTime;

			m_pitchPosition = actualPitchRate / maxPitchRate;

			shipTransform.pitch_l(actualPitch);
		}
	}

	// Roll

	bool reachedRoll = false;
	{
		float const maxRollRate = shipInterface.getMaximumRoll();

		if (maxRollRate > 0.f)
		{
			Vector goalUp_o = shipTransform.rotateTranslate_p2l(up_w);
			goalUp_o.normalize();

			float const dot = Vector::unitY.dot(goalUp_o);
			float const angleBetween = acos(dot);

			if (   (goalUp_o.y > 0.0f)
			    && (angleBetween < convertDegreesToRadians(1.0f)))
			{
				m_rollPosition = 0.0f;
				reachedRoll = true;
			}
			else if (goalUp_o.x > 0.0f)
			{
				m_rollPosition = -1.0f;
			}
			else if (goalUp_o.x < 0.0f)
			{
				m_rollPosition = 1.0f;
			}

			if (   (m_rollPosition > 0.0f)
			    || (m_rollPosition < 0.0f))
			{
				float const goalRollRate = angleBetween * elapsedTime * 10.0f;
				float const actualRollRate = clamp(-maxRollRate, goalRollRate, maxRollRate);
				m_rollPosition *= actualRollRate / maxRollRate;
			}
		}
	}

	return reachedYaw && reachedPitch && reachedRoll;
}

// ----------------------------------------------------------------------

void ShipController::setThrottle(float throttle)
{
	m_throttlePosition = clamp(0.0f,throttle,1.0f);
}

// ----------------------------------------------------------------------

float ShipController::getLargestTurnRadius() const
{
	float result = 0.0f;

	ShipObject const * const shipObject = getShipOwner();

	if (shipObject != nullptr)
	{
		float const maxYawRate = shipObject->getShipActualYawRateMaximum();
		float const maxPitchRate = shipObject->getShipActualPitchRateMaximum();
		float const maxRollRate = shipObject->getShipActualRollRateMaximum();
		float const maxTurnRate = std::min(std::min(maxYawRate, maxPitchRate), maxRollRate);
		float const speedMaximum = shipObject->getShipActualSpeedMaximum();

		result = (maxTurnRate > 0.0f) ? (speedMaximum / maxTurnRate) : 0.0f;
	}

	return result;
}

// ----------------------------------------------------------------------

void ShipController::dock(ShipObject & dockTarget, float const secondsAtDock)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipController::dock() owner(%s) dockTarget(%s) secondsAtDock(%.2f)", getOwner()->getNetworkId().getValueString().c_str(), dockTarget.getNetworkId().getValueString().c_str(), secondsAtDock));

	// Create the docking behavior

	delete m_pendingDockingBehavior;
	m_pendingDockingBehavior = new AiShipBehaviorDock(*this, dockTarget, secondsAtDock);
}

// ----------------------------------------------------------------------

void ShipController::unDock()
{
	if (m_pendingDockingBehavior != nullptr)
	{
		m_pendingDockingBehavior->unDock();
	}
	else if (m_dockingBehavior != nullptr)
	{
		m_dockingBehavior->unDock();
	}
}

// ----------------------------------------------------------------------

bool ShipController::isDocking() const
{
	return (m_dockingBehavior != nullptr) || (m_pendingDockingBehavior != nullptr);
}

// ----------------------------------------------------------------------

bool ShipController::isDocked() const
{
	return ((m_dockingBehavior != nullptr) && m_dockingBehavior->isDocked());
}

// ----------------------------------------------------------------------

void ShipController::onTargetedByMissile(int const /*missileId*/)
{
}

//----------------------------------------------------------------------

ShipController * ShipController::asShipController()
{
	return this;
}

//----------------------------------------------------------------------

ShipController const * ShipController::asShipController() const
{
	return this;
}

// ----------------------------------------------------------------------

PlayerShipController * ShipController::asPlayerShipController()
{
	return nullptr;
}

// ----------------------------------------------------------------------

PlayerShipController const * ShipController::asPlayerShipController() const
{
	return nullptr;
}

// ----------------------------------------------------------------------

AiShipController * ShipController::asAiShipController()
{
	return nullptr;
}

// ----------------------------------------------------------------------

AiShipController const * ShipController::asAiShipController() const
{
	return nullptr;
}

// ----------------------------------------------------------------------

AiShipAttackTargetList const & ShipController::getAttackTargetList() const
{
	return *m_attackTargetList;
}

// ----------------------------------------------------------------------

bool ShipController::addDamageTaken(NetworkId const & attackingUnit, float const damage, bool const verifyAttacker)
{
	bool result = false;
	ShipObject * const attackingUnitShipObject = ShipObject::asShipObject(NetworkIdManager::getObjectById(attackingUnit));

	if (   (attackingUnitShipObject != nullptr)
	    && !attackingUnitShipObject->isDamageAggroImmune())
	{
		result = verifyAttacker ? Pvp::canAttack(*attackingUnitShipObject, *NON_NULL(getShipOwner())) : true;

		if (result)
		{
			m_attackTargetList->add(*attackingUnitShipObject, damage);
		}																		    
	}

	return result;
}

// ----------------------------------------------------------------------

bool ShipController::hasTurretTargetingSystem() const
{
	return (m_turretTargetingSystem != nullptr);
}

// ----------------------------------------------------------------------

float ShipController::getTurretMissChance() const
{
	return 0.5f;
}

// ----------------------------------------------------------------------

float ShipController::getTurretMissAngle() const
{
	// Once player auto-turrets come online, this needs to be overloaded in PlayerShipController
	return tan(convertDegreesToRadians(10.0f));
}

// ----------------------------------------------------------------------

void ShipController::clearEnemyCheckQueuedFlag()
{
	m_enemyCheckQueued = false;
}

// ----------------------------------------------------------------------

bool ShipController::removeAttackTarget(NetworkId const & unit)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipController::removeAttackTarget() owner(%s) unit(%s)", getOwner()->getNetworkId().getValueString().c_str(), unit.getValueString().c_str()));

	return m_attackTargetList->remove(unit);
}

// ----------------------------------------------------------------------

void ShipController::onAttackTargetChanged(NetworkId const & target)
{
	if (m_turretTargetingSystem != nullptr)
	{
		m_turretTargetingSystem->onTargetChanged(target);
	}
}

// ----------------------------------------------------------------------

void ShipController::onAttackTargetLost(NetworkId const & target)
{
	if (m_turretTargetingSystem != nullptr)
	{
		m_turretTargetingSystem->onTargetLost(target);
	}
}

// ----------------------------------------------------------------------

void ShipController::onAttackTargetListEmpty()
{
	// This is intentionally left blank
}

// ----------------------------------------------------------------------

float ShipController::getThrottle() const
{
	return m_throttlePosition;
}

// ----------------------------------------------------------------------

int ShipController::getNumberOfAiUnitsAttackingMe() const
{
	int result = 0;
	CachedNetworkIdList::const_iterator iterAiTargetingMeList = m_aiTargetingMeList->begin();

	for (; iterAiTargetingMeList != m_aiTargetingMeList->end(); ++iterAiTargetingMeList)
	{
		CachedNetworkId const & ai = (*iterAiTargetingMeList);
		Object const * const aiObject = ai.getObject();

		if (aiObject != nullptr)
		{
			ShipController const * const shipController = aiObject->getController()->asShipController();
			AiShipController const * const aiShipController = (shipController != nullptr) ? shipController->asAiShipController() : nullptr;

			if (aiShipController != nullptr)
			{
				if (aiShipController->getPrimaryAttackTarget() == getOwner()->getNetworkId())
				{
					++result;
				}
			}
		}
		else
		{
			DEBUG_WARNING(true, ("ShipController::getNumberOfAiShipAttackingMe() ERROR: Why am I(%s) being targeted by a nullptr object(%s)?", getOwner()->getDebugInformation().c_str(), ai.getValueString().c_str()));
		}
	}

	return result;
}

// ======================================================================
// PROTECTED ShipController
// ======================================================================

ShipObject * ShipController::getShipOwner()
{
	Object * const object = getOwner();
	ServerObject * const serverObject = object ? object->asServerObject() : 0;
	return serverObject ? serverObject->asShipObject() : 0;
}

// ----------------------------------------------------------------------

ShipObject const * ShipController::getShipOwner() const
{
	return const_cast<ShipController *>(this)->getShipOwner();
}

// ======================================================================
// PRIVATE ShipController
// ======================================================================

void ShipController::synchronizeTransform()
{
}

// ----------------------------------------------------------------------

void ShipController::experiencedCollision()
{
}

// ----------------------------------------------------------------------

void ShipController::clearAiTargetingMeList()
{
	// Notify all the ships targeting me that I am no longer valid
	
	s_purgeList.clear();
	CachedNetworkIdList::const_iterator iterTargetByAiList = m_aiTargetingMeList->begin();

	for (; iterTargetByAiList != m_aiTargetingMeList->end(); ++iterTargetByAiList)
	{
		CachedNetworkId const & id = *iterTargetByAiList;

		s_purgeList.push_back(id);
	}

	PurgeList::const_iterator iterPurgeList = s_purgeList.begin();

	for (; iterPurgeList != s_purgeList.end(); ++iterPurgeList)
	{
		CachedNetworkId const & id = (*iterPurgeList);

		if (id.getObject() != nullptr)
		{
			ShipController * const shipController = id.getObject()->getController()->asShipController();

			if (shipController != nullptr)
			{
				IGNORE_RETURN(shipController->removeAttackTarget(getOwner()->getNetworkId()));
			}
			else
			{
				LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipController::~ShipController() ERROR: owner(%s) targetedByUnit(%s) Why does this unit not have a ShipController?", getOwner()->getNetworkId().getValueString().c_str(), id.getValueString().c_str()));
			}
		}
	}
}

// ----------------------------------------------------------------------

void ShipController::getApproximateFutureTransform(Transform & transform, Vector & velocity, float elapsedTime) const
{
	ServerShipObjectInterface const serverShipObjectInterface(getShipOwner());

	m_shipDynamicsModel->getApproximateFutureTransform(transform, velocity, elapsedTime, serverShipObjectInterface);
}

// ======================================================================
