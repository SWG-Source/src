// ======================================================================
//
// PlayerCreatureController.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef PlayerCreatureController_H
#define PlayerCreatureController_H

// ======================================================================

#include "serverGame/CellObject.h"
#include "serverGame/CreatureController.h"
#include "serverUtility/RecentMaxSyncedValue.h"

// ======================================================================

class MessageQueueTeleportAck;
class MessageQueueObjectMenuRequest;

// ======================================================================

class PlayerCreatureController : public CreatureController
{
public:
	explicit PlayerCreatureController(CreatureObject *newOwner);
	virtual ~PlayerCreatureController();

	static PlayerObject * getPlayerObject(CreatureObject *creature);
	static PlayerObject const * getPlayerObject(CreatureObject const *creature);

	static bool objectMenuRequestCanManipulateObject(CreatureObject const & player, ServerObject const & item, bool & blockedByNoTrade, bool & showPickUpMenu);

	virtual PlayerCreatureController * asPlayerCreatureController();
	virtual PlayerCreatureController const * asPlayerCreatureController() const;

	virtual void   teleport           (Transform const &goal, ServerObject *goalObj);
	virtual void   setAuthoritative   (bool newAuthoritative);
	virtual void   onClientReady      ();
	virtual void   onClientLost       ();

	float getLastSpeed       () const;
	float const getServerSpeedForPlayer() const;
protected:

	virtual float  realAlter (float time);
	virtual void   onMoveFailed ();

	virtual void   updateHibernate();
	virtual bool   shouldHibernate() const;

private:
	PlayerCreatureController(PlayerCreatureController const &);
	PlayerCreatureController &operator=(PlayerCreatureController const &);

private:

	class MoveSnapshot
	{
	public:
		MoveSnapshot();
		MoveSnapshot(Vector const &position_p, CellObject *cell, uint32 syncStamp);
		MoveSnapshot(MoveSnapshot const &rhs);
		MoveSnapshot &operator=(MoveSnapshot const &rhs);

		void              clear          ();
		CellObject *      getCell        ();
		CellObject const *getCell        () const;

		bool            isValid        () const;
		bool            isAllowed      (CreatureObject const &who) const;
		Vector const    getPosition_w  () const;
		Vector const &  getPosition_p  () const;
		uint32          getSyncStamp   () const;

	private:
		bool m_inited;
		Watcher<CellObject> m_cell;
		Vector m_position_p;
		Vector m_position_w;
		uint32 m_syncStamp;
	};

	bool isGod               () const;
	bool isTeleporting       () const;
	void handleTeleportAck   (MessageQueueTeleportAck const &msg);
	void preventMovementUpdates();
	void resyncMovementUpdates();
	bool shouldValidateMove  () const;
	bool checkValidMove      (MoveSnapshot const &m, float reportedSpeed);
	bool isLocationValid     (Vector const &position_w, CellObject const *cell) const;
	void logMoveFailed       (char const *reason);
	bool handleInvalidMove   (char const *reason);
	bool handleMove          (Vector const &position_w, uint32 syncStamp, float reportedSpeed);
	bool handleMove          (Vector const &position_p, NetworkId const &cellObjectId, uint32 syncStamp, float reportedSpeed);
	void updateMaxMoveSpeed  ();
	uint32 getCurSyncStamp   () const;
	void updateMovementType  (float time);
	void handleObjectMenuRequest(MessageQueueObjectMenuRequest const *msg);
	void playerMovedAndNeedsToCancelWarmup( CreatureObject &owner );

	static PlayerObject * getPlayerObjectInternal(CreatureObject const * creature);

protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
	std::set<int>  m_teleportIds;
	MoveSnapshot   m_lastVerifiedMove;
	int64          m_lastTimeSynchronization;
	int            m_lastWeatherIndex;
	Vector         m_lastWeatherWindVelocity_w;
	RecentMaxSyncedValue m_speedMaximum;
	float          m_movingTimeout;
	uint32         m_lastMoveSyncStamp;
	float          m_lastSpeed;
	time_t         m_lastSpeedCheckFailureTime;
	int            m_speedCheckConsecutiveFailureCount;
	bool           m_resendSpeedMax;
};

// ----------------------------------------------------------------------

inline float const PlayerCreatureController::getServerSpeedForPlayer() const
{
	return m_speedMaximum.getLastSetValue();
}

// ======================================================================

#endif // PlayerCreatureController_H

