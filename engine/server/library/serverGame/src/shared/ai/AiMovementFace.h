// ======================================================================
//
// AiMovementFace.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementFace_H
#define	INCLUDED_AiMovementFace_H

#include "serverGame/AiMovementBase.h"
#include "serverGame/AiLocation.h"

class ServerObject;


// ======================================================================

class AiMovementFace : public AiMovementBase
{
public:

	explicit AiMovementFace( AICreatureController * controller );
	AiMovementFace( AICreatureController * controller, CellProperty const * targetCell, Vector const & target_p );
	AiMovementFace( AICreatureController * controller, ServerObject const * target );
	AiMovementFace( AICreatureController * controller, Archive::ReadIterator & source );

	virtual ~AiMovementFace();

	virtual AiMovementType getType() const;
	virtual AiMovementFace * asAiMovementFace();

	virtual void  pack  ( Archive::ByteStream & target ) const;
	virtual void  alter ( float time );
	
	virtual AiStateResult stateWaiting ( float time );

	virtual void  getDebugInfo ( std::string & outString ) const;

	AiLocation const & getTarget() const;

protected:

	AiLocation m_target;
};

// ======================================================================

#endif

