// ======================================================================
//
// CollisionResolve.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionResolve_H
#define INCLUDED_CollisionResolve_H

#include "sharedCollision/CollisionEnums.h"
#include "sharedMath/Vector.h"

class CollisionProperty;
class CellProperty;
class BaseExtent;
class Vector;
class Contact;
class BaseExtent;
class SimpleExtent;
class ComponentExtent;
class DetailExtent;
class Footprint;
class FloorContactShape;
class Floor;

struct ObstacleInfo
{
	ObstacleInfo()
	: m_cell(nullptr),
	  m_extent(nullptr),
	  m_floorContact(nullptr)
	{
	}

	ObstacleInfo ( CellProperty const * cell, SimpleExtent const * extent ) 
	: m_cell(cell),
	  m_extent(extent), 
	  m_floorContact(nullptr) 
	{
	}

	ObstacleInfo ( CellProperty const * cell, FloorContactShape * contact ) 
	: m_cell(cell),
	  m_extent(nullptr), 
	  m_floorContact(contact)
	{
	}

	CellProperty const *    m_cell;
	SimpleExtent const *    m_extent;
	FloorContactShape *     m_floorContact;
};

struct CellPosition
{
	CellPosition( CellProperty const * cell, Vector const & position )
	{
		m_cell = cell;
		m_position = position;
	}

	CellProperty const *  m_cell;
	Vector                m_position;
};


typedef std::vector<CollisionProperty*> ColliderList;
typedef std::vector<ObstacleInfo> ObstacleList;
typedef std::vector<Contact> ContactList;
typedef std::vector<FloorContactShape*> FloorContactList;
typedef std::vector<CellPosition> CellPositionList;

struct MoveSegment
{
	CellProperty const * m_cellA;
	Vector               m_pointA;
	CellProperty const * m_cellB;
	Vector               m_pointB;


	Vector getBegin ( CellProperty const * relativeCell ) const;
	Vector getEnd   ( CellProperty const * relativeCell ) const;

	Vector getDelta ( CellProperty const * relativeCell ) const;

	Vector getDelta_a ( void ) const;

	Vector getDelta_b ( void ) const;

	Vector atParam ( CellProperty const * relativeCell, float time ) const;

	float lengthSquared ( void ) const;
};

// ======================================================================

class CollisionResolve
{
public:

	static void install ( void );
	static void remove  ( void );

	static void resolveAllCollisions ( void );

	static int  getColliderCount     ( void );
	static int  getObstacleCount     ( void );
	static int  getCollisionCount    ( void );
	static int  getBounceCount       ( void );

	static void resetCounters        ( void );

	static ResolutionResult resolveCollisions(CollisionProperty * colliderA, ColliderList const & colliderList);
	
protected:

	static Contact findContactWithObstacle ( CellProperty const * cellA, SimpleExtent const * extentA, MoveSegment const & moveSeg, ObstacleInfo const & obstacle, Footprint * footprint);
	static Contact findContactWithExtent ( CellProperty const * cellA, SimpleExtent const * extentA, MoveSegment const & moveSeg, CellProperty const * cellB, SimpleExtent const * extentB );
	static Contact findContactWithFloor ( FloorContactShape * floorContact, MoveSegment const & moveSeg, Footprint * footprint);

	// ----------

	static ResolutionResult resolveCollisions(CollisionProperty * colliderA, MoveSegment const & moveSeg, ColliderList const & colliderList);
	
	static ResolutionResult resolveCollisions ( CellProperty const * cellA, SimpleExtent * extentA, FloorContactList & floorContactsA,
	                                            MoveSegment const & moveSeg,
	                                            ObstacleList * extentList, 
												CellPositionList & outList, Footprint * footprint );
	
	static bool findFirstContact ( CellProperty const * cellA, SimpleExtent const * extentA, 
	                               MoveSegment const & moveSeg,
	                               ObstacleList * ObstacleList, 
	                               int & outMinIndex, Contact & outContact, Footprint * footprint);

	// ----------

	static void explodeExtent ( CellProperty const * cell, SimpleExtent const * extent, ObstacleList & list );
	static void explodeExtent ( CellProperty const * cell, ComponentExtent const * extent, ObstacleList & list );
	static void explodeExtent ( CellProperty const * cell, DetailExtent const * extent, ObstacleList & list );
	static void explodeExtent ( CellProperty const * cell, BaseExtent const * extent, ObstacleList & list );
	
	static void explodeFootprint ( Footprint * foot, ObstacleList & obstacleList, FloorContactList & floorContacts );

	static void explodeCollider(CollisionProperty * colliderA, ColliderList const & colliderList);

	static void translateEverything ( CellProperty const * cell, SimpleExtent * extent, FloorContactList & floorContacts, MoveSegment const & moveSeg, float moveParam, int ignoreTriId, int ignoreEdge );

};

// ======================================================================

#endif // #ifndef INCLUDED_CollisionResolve_H

