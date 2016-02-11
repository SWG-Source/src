// ======================================================================
//
// Missile.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/Missile.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/MissileManager.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ShipObject.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedMath/MultiShape.h"
#include "sharedNetworkMessages/CreateMissileMessage.h"
#include "sharedNetworkMessages/UpdateMissileMessage.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

int Missile::ms_nextMissileId=1;

// ======================================================================

Missile::Missile(const NetworkId &sourcePlayer, const NetworkId &sourceShip, int weaponIndex, int missileType, const NetworkId &target, ShipChassisSlotType::Type targetedComponent) :
		m_missileId(ms_nextMissileId++),
		m_sourceShip(sourceShip),
		m_sourcePlayer(sourcePlayer),
		m_targetShip(target),
		m_impactTime(0),
		m_state(MS_None),
		m_weaponIndex(weaponIndex),
		m_missileType(missileType),
		m_targetedComponent(targetedComponent),
		m_fireTime(0)
{
}

// ----------------------------------------------------------------------

Missile::~Missile()
{
}

// ----------------------------------------------------------------------

bool Missile::fire()
{
	if (m_state != MS_None)
		return false;

	m_state=MS_Launched;
	
	ServerObject *sourceServerObject = getSourceServerObject();
	ServerObject *targetServerObject = getTargetServerObject();

	if (sourceServerObject && targetServerObject)
	{
		Vector sourceLocation = sourceServerObject->getPosition_w();
		Vector targetLocation = targetServerObject->getPosition_w();

		float distance = (sourceLocation - targetLocation).magnitude();

		// check cone of fire
		if(distance > 0.0f)
		{

			Vector firingAngle = targetLocation - sourceLocation;

			Vector shipForwardEnd = sourceLocation + (sourceServerObject->getObjectFrameK_w() * 100.0f); // Arbitrarily scale this vector out a few meters.
			Vector shipForwardVec = shipForwardEnd - sourceLocation;

			float dotProd = shipForwardVec.dot(firingAngle);
			float shipForwardLength = shipForwardVec.magnitude();
			float firingAngleLength = firingAngle.magnitude();

			float finalAngle = acos(dotProd / (shipForwardLength * firingAngleLength));

			float const firingRadius = convertDegreesToRadians(MissileManager::getInstance().getTargetAcquisitionAngle(m_missileType));

			if(finalAngle > firingRadius)
			{
				// People trying to mess with the shared data table, fail this launch.
				m_state = MS_Miss;
				return false;
			}
		}


		// Compute time  TODO:  adjustments based on quality of missile
		float totalTime = distance / MissileManager::getInstance().getSpeedByMissileType(m_missileType);
		totalTime = std::max(totalTime, MissileManager::getInstance().getMinTimeByMissileType(m_missileType));

		// tell clients about the missile
		CreateMissileMessage const createMissileMessage(m_missileId, m_sourceShip, m_targetShip, sourceLocation, targetLocation, static_cast<int>(totalTime), m_missileType, m_weaponIndex, m_targetedComponent);
		sendToAllObservers(createMissileMessage);

		// check for out-of-range
		if (totalTime > MissileManager::getInstance().getMaxTimeByMissileType(m_missileType))
		{
			totalTime = MissileManager::getInstance().getMaxTimeByMissileType(m_missileType);
			m_state=MS_Miss;
		}

		// check for obstacles that would block getting a lock
		{
			Capsule const missileCapsule_w(sourceLocation, targetLocation, 1.0f);
			ColliderList collidedWith;
			CollisionWorld::getDatabase()->queryFor(static_cast<int>(SpatialDatabase::Q_Physicals), CellProperty::getWorldCellProperty(), true, missileCapsule_w, collidedWith);
			for (ColliderList::const_iterator i = collidedWith.begin(); i != collidedWith.end(); ++i)
			{
				Object const * const collider = &(NON_NULL(*i)->getOwner());
				if (collider != sourceServerObject && collider != targetServerObject)
					destroy();
			}
		}
				
		// Schedule the missile
		m_fireTime=ServerClock::getInstance().getGameTimeSeconds();
		m_impactTime=m_fireTime + static_cast<uint32>(totalTime);
		MissileManager::getInstance().scheduleMissile(m_missileId, m_impactTime);

		// Run the missile script on the source ship, to do die rolls for success, etc.
		// This script may call functions to make the missile lock, miss, etc.  If it does
		// nothing, the missile will continue towards the target but can still be countermeasured.
		if (m_state==MS_Launched)
		{
			ScriptParams params;
			params.addParam(m_missileId);
			params.addParam(m_weaponIndex);
			params.addParam(static_cast<int>(m_missileType));
			params.addParam(m_sourcePlayer);
			params.addParam(m_targetShip);
			params.addParam(m_targetedComponent);
			IGNORE_RETURN (sourceServerObject->getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_FIRED_MISSILE, params));
		}

		// Let the ship object react to the missile
		if (m_state==MS_Launched) //lint !e774 (not always true because the triggers above could change it)
		{
			if (targetServerObject->asShipObject())
				targetServerObject->asShipObject()->onTargetedByMissile(m_missileId);
		}
		
		// Warn the target about the missile, so that AIs can react, etc.
		if (m_state==MS_Launched) //lint !e774 (not always true because the triggers above could change it)
		{
			ScriptParams params;
			params.addParam(m_missileId);
			params.addParam(m_weaponIndex);
			params.addParam(static_cast<int>(m_missileType));
			params.addParam(m_sourceShip);
			params.addParam(m_sourcePlayer);
			params.addParam(m_targetedComponent);
			IGNORE_RETURN (targetServerObject->getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_TARGETED_BY_MISSILE, params));
		}
	}
	else
		WARNING_STRICT_FATAL(true,("Missile %i was fired from object %s to object %s, but one or both of these objects did not exist at the time the missile was fired.  Both of these objects should exist when the missile is fired (although they can go away later).",m_missileId, m_sourceShip.getValueString().c_str(), m_targetShip.getValueString().c_str()));

	return true;
}

// ----------------------------------------------------------------------

void Missile::impactTarget()
{
	ServerObject * const source = getSourceServerObject();
	ServerObject * const target = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(m_targetShip));
	if (source && target && !target->getKill())
	{
		ShipObject * const sourceShip = source->asShipObject();
		ShipObject * const targetShip = target->asShipObject();
		if (sourceShip && targetShip && Pvp::canAttack(*sourceShip, *targetShip))
		{
			ScriptParams params;
			params.addParam(m_sourceShip);
			params.addParam(m_weaponIndex);
			params.addParam(true);
			params.addParam(static_cast<int>(m_missileType));
			params.addParam(m_targetedComponent);
			params.addParam(false);
			params.addParam(0.0f);
			params.addParam(0.0f);
			params.addParam(0.0f);
			IGNORE_RETURN(target->getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_HIT, params));
		}
	}

	// Notify the clients
	UpdateMissileMessage const updateMissileMessage(m_missileId, NetworkId::cms_invalid, 0, UpdateMissileMessage::UT_hit);
	sendToAllObservers(updateMissileMessage);
}

// ----------------------------------------------------------------------

/**
 * Flag the missile for destruction due to miss/failed skill check/etc. and notify the clients.
 */
void Missile::destroy()
{
	UpdateMissileMessage const updateMissileMessage(m_missileId, NetworkId::cms_invalid, 0, UpdateMissileMessage::UT_miss);
	sendToAllObservers(updateMissileMessage);
	m_state=MS_Kill;
}

// ----------------------------------------------------------------------

/**
 * Countermeasure a missile and notify the clients.
 * @param ship The ship that launched the countermeasure.  If cms_invalid, the
 * ship targetted by the missile is assumed.
 */
void Missile::countermeasure(NetworkId const & ship, int const countermeasureType)
{
	UpdateMissileMessage const updateMissileMessage(m_missileId, ship!=NetworkId::cms_invalid ? ship : static_cast<NetworkId>(m_targetShip), countermeasureType, UpdateMissileMessage::UT_countermeasured);
	sendToAllObservers(updateMissileMessage);
	m_state=MS_Kill;
}

// ----------------------------------------------------------------------

/**
 * Launch a countermeasure that does not succeed at destroying a missile.
 */
void Missile::launchFailedCountermeasure(ServerObject const & ship, int const missileId, int const countermeasureType)
{
	UpdateMissileMessage const updateMissileMessage(missileId, ship.getNetworkId(), countermeasureType, UpdateMissileMessage::UT_countermeasureFailed);
	sendToAllObservers(ship, updateMissileMessage);
}

// ----------------------------------------------------------------------

/**
 * Update the missiles status as needed.
 *
 * If the missile will need another update, it should call
 * MissileManager::scheduleUpdate().  Note that the missile might get
 * another update() called before that time in some circumstances, so it should
 * always check the effectiveTime.
 *
 * @param effectiveTime The time (ServerClock) for the purpose of the update.
 * @return true if the missile is still active, false if it should be deleted.
 */
bool Missile::update(uint32 effectiveTime)
{
	bool result=true;
	
	switch (m_state)
	{
		case MS_None:
		{
			break;
		}
 
		case MS_Launched:
		{
			if (effectiveTime >= m_impactTime)
			{
				impactTarget();
				result=false;
			}
			break;
		}

		case MS_Miss:
		{
			if (effectiveTime >= m_impactTime)
			{
				destroy();
				result=false;
			}
			break;
		}
		
		case MS_Kill:
		{
			// missile is already dead
			result=false;
		}
	}

	return result;
}

// ----------------------------------------------------------------------

ServerObject * Missile::getSourceServerObject() const
{
	Object *sourceObject = m_sourceShip.getObject();
	if (sourceObject)
		return sourceObject->asServerObject();
	else
		return nullptr;
}

// ----------------------------------------------------------------------

ServerObject * Missile::getTargetServerObject() const
{
	Object *targetObject = m_targetShip.getObject();
	if (targetObject)
		return targetObject->asServerObject();
	else
		return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Send a network message to all clients observing this missile
 */
void Missile::sendToAllObservers(GameNetworkMessage const & message) const
{
	ServerObject * const targetAsServerObject = getTargetServerObject();
	if (targetAsServerObject)
	{
		sendToAllObservers(*targetAsServerObject, message);
	}
}
	
// ----------------------------------------------------------------------

/**
 * Send a network message to all clients observing an object
 */
void Missile::sendToAllObservers(ServerObject const & object, GameNetworkMessage const & message)
{
	typedef std::map<ConnectionServerConnection *, std::vector<NetworkId> > DistributionList;
	DistributionList distributionList;

	{
		typedef std::set<Client *> ObserverList;
		ObserverList const & observers = object.getObservers();
		for (ObserverList::const_iterator iter = observers.begin(); iter != observers.end(); ++iter)
		{
			NetworkId const & characterObjectId = (*iter)->getCharacterObjectId();
			distributionList[(*iter)->getConnection()].push_back(characterObjectId);
		}
	}
		
	for (DistributionList::const_iterator iter = distributionList.begin(); iter != distributionList.end(); ++iter)
	{
		GameClientMessage const gameClientMessage(iter->second, true, message);
		iter->first->send(gameClientMessage, true);
	}
}

// ----------------------------------------------------------------------

int Missile::getTimeSinceFired() const
{
	return static_cast<int>(ServerClock::getInstance().getGameTimeSeconds() - m_fireTime);
}

// ======================================================================
