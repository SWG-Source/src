// ======================================================================
//
// AiShipController.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiShipController_H
#define INCLUDED_AiShipController_H


// ======================================================================

#include "serverGame/AiShipBehaviorAttack.h"
#include "serverGame/ShipAiReactionManager.h"
#include "serverGame/ShipController.h"
#include "sharedFoundation/PersistentCrcString.h"

class AiDebugString;
class AiPilotManager;
class AiShipBehaviorAttack;
class AiShipBehaviorBase;
class AiShipBehaviorDock;
class CachedNetworkId;
class ShipObject;
class SpaceAttackSquad;
class SpacePath;
class SpaceSquad;
class Timer;
struct AiShipPilotData;

// ======================================================================

class AiShipController : public ShipController
{
public:

	enum AttackOrders
	{
		AO_holdFire,
		AO_returnFire,
		AO_attackFreely,
		AO_count
	};

	typedef std::vector<Transform> TransformList;
	typedef std::vector<CachedNetworkId> CachedNetworkIdList;
	typedef std::vector<NetworkId> NetworkIdList;

	static void install();

	static void setClientDebugEnabled(bool const enabled);
	static bool isClientDebugEnabled();

	static AiShipController * getAiShipController(NetworkId const & unit);

	static AiShipController * asAiShipController(Controller * controller);
	static AiShipController const * asAiShipController(Controller const * controller);

	static void setAttackingEnabled(bool const enabled);
	static float getGoalStopDistance();
	static char const * getAttackOrdersString(AttackOrders const attackOrders);

public:

	explicit AiShipController(ShipObject * const owner);
	virtual ~AiShipController();

	virtual AiShipController * asAiShipController();
	virtual AiShipController const * asAiShipController() const;

	virtual void endBaselines();
	virtual void onTargetedByMissile(int const missileId);
	virtual void onAttackTargetListEmpty();

	void setSquad(SpaceSquad * const squad);
	SpaceSquad & getSquad();
	SpaceSquad const & getSquad() const;

	bool isSquadLeader() const;

	void setAttackSquad(SpaceAttackSquad * const attackSquad);
	SpaceAttackSquad & getAttackSquad();
	SpaceAttackSquad const & getAttackSquad() const;

	bool isAttackSquadLeader() const;

	PersistentCrcString const & getShipName() const;
	void setPilotType(std::string const & pilotType);
	std::string const & getPilotType() const;
	AiShipPilotData const * getPilotData(void) const;
	float getPilotSkill() const;
	float getPilotAggression() const;

	int getBehaviorType() const;

	virtual bool addDamageTaken(NetworkId const & attackingUnit, float const damage, bool const verifyAttacker);
	bool addDamageTaken(NetworkId const & attackingUnit, float const damage, bool const verifyAttacker, bool const notifySquad, bool const checkPlayerAttacker);
	CachedNetworkId const & getPrimaryAttackTarget() const;
	ShipObject const * getPrimaryAttackTargetShipObject() const;
	virtual bool removeAttackTarget(NetworkId const & targetUnit);
	bool isValidTarget(ShipObject const & unit) const;
	bool isAttacking() const;

	void setAttackOrders(AttackOrders const attackOrders);
	AttackOrders getAttackOrders() const;

	void setLeashRadius(float const radius);

	void moveTo(SpacePath * const path);
	void addPatrolPath(SpacePath * const path);
	void clearPatrolPath();
	void follow(NetworkId const & followedUnit, Vector const & direction_l, float const distance);
	void idle();
	void track(Object const & target);
	void requestSlowDown();
	virtual void dock(ShipObject & dockTarget, float const secondsAtDock);

	float getAggroRadius() const;
	void setAggroRadius(float const radius);

	void moveTo(Vector const & position_w, float const throttle, float const deltaTime);
	Vector const & getMoveToGoalPosition_w() const;

	Vector getOwnerPosition_w() const;
	Vector getOwnerObjectFrameK_w() const;

	virtual bool shouldCheckForEnemies() const;

	void setFormationPosition_l(Vector const & position_l);
	Vector const & getFormationPosition_l() const;

	void setAttackFormationPosition_l(Vector const & position_l);
	Vector const & getAttackFormationPosition_l() const;

	float getShipRadius() const;
	SpacePath * getPath();
	void setCurrentPathIndex(unsigned int const index);
	unsigned int getCurrentPathIndex() const;
	float calculateThrottleToPosition_w(Vector const & position_w, float const slowDownDistance) const;

	void switchToFighterAttack();
	void switchToBomberAttack();

	AiPilotManager const & getPilotManagerInfo() const;

	void clampPositionToZone(Vector & position_w) const;

	void addExclusiveAggro(NetworkId const & unit);
	void removeExclusiveAggro(NetworkId const & unit);
	bool isExclusiveAggro(CreatureObject const & pilot) const;
	bool hasExclusiveAggros() const;

	ShipAiReactionManager::ShipClass getShipClass() const;

	virtual float getTurretMissChance() const;
	virtual float getTurretMissAngle() const;

#ifdef _DEBUG
	bool debug_forceFighterAttackManeuver(int maneuverType);
#endif // _DEBUG

protected:

	virtual float realAlter(float elapsedTime);

private:

	typedef std::set<CachedNetworkId> CachedNetworkIdSet;

	enum CountermeasureState
	{
		CS_none,
		CS_reacting,
		CS_launching
	};

	void triggerBehaviorChanged(AiShipBehaviorType const newBehavior);
	void triggerEnterCombat(NetworkId const & attackTarget);
	bool hasFunctionalEngines() const;

#ifdef _DEBUG
	void sendDebugAiToClients(AiDebugString & aiDebugString);
#endif // _DEBUG

	AiShipPilotData const * m_pilotData;
	AiShipBehaviorBase * m_pendingNonAttackBehavior;
	AiShipBehaviorBase * m_nonAttackBehavior;
	AiShipBehaviorAttack * m_pendingAttackBehavior;
	AiShipBehaviorAttack * m_attackBehavior;
	PersistentCrcString m_shipName;
	ShipAiReactionManager::ShipClass m_shipClass;
	bool m_requestedSlowDown;
	SpaceSquad * m_squad;
	SpaceAttackSquad * m_attackSquad;
	Vector m_formationPosition_l;
	Vector m_attackFormationPosition_l;
	SpacePath * m_path;
	unsigned int m_currentPathIndex;
	float m_aggroRadius;
	CountermeasureState m_countermeasureState;
	Timer * const m_reactToMissileTimer;
#ifdef _DEBUG
	uint32 m_aiDebugStringCrc;
#endif // _DEBUG
	Vector m_moveToGoalPosition_w;
	int m_lastAttackUpdate;
	AiPilotManager * const m_pilotManagerInfo;
	AttackOrders m_attackOrders;
	CachedNetworkIdSet * const m_exclusiveAggroSet;

	// Disabled

	AiShipController();
	AiShipController(AiShipController const &);
	AiShipController & operator =(AiShipController const &);
};

// ----------------------------------------------------------------------

inline AiShipPilotData const * AiShipController::getPilotData(void) const
{
	return m_pilotData;
}

inline AiPilotManager const & AiShipController::getPilotManagerInfo() const
{
	return *m_pilotManagerInfo;
}

// ======================================================================

#endif
