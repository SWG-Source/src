// ======================================================================
//
// AiShipBehaviorBase.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AiShipBehaviorBase_H
#define INCLUDED_AiShipBehaviorBase_H

class AiDebugString;
class AiShipController;

// ----------
// duped in ship_ai.scriptlib

enum AiShipBehaviorType
{
	ASBT_idle = 0,
	ASBT_track,
	ASBT_moveTo,
	ASBT_patrol,
	ASBT_follow,
	ASBT_dock,
	ASBT_invalid
};

// ======================================================================

class AiShipBehaviorBase
{
public:

	static char const * getBehaviorString(AiShipBehaviorType const behavior);

public:

	explicit AiShipBehaviorBase(AiShipController & aiShipController);
	AiShipBehaviorBase(AiShipBehaviorBase const & rhs);
	virtual ~AiShipBehaviorBase() = 0;
	
	virtual void alter(float deltaSeconds) = 0;

	virtual AiShipBehaviorType getBehaviorType() const;

	AiShipController & getAiShipController();
	AiShipController const & getAiShipController() const;

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

protected:

	AiShipController & m_aiShipController;

private:

	// Disabled
	AiShipBehaviorBase();
	AiShipBehaviorBase &operator =(AiShipBehaviorBase const &);
};

// ======================================================================

#endif // INCLUDED_AiShipBehaviorBase_H
