// ======================================================================
//
// AiCreatureController.h
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef	INCLUDED_AiCreatureController_H
#define	INCLUDED_AiCreatureController_H

//-----------------------------------------------------------------------

#include "serverGame/AiCreatureWeaponActions.h"
#include "serverGame/AiMovementBase.h"
#include "serverGame/CreatureController.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedUtility/Location.h"

//----------------------------------------------------------------------

class AiMovementBase;
class AiCreatureData;
class AiDebugString;
class CrcString;
class CreatureObject;
class FloorLocator;
class TargetPathNode;
class WeightedTerrainPathEvaluator;

// ----------------------------------------------------------------------

class AICreatureController : public CreatureController
{
public:

	static void install();

	static AICreatureController * getAiCreatureController(NetworkId const & networkId);
	static AICreatureController * asAiCreatureController(Controller * controller);
	static AICreatureController const * asAiCreatureController(Controller const * controller);

	static float getLeashRadius();

public:
	explicit AICreatureController (CreatureObject * newOwner);
	~AICreatureController   ();

	virtual AICreatureController * asAiCreatureController();
	virtual AICreatureController const * asAiCreatureController() const;

	virtual void handleMessage (int message, float value, const MessageQueue::Data *data, uint32 flags);

	// ----------
	// Script Hooks

	void setCreatureName(std::string const & creatureName);
	PersistentCrcString const & getCreatureName() const; // The "creatureName" column from creatures_datatable.xml

	// Movement commands for scripting use - Action States

	const AiMovementBase * getCurrentMovement() const;

	void                 moveTo                  (CellProperty const * cell, Vector const & target_p, float const radius = 0.0f);
	void                 moveTo                  (Unicode::String const & targetName);
	void                 patrol                  ( std::vector<Location> const & locations, bool random, bool flip, bool repeat, int startPoint);
	void                 patrol                  ( std::vector<Unicode::String> const & locations, bool random, bool flip, bool repeat, int startPoint);
	bool                 wander                  ( float minLength, float maxLength, float minAngle, float maxAngle, float minDelay, float maxDelay );
	void                 loiter                  (CellProperty const * homeCell, Vector const & home_p, float const minDistance, float const maxDistance, float const minDelay, float const maxDelay);
	void                 loiter                  (NetworkId const & target, float const minDistance, float const maxDistance, float const minDelay, float const maxDelay);
	bool                 follow                  ( NetworkId const & target, float minDistance, float maxDistance );
	bool                 follow                  ( NetworkId const & target, Vector const & offset );
	bool                 swarm                   ( NetworkId const & target );
	bool                 swarm                   ( NetworkId const & target, float offset );
	bool                 flee                    ( NetworkId const & target, float minDistance, float maxDistance );
	bool                 faceTo                  ( CellProperty const * targetCell, Vector const & target_p );
	bool                 faceTo                  ( NetworkId const & target );
	void                 stop                    ();

	void                 setMovementRun          ( void );
	void                 setMovementWalk         ( void );

	// ----------

	void                 moveCreature            ( CellProperty const * newCell, Vector const & newPosition, bool testMove );

	void                 getBehaviorDebugInfo    ( std::string & outString ) const;
	void 				 changeMovement          (AiMovementBasePtr newMovement);
	bool 				 hasPendingMovement      () const;
	bool                 suspendMovement         ();
	bool                 resumeMovement          ();
	bool                 hasSuspendedMovement    ();

	float                getSpeed                () const;
	void                 setSpeed                (float const newSpeed);

	bool                 isRunning               ( void ) const;
	float                getAcceleration         ( void ) const;

	void                 moveTowards             ( CellProperty const * cell, Vector const & position, float time );
	bool                 moveAlongGround         ( CellProperty const * cell, Vector const & target, float distance );
	void                 turnToward              ( CellProperty const * cell, Vector const & target );
	float                getDistanceToLocationSquared(CellProperty const * cell, Vector const & target) const;
	bool                 reachedPoint            ( CellProperty const * cell, Vector const & point, float radius ) const;
	void                 warpToTarget            ( void );
	void                 warpToPathpoint         ( void );
	void                 warpToWaypoint          ( void );
	void                 warpTo                  ( CellProperty const * cell, Vector const & position );

	void                 updateMovementType      ( void );

	// ----------

	CreatureObject const * getCreature           ( void ) const;
	CreatureObject *     getCreature             ( void );

	CellProperty const * getCreatureCell         ( void ) const;
	Vector               getCreaturePosition_p   ( void ) const;
	Vector               getCreaturePosition_w   ( void ) const;
	float                getCreatureRadius       ( void ) const;

	// ----------

	void                 triggerScriptsSimple    ( int scriptId );
	void                 triggerScriptsWithTarget( int scriptId, NetworkId const & targetId );
	void                 triggerScriptsWithParam ( int scriptId, float & io_param );

	void                 sleep                   ( float sleepTime );

	virtual bool         shouldHibernate         ( void ) const;
	virtual void         setHibernate            (bool hibernate);

	void                 setHibernationDelay     (float delay);
//	float                getHibernationDelay     () const;

	int                  getStuckCounter         ( void ) const;

	void                 setAuthority            ( bool authority );
	void                 forwardServerObjectSpecificBaselines() const;

	void equipPrimaryWeapon();
	void equipSecondaryWeapon();
	void unEquipWeapons();
	bool hasPrimaryWeapon() const;
	bool hasSecondaryWeapon() const;
	bool usingPrimaryWeapon();
	bool usingSecondaryWeapon();
	NetworkId const & getPrimaryWeapon() const;
	NetworkId const & getSecondaryWeapon() const;

	void setHomeLocation(Location const & homeLocation);
	Location const & getHomeLocation() const;

	void markCombatStartLocation();
	Location const & getCombatStartLocation() const;

	float getMovementSpeedPercent() const; // Percent relative to an unarmed player

	void setFrozen(bool const frozen);
	bool isFrozen() const;

	void addServerNpAutoDeltaVariables(Archive::AutoDeltaByteStream & stream);

	bool isAggressive() const;
	bool isAssist() const;
	bool isStalker() const;
	bool isKiller() const;

	void setRetreating(bool const retreating);
	bool isRetreating() const;
	bool isCombatAi() const;

	void setLogging(bool const logging);
	bool isLogging() const;

	void setMovementPaused(bool const paused);
	bool isMovementPaused() const;

	float getAggroRadius() const;
	float getRespectRadius(NetworkId const & target) const;
	float getAssistRadius() const;

	AiMovementType getMovementType() const;
	AiMovementType getPendingMovementType() const;

	PersistentCrcString const & getCombatAction();
	time_t getKnockDownRecoveryTime() const;

	std::string const getCombatActionsString();

protected:

	static float         getGroundDistance       ( Vector const & A, Vector const & B );

	virtual float        realAlter               ( float time );

private:

#ifdef _DEBUG
	void sendDebugAiToClients(AiDebugString & aiDebugString);
#endif // _DEBUG

	void setPrimaryWeapon(CrcString const & objectTemplate);
	void setSecondaryWeapon(CrcString const & objectTemplate);
	void destroyPrimaryWeapon();
	void destroySecondaryWeapon();

	void applyMovementChange();

	void setupCreatureData(std::string const & creatureName);
	NetworkId createWeapon(char const * const functionName, CrcString const & weaponName, bool const required);
	NetworkId getUnarmedWeapon();
	std::string getDebugInformation() const;
	void sendMessageToAuthServer(GameControllerMessage const gameControllerMessage);
	void sendMessageToAuthServer(GameControllerMessage const gameControllerMessage, MessageQueue::Data * messageQueue);
	void sendMessageToProxyServer(GameControllerMessage const gameControllerMessage, MessageQueue::Data * messageQueue);
	void onCreatureNameChanged(std::string const & creatureName);

	struct CreatureNameChangedCallback
	{
		void modified(AICreatureController & target, std::string const & oldValue, std::string const & newValue, bool local) const;
	};
	friend struct CreatureNameChangedCallback;

	AiMovementBasePtr    m_movement;
	AiMovementBasePtr    m_pendingMovement;
	AiMovementBasePtr    m_suspendedMovement;
	Vector               m_lastStartPosition;
	Vector               m_lastEndPosition;
	float                m_sleepTimer;
	bool                 m_inPathfindingRegion;
	int                  m_stuckCounter;
	bool                 m_authority;
	float m_speed;
	AiCreatureData const * m_aiCreatureData;
	AiCreatureWeaponActions m_primaryWeaponActions;
	AiCreatureWeaponActions m_secondaryWeaponActions;

	Archive::AutoDeltaVariable<bool> m_running;
	Archive::AutoDeltaVariable<Location> m_homeLocation;
	Archive::AutoDeltaVariable<NetworkId> m_primaryWeapon;
	Archive::AutoDeltaVariable<NetworkId> m_secondaryWeapon;
	Archive::AutoDeltaVariable<bool> m_frozen;
	Archive::AutoDeltaVariable<Location> m_combatStartLocation;
	Archive::AutoDeltaVariable<bool> m_retreating;
	Archive::AutoDeltaVariable<time_t> m_retreatingStartTime;
	Archive::AutoDeltaVariable<bool> m_logging;
	Archive::AutoDeltaVariableCallback<std::string, CreatureNameChangedCallback, AICreatureController> m_creatureName;
	Archive::AutoDeltaVariable<float> m_hibernationDelay;
	Archive::AutoDeltaVariable<float> m_hibernationTimer;

#ifdef _DEBUG
	uint32 m_aiDebugStringCrc;
#endif // _DEBUG

	bool   m_invalidTarget;
private:
	AICreatureController();
	AICreatureController(AICreatureController const & rhs);
	AICreatureController & operator =(AICreatureController const & rhs);

};

//-----------------------------------------------------------------------

inline const AiMovementBase * AICreatureController::getCurrentMovement() const
{
	return m_movement.get();
}


//-----------------------------------------------------------------------

#endif	// INCLUDED_AiCreatureController_H
