// ======================================================================
//
// CollisionMesh.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionMesh_H
#define INCLUDED_CollisionMesh_H

// ======================================================================

#include "sharedCollision/CollisionSurface.h"

class Iff;
class IndexedTri;
class ConnectedTri;
class Triangle3d;
class IndexedTriangleList;
class AxialBox;
class SimpleExtent;
class Sphere;
class Vector;
class Line3d;
class Ray3d;
class Segment3d;
class Range;
class BoxTree;
class FloorLocator;

typedef std::vector<Vector>      VectorVector;
typedef std::vector<int>         IntVector;

// ======================================================================

class CollisionMesh : public CollisionSurface
{
public:
	
	explicit CollisionMesh();
	virtual ~CollisionMesh();
	
	// ----------
	// Interface inherited from CollisionSurface
	
	virtual Transform const &   getTransform_o2p    ( void ) const;
	virtual Transform const &   getTransform_o2w    ( void ) const;
	virtual float               getScale            ( void ) const;

	virtual BaseExtent const *  getExtent_l         ( void ) const;
	virtual BaseExtent const *  getExtent_p         ( void ) const;
	
	// ----------
	
	virtual void                clear               ( void );
	virtual void                merge               ( void );
	virtual int                 scrub               ( void );
	
	virtual void                calcBounds          ( void ) const;
	
	virtual bool                getBoundsDirty      ( void ) const;
	virtual void                setBoundsDirty      ( bool dirty ) const;
	
	virtual AxialBox            getBoundingABox     ( void ) const;
	virtual Sphere              getBoundingSphere   ( void ) const;
	
	// ----------
	// Basic interface - accessors
	
	virtual int                 getVertexCount  ( void ) const = 0;
	virtual Vector const &      getVertex       ( int whichVertex ) const = 0;
	virtual void                setVertex       ( int whichVertex, Vector const & newPoint ) = 0;
	
	virtual int                 getTriCount     ( void ) const = 0;
	virtual IndexedTri const &  getIndexedTri   ( int whichTri ) const = 0;
	virtual void                setIndexedTri   ( int whichTri, IndexedTri const & newTri ) = 0;
	
	virtual Triangle3d          getTriangle     ( int whichTri ) const = 0;
	
	virtual void                deleteVertex    ( int whichVertex ) = 0;
	virtual void                deleteVertices  ( IntVector const & vertIndices ) = 0;

	virtual void                deleteTri       ( int whichTri ) = 0;
	virtual void                deleteTris      ( IntVector const & triIndices ) = 0;
	
	// ----------
	// Single-triangle intersection methods

	virtual bool intersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time ) const;

	virtual bool                testIntersect           ( Line3d const & L,    int triId ) const;
	virtual bool                testIntersect           ( Ray3d const & R,     int triId ) const;
	virtual bool                testIntersect           ( Segment3d const & S, int triId ) const;
	
	virtual bool                testIntersectUnsided    ( Line3d const & L,    int triId ) const;
	virtual bool                testIntersectUnsided    ( Ray3d const & R,     int triId ) const;
	virtual bool                testIntersectUnsided    ( Segment3d const & S, int triId ) const;
	
	virtual bool                testIntersect           ( Line3d const & L,    int triId, ContactPoint & outLoc ) const;
	virtual bool                testIntersect           ( Ray3d const & R,     int triId, ContactPoint & outLoc ) const;
	virtual bool                testIntersect           ( Segment3d const & S, int triId, ContactPoint & outLoc ) const;
	
	virtual bool                testIntersectUnsided    ( Line3d const & L,    int triId, ContactPoint & outLoc ) const;
	virtual bool                testIntersectUnsided    ( Ray3d const & R,     int triId, ContactPoint & outLoc ) const;
	virtual bool                testIntersectUnsided    ( Segment3d const & S, int triId, ContactPoint & outLoc ) const;
	
	virtual bool                testIntersect           ( Line3d const & L,    int triId, LineIntersectResult & result ) const;
	virtual bool                testIntersect           ( Ray3d const & R,     int triId, LineIntersectResult & result ) const;
	virtual bool                testIntersect           ( Segment3d const & S, int triId, LineIntersectResult & result ) const;
	
	// ----------
	// Whole-mesh intersection methods
	
	// Search for any intersection with the mesh, return true if there is one

	virtual bool                testIntersect   ( Line3d const & line ) const;
	virtual bool                testIntersect   ( Ray3d const & ray ) const;
	virtual bool                testIntersect   ( Segment3d const & seg ) const;

	// Search for any point of intersection with the linear primitive
	
	virtual bool                intersectAny    ( Line3d const & line, ContactPoint & outLoc ) const;
	virtual bool                intersectAny    ( Ray3d const & ray, ContactPoint & outLoc ) const;
	virtual bool                intersectAny    ( Segment3d const & seg, ContactPoint & outLoc ) const;
	
	// Search for the point of intersection with the linear primitive that has the minimum
	// parametric intersection time.
	
	virtual bool                intersectClosest( Line3d const & line, ContactPoint & outLoc ) const;
	virtual bool                intersectClosest( Ray3d const & ray, ContactPoint & outLoc ) const;
	virtual bool                intersectClosest( Segment3d const & seg, ContactPoint & outLoc) const;
	
	// Find all intersection points with the linear primitive. Return value is the number of hits
	
	virtual int                 intersectAll    ( Line3d const & line, LineIntersectResultVec & results ) const;
	virtual int                 intersectAll    ( Ray3d const & ray, LineIntersectResultVec & results ) const;
	virtual int                 intersectAll    ( Segment3d const & seg, LineIntersectResultVec & results ) const;
	
	// Return the parametric intersection time range with the linear primitive
	
	virtual Range               intersectRange  ( Line3d const & line ) const;
	virtual Range               intersectRange  ( Ray3d const & ray ) const;
	virtual Range               intersectRange  ( Segment3d const & seg ) const;

	// ----------
	
	virtual bool                findClosestPair ( Line3d const & line,
	                                              int ignoreId,
	                                              FloorLocator & outClosestFront,
	                                              FloorLocator & outClosestBack ) const;
	
	virtual bool                findClosestPair ( FloorLocator const & testLoc, 
	                                              Vector const & down,
	                                              FloorLocator & outClosestAbove,
	                                              FloorLocator & outClosestBelow ) const;
	
	
	// ----------
	
	virtual bool                testBounds      ( Line3d const & line ) const;
	virtual bool                testBounds      ( Ray3d const & ray ) const;
	virtual bool                testBounds      ( Segment3d const & seg ) const;
	
	// ----------

	virtual void                transform       ( CollisionMesh const * sourceMesh, Transform const & tform, float scale );

	virtual bool                hasBoxTree      ( void ) const;
	virtual void                buildBoxTree    ( void );
	
	virtual BoxTree const *     getBoxTree      ( void ) const;

	IntVector &                 getTempResultVector ( void ) const;

	void                        getVertices     ( VectorVector & outVerts ) const;
	void                        getIndices      ( IntVector & outIndices ) const;
	
	// ----------
	// Utility functions
	
	//@todo - These could be moved elsewhere
	
	static int      FindEdgeDirected     ( IndexedTri const & T, int index1, int index2 );
	static int      FindEdgeUndirected   ( IndexedTri const & T, int index1, int index2 );
	static void     LinkTris             ( ConnectedTri & A, ConnectedTri & B );

	void makeResult ( bool hit, int id, Vector const & pos, float param, ContactPoint & result ) const;
	void makeResult ( bool hit, int id, Vector const & pos, float param, LineIntersectResult & result ) const;
	void makeResult ( bool hit, int id, Vector const & pos, float param, bool & result ) const;

protected:

	// Disable copying
	
	CollisionMesh(const CollisionMesh &source);
	CollisionMesh & operator =(const CollisionMesh &source);

private:

	virtual void finalizeData();

protected:

	int             m_id;
	int             m_version;
	BoxTree *       m_boxTree;

	mutable SimpleExtent *  m_extent;

private:

	mutable bool            m_boundsDirty;
};

// ----------------------------------------------------------------------

inline bool CollisionMesh::getBoundsDirty ( void ) const
{
	return m_boundsDirty;
}

// ----------

inline BoxTree const * CollisionMesh::getBoxTree ( void ) const
{
	return m_boxTree;
}

// ======================================================================

#endif
