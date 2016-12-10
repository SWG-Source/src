// ======================================================================
//
// AiShipBehaviorAttackFighter_Maneuver.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorAttackFighter_Maneuver_H
#define	INCLUDED_AiShipBehaviorAttackFighter_Maneuver_H

// ======================================================================

#include "serverGame/AiShipBehaviorAttackFighter.h"

class AiShipController;

//-----------------------------------------------------------------------

class AiShipBehaviorAttackFighter::Maneuver
{
public:

	enum FighterManeuver
	{
		FM_none,
		FM_chase,
		FM_evade,
		FM_leash
	};

	enum ManeuverFlags
	{
		MF_targetInAttackRange			= 0x00000001,
		MF_targetIsTooClose				= 0x00000002,
		MF_missileLockedOnMe			= 0x00000004
	};
	
	Maneuver(FighterManeuver fm, AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo, float const maxTime, bool updateThrottle = true);
	virtual ~Maneuver();
	
	Vector const & getDesiredPosition_w() const;
	void setDesiredPosition_w(Vector const & position);
	
	AiAttackTargetInformation const & getTargetInfo() const;

	bool isDesiredPositionBehindMe() const;
	
	// Path management.
	class Path;
	typedef std::vector<Path*> PathList;
	void addPath(Path * path);
	Path * getCurrentPath();
	bool selectNextPath();
	Path * addEmptyPath();
	int getCurrentPathIndex() const;
	
	// Return positive if it should continue.
	virtual bool alter(float timeDelta, int stateFlags);
	
	virtual char const * const getFighterManeuverString() const = 0;
	virtual FighterManeuver getFighterManeuver() const;
	
#ifdef _DEBUG
	int getSequenceId() const;
#endif // _DEBUG
	
	public:
		static Maneuver * createManeuver(FighterManeuver const manueverType, AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo);
		static Maneuver * createManeuverChase(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo);
		static Maneuver * createManeuverEvade(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo);
		static Maneuver * createManeuverLeash(AiShipBehaviorAttackFighter & aiShipBehaviorAttack, AiAttackTargetInformation const & targetInfo);
		
	protected:
		virtual bool alterPath(float const timeDelta);
		virtual void alterThrottle(float const timeDelta);
		virtual bool alterInternal(float const timeDelta);
		
		virtual void modifyController(float const timeDelta);
		
		AiShipController & getAiShipController();
		AiShipController const & getAiShipController() const;
		
		AiShipBehaviorAttackFighter & m_aiShipBehaviorAttack;
		AiAttackTargetInformation const & m_targetInfo;
		PathList * const m_pathList;
		PathList::iterator m_currentPath;
		int m_currentPathNode;
		int m_abortFlags;
		bool m_updateThrottle;
		float m_throttleValue;
		Vector m_desiredPosition_w;
		FighterManeuver m_maneuverCategory; 
		
#ifdef _DEBUG
		int m_sequenceId;
#endif // _DEBUG
		
private:

	Timer m_expirationTimer;

	// Disabled

	Maneuver();
	Maneuver(Maneuver const &);
	Maneuver const & operator=(Maneuver const & rhs);
};

// ======================================================================

#endif // INCLUDED_AiShipBehaviorAttackFighter_Maneuver_H
