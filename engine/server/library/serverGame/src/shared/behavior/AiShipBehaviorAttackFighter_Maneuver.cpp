// ======================================================================
//
// AiShipBehaviorAttackFighter_Maneuver.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorAttackFighter_Maneuver.h"
#include "serverGame/AiShipBehaviorAttackFighter_Maneuver_Path.h"

#include "serverGame/AiPilotManager.h"
#include "serverGame/AiShipController.h"
#include "serverGame/AiShipPilotData.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceAttackSquad.h"
#include "serverGame/SpaceSquad.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedLog/Log.h"
#include "sharedMath/Range.h"
#include "sharedTerrain/TerrainObject.h"

#include <vector>


// ======================================================================

namespace AiShipBehaviorAttackFighter_ManeuverNamespace
{
#ifdef _DEBUG
	int s_sequenceId = 0;
#endif // _DEBUG
}

using namespace AiShipBehaviorAttackFighter_ManeuverNamespace;

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver::Maneuver(FighterManeuver const fm, AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo, float const maxTime, bool updateThrottle) :
m_aiShipBehaviorAttack(aiShipBehaviorAttack),
m_targetInfo(targetInfo),
m_pathList(new PathList),
m_currentPath(m_pathList->begin()),
m_currentPathNode(0),
m_abortFlags(0),
m_updateThrottle(updateThrottle),
m_throttleValue(0.0f),
m_desiredPosition_w(),
m_maneuverCategory(fm),
#ifdef _DEBUG
m_sequenceId(s_sequenceId++),
#endif // _DEBUG
m_expirationTimer(maxTime)
{
#ifdef _DEBUG
	float const weaponAttackRange = getAiShipController().getAttackSquad().getWeaponAttackRange();

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled() && (weaponAttackRange <= 0.0f), "debug_ai", ("AiShipBehaviorAttackFighter::Maneuver::Maneuver() owner(%s) Why are we attacking if we have no weapons? weaponAttackRange(%.0f)", getAiShipController().getOwner()->getNetworkId().getValueString().c_str(), weaponAttackRange));
#endif // _DEBUG
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver::~Maneuver()
{
	for(PathList::iterator itPath = m_pathList->begin(); itPath != m_pathList->end(); ++itPath)
	{
		Path::deletePath(*itPath);
	}
	
	delete m_pathList;

	//m_currentPath = nullptr;
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver::FighterManeuver AiShipBehaviorAttackFighter::Maneuver::getFighterManeuver() const
{
	return m_maneuverCategory;
}

// ----------------------------------------------------------------------

Vector const & AiShipBehaviorAttackFighter::Maneuver::getDesiredPosition_w() const
{
	return m_desiredPosition_w;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::Maneuver::setDesiredPosition_w(Vector const & position)
{
	m_desiredPosition_w = position;
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::AiAttackTargetInformation const & AiShipBehaviorAttackFighter::Maneuver::getTargetInfo() const
{
	return m_targetInfo;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::isDesiredPositionBehindMe() const
{
	Vector const position_o(getAiShipController().getOwner()->rotateTranslate_w2o(m_desiredPosition_w));

	return (position_o.z < 0.0f);
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::Maneuver::addPath(Path * const path)
{
	if (path)
	{
		m_pathList->push_back(path);
	}

	m_currentPath = m_pathList->begin();
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver::Path * AiShipBehaviorAttackFighter::Maneuver::getCurrentPath()
{
	Path * currentPath = nullptr;
	
	if (!m_pathList->empty() && m_currentPath != m_pathList->end())
	{
		currentPath = *m_currentPath;
	}
	
	return currentPath;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::alter(float timeDelta, int stateFlags)
{
	bool shouldContinue = !m_expirationTimer.updateZero(timeDelta);
	
	if (shouldContinue)
	{
		shouldContinue = alterPath(timeDelta);
	}

	if (shouldContinue)
	{
		shouldContinue = !(stateFlags & m_abortFlags);
	}

	// Modify the controller.
	if (shouldContinue) 
	{
		alterThrottle(timeDelta);
		modifyController(timeDelta);
	}

	// let the caller know if it should continue.
	return shouldContinue;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::alterInternal(float const /*timeDelta*/)
{
	return (getDesiredPosition_w() - getAiShipController().getOwnerPosition_w()).approximateMagnitude() > getAiShipController().getLargestTurnRadius();
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::alterPath(float const timeDelta)
{
	bool shouldContinue = alterInternal(timeDelta);

	//if (!shouldContinue)
	//{
	//	Path * const currentPath = getCurrentPath();
	//	shouldContinue = currentPath ? selectNextPath() : false;
	//}

	return shouldContinue;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::Maneuver::selectNextPath()
{
	bool shouldContinue = false;

	if (!m_pathList->empty() && (m_currentPath != m_pathList->end()))
	{
		shouldContinue = ++m_currentPath != m_pathList->end();
		m_currentPathNode = 0;
	}

	return shouldContinue;
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver::Path * AiShipBehaviorAttackFighter::Maneuver::addEmptyPath()
{
	Path * newPath = Path::createPath();
	addPath(newPath);
	return newPath;
}


// ----------------------------------------------------------------------
// RLS -- Ensure the ship controller is modified in this block.
void AiShipBehaviorAttackFighter::Maneuver::modifyController(float const timeDelta)
{
	getAiShipController().moveTo(m_desiredPosition_w, m_throttleValue, timeDelta);
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::Maneuver::alterThrottle(float const /*timeDelta*/)
{
	m_throttleValue = 1.0f;
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver * AiShipBehaviorAttackFighter::Maneuver::createManeuver(FighterManeuver const manueverType, AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo)
{
	Maneuver * aiManeuver = nullptr;

	switch(manueverType)
	{
	case FM_chase:
		aiManeuver = createManeuverChase(aiShipBehaviorAttack, targetInfo);
		break;
		
	case FM_leash:
		aiManeuver = createManeuverLeash(aiShipBehaviorAttack, targetInfo);
		break;
		
	default:
	case FM_none:
	case FM_evade:
		aiManeuver = createManeuverEvade(aiShipBehaviorAttack, targetInfo);
		break;
	}

	return aiManeuver;
}

// ----------------------------------------------------------------------

AiShipController & AiShipBehaviorAttackFighter::Maneuver::getAiShipController()
{
	return m_aiShipBehaviorAttack.getAiShipController();
}

// ----------------------------------------------------------------------

AiShipController const & AiShipBehaviorAttackFighter::Maneuver::getAiShipController() const
{
	return m_aiShipBehaviorAttack.getAiShipController();
}

#ifdef _DEBUG
// ----------------------------------------------------------------------

int AiShipBehaviorAttackFighter::Maneuver::getSequenceId() const
{
	return m_sequenceId;
}
#endif // _DEBUG

// ----------------------------------------------------------------------

int AiShipBehaviorAttackFighter::Maneuver::getCurrentPathIndex() const
{
	return m_currentPathNode;
}

//=======================================================================

class AiManeuver_Chase : public AiShipBehaviorAttackFighter::Maneuver
{
public:
	AiManeuver_Chase(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiShipBehaviorAttackFighter::AiAttackTargetInformation const & targetInfo)
	 : Maneuver(FM_chase, aiShipBehaviorAttack, targetInfo, aiShipBehaviorAttack.getAiShipController().getPilotData()->m_fighterChaseMaxTime, true)
	 , m_chaseOnTailTimer(getAiShipController().getPilotData()->m_fighterChaseMaxOnTailTime)
	 , m_pursuitType(PT_lag)
	 , m_frontComfortDistance(0.0f)
	 , m_rearComfortDistance(0.0f)
	{
		// Calculate the front and rear comfort distances, when these distances are penetrated, the AI wants to stop chasing

		ShipObject const * const primaryTargetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

		if (primaryTargetShipObject != nullptr)
		{
			float const ownerShipRadius = getAiShipController().getShipOwner()->getRadius();
			float const ownerTurnRadius = getAiShipController().getLargestTurnRadius();
			float const targetShipRadius = primaryTargetShipObject->getRadius();

			m_frontComfortDistance = (targetShipRadius + (ownerShipRadius * 2.0f) + (ownerTurnRadius * 0.7f));
			m_rearComfortDistance = (targetShipRadius + (ownerShipRadius * 2.0f) + (ownerTurnRadius * 0.3f));
		}
		else
		{
			// ERROR
		}

		calculateChaseDistance();

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiManeuver_Chase() owner(%s) isAttackSquadLeader(%s) NEW CHASE MANEUVER", getAiShipController().getOwner()->getNetworkId().getValueString().c_str(), getAiShipController().isAttackSquadLeader() ? "yes" : "no"));

		if (getAiShipController().isAttackSquadLeader())
		{
			getAiShipController().getAttackSquad().setTooCloseToTarget(false);
		}

		setDesiredPosition_w(m_targetInfo.m_position_w);
	}

	virtual char const * const getFighterManeuverString() const;

protected:
	  
	bool alterInternal(float const /*timeDelta*/)
	{
		bool shouldContinue = true;
		bool const attackSquadLeader = getAiShipController().isAttackSquadLeader();

		calculateChaseDistance();

		ShipObject const * const primaryTargetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

		if (primaryTargetShipObject != nullptr)
		{
			bool const facingTarget = m_aiShipBehaviorAttack.isFacingTarget();

			if (facingTarget)
			{
				// Lead pursuit

				setDesiredPosition_w(m_aiShipBehaviorAttack.getNextShotPosition_w());
				m_pursuitType = PT_lead;
			}
			else
			{
				// Lag pursuit

				float const chaseDistance = getAiShipController().getAttackSquad().getChaseDistance();
				Vector const lagPursuitPosition_l(Vector(0.0f, 0.0f, -chaseDistance));
				Vector const lagPursuitPosition_w(primaryTargetShipObject->getTransform_o2w().rotateTranslate_l2p(lagPursuitPosition_l));

				setDesiredPosition_w(lagPursuitPosition_w);
				m_pursuitType = PT_lag;
			}

			// If anyone in the attack squad is too close to the target, flag it

			bool const behindTarget = m_aiShipBehaviorAttack.isBehindTarget();
			float const tooCloseDistance = behindTarget ? m_rearComfortDistance : m_frontComfortDistance;

			if (getDistanceToTargetSquared() <= sqr(tooCloseDistance))
			{
				getAiShipController().getAttackSquad().setTooCloseToTarget(true);
			}

			if (attackSquadLeader)
			{
				if (m_aiShipBehaviorAttack.shouldStartToLeash())
				{
					shouldContinue = false;

					LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiManeuver_Chase() owner(%s) CHASE BAILING - NEED TO LEASH", getAiShipController().getOwner()->getNetworkId().getValueString().c_str()));
				}
				else if (m_aiShipBehaviorAttack.shouldStartToEvade())
				{
					shouldContinue = false;

					LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiManeuver_Chase() owner(%s) CHASE BAILING - NEED TO EVADE", getAiShipController().getOwner()->getNetworkId().getValueString().c_str()));
				}
			}
		}
		else
		{
			// ERROR
		}

		//if (shouldContinue)
		//{
		//	// See if we have been on the target's ass too long
		//
		//	float const chaseDistance = getAiShipController().getAttackSquad().getChaseDistance();
		//
		//	if (getDistanceToTargetSquared() < sqr(chaseDistance))
		//	{
		//		if (m_chaseOnTailTimer.updateZero(timeDelta))
		//		{
		//			shouldContinue = false;
		//		}
		//	}
		//	else
		//	{
		//		m_chaseOnTailTimer.reset();
		//	}
		//}

		return shouldContinue;
	}

	virtual void alterThrottle(float const /*timeDelta*/)
	{
		float const chaseDistance = getAiShipController().getAttackSquad().getChaseDistance();
		float const distanceToTargetSquared = getDistanceToTargetSquared();

		if (   (distanceToTargetSquared < sqr(chaseDistance))
		    && m_targetInfo.m_playerControlled)
		{
			AiShipPilotData const & pilotData = *NON_NULL(getAiShipController().getPilotData());
			m_throttleValue = pilotData.m_fighterChaseMinThrottle;
		}
		else
		{
			if (isDesiredPositionBehindMe())
			{
				m_throttleValue = 0.5f;
			}
			else
			{
				m_throttleValue = 1.0f;
			}
		}
	}

private:

	float getDistanceToTargetSquared()
	{
		return m_targetInfo.m_position_w.magnitudeBetweenSquared(getAiShipController().getOwnerPosition_w());
	}

	void calculateChaseDistance()
	{
		if (getAiShipController().isAttackSquadLeader())
		{
			ShipObject const * const primaryTargetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

			if (primaryTargetShipObject != nullptr)
			{
				AiShipPilotData const & pilotData = *NON_NULL(getAiShipController().getPilotData());
				float const currentSpeed = getAiShipController().getShipOwner()->getCurrentSpeed();
				float const seperationDistance = (currentSpeed * pilotData.m_fighterChaseSeperationTime);
				float const chaseDistance = (m_rearComfortDistance * 1.2f) + seperationDistance;

				getAiShipController().getAttackSquad().setChaseDistance(chaseDistance);
			}
			else
			{
				// ERROR
			}
		}
	}

	enum PursuitType
	{
		PT_lead,
		PT_lag
	};

	Timer m_chaseOnTailTimer;
	PursuitType m_pursuitType;
	float m_frontComfortDistance;
	float m_rearComfortDistance;

	AiManeuver_Chase(AiManeuver_Chase const &);
	AiManeuver_Chase();
	AiManeuver_Chase const & operator=(AiManeuver_Chase const & rhs);
};

// ----------------------------------------------------------------------

char const * const AiManeuver_Chase::getFighterManeuverString() const
{
	switch (m_pursuitType)
	{
		case PT_lead: { return "CHASE: LEAD PURSUIT"; }
		case PT_lag: { return "CHASE: LAG PURSUIT"; }
		default: {}
	}

	return "CHASE: INVALID";
}

//=======================================================================

class AiManeuver_Leash : public AiShipBehaviorAttackFighter::Maneuver
{
public:
	AiManeuver_Leash(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiShipBehaviorAttackFighter::AiAttackTargetInformation const & targetInfo) :
	Maneuver(FM_leash, aiShipBehaviorAttack, targetInfo, 100.0f, true)
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiManeuver_Leash() owner(%s) isAttackSquadLeader(%s) NEW LEASH MANEUVER", getAiShipController().getOwner()->getNetworkId().getValueString().c_str(), getAiShipController().isAttackSquadLeader() ? "yes" : "no"));

		setDesiredPosition_w(linearInterpolate(getAiShipController().getOwnerPosition_w(), getAiShipController().getSquad().getLeashAnchorPosition_w(), 1.0f - getAiShipController().getPilotAggression() * 0.5f));

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled() && (getDesiredPosition_w() == Vector::zero), "debug_ai", ("AiManeuver_Leash() ERROR: Desired Leash position set to Vector::zero."));
	}

	virtual char const * const getFighterManeuverString() const;

protected:

	virtual bool alterInternal(float const timeDelta)
	{
		bool shouldContinue = Maneuver::alterInternal(timeDelta);

		// See if we have leashed far enough back in

		float const percentWithinLeashDistance = m_aiShipBehaviorAttack.getPercentWithinLeashDistance();
		
		if (percentWithinLeashDistance > 0.25f)
		{
			shouldContinue = false;

			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiManeuver_Leash() owner(%s) LEASH BAILING - WE HAVE LEASHED FAR ENOUGH", getAiShipController().getOwner()->getNetworkId().getValueString().c_str()));
		}

		return shouldContinue;
	}

private:

	AiManeuver_Leash(AiManeuver_Leash const &);
	AiManeuver_Leash();
	AiManeuver_Leash const & operator=(AiManeuver_Leash const & rhs);
};

// ----------------------------------------------------------------------

char const * const AiManeuver_Leash::getFighterManeuverString() const
{
	return "LEASH";
}

//=======================================================================

class AiManeuver_Evade : public AiShipBehaviorAttackFighter::Maneuver
{
public:

	AiManeuver_Evade(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiShipBehaviorAttackFighter::AiAttackTargetInformation const & targetInfo)
	 : Maneuver(FM_evade, aiShipBehaviorAttack, targetInfo, aiShipBehaviorAttack.getAiShipController().getPilotData()->m_fighterEvadeMaxTime, true)
	 , m_evadePositionUpdateTimer_w(getAiShipController().getPilotData()->m_fighterEvadePositionUpdateDelay)
	 , m_evadePositionUpdateTimer_l(4.0f)
	 , m_evadePosition_l()
	 , m_forceGetBehindTarget((rand() % 2) == 0)
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "debug_ai", ("AiManeuver_Evade() owner(%s) isAttackSquadLeader(%s) NEW EVADE MANEUVER", getAiShipController().getOwner()->getNetworkId().getValueString().c_str(), getAiShipController().isAttackSquadLeader() ? "yes" : "no"));

		calculateEvadePosition_l();
		calculateDesiredPosition(0.0f);
	}

	virtual char const * const getFighterManeuverString() const;

protected:
	
	bool alterInternal(float const timeDelta)
	{
		bool shouldContinue = true;

		if (m_aiShipBehaviorAttack.isBehindTarget())
		{
			// If I am behind the target and and the target is moving fast enough, I need to turn around
			// immediatelly to chase the target

			float const ownerShipActualSpeedMaximum = getAiShipController().getShipOwner()->getShipActualSpeedMaximum();
		
			if (m_targetInfo.m_speed > (ownerShipActualSpeedMaximum * 0.9f))
			{
				// Stop evading so we can do a lag pursuit
		
				shouldContinue = false;
			}
		}

		if (shouldContinue)
		{
			float const turnRadius = getAiShipController().getLargestTurnRadius();
			float const weaponRange = getAiShipController().getAttackSquad().getProjectileAttackRange();

			if (m_aiShipBehaviorAttack.getDistanceToTargetSquared() > sqr(weaponRange + turnRadius))
			{
				// We are far enough from the target to make an attack run, stop evading

				shouldContinue = false;
			}
			else
			{
				if (m_forceGetBehindTarget)
				{
					if (m_targetInfo.m_inTargetsProjectileConeOfFire)
					{
						if (m_evadePositionUpdateTimer_l.updateZero(timeDelta))
						{
							// We have been targeted too much, change direction

							m_evadePositionUpdateTimer_l.reset();
							calculateEvadePosition_l();
						}
					}

					if (m_evadePositionUpdateTimer_w.updateZero(timeDelta))
					{
						// Calculate a new evade position

						m_evadePositionUpdateTimer_w.reset();
						calculateDesiredPosition(timeDelta);
					}
				}
				else if (m_targetInfo.m_inTargetsProjectileConeOfFire)
				{
					if (m_evadePositionUpdateTimer_l.updateZero(timeDelta))
					{
						// We have been targeted too much, change direction

						m_evadePositionUpdateTimer_l.reset();
						calculateEvadePosition_l();
					}

					if (m_evadePositionUpdateTimer_w.updateZero(timeDelta))
					{
						// Calculate a new evade position

						m_evadePositionUpdateTimer_w.reset();
						calculateDesiredPosition(timeDelta);
					}
				}
			}
		}

		return shouldContinue;
	}

	virtual void alterThrottle(float const /*timeDelta*/)
	{
		if (isDesiredPositionBehindMe())
		{
			m_throttleValue = 0.5f;
		}
		else
		{
			m_throttleValue = 1.0f;
		}
	}

private:

	void calculateDesiredPosition(float const deltaSeconds)
	{
		ShipObject const * const primaryTargetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

		if (primaryTargetShipObject!= nullptr)
		{
			ShipController const * const targetShipController = primaryTargetShipObject->getController()->asShipController();

			if (targetShipController != nullptr)
			{
				Transform transform;
				Vector velocity;

				targetShipController->getApproximateFutureTransform(transform, velocity, deltaSeconds);

				setDesiredPosition_w(transform.rotateTranslate_l2p(m_evadePosition_l));
			}
			else
			{
				setDesiredPosition_w(primaryTargetShipObject->getTransform_o2w().rotateTranslate_l2p(m_evadePosition_l));
			}
		}
	}

	void calculateEvadePosition_l()
	{
		// Calculate the evade position relative to the -z axis of the target

		AiShipPilotData const & pilotData = *NON_NULL(getAiShipController().getPilotData());
		Transform transform;

		float const rollAngle = Random::randomReal() * PI_TIMES_2;
		transform.roll_l(rollAngle);

		float const halfEvadeAngle = Random::randomReal() * pilotData.m_fighterEvadeAngle * 0.5f;
		transform.pitch_l(halfEvadeAngle);

		float const x = 0.0f;
		float const y = 0.0f;
		float const z = -10000.0f;

		m_evadePosition_l = transform.rotateTranslate_l2p(Vector(x, y, z));
	}

	Timer m_evadePositionUpdateTimer_w;
	Timer m_evadePositionUpdateTimer_l;
	Vector m_evadePosition_l;
	bool m_forceGetBehindTarget;

	AiManeuver_Evade(AiManeuver_Evade const &);
	AiManeuver_Evade();
	AiManeuver_Evade const & operator=(AiManeuver_Evade const & rhs);
};

// ----------------------------------------------------------------------

char const * const AiManeuver_Evade::getFighterManeuverString() const
{
	return "EVADE";
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver * AiShipBehaviorAttackFighter::Maneuver::createManeuverChase(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo)
{
	return new AiManeuver_Chase(aiShipBehaviorAttack, targetInfo);
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver * AiShipBehaviorAttackFighter::Maneuver::createManeuverEvade(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo)
{
	return new AiManeuver_Evade(aiShipBehaviorAttack, targetInfo);
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::Maneuver * AiShipBehaviorAttackFighter::Maneuver::createManeuverLeash(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo)
{
	return new AiManeuver_Leash(aiShipBehaviorAttack, targetInfo);
}

// ======================================================================
