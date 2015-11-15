// ======================================================================
//
// SimpleCollisionMesh.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SimpleCollisionMesh_H
#define INCLUDED_SimpleCollisionMesh_H

// ======================================================================

#include "sharedCollision/CollisionMesh.h"

class DebugShapeRenderer;
class CollisionBuckets;

// ======================================================================

class SimpleCollisionMesh : public CollisionMesh
{
public:

	SimpleCollisionMesh();
	SimpleCollisionMesh( IndexedTriangleList * tris );
	virtual ~SimpleCollisionMesh();

	static void setIntersectWithCollisionBuckets(bool const intersectWithCollisionBuckets);

	SimpleCollisionMesh *		clone			( void ) const;

	// ----------
	// Basic interface - accessors

	virtual int                 getVertexCount  ( void ) const;
	virtual Vector const &      getVertex       ( int whichVertex ) const;
	virtual void                setVertex       ( int whichVertex, Vector const & newPoint );

	virtual int                 getTriCount     ( void ) const;
	virtual IndexedTri const &  getIndexedTri   ( int whichTri ) const;
	virtual void                setIndexedTri   ( int whichTri, IndexedTri const & newTri );

	virtual Triangle3d          getTriangle     ( int whichTri ) const;

	virtual void                deleteVertex    ( int whichVertex );
	virtual void                deleteVertices  ( IntVector const & vertIndices );

	virtual void                deleteTri       ( int whichTri );
	virtual void                deleteTris      ( IntVector const & triIndices );

	// ----------

	virtual void                load            ( Iff & iff );
	virtual void                write           ( Iff & iff ) const;

	// ----------
	// Debugging

	virtual void				drawDebugShapes	( DebugShapeRenderer * renderer ) const;

	// ----------
	// Misc

	virtual void				attachTris		( IndexedTriangleList * tris );
	virtual void				copy			( CollisionMesh const * mesh );
	virtual IndexedTriangleList const * getTris	( void ) const;

	virtual void				transform		( CollisionMesh const * sourceMesh, Transform const & tform, float scale );

	virtual bool intersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time ) const;

protected:

	// Disable copying

	SimpleCollisionMesh(const SimpleCollisionMesh &source);
	SimpleCollisionMesh & operator =(const SimpleCollisionMesh &source);

private:

	virtual void finalizeData();

protected:

	IndexedTriangleList *   m_tris;
	CollisionBuckets * m_bucket;
};

// ======================================================================

#endif
