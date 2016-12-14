// ======================================================================
//
// DoorObject.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DoorObject_H
#define INCLUDED_DoorObject_H

#include "sharedObject/Object.h"

#include "sharedCollision/DoorHelper.h"

#include <vector>

class Portal;
class BarrierObject;
class DoorObject;
class NeighborObject;
class CollisionProperty;

typedef std::vector<Vector> VertexList;

// ----------------------------------------------------------------------

class DoorObject : public Object
{
public:

	DoorObject( DoorInfo const & info, Portal * portal );
	virtual ~DoorObject();

	// ----------

	Vector const    getCurrentPos   ( void ) const;

	// ----------
	// Delegated to the door helper

	float           getOpenParam ( void ) const;

	bool            isOpen ( void ) const;
	bool            isClosed ( void ) const;
	bool            isOpening ( void ) const;
	bool            isClosing ( void ) const;

	// ----------

	virtual bool    isPassageAllowed ( void ) const;	// true if I can go through the door
	virtual bool    isAllPassageAllowed ( void ) const; // true if I can go through the door and come back

	// ----------

	virtual float   alter           ( float time );

	virtual float   tween           ( float t ) const;

	virtual void    hitBy           ( CollisionProperty const * collision );

	// ----------

	virtual Portal *        getPortal ( void );
	virtual Portal const *  getPortal ( void ) const;
	virtual void            setPortal ( Portal * newPortal );

	// ----------

	virtual DoorObject *        getNeighbor ( void );
	virtual DoorObject const *  getNeighbor ( void ) const;
	virtual void                setNeighbor ( DoorObject * newNeighbor );

	virtual int                 getNumberOfDrawnDoors ( void );
	virtual Object *            getDrawnDoor ( int index );
	virtual Object *            getBarrier   ( void );

	virtual void                scheduleForAlter();

protected:

	void createAppearance ( DoorInfo const & info );
	void createTrigger    ( DoorInfo const & info );
	void createBarrier    ( DoorInfo const & info );

	// ----------

	DoorHelper       m_doorHelper;
	DoorHelper       m_doorHelper2;

	Vector           m_delta;

	Portal *         m_portal;
	DoorObject *     m_neighbor;

	float            m_spring;
	float            m_smoothness;

	bool             m_draw;

	enum             { MAX_DRAWN_DOORS = 2 };
	Object *         m_drawnDoor[MAX_DRAWN_DOORS];

	BarrierObject *  m_barrier;

	Vector           m_oldDoorPos;

	bool             m_wasOpen;

	const char *     m_effects[4];

	bool             m_isForceField;

	bool const       m_alwaysOpen;

private:

	typedef std::vector<ConstWatcher<Object> >  WatcherObjectVector;

private:

	void  trackHitByObject(Object const &hitByObject);
	void  maintainHitByObjectVector();

	// Disabled.
	DoorObject( DoorObject const & copy );
	DoorObject & operator = ( DoorObject const & copy );

private:

	float const          m_triggerRadiusSquared;
	WatcherObjectVector  m_hitByObjects;

};

// ======================================================================

inline float DoorObject::getOpenParam ( void ) const
{
	return m_doorHelper.getPos();
}

// ----------------------------------------------------------------------

inline bool DoorObject::isOpen ( void ) const
{
	return m_doorHelper.isOpen();
}

// ----------------------------------------------------------------------

inline bool DoorObject::isClosed ( void ) const
{
	return m_doorHelper.isClosed();
}

// ----------------------------------------------------------------------

inline bool DoorObject::isOpening ( void ) const
{
	return m_doorHelper.isOpening();
}

// ----------------------------------------------------------------------

inline bool DoorObject::isClosing ( void ) const
{
	return m_doorHelper.isClosing();
}

// ======================================================================

#endif
