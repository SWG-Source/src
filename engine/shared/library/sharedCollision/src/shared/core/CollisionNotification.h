// ======================================================================
//
// CollisionNotification.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_CollisionNotification_H
#define INCLUDED_CollisionNotification_H

#include "sharedObject/ObjectNotification.h"
#include "sharedMath/Vector.h"

// ======================================================================

class CollisionNotification : public ObjectNotification
{
public:

	CollisionNotification();
	virtual ~CollisionNotification();

	static CollisionNotification & getInstance ( void );

	static void purgeQueue ( void );

	virtual int     getPriority                 ( void ) const;

	virtual void    visibilityDataChanged       ( Object & object ) const;
	virtual void    extentChanged               ( Object & object ) const;

	virtual void    addToWorld                  ( Object & object ) const;
	virtual void    removeFromWorld             ( Object & object ) const;

	virtual bool    positionChanged             ( Object & object, bool dueToParentChange, Vector const & oldPosition) const;
	virtual void    rotationChanged             ( Object & object, bool dueToParentChange ) const;
	virtual bool    positionAndRotationChanged  ( Object & object, bool dueToParentChange, Vector const & oldPosition ) const;
	virtual void    cellChanged                 ( Object & object, bool dueToParentChange ) const;

protected:

private:

	CollisionNotification(const CollisionNotification &);
	CollisionNotification &operator =(const CollisionNotification &);
};

// ======================================================================

#endif // #ifndef INCLUDED_CollisionNotification_H
