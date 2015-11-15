// ======================================================================
//
// AiShipBehaviorAttackFighter.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorAttackFighter_H
#define	INCLUDED_AiShipBehaviorAttackFighter_H

#include "serverGame/AiShipBehaviorAttack.h"
#include "sharedFoundation/Timer.h"
#include "sharedObject/Object.h"

class AiPilotManager;
class NetworkId;
class ShipObject;

// ======================================================================

class AiShipBehaviorAttackFighter : public AiShipBehaviorAttack
{
public:
	class Maneuver;

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
	void debug_forceManeuver(int maneuverType);
#endif // _DEBUG

	static Vector const calculateEvadePositionWithinLeashDistance_w(AiShipBehaviorAttack const & aiShipBehaviorAttack);

protected:
	void setManeuver(Maneuver * const);
	
	float getTimeDelta() const;
	void setTimeDelta(float const timeDelta);
	
	float getWeaponPriority(bool const missilesReady, bool const projectileReady, int const weaponIndex);
	
	void alterWeapons();
	void alterManeuverFlags();
	void alterTargetInformation();
	void alterManeuver();
	void alterBooster();
	
	AiPilotManager const & getPilotManagerInfo() const;

	void setManeuverFlag(int const activeFlags, bool const setFlag);
	bool hasManeuverFlag(int const activeFlags) const;

public:
	
	class AiAttackTargetInformation
	{
	public:
		
		AiAttackTargetInformation();
		
		bool m_playerControlled;
		float m_distanceToTarget;
		float m_speed;
		float m_weaponRange;
		float m_radius;
		bool m_facingTarget;
		bool m_behindTarget;
		
		Vector m_position_w;
		Vector m_orientation_w;
		Vector m_directionFromShip_w;
		Vector m_offset; // aim offset
		Vector m_maneuverPosition_w;
		
		Timer m_offsetUpdate;
		Timer m_positionUpdate;
		
		bool m_isBoosting;
		bool m_targetInProjectileConeOfFire;
		bool m_targetWasInProjectileConeOfFire;
		bool m_targetInMissileLockOnConeOfFire;
		bool m_collisionCourse;
		bool m_requestPositionUpdate;
		bool m_inTargetsProjectileConeOfFire;
	};
	
public:
	explicit AiShipBehaviorAttackFighter(AiShipController & aiShipController);
	explicit AiShipBehaviorAttackFighter(AiShipBehaviorAttack const & sourceBehavior);
	virtual ~AiShipBehaviorAttackFighter();

	virtual void alter(float timeDelta);

	Vector const & getNextShotPosition_w() const;
	float const getProjectileSpeed() const;
	bool shouldStartToLeash() const;
	bool shouldStartToEvade() const;
	void alterAttackBehavior();
	bool isFacingTarget() const;
	bool isBehindTarget() const;
	bool isTargetInProjectileConeOfFire() const;
	float getDistanceToTargetSquared() const;
	float getTimeSinceLastShot() const;
	bool isNextProjectileShotPerfect() const;
	
private:

	void initializeTimers();
	void calculateEvadeHealthPercent();
	int selectWeaponToFire(bool const includeMissles);
	void calculateNextProjectileShotPerfect();
	void calculateNextShotPosition_w();
	void calculateWeaponProjectileSpeed();

	Maneuver * m_currentManeuver;
	float m_timeDelta;
	Timer m_projectileTimer;
	Timer m_missileLockOnTimer;
	Timer m_missileFireDelayTimer;
	Timer m_hitDecayTimer;
	int m_stateFlags;
	AiAttackTargetInformation * const m_targetInfo;
	Vector m_missRadians;
	int m_lastManeuverState;
	int m_maneuverFlags;
	int m_lastManeuverFlags;
	float m_lastEvadeHealthPercent;
	float m_evadeHealthPercent;
	float m_timeSinceLastShot;
	float m_projectileSpeed;
	bool m_nextProjectileShotPerfect;
	Vector m_shotErrorPosition_l;
	Vector m_nextShotPosition_w;
		
private: // Disabled

	AiShipBehaviorAttackFighter();
	AiShipBehaviorAttackFighter(AiShipBehaviorAttackFighter const &);
	AiShipBehaviorAttackFighter &operator =(AiShipBehaviorAttackFighter const &);

	friend class Maneuver;
};

// ======================================================================

#endif //INCLUDED_AiShipBehaviorAttackFighter_H
