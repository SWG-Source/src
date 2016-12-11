// ======================================================================
// 
// SpaceSquad.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_SpaceSquad_H
#define INCLUDED_SpaceSquad_H

#include "serverGame/Squad.h"

class AiShipController;
class NetworkId;
class Object;
class ShipObject;
class SpaceAttackSquad;
class SpacePath;
class Vector;

// ----------------------------------------------------------------------
class SpaceSquad : public Squad
{
friend class SpaceSquadManager;

public:

	typedef std::set<SpaceSquad *> SpaceSquadList;

	static void install();

public:

	virtual char const * getClassName() const;
	virtual void alter(float deltaSeconds);

	SpacePath * getPath() const;

	void setCurrentPathIndex(unsigned int const index);

	void idle() const;
	void track(Object const & target) const;
	void moveTo(SpacePath * const path) const;
	void addPatrolPath(SpacePath * const path) const;
	void clearPatrolPath();
	void follow(NetworkId const & followedUnit, Vector const & direction_l, float direction) const;

	bool setGuardTarget(int const squadId);
	SpaceSquad * getGuardTarget();
	SpaceSquadList & getGuardedByList();
	void removeGuardTarget();

	void addDamageTaken(NetworkId const & attackingUnit, float const damage) const;
	virtual void assignNewLeader();
	void assignAttackSquad(AiShipController & unitAiShipController);

	void setGuarding(bool const guarding);
	bool isGuarding() const;

	bool isAttackTargetListEmpty() const;

	Vector getAvoidanceVector(ShipObject const & unit) const;
	Vector const & getLeashAnchorPosition_w() const;

	float getLargestShipRadius() const;
	void refreshPathInfo() const;

protected:

	virtual void onAddUnit(NetworkId const & unit);
	virtual void onRemoveUnit();
	virtual void onNewLeader(NetworkId const & oldLeader);
	virtual void onSetUnitFormationPosition_l(NetworkId const & unit, Vector const & position_l);

private:

	static void remove();

	void setGuardedBy(SpaceSquad & squad);
	void removeGuardedBy(SpaceSquad & squad);
	void updateLargestShipRadius();

#ifdef _DEBUG
	void verifyAttackSquads();
#endif // _DEBUG

	typedef std::list<SpaceAttackSquad *> AttackSquadList;

	SpaceSquad * m_guardTarget;
	SpaceSquadList * const m_guardedByList;
	AttackSquadList * const m_attackSquadList;
	bool m_guarding;
	Vector m_leashAnchorPosition_w;
	Timer m_leashAnchorPositionTimer;

	// Disabled

	SpaceSquad();
	virtual ~SpaceSquad();
	SpaceSquad(SpaceSquad const &);
	SpaceSquad & operator =(SpaceSquad const &);
};

// ======================================================================

#endif // INCLUDED_SpaceSquad_H
