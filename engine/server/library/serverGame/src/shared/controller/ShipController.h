// ======================================================================
//
// ShipController.h
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_ShipController_H
#define	INCLUDED_ShipController_H

// ======================================================================

#include "serverGame/TangibleController.h"
#include "sharedFoundation/MessageQueue.h"

class AiShipAttackTargetList;
class AiShipBehaviorDock;
class AiShipController;
class CachedNetworkId;
class NetworkId;
class PlayerShipController;
class ShipDynamicsModel;
class ShipObject;
class ShipTurretTargetingSystem;
class Timer;

// ======================================================================

class ShipController : public TangibleController
{
public:

	typedef std::set<CachedNetworkId> CachedNetworkIdList;
	typedef std::multiset<CachedNetworkId> DockedByList;

	explicit ShipController(ShipObject * newOwner);
	virtual ~ShipController();

	virtual void endBaselines();
	virtual void teleport(Transform const &goal, ServerObject *goalObj);
	virtual void onAddedToWorld();
	virtual void onTargetedByMissile(int const missileId);

	virtual ShipController * asShipController();
	virtual ShipController const * asShipController() const;
	virtual PlayerShipController * asPlayerShipController();
	virtual PlayerShipController const * asPlayerShipController() const;
	virtual AiShipController * asAiShipController();
	virtual AiShipController const * asAiShipController() const;

	Transform const &getTransform() const;
	Vector const &getVelocity() const;
	float getSpeed() const;
	float getYawRate() const;
	float getPitchRate() const;
	float getRollRate() const;

	void respondToCollision(Vector const & deltaToMove_p, Vector const & newReflection_p, Vector const & normalOfSurface_p);

	ShipObject * getShipOwner();
	ShipObject const * getShipOwner() const;

	void addTurretTargetingSystem(ShipTurretTargetingSystem * newSystem);
	void removeTurretTargetingSystem();
	bool hasTurretTargetingSystem() const;
	virtual float getTurretMissChance() const;
	virtual float getTurretMissAngle() const;

	virtual void addAiTargetingMe(NetworkId const & unit);
	void removeAiTargetingMe(NetworkId const & unit);
	CachedNetworkIdList const & getAiTargetingMeList() const;
	AiShipAttackTargetList const & getAttackTargetList() const;
	int getNumberOfAiUnitsAttackingMe() const;

	void setPosition_w(Vector const & position, bool allowOrientation);
	bool face(Vector const & goalPosition_w, float const elapsedTime);
	bool face(Vector const & goalPosition_w, Vector const & up_w, float const elapsedTime);

	void setThrottle(float const position);
	float getThrottle() const;
	float getLargestTurnRadius() const;

	virtual void dock(ShipObject & dockTarget, float const secondsAtDock);
	void unDock();
	bool isDocking() const;
	bool isDocked() const;
	bool isBeingDocked() const;
	void addDockedBy(Object const & unit);
	void removeDockedBy(Object const & unit);
	DockedByList const & getDockedByList() const;

	virtual bool addDamageTaken(NetworkId const & attackingUnit, float const damage, bool const verifyAttacker);
	virtual bool removeAttackTarget(NetworkId const & targetUnit);
	void clearEnemyCheckQueuedFlag();
	virtual bool shouldCheckForEnemies() const = 0;
	void onAttackTargetChanged(NetworkId const & target);
	void onAttackTargetLost(NetworkId const & target);
	virtual void onAttackTargetListEmpty();

	void getApproximateFutureTransform(Transform & transform, Vector & velocity, float elapsedTime) const;

protected:

	virtual float realAlter(float elapsedTime);
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);

	ShipDynamicsModel * const m_shipDynamicsModel;
	float m_yawPosition; // [-1...1]
	float m_pitchPosition; // [-1...1]
	float m_rollPosition; // [-1...1]
	float m_throttlePosition;
	AiShipBehaviorDock * m_pendingDockingBehavior;
	AiShipBehaviorDock * m_dockingBehavior;
	AiShipAttackTargetList * const m_attackTargetList;
	Timer * const m_attackTargetDecayTimer;
	bool m_enemyCheckQueued;
	ShipTurretTargetingSystem * m_turretTargetingSystem;

private:

	DockedByList * const m_dockedByList;
	CachedNetworkIdList * const m_aiTargetingMeList;

	void clearAiTargetingMeList();

private:
	
	ShipController();
	ShipController(ShipController const &);
	ShipController & operator=(ShipController const &);

protected:

	virtual void synchronizeTransform();
	virtual void experiencedCollision();
};

// ======================================================================

#endif
