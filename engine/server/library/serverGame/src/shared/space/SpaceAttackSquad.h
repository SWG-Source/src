// ======================================================================
// 
// SpaceAttackSquad.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_SpaceAttackSquad_H
#define INCLUDED_SpaceAttackSquad_H

#include "serverGame/Squad.h"

class AiPilotManager;
class NetworkId;

// ----------------------------------------------------------------------
class SpaceAttackSquad : public Squad
{
public:

	SpaceAttackSquad();
	virtual ~SpaceAttackSquad();

	virtual char const * getClassName() const;

	void addDamageTaken(NetworkId const & attackingUnit, float const damage) const;

	CachedNetworkId const & getPrimaryAttackTarget() const;
	bool isAttacking() const;

	void setInFormation(bool const inFormation);
	bool isInFormation() const;

	void setTooCloseToTarget(bool const tooCloseToTarget);
	bool isTooCloseToTarget() const;

	int getMaxNumberOfUnits() const;
	bool isFull() const;

	void setChaseDistance(float const chaseDistance);
	float getChaseDistance() const;

	AiPilotManager const & getLeaderPilotManagerInfo() const;

	float getProjectileAttackRange() const;
	float getWeaponAttackRange() const;

	virtual void assignNewLeader();

	int getNumberOfShotsToFire() const;
	int getTotalShotsFired() const;
	void clearShotsFired();
	void increaseShotsFired();

protected:

	virtual void onAddUnit(NetworkId const & unit);
	virtual void onRemoveUnit();
	virtual void onNewLeader(NetworkId const & oldLeader);
	virtual void onSetUnitFormationPosition_l(NetworkId const & unit, Vector const & position_l);

private:

	void calculateAttackRanges();
	void calculateAverageVelocity();
	void calculateNumberOfShotsToFirePerUnit();

	bool m_inFormation;
	bool m_tooCloseToTarget;
	int m_maxNumberOfUnits;
	float m_chaseDistance;
	float m_projectileAttackRange;
	float m_weaponAttackRange;
	int m_totalShotsFired;

	// Disabled

	SpaceAttackSquad(SpaceAttackSquad const &);
	SpaceAttackSquad & operator =(SpaceAttackSquad const &);
};

// ======================================================================

#endif // INCLUDED_SpaceAttackSquad_H
