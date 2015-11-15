// ======================================================================
//
// Floor.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------
//
// ======================================================================

#ifndef INCLUDED_Floor_H
#define INCLUDED_Floor_H

#include "sharedCollision/BaseClass.h"
#include "sharedCollision/CollisionSurface.h"
#include "sharedCollision/MultiList.h"
#include "sharedCollision/CollisionEnums.h"

#include "sharedMath/Sphere.h"

class Appearance;
class AxialBox;
class CellProperty;
class DebugShapeRenderer;
class FloorEdgeId;
class FloorLocator;
class FloorMesh;
class IndexedTriangleList;
class NetworkId;
class Object;
class Ray3d;
class SpatialSubdivisionHandle;
class Sphere;
class Transform;
class Vector;

// ======================================================================

class Floor : public BaseClass, public CollisionSurface
{
public:
	
	Floor( FloorMesh const * pMesh, Object const * owner, Appearance const * appearance, bool objectFloor );
	virtual ~Floor();
	
	// ----------
	// Interface inherited from CollisionSurface
	
	virtual Transform const &   getTransform_o2p    ( void ) const;
	virtual Transform const &   getTransform_o2w    ( void ) const;
	virtual float               getScale            ( void ) const;
	virtual BaseExtent const *  getExtent_l         ( void ) const;
	virtual BaseExtent const *  getExtent_p         ( void ) const;
	
	// ----------
	
	FloorMesh const *   getFloorMesh        ( void ) const;
	Object const *      getOwner            ( void ) const;
	Appearance const *  getAppearance       ( void ) const;
	CellProperty const * getCell            ( void ) const;
	NetworkId const & getId() const;
	
	// ----------
	// Other tests
	
	PathWalkResult      canMove             ( FloorLocator const & worldLoc, Vector const & worldDelta, int ignoreTriId, int ignoreEdge, FloorLocator & result ) const;
	PathWalkResult      moveLocator         ( FloorLocator const & startLoc, Vector const & worldGoal, int ignoreTriId, int ignoreEdge, FloorLocator & outLoc ) const;
	
	bool                findClosestLocation ( Vector const & position_p, FloorLocator & outLoc ) const;
	
	bool                findFloorTri        ( Vector const & position_p, float heightTolerance, bool bAllowJump, FloorLocator & outLoc ) const;
	bool                findFloorTri        ( Vector const & position_p, bool bAllowJump, FloorLocator & outLoc ) const;
	
	bool                dropTest            ( Vector const & position_p, FloorLocator & outLoc ) const;
	bool                dropTest            ( Vector const & position_p, float hopHeight, FloorLocator & outLoc ) const;
	bool                dropTest            ( Vector const & position_p, int triID, FloorLocator & outLoc ) const;
	bool                dropTestBounds      ( Vector const & position_p ) const; 
	bool                segTestBounds       ( Vector const & begin, Vector const & end ) const; 
	
	void                drawDebugShapes     ( DebugShapeRenderer * renderer, bool drawExtent ) const;
	
	bool                findElevatorNeighbors   ( FloorLocator const & testLoc, FloorLocator & outClosestAbove, FloorLocator & outClosestBelow ) const;
	
	bool                findEntryPoint      ( FloorLocator const & startLoc, Vector const & delta, bool useRadius, FloorLocator & outLoc ) const;
	
	bool                intersectClosest    ( Ray3d const & ray, FloorLocator & outLoc ) const;
	bool                intersectClosest    ( Segment3d const & seg, FloorLocator & outLoc ) const;
	bool                intersectClosest    ( Line3d const & line, FloorLocator & outLoc ) const;
	
	// ----------
	
	MultiListHandle &       getFootList         ( void );
	MultiListHandle const & getFootList         ( void ) const;
	
	SpatialSubdivisionHandle * getSpatialSubdivisionHandle ( void );
	void                       setSpatialSubdivisionHandle ( SpatialSubdivisionHandle * newHandle );
	
	// ----------
	
	Sphere              getBoundingSphere_l ( void ) const;
	Sphere              getBoundingSphere_w ( void ) const;
	
	bool                isCellFloor         ( void ) const;
	
	virtual bool        getExtentDirty      ( void ) const;
	virtual void        updateExtent        ( void ) const;
	
	bool                getGoodLocation     ( float radius, Vector & outLoc ) const;
	
	Vector              transform_p2l       ( Vector const & point ) const;
	Vector              transform_l2p       ( Vector const & point ) const;
	
	Vector              rotate_p2l          ( Vector const & dir ) const;
	Vector              rotate_l2p          ( Vector const & dir ) const;
	
	bool                getDistanceUncrossable2d  ( Vector const & position_w, float maxDistance, float & outDistance, FloorEdgeId & outEdgeId ) const;

private:
	
	friend class FloorManager;
	
	// ----------
	// Data
	
	Object const *              m_owner;
	Appearance const *          m_appearance;
	bool                        m_objectFloor;
	FloorMesh const *           m_mesh;             //! The collision mesh for this floor
	MultiListHandle             m_footList;         //! List of footprints on this floor
	MultiListHandle             m_databaseList;     //! List of spatial databases this floor is in
	SpatialSubdivisionHandle *  m_spatialSubdivisionHandle;
	
	mutable BaseExtent *        m_extent;
	mutable bool                m_extentDirty;
	mutable Sphere              m_sphere_l;
	mutable Sphere              m_sphere_w;
	
	// ----------
	// disable copying
	
	Floor( Floor const & copy );
	Floor & operator = ( Floor const & copy );
};

// ----------------------------------------------------------------------

inline FloorMesh const * Floor::getFloorMesh ( void ) const
{
	return m_mesh;
}

// ----------------------------------------------------------------------

inline SpatialSubdivisionHandle * Floor::getSpatialSubdivisionHandle ( void )
{
    return m_spatialSubdivisionHandle;
}

inline void Floor::setSpatialSubdivisionHandle ( SpatialSubdivisionHandle * handle )
{
    m_spatialSubdivisionHandle = handle;
}

// ----------------------------------------------------------------------

inline MultiListHandle & Floor::getFootList ( void )
{
	return m_footList;
}

inline MultiListHandle const & Floor::getFootList ( void ) const
{
	return m_footList;
}

// ----------------------------------------------------------------------

inline bool Floor::isCellFloor ( void ) const
{
    return !m_objectFloor;
}

// ======================================================================

#endif
