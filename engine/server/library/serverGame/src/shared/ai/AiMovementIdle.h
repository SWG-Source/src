// ======================================================================
//
// AiMovementIdle.h
// copyright 2005 Sony Online Entertainment Inc.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementIdle_H
#define	INCLUDED_AiMovementIdle_H

#include "serverGame/AiMovementBase.h"

class AiDebugString;

// ----------------------------------------------------------------------
class AiMovementIdle : public AiMovementBase
{
public:

	explicit AiMovementIdle(AICreatureController * controller);
	AiMovementIdle(AICreatureController * controller, Archive::ReadIterator & source);
	virtual ~AiMovementIdle();

	virtual AiMovementType getType() const;
	virtual AiMovementIdle * asAiMovementIdle();

	virtual void pack(Archive::ByteStream & target) const;
	virtual void alter(float time);
	
	virtual AiStateResult stateWaiting (float time);

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG
};

// ======================================================================

#endif // INCLUDED_AiMovementIdle_H
