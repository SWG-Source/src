// ======================================================================
//
// ServerCollisionProperty.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ServerCollisionProperty_H
#define INCLUDED_ServerCollisionProperty_H

#include "sharedCollision/CollisionProperty.h"

class ServerObject;
class SharedObjectTemplate;

// ----------------------------------------------------------------------

class ServerCollisionProperty : public CollisionProperty
{
public:

	ServerCollisionProperty( ServerObject & owner );
	ServerCollisionProperty( ServerObject & owner, SharedObjectTemplate const * objTemplate );

	virtual bool canCollideWith ( CollisionProperty const * otherCollision ) const;

private:
	
	ServerCollisionProperty();
	ServerCollisionProperty( ServerCollisionProperty const & copy );
	ServerCollisionProperty & operator = ( ServerCollisionProperty const & copy );
};

// ======================================================================

#endif
