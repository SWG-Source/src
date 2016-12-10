// ======================================================================
//
// AiMovementPathFollow.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementPathFollow_H
#define	INCLUDED_AiMovementPathFollow_H

#include "serverGame/AiMovementWaypoint.h"

typedef std::list<AiLocation> AiPath;
class ServerPathBuilder;

// ======================================================================

class AiMovementPathFollow : public AiMovementWaypoint
{
public:

	explicit AiMovementPathFollow( AICreatureController * controller );
	AiMovementPathFollow( AICreatureController * controller, 
	                      Archive::ReadIterator & source );

	virtual ~AiMovementPathFollow();

	// ----------
	// from AiMovementBase

	virtual void               pack              ( Archive::ByteStream & target ) const;
	virtual void               alter             ( float time );

	virtual void               getDebugInfo      ( std::string & outString ) const;

	virtual void               clear             ( void );
	virtual void               refresh           ( void );

	// states & triggers
	
	virtual AiStateResult      triggerWaypoint     ( void );
	virtual AiStateResult      triggerPathComplete ( void );

	// waypoints

	virtual bool               hasWaypoint       ( void ) const;
	virtual AiLocation const & getWaypoint       ( void ) const;
	virtual bool               updateWaypoint    ( void );
	virtual void               clearWaypoint     ( void );

	// ----------

	virtual bool               getDecelerate     ( void ) const;

	virtual void               addPathNode       ( AiLocation const & newNode );
	virtual void               popNodeFront      ( void );
	virtual void               popNodeBack       ( void );

	virtual bool               hasPath           ( void ) const;
	virtual int                getPathLength     ( void ) const;
	virtual AiPath const *     getPath           ( void ) const;
	virtual void               swapPath          ( AiPath * newPath );
	virtual void               clearPath         ( void );

protected:

	AiStateResult cutPath ( void );

private:

	AiPath * m_path;
};

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, AiPath & target);
	void put(ByteStream & target, const AiPath & source);
};

// ======================================================================


#endif

