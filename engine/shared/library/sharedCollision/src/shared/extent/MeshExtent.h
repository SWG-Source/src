// ======================================================================
//
// MeshExtent.h
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MeshExtent_H
#define INCLUDED_MeshExtent_H

// ======================================================================

#include "sharedCollision/Extent.h"

#include "sharedMath/AxialBox.h"

class CollisionMesh;
class Iff;
class IndexedTriangleList;
class SimpleCollisionMesh;

// ======================================================================

// Grr, MeshExtent has to derive from Extent because of the ExtentList
// stuff

class MeshExtent : public Extent
{
public:

	MeshExtent();
	MeshExtent( IndexedTriangleList * tris );
	MeshExtent( SimpleCollisionMesh * mesh );
	virtual ~MeshExtent();

	static void         install     ( void );
	static void         remove      ( void );

	static Extent *     build       ( Iff & iff );

	// ----------
	// Virtual Extent interface

	virtual void  load  ( Iff & iff );
	virtual void  write ( Iff & iff ) const;

	virtual bool            contains            ( Vector const & vector ) const;
	virtual bool            contains            ( Vector const & begin, Vector const & end ) const;

	virtual Range rangedIntersect(Line3d const & line) const;
	virtual Range rangedIntersect(Ray3d const & ray) const;
	virtual Range rangedIntersect(Segment3d const & seg) const;

	virtual void            drawDebugShapes     ( DebugShapeRenderer * renderer ) const;

	virtual BaseExtent *    clone               ( void ) const;
	virtual void            copy                ( BaseExtent const * source );
	virtual void            transform           ( BaseExtent const * parent, Transform const & tform, float scale );

	virtual AxialBox        getBoundingBox      ( void ) const;
	virtual Sphere          getBoundingSphere   ( void ) const;

	// ----------

	void attachMesh ( IndexedTriangleList * mesh );

	CollisionMesh *           getMesh     ( void );
	CollisionMesh const *     getMesh     ( void ) const;

protected:

	virtual bool realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const;


	void updateBounds ( void );

	void load_0000 ( Iff & iff );

	static SimpleCollisionMesh *  createMesh          ( IndexedTriangleList * tris );

	// ----------

	SimpleCollisionMesh * m_mesh;

	AxialBox m_box;

private:

	MeshExtent(const MeshExtent &);
	MeshExtent &operator =(const MeshExtent &);
};

// ======================================================================

#endif
