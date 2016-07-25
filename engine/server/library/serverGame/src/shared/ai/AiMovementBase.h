// ======================================================================
//
// AiMovementBase.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AiMovementBase_H
#define INCLUDED_AiMovementBase_H

#include <memory>

class AICreatureController;
class AiDebugString;
class AiLocation;
class Vector;

class AiMovementFace;
class AiMovementFlee;
class AiMovementFollow;
class AiMovementLoiter;
class AiMovementMove;
class AiMovementPathFollow;
class AiMovementPatrol;
class AiMovementSwarm;
class AiMovementWander;

namespace Archive
{
	class ByteStream;
	class ReadIterator;
}

#define CHANGE_STATE(A) { setState(reinterpret_cast<StateAlterFunction>(&A),#A); }
#define CAST_STATE(A) reinterpret_cast<StateAlterFunction>(&A)
#define INITIAL_STATE(A) { m_stateFunction = reinterpret_cast<StateAlterFunction>(&A); m_stateName = #A; }

#define SETUP_SYNCRONIZED_STATE( A ) \
	if (m_stateName == #A)\
		m_stateFunction = reinterpret_cast<StateAlterFunction>(&A);\
	if (m_pendingName == #A)\
		m_pendingFunction = reinterpret_cast<StateAlterFunction>(&A);

enum AiStateResult
{
	ASR_Done,
	ASR_Continue,
};

// ----------
// duped in base_class.java	(as MOVEMENT_*******)
enum AiMovementType
{
	AMT_idle,
	AMT_loiter,
	AMT_wander,
	AMT_follow,
	AMT_flee,
	AMT_move,
	AMT_patrol,
	AMT_face,
	AMT_swarm,
	AMT_invalid,
};

	
// ======================================================================

class AiMovementBase
{
public:

	static char const * getMovementString(AiMovementType const aiMovementType);

public:

	explicit AiMovementBase(AICreatureController * controller);
	AiMovementBase(AICreatureController * controller, Archive::ReadIterator & source);
	virtual ~AiMovementBase();
	
	typedef AiStateResult(AiMovementBase::*StateAlterFunction)(float time);

	virtual AiMovementType getType() const = 0;
	virtual AiMovementFace * asAiMovementFace();
	virtual AiMovementFlee * asAiMovementFlee();
	virtual AiMovementFollow * asAiMovementFollow();
	virtual AiMovementLoiter * asAiMovementLoiter();
	virtual AiMovementMove * asAiMovementMove();
	virtual AiMovementPatrol * asAiMovementPatrol();
	virtual AiMovementSwarm * asAiMovementSwarm();
	virtual AiMovementWander * asAiMovementWander();
	
	// ----------
	
	virtual float getDefaultAlterTime() const;
	virtual void  pack                ( Archive::ByteStream & target ) const = 0;
	virtual void alter(float time);
	
	virtual bool getHibernateOk() const;

	virtual void getDebugInfo(std::string & outString) const;

	virtual void endBehavior();

	virtual void clear();

	virtual void refresh();

	// state stuff
	void setState(StateAlterFunction newState, char const * stateName);
	void applyStateChange();

	// utilities that call up through AiCreatureController

	virtual void debugSpeak(char const * string) const;
	virtual bool reachedLocation(AiLocation const & location) const;
	virtual void warpToLocation(AiLocation const & location);

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

protected:

	AICreatureController * m_controller;
	
	StateAlterFunction      m_stateFunction;
	std::string             m_stateName;
	
	StateAlterFunction      m_pendingFunction;
	std::string             m_pendingName;
};


//-----------------------------------------------------------------------

// define shared pointer template for AiMovementBase
typedef std::shared_ptr<AiMovementBase> AiMovementBasePtr;

#define AiMovementBaseNullPtr AiMovementBasePtr()


// ======================================================================


#endif

