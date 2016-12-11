// ======================================================================
//
// CollisionWorld.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_CollisionWorld_H
#define INCLUDED_CollisionWorld_H

#include "sharedCollision/CollisionEnums.h"
#include "sharedCollision/CollisionNotification.h"
#include "sharedCollision/FloorContactShape.h"
#include "sharedCollision/MultiList.h"
#include "sharedMath/Vector.h"

class Camera;
class CellProperty;
class Circle;
class CollisionProperty;
class Floor;
class FloorLocator;
class Footprint;
class Line3d;
class Object;
class Segment3d;
class SpatialDatabase;
class Sphere;
class MultiShape;

typedef std::vector<Object*>             ObjectVec;
typedef std::vector<CollisionProperty*>  ColliderList;
typedef std::vector<Footprint*>          FootprintList;

// ======================================================================

class CollisionWorld
{
public:

	typedef void (*WarpWarningCallback)(Object const &object, Vector const &oldPosition_w, Vector const &newPosition_w, int segmentCount);

	// collision callbacks that are used by non-Footprint objects that are in a 3D environment.
	// these are mostly used by space objects such as ShipObject
	typedef bool (*CanTestCollisionDetectionOnObjectThisFrameFunction)(Object * const object);
	typedef void (*NoCollisionDetectionThisFrameFunction)(Object * const object);
	typedef bool (*CollisionDetectionOnHitFunction)(Object * const object, Object * const wasHitByThisObject);
	typedef bool (*DoCollisionWithTerrainFunction)(Object * const object);

	static void registerCanTestCollisionDetectionOnObjectThisFrame(CanTestCollisionDetectionOnObjectThisFrameFunction);
	static void registerNoCollisionDetectionThisFrame(NoCollisionDetectionThisFrameFunction);
	static void registerDoCollisionDetectionOnHit(CollisionDetectionOnHitFunction);
	static void registerDoCollisionWithTerrain(DoCollisionWithTerrainFunction);

public:
	
	// ----------
	
	static void     install     ( bool serverSide );
	static void     remove      ( void );

	static void     update      ( float time );

	static bool     isUpdating  ( void );
	
	static void     reportCallback ( void );

	static void     setNearWarpWarningCallback(WarpWarningCallback callback);
	static void     setFarWarpWarningCallback(WarpWarningCallback callback);

	// ----------
	
	static void    addObject         ( Object * object );
	static void    removeObject      ( Object * object );
	static void    moveObject        ( Object * object );
	static void    cellChanged       ( Object * object );
	static void    appearanceChanged ( Object * object );


	// ----------

	static void    objectWarped    ( Object * object );
	
	static SpatialDatabase * getDatabase ( void );
	
	// ----------
	// Queries
	
	static bool     query               ( Line3d const & line, ObjectVec * outList );
	
	static bool     query               ( Sphere const & sphere, ObjectVec * outList );
	
	static QueryInteractionResult queryInteraction  ( CellProperty const * cellA, Vector const & pointA, 
	                                                  CellProperty const * cellB, Vector const & pointB, 
	                                                  Object const * ignoreObject, 
	                                                  bool useTerrainLos, 
	                                                  bool generateTerrainLos, 
	                                                  float terrainLosMinDistance, 
	                                                  float terrainLosMaxDistance, 
	                                                  float & outHitTime, Object const * & outHitObject );
	
	// ----------
	// CanMove

	// checkY - if true, the canMove succeeds only if the object would end up at the correct X,Y, and Z coords. if false, only checks X and Z

	static CanMoveResult     canMove    ( Object const * object, Vector const & goal, float moveRadius, bool checkY, bool checkFlora, bool checkFauna );
	static CanMoveResult     canMove    ( CellProperty const * startCell, Vector const & startPos, Vector const & goal, float moveRadius, bool checkY, bool checkFlora, bool checkFauna );

	static bool     findFirstObstacle   ( Object const * object, float radius, Vector const & testPos, Vector const & goalPos, bool testStatics, bool testCreatures, Object const * & outObject );
	static bool     findFirstObstacle   ( Sphere const & sphere, Vector const & goalPos, bool testStatics, bool testCreatures, Object const * & outObject );

	static bool     calcBubble          ( CellProperty const * cell, Vector const & point_p, float maxDistance, float & outRadius );

	static bool     calcBubble          ( CellProperty const * cell, Vector const & point_p, Object const * ignoreObject, float maxDistance, float & outRadius );

	static bool     testClear           ( CellProperty const * cell, Sphere const & sphere_p );

	static bool     canMoveInSpace      ( Object const * object, Sphere const & sphere, Vector const & goal );
	
	// ----------
	
	static bool     findLocators        ( CellProperty const * cell, Vector const & point, std::vector<FloorLocator> & outLocs );
	
	static bool		makeLocator  ( CellProperty const * cell, Vector const & point, FloorLocator & outLoc);

	static void    environmentChanged   ( MultiShape const & shape );

	static void shoveAway(CollisionProperty * const collisionProperty, ColliderList const & colliderList, float time);

	static bool    isServerSide         ( void );
	static void    setServerSide        ( bool serverSide );

	static void handleSceneChange(std::string const & sceneId);

	static Floor const * getFloorStandingOn(Object const & object);

protected:
	// collision callbacks that are used by non-Footprint objects such as ShipObject
	static bool canTestCollisionDetectionOnObjectThisFrame(Object * object);
	static void noCollisionDetectionThisFrame(Object * object);
	static bool doCollisionDetectionOnHit(Object * object, Object * wasHitByThisObject);
	static bool doCollisionWithTerrain(Object * object);

	static void             update        ( CollisionProperty * collision, float time );
	static void             updateSegment ( CollisionProperty * collision, float time );
	static ResolutionResult updateStep    ( CollisionProperty * collision, float time );

	static bool spatialSweepAndResolve(CollisionProperty * collider);

	// ----------
	
	static CanMoveResult    canMove              ( Object const * object, CellProperty const * startCell, FloorLocator const & startLoc, CellProperty const * goalCell, FloorLocator const & goalLoc, bool checkY, bool checkFlora, bool checkFauna );
	static CanMoveResult    canMove              ( Object const * object, CellProperty const * startCell, FloorLocator const & startLoc, Vector const & goalPos, bool checkY, bool checkFlora, bool checkFauna, FloorLocator & endLoc );

	static CanMoveResult    canMoveOnFloor       ( Object const * object, Vector const & startPos, float moveRadius, Floor const * floor, FloorLocator const & goalLoc, bool checkY, bool checkFlora, bool checkFauna, FloorLocator & outLoc, int & outHitPortalId );
	static CanMoveResult    canMoveOnFloor       ( Object const * object, FloorLocator const & startLoc, Floor const * floor, FloorLocator const & goalLoc, bool checkY, bool checkFlora, bool checkFauna, FloorLocator & outLoc, int & outHitPortalId );
	
	static CanMoveResult    canMoveInCell        ( Object const * object, CellProperty const * startCell, FloorLocator const & startLoc, Vector const & goalPos, bool checkY, bool checkFlora, bool checkFauna, FloorLocator & outLoc, int & outHitPortalId );
	static CanMoveResult    canMoveInCell        ( Object const * object, CellProperty const * startCell, FloorLocator const & startLoc, FloorLocator const & goalLoc, bool checkY, bool checkFlora, bool checkFauna, FloorLocator & outLoc, int & outHitPortalId );
	
	// ----------

	static void    updateReportString  ( void );

private:

	CollisionWorld();
	CollisionWorld( CollisionWorld const & copy );
	CollisionWorld & operator = ( CollisionWorld const & copy );
};

// ======================================================================

#endif // #ifndef INCLUDED_CollisionWorld_H

