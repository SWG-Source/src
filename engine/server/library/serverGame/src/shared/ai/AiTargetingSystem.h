// ======================================================================
//
// AiTargetingSystem.h
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#ifndef	INCLUDED_AiTargetingSystem_H
#define	INCLUDED_AiTargetingSystem_H

#include "sharedObject/CachedNetworkId.h"

class TangibleObject;

// This class picks the highest priority target for an AI to attack.
//
// ----------------------------------------------------------------------
class AiTargetingSystem
{
public:

	explicit AiTargetingSystem(TangibleObject const & owner);

	NetworkId const & getTarget();

private:

	bool canAttackTarget(TangibleObject const * target);

	TangibleObject const & m_owner;
	CachedNetworkId m_target;
	time_t m_verifyTargetTimer;

	AiTargetingSystem();
	AiTargetingSystem(AiTargetingSystem const & rhs);
	AiTargetingSystem & operator =(AiTargetingSystem const & rhs);
};

//-----------------------------------------------------------------------

#endif	// INCLUDED_AiTargetingSystem_H
