// ======================================================================
//
// SpatialDatabase.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_SpatialDatabase_H
#define INCLUDED_SpatialDatabase_H

#include "sharedCollision/CollisionEnums.h"

class AxialBox;
class Capsule;
class CellProperty;
class CollisionProperty;
class CollisionSphereAccessor;
class DebugShapeRenderer;
class DoorObject;
class Floor;
class FloorSphereAccessor;
class Line3d;
class MultiShape;
class Object;
class Range;
class Segment3d;
class SpatialSubdivisionHandle;
class Sphere;
class Vector;
class VectorArgb;

typedef std::vector<Floor*> FloorVec;
typedef std::vector<Object*> ObjectVec;
typedef std::vector<Object const *> ObjectConstVec;
typedef std::vector<CollisionProperty*>  ColliderList;


template<typename T, typename U>
class SphereTree;

// ======================================================================

class SpatialDatabase
{
public:
	enum Query
	{
		Q_None      = 0,
		Q_Static    = (1 << 0),
		Q_Dynamic   = (1 << 1),
		Q_Barriers  = (1 << 2),
		Q_Doors     = (1 << 3),
		//Q_Floor   = (1 << X), // not yet implemented
		Q_Physicals = (Q_Static | Q_Dynamic),
		Q_All       = (Q_Static | Q_Dynamic | Q_Barriers | Q_Doors)
	};

	typedef bool (*ShouldTriggerDoorHitCallback)(Object const &mobileObject, DoorObject const &doorObject);

public:

	static void                          setShouldTriggerDoorHitCallback (ShouldTriggerDoorHitCallback callback);
	static ShouldTriggerDoorHitCallback  getShouldTriggerDoorHitCallback ();

public:

	SpatialDatabase();
	~SpatialDatabase();
	
	// ----------
	
	void        update                  ( float time );

	void        drawDebugShapes         ( DebugShapeRenderer * renderer, VectorArgb color ) const;
	
	bool        moveObject              ( CollisionProperty * collision );
		

	// ----------------------------------------------------------------------

	// This interface is meant to be a simpler interface for finding potential
	// collisions with objects in the world.  It is primarily used for the
	// space game and is meant to eventually be the focal point for accessing
	// data from the databases

	// this doesn't support floors, yet...

	bool addObject(Query query, Object * object);
	bool removeObject(Query query, Object * object);

	bool hasObject(int queryMask, Object * object) const;
	int getObjectCount(int queryMask) const;

	void queryFor(int queryMask, CellProperty const * cell_p, bool restrictToSameCell, Capsule const & capsule_p, ColliderList & collidedWith) const;

	// ----------------------------------------------------------------------

	bool        addFloor                ( Floor * floor );
	bool        removeFloor             ( Floor * floor );
	bool        hasFloor                ( Floor * floor ) const;
	
	int         getFloorCount           ( void ) const;
	
	// ----------
	
	bool        queryInteraction        ( CellProperty const * cell, 
	                                      Segment3d const & seg, 
										  Object const * ignoreObject,
										  Object const * & outHitObject,
	                                      float & outHitTime ) const;
	
	bool        queryMaterial           ( CellProperty const * cell,
	                                      Vector const & point,
	                                      MaterialType material ) const;

	bool        queryMaterial           ( CellProperty const * cell,
	                                      Segment3d const & seg,
	                                      MaterialType material ) const;
	
	bool        queryMaterial           ( CellProperty const * cell,
	                                      Sphere const & sphere,
										  Vector const & velocity,
	                                      MaterialType material,
										  bool checkFlora,
										  bool checkFauna,
										  Object const * ignoreObject,
										  float & outCollisionTime ) const;

	bool        queryMaterial           ( CellProperty const * cell,
	                                      Sphere const & sphere,
										  MaterialType material) const;
	
	// ----------
	// The versions that don't take a cell argument query the worldspace
	
	bool        queryStatics            ( Sphere const & sphere, ObjectVec * outList ) const;
	bool        queryStatics            ( AxialBox const & box, ObjectVec * outList ) const;
	bool        queryStatics            ( MultiShape const & shape, ObjectVec * outList ) const;
	bool        queryStatics            ( Line3d const & line, ObjectVec * outList ) const;
	
	bool        queryStatics            ( CellProperty const * cell, MultiShape const & shape, ObjectVec * outList ) const;
	
	bool        queryDynamics           ( Sphere const & sphere, ObjectVec * outList ) const;
	bool        queryDynamics           ( MultiShape const & shape, ObjectVec * outList ) const;

	bool        queryObjects            ( CellProperty const * cell, MultiShape const & shape, ObjectVec * outStatics, ObjectVec * outCreatures ) const;
	bool        queryObjects            ( CellProperty const * cell, MultiShape const & shape, Vector const & delta, ObjectVec * outStatics, ObjectVec * outCreatures ) const;

	bool        queryFloors             ( Sphere const & sphere_w, FloorVec * outList ) const;
	bool        queryFloors             ( Capsule const & capsule_w, FloorVec * outList ) const;
	bool        queryFloors             ( Sphere const & sphere_w, Vector const & delta, FloorVec * outList ) const;
	
	bool        queryFloors             ( CellProperty const * cell, Segment3d const & seg, FloorVec * outList ) const;

	// [1]

	bool        queryCloseStatics       ( Vector const & point, float maxDistance, ObjectVec * outList ) const;
	bool        queryCloseFloors        ( Vector const & point, float maxDistance, FloorVec * outList ) const;
	
	// ----------

	void        updateStaticCollision(CollisionProperty * mobCollision, ColliderList & colliderList);
	void        updateFloorCollision(CollisionProperty * mobCollision, bool attachFromObject );
	void        updateCreatureCollision(CollisionProperty * playerCollision, ColliderList & colliderList);

	// ----------------------------------------------------------------------

	void        pushIgnoreObject        ( Object const * object );
	void        popIgnoreObject         ( void );

protected:
	
	typedef SphereTree<CollisionProperty *, CollisionSphereAccessor> CollisionSphereTree;
	typedef SphereTree<Floor *, FloorSphereAccessor> FloorSphereTree;

	// ----------

	bool        canCollideWithStatics   ( CollisionProperty * collision ) const;
	bool        canCollideWithStatic    ( Object const * pDynamic, Object * pStatic ) const;
	
	bool        canCollideWithCreatures ( CollisionProperty const * collision ) const;
	bool        canCollideWithCreature  ( Object * pPlayer, Object * pCreature ) const;
	
	bool        canWalkOnFloor          ( CollisionProperty * collision ) const;

	bool        checkIgnoreObject       ( Object const * object ) const;

	// ----------

	CollisionSphereTree *   m_staticTree;
	CollisionSphereTree *   m_dynamicTree;
	CollisionSphereTree *   m_doorTree;
	CollisionSphereTree *   m_barrierTree;

	FloorSphereTree *       m_floorTree;

	ObjectConstVec *        m_ignoreStack;

private:

	SpatialDatabase( SpatialDatabase const & copy );
	SpatialDatabase & operator = ( SpatialDatabase const & copy );
};

// ----------
// [1] - queryCloseStatics/Floors -

// If I'm trying to find the closest static object to a given point, I
// can't just use the closest sphere in the sphere tree - since the spheres
// fit their objects loosely, there may be a sphere slightly farther away
// than the closest sphere whose contents are closer than the contents of
// the closest sphere. So, in order to find the actual closest static we
// find the closest sphere, find the farthest distance from the point to
// the closest sphere, query the static tree for everything within
// that far distance, and then look through the results of the query to
// see which object is actually closest. The closest static object _must_ 
// be inside that far distance, and most of the time it'll be the object 
// in the closest sphere (but not always)


// ======================================================================

#endif // #ifndef INCLUDED_SpatialDatabase_H

