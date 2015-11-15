// ======================================================================
//
// FloorLocator.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FloorLocator_H
#define INCLUDED_FloorLocator_H

#include "sharedMath/Vector.h"
#include "sharedCollision/ContactPoint.h"
#include "sharedCollision/CollisionEnums.h"
#include "sharedCollision/CollisionUtils.h"

class CellProperty;
class CollisionSurface;
class Floor;
class FloorMesh;
class FloorTri;
class Iff;
class Transform;

// ======================================================================

class FloorLocator : public ContactPoint
{
public:

	FloorLocator();

	FloorLocator( Floor const * floor, Vector const & localPoint, int triId, float offset, float radius );
	FloorLocator( CollisionSurface const * mesh, Vector const & point, int triId, float offset, float radius );

	FloorLocator( Floor const * floor, Vector const & localPoint );

	FloorLocator( CollisionSurface const * mesh, Vector const & point );

	FloorLocator( Vector const & point, float radius );

	FloorLocator( FloorLocator const & copy );

	~FloorLocator();

	FloorLocator & operator = ( ContactPoint const & contact );

	static FloorLocator invalid;

	// ----------

	Vector               getPosition_p ( void ) const;
	Vector               getPosition_p ( CellProperty const * relativeCell ) const;
	void                 setPosition_p ( Vector const & newPos );

	Vector               getPosition_w ( void ) const;

	bool                 isValid       ( void ) const;
	void                 setValid      ( bool valid );

	Floor const *        getFloor      ( void ) const;
	void                 setFloor      ( Floor const * newFloor );

	FloorMesh const *    getFloorMesh  ( void ) const;
	void                 setFloorMesh  ( FloorMesh const * newFloorMesh );

	FloorTri const &     getFloorTri   ( void ) const;

	CellProperty *       getCell       ( void );
	CellProperty const * getCell       ( void ) const;

	float                getRadius     ( void ) const;
	void                 setRadius     ( float newRadius );

	float                getTime       ( void ) const;
	void                 setTime       ( float newTime );

	int                  getTriId      ( void ) const;
	void                 setTriId      ( int triId );

	int                  getEdgeId     ( void ) const;
	void                 setEdgeId     ( int edgeId );

	int                  getHitTriId   ( void ) const;
	void                 setHitTriId   ( int hitTriId );

	int                  getHitEdgeId  ( void ) const;
	void                 setHitEdgeId  ( int hitEdgeId );

	Vector const &       getContactNormal ( void ) const;
	void                 setContactNormal ( Vector const & normal );

	Vector               getSurfaceNormal_l ( void ) const;
	Vector               getSurfaceNormal_p ( void ) const;
	Vector               getSurfaceNormal_w ( void ) const;

	void                 attachTo      ( FloorMesh const * floorMesh, int triId );
	void                 attachTo      ( FloorMesh const * floorMesh, int triId, int edgeId );

	// ----------

	bool    isFallthrough     ( void ) const;
	bool    isAttachedTo      ( Floor const * pFloor ) const;

	// ----------

	void    read_0000         ( Iff & iff );
	void    write             ( Iff & iff ) const;

	void    detach            ( void );
	void    reattach          ( void );

	void    snapToFloor       ( void );


private:

	bool          m_valid;
	Floor const * m_floor;
	float         m_radius;
	float         m_time;
	int           m_edgeId;
	int           m_hitTriId;
	int           m_hitEdgeId;
	Vector        m_contactNormal;
};

// ----------------------------------------------------------------------

inline bool FloorLocator::isValid ( void ) const
{
	return m_valid;
}

inline void FloorLocator::setValid ( bool valid )
{
	m_valid = valid;
}

// ----------

inline float FloorLocator::getRadius ( void ) const
{
	return m_radius;
}

inline void FloorLocator::setRadius ( float newRadius )
{
	NAN_CHECK(newRadius);

	m_radius = newRadius;
}

// ----------

inline float FloorLocator::getTime ( void ) const
{
	return m_time;
}

inline void FloorLocator::setTime ( float newTime )
{
	NAN_CHECK(newTime);

	m_time = newTime;
}

// ----------

inline int FloorLocator::getTriId ( void ) const
{
	return getId();
}

inline void FloorLocator::setTriId ( int newId )
{
	setId(newId);
}

// ----------

inline int FloorLocator::getEdgeId ( void ) const
{
	return m_edgeId;
}

inline void FloorLocator::setEdgeId ( int newId )
{
	m_edgeId = newId;
}

// ----------

inline int FloorLocator::getHitTriId ( void ) const
{
	return m_hitTriId;
}

inline void FloorLocator::setHitTriId ( int hitTriId )
{
	m_hitTriId = hitTriId;
}

// ----------

inline int FloorLocator::getHitEdgeId ( void ) const
{
	return m_hitEdgeId;
}

inline void FloorLocator::setHitEdgeId ( int hitEdgeId )
{
	m_hitEdgeId = hitEdgeId;
}

// ----------

inline Vector const & FloorLocator::getContactNormal ( void ) const
{
	return m_contactNormal;
}

inline void FloorLocator::setContactNormal ( Vector const & contactNormal )
{
	NAN_CHECK(contactNormal);

	m_contactNormal = contactNormal;
}

// ----------------------------------------------------------------------

#endif

