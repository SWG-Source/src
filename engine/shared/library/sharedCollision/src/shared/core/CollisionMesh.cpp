// ======================================================================
//
// CollisionMesh.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionMesh.h"

#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/FloorTri.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Overlap3d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/BoxTree.h"
#include "sharedCollision/FloorLocator.h"

#include "sharedMath/Triangle3d.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Transform.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Range.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/ShapeUtils.h"

#include <vector>
#include <algorithm>
#include <stack>

namespace CollisionMeshNamespace
{

static std::vector<int> gs_tempResultVec;

}

using namespace CollisionMeshNamespace;

// ----------------------------------------------------------------------
// Accessories

int CollisionMesh::FindEdgeDirected	( IndexedTri const & T, int index1, int index2 )
{
	for(int i = 0; i < 3; i++)
	{
		if( (T.getCornerIndex(i)   == index1) &&
			(T.getCornerIndex(i+1) == index2) )
		{
			return i;
		}
	}

	return -1;
}

// ----------

int CollisionMesh::FindEdgeUndirected	( IndexedTri const & T, int index1, int index2 )
{
	for(int i = 0; i < 3; i++)
	{
		if( (T.getCornerIndex(i)   == index1) &&
			(T.getCornerIndex(i+1) == index2) )
		{
			return i;
		}

		if( (T.getCornerIndex(i+1) == index1) &&
			(T.getCornerIndex(i)   == index2) )
		{
			return i;
		}
	}

	return -1;
}

// ----------

void	CollisionMesh::LinkTris( ConnectedTri & A, ConnectedTri & B)
{
	for(int edgeA = 0; edgeA < 3; edgeA++)
	{
		int index1 = A.getCornerIndex(edgeA);
		int index2 = A.getCornerIndex(edgeA+1);

		int edgeB = FindEdgeDirected(B,index2,index1);

		if(edgeB != -1)
		{
			A.setNeighborIndex(edgeA,B.getIndex());
			B.setNeighborIndex(edgeB,A.getIndex());
		}
	}
}

// ----------------------------------------------------------------------

CollisionMesh::CollisionMesh()
: CollisionSurface(),
  m_id(0),
  m_version(1),
  m_boxTree(nullptr),
  m_extent(nullptr),
  m_boundsDirty(true)
{
    m_extent = new SimpleExtent( MultiShape(AxialBox()) );
}

CollisionMesh::~CollisionMesh()
{
	delete m_boxTree;
	m_boxTree = nullptr;

    delete m_extent;
    m_extent = nullptr;
}

// ----------------------------------------------------------------------

Transform const & CollisionMesh::getTransform_o2p ( void ) const
{
	return Transform::identity;
}

Transform const & CollisionMesh::getTransform_o2w ( void ) const
{
	return Transform::identity;
}

float CollisionMesh::getScale ( void ) const
{
	return 1.0f;
}

BaseExtent const * CollisionMesh::getExtent_l ( void ) const
{
	return m_extent;
}

BaseExtent const * CollisionMesh::getExtent_p ( void ) const
{
	return m_extent;
}

// ----------------------------------------------------------------------

void CollisionMesh::clear ( void )
{
	delete m_boxTree;
	m_boxTree = nullptr;
}

// ----------------------------------------------------------------------
// Brute-force vertex merge

void CollisionMesh::merge ( void )
{
	int nVerts = getVertexCount();
	int nTris = getTriCount();

	real epsilon = 0.0000000000001f;

	// ----------
	// A temporary table that we'll use to remap vertex indices when
	// we're done with the merge.

	IntVector vertRemap(nVerts);

	for(int iRemap = 0; iRemap < nVerts; iRemap++)
	{
		vertRemap[iRemap] = static_cast<int>(iRemap);
	}

	// ----------

	for(int iVert = 0; iVert < nVerts; iVert++)
	{
		// If this vertex has already been remapped, skip it.

		if(vertRemap[iVert] != static_cast<int>(iVert)) continue;

		// ----------
		// Otherwise, remap all vertices in the rest of the array that are
		// mergeable with this vertex so that they reference this vertex

		for (int jVert = iVert+1; jVert < nVerts; jVert++)
		{
			Vector const & A = getVertex(iVert);
			Vector const & B = getVertex(jVert);

			if(A == B)
			{
				vertRemap[jVert] = static_cast<int>(iVert);
			}
			else if(A.magnitudeBetween(B) <= epsilon)
			{
				vertRemap[jVert] = static_cast<int>(iVert);
			}
		}
	}

	// ----------
	// Done building the remapping table, remap all our floor tris

	for(int iTri = 0; iTri < nTris; iTri++)
	{
		IndexedTri I = getIndexedTri(iTri);

		int iA = vertRemap[ I.getCornerIndex(0) ];
		int iB = vertRemap[ I.getCornerIndex(1) ];
		int iC = vertRemap[ I.getCornerIndex(2) ];

		I.setCornerIndex( 0, iA );
		I.setCornerIndex( 1, iB );
		I.setCornerIndex( 2, iC );

		setIndexedTri(iTri,I);
	}

	setBoundsDirty(true);
}

// ----------------------------------------------------------------------
// Remove all degenerate triangles from the mesh

int		CollisionMesh::scrub	( void )
{
	IntVector badTris;

	for(int iTri = 0; iTri < getTriCount(); iTri++)
	{
		if(getTriangle(iTri).isDegenerate())
		{
			badTris.push_back(iTri);
		}
	}

	int nBadTris = static_cast<int>(badTris.size());

	deleteTris(badTris);

	setBoundsDirty(true);

	return nBadTris;
}

// ----------------------------------------------------------------------

void CollisionMesh::calcBounds() const
{
    AxialBox bounds;

	for(int i = 0; i < getVertexCount(); i++)
	{
		bounds.add( getVertex(i) );
	}

	m_extent->setShape( MultiShape(bounds) );

	setBoundsDirty(false);
}

// ----------------------------------------------------------------------

void CollisionMesh::setBoundsDirty(bool const dirty) const
{
	bool const finalizeTheData = m_boundsDirty && !dirty;
	m_boundsDirty = dirty;

	if (finalizeTheData)
		const_cast<CollisionMesh *>(this)->finalizeData();
}

// ----------------------------------------------------------------------

AxialBox	CollisionMesh::getBoundingABox		( void ) const
{
	if(m_boundsDirty)
	{
		calcBounds();
	}

	return m_extent->getShape().getAxialBox();
}

// ----------

Sphere	CollisionMesh::getBoundingSphere	( void ) const
{
	if(m_boundsDirty)
	{
		calcBounds();
	}

	return m_extent->getShape().getBoundingSphere();
}

// ----------------------------------------------------------------------

// This has the potential to be a little slow depending on the size of the
// polygon soup.  If that becomes the case we can look at storing the soup
// more efficiently (octree, etc)
bool CollisionMesh::intersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time) const
{
	Vector direction(end - begin);

	bool hit = false;

	if (direction.normalize())
	{
		Ray3d const ray(begin, direction);

		real bestTime = REAL_MAX;
		int bestTriangle = -1;

		int const nTris = getTriCount();

		for(int i = 0; i < nTris; ++i)
		{
			// be nice if getTriangle returned a pointer or a const ref
			// I'm hesitant to change it at this moment -- tmf
			Triangle3d const triangle(getTriangle(i));
			Vector hitPoint;
			real t;

			if (Intersect3d::IntersectRayTriSided(ray, triangle, hitPoint, t))
			{
				if (t < bestTime)
				{
					bestTime = t;
					bestTriangle = i;
				}
				hit = true;
			}
		}

		if (hit)
		{
			if (surfaceNormal)
			{
				*surfaceNormal = getTriangle(bestTriangle).getNormal();
			}
			if (time)
			{
				float const magnitude = (end - begin).magnitude();
				*time = bestTime / magnitude;
			}
		}
	}

	return(hit);
}

bool CollisionMesh::testIntersect        ( Line3d const & L,    int triId ) const { return Overlap3d::TestSided(L,getTriangle(triId)); }
bool CollisionMesh::testIntersect        ( Ray3d const & R,     int triId ) const { return Overlap3d::TestSided(R,getTriangle(triId)); }
bool CollisionMesh::testIntersect        ( Segment3d const & S, int triId ) const { return Overlap3d::TestSided(S,getTriangle(triId)); }

bool CollisionMesh::testIntersectUnsided ( Line3d const & L,    int triId ) const { return Overlap3d::TestUnsided(L,getTriangle(triId)); }
bool CollisionMesh::testIntersectUnsided ( Ray3d const & R,     int triId ) const { return Overlap3d::TestUnsided(R,getTriangle(triId)); }
bool CollisionMesh::testIntersectUnsided ( Segment3d const & S, int triId ) const { return Overlap3d::TestUnsided(S,getTriangle(triId)); }

// ----------------------------------------------------------------------

template<class TestShape>
static inline bool templateTestIntersectSided ( TestShape const & shape, CollisionMesh const & mesh, int triId )
{
	return Overlap3d::TestSided(shape,mesh.getTriangle(triId));
}

template<class TestShape>
static inline bool templateTestIntersectUnsided ( TestShape const & shape, CollisionMesh const & mesh, int triId )
{
	return Overlap3d::TestUnsided(shape,mesh.getTriangle(triId));
}

// ----------

template<class TestShape>
static inline bool templateTestIntersectSided_slow ( TestShape const & shape, CollisionMesh const & mesh )
{
	int nTris = mesh.getTriCount();

	for(int triId = 0; triId < nTris; triId++)
	{
		if(templateTestIntersectSided(shape,mesh,triId))
		{
			return true;
		}
	}

	return false;
}

// ----------

template<class TestShape>
static inline bool templateTestIntersectSided_fast ( TestShape const & shape, CollisionMesh const & mesh )
{
	BoxTree const * tree = mesh.getBoxTree();

	if(!tree)
	{
		return templateTestIntersectSided_slow(shape,mesh);
	}

	// ----------

	IntVector & tempResults = mesh.getTempResultVector();

	tempResults.clear();

	tree->testOverlap(shape,tempResults);

	int resultCount = tempResults.size();

	for(int i = 0; i < resultCount; i++)
	{
		int triId = tempResults[i];

		if(templateTestIntersectSided(shape,mesh,triId))
		{
			return true;
		}
	}

	return false;
}

// ----------

template<class TestShape>
static inline bool templateTestIntersectSided ( TestShape const & shape, CollisionMesh const & mesh )
{
#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG

	if(ConfigSharedCollision::getEnableCollisionValidate())
	{
		bool hitFast = templateTestIntersectSided_fast(shape,mesh);
		bool hitSlow = templateTestIntersectSided_slow(shape,mesh);

		DEBUG_FATAL(hitFast != hitSlow,("CollisionMesh::test - Fast and slow versions disagree\n"));

		return hitFast;
	}
	else
	{
		return templateTestIntersectSided_fast(shape,mesh);
	}

#else

	return templateTestIntersectSided_fast(shape,mesh);

#endif
}

// ----------

bool CollisionMesh::testIntersect ( Line3d const & line ) const         { return templateTestIntersectSided(line,*this); }
bool CollisionMesh::testIntersect ( Ray3d const & ray ) const           { return templateTestIntersectSided(ray,*this); }
bool CollisionMesh::testIntersect ( Segment3d const & segment ) const   { return templateTestIntersectSided(segment,*this); }

// ----------------------------------------------------------------------

void CollisionMesh::makeResult ( bool hit, int id, Vector const & pos, float param, ContactPoint & result ) const
{
	UNREF(hit);

	result = ContactPoint(this,pos,id,param);
}

void CollisionMesh::makeResult ( bool hit, int id, Vector const & pos, float param, LineIntersectResult & result ) const
{
	UNREF(hit);

	result = LineIntersectResult( true, param, ContactPoint(this,pos,id,param) );
}

void CollisionMesh::makeResult ( bool hit, int id, Vector const & pos, float param, bool & result ) const
{
	result = hit;

	UNREF(id);
	UNREF(pos);
	UNREF(param);
}

// ----------

template< class TestShape, class ResultType >
static inline bool templateTestIntersectSided ( TestShape const & shape, CollisionMesh const & mesh, int triId, ResultType & result )
{
	Vector newPos;
	real param;

	bool hit = Intersect3d::IntersectSided(shape,mesh.getTriangle(triId),newPos,param);

	if(hit)
	{
		mesh.makeResult(hit,triId,newPos,param,result);
	}

	return hit;
}

template< class TestShape, class ResultType >
static inline bool templateTestIntersectUnsided ( TestShape const & shape, CollisionMesh const & mesh, int triId, ResultType & result )
{
	Vector newPos;
	real param;

	bool hit = Intersect3d::IntersectUnsided(shape,mesh.getTriangle(triId),newPos,param);

	if(hit)
	{
		mesh.makeResult(hit,triId,newPos,param,result);
	}

	return hit;
}

// ----------

bool CollisionMesh::testIntersect        ( Line3d const & L,    int triId, ContactPoint & result ) const        { return templateTestIntersectSided(L,*this,triId,result); }
bool CollisionMesh::testIntersect        ( Ray3d const & R,     int triId, ContactPoint & result ) const        { return templateTestIntersectSided(R,*this,triId,result); }
bool CollisionMesh::testIntersect        ( Segment3d const & S, int triId, ContactPoint & result ) const        { return templateTestIntersectSided(S,*this,triId,result); }

bool CollisionMesh::testIntersectUnsided ( Line3d const & L,    int triId, ContactPoint & result ) const        { return templateTestIntersectUnsided(L,*this,triId,result); }
bool CollisionMesh::testIntersectUnsided ( Ray3d const & R,     int triId, ContactPoint & result ) const        { return templateTestIntersectUnsided(R,*this,triId,result); }
bool CollisionMesh::testIntersectUnsided ( Segment3d const & S, int triId, ContactPoint & result ) const        { return templateTestIntersectUnsided(S,*this,triId,result); }

bool CollisionMesh::testIntersect        ( Line3d const & L,    int triId, LineIntersectResult & result ) const { return templateTestIntersectSided(L,*this,triId,result); }
bool CollisionMesh::testIntersect        ( Ray3d const & R,     int triId, LineIntersectResult & result ) const { return templateTestIntersectSided(R,*this,triId,result); }
bool CollisionMesh::testIntersect        ( Segment3d const & S, int triId, LineIntersectResult & result ) const { return templateTestIntersectSided(S,*this,triId,result); }

// ----------------------------------------------------------------------

template< class TestShape, class ResultType >
static inline bool templateIntersectAny_slow ( TestShape const & shape, CollisionMesh const & mesh, ResultType & result )
{
	int nTris = mesh.getTriCount();

	for(int i = 0; i < nTris; i++)
	{
		if(templateTestIntersectSided(shape,mesh,i,result))
		{
			return true;
		}
	}

	return false;
}

// ----------

template< class TestShape, class ResultType >
static inline bool templateIntersectAny_fast ( TestShape const & shape, CollisionMesh const & mesh, ResultType & result )
{
	BoxTree const * tree = mesh.getBoxTree();

	if(!tree)
	{
		return templateIntersectAny_slow(shape,mesh,result);
	}

	// ----------

	IntVector & tempResults = mesh.getTempResultVector();

	tempResults.clear();

	tree->testOverlap(shape,tempResults);

	int resultCount = tempResults.size();

	for(int i = 0; i < resultCount; i++)
	{
		int triId = tempResults[i];

		if(templateTestIntersectSided(shape,mesh,triId,result))
		{
			return true;
		}
	}

	return false;
}

// ----------

template< class TestShape, class ResultType >
static inline bool templateIntersectAny ( TestShape const & shape, CollisionMesh const & mesh, ResultType & result )
{
#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG

	if(ConfigSharedCollision::getEnableCollisionValidate())
	{
		bool hitFast = templateIntersectAny_fast(shape,mesh,result);
		bool hitSlow = templateIntersectAny_slow(shape,mesh,result);

		DEBUG_FATAL(hitFast != hitSlow,("CollisionMesh::test - Fast and slow versions disagree\n"));

		return hitFast;
	}
	else
	{
		return templateIntersectAny_fast(shape,mesh,result);
	}

#else

	return templateIntersectAny_fast(shape,mesh,result);

#endif
}

// ----------

bool    CollisionMesh::intersectAny     ( Line3d const & L,    ContactPoint & result ) const    { return templateIntersectAny(L,*this,result); }
bool    CollisionMesh::intersectAny     ( Ray3d const & R,     ContactPoint & result ) const    { return templateIntersectAny(R,*this,result); }
bool    CollisionMesh::intersectAny     ( Segment3d const & S, ContactPoint & result ) const    { return templateIntersectAny(S,*this,result); }

// ----------------------------------------------------------------------

bool CollisionMesh::intersectClosest ( Line3d const & line, ContactPoint & closest ) const
{
	if(!testBounds(line))
	{
		return false;
	}

	// ----------

	FloorLocator closestFront;
	FloorLocator closestBack;

	IGNORE_RETURN( findClosestPair(line,-1,closestFront,closestBack) );

	if(closestFront.isAttached())
	{
		closest = closestFront;
		return true;
	}
	else if(closestBack.isAttached())
	{
		closest = closestBack;
		return true;
	}
	else
	{
		closest = FloorLocator::invalid;
		return false;
	}
}

// ----------

bool CollisionMesh::intersectClosest ( Ray3d const & ray, ContactPoint & closest ) const
{
	Line3d line(ray.getPoint(),ray.getNormal());

	FloorLocator closestFront;
	FloorLocator closestBack;

	IGNORE_RETURN( findClosestPair(line,-1,closestFront,closestBack) );

	if(closestFront.isAttached())
	{
		closest = closestFront;
		return true;
	}
	else
	{
		closest = FloorLocator::invalid;
		return false;
	}
}

// ----------

bool CollisionMesh::intersectClosest ( Segment3d const & seg, ContactPoint & closest ) const
{
	Line3d line(seg.getBegin(),seg.getDelta());

	FloorLocator closestFront;
	FloorLocator closestBack;

	IGNORE_RETURN( findClosestPair(line,-1,closestFront,closestBack) );

	if(closestFront.isAttached() && (closestFront.getOffset() <= 1.0f))
	{
		closest = closestFront;
		return true;
	}
	else
	{
		closest = FloorLocator::invalid;
		return false;
	}
}

// ----------------------------------------------------------------------

template< class TestShape, class ResultType >
static inline int templateIntersectAll_slow ( TestShape const & shape, CollisionMesh const & mesh, std::vector<ResultType> & results )
{
	ResultType result;
	int counter = 0;

	for(int i = 0; i < mesh.getTriCount(); i++)
	{
		if(templateTestIntersectSided(shape,mesh,i,result))
		{
			results.push_back( result );
			counter++;
		}
	}

	return counter;
}

// ----------

template< class TestShape, class ResultType >
static inline int templateIntersectAll_fast ( TestShape const & shape, CollisionMesh const & mesh, std::vector<ResultType> & results )
{
	BoxTree const * tree = mesh.getBoxTree();

	if(!tree)
	{
		return templateIntersectAll_slow(shape,mesh,results);
	}

	// ----------

	IntVector & tempResults = mesh.getTempResultVector();

	tempResults.clear();

	tree->testOverlap(shape,tempResults);

	ResultType result;
	int counter = 0;

	int resultCount = tempResults.size();

	for(int i = 0; i < resultCount; i++)
	{
		int triId = tempResults[i];

		if(templateTestIntersectSided(shape,mesh,triId,result))
		{
			results.push_back( result );
			counter++;
		}
	}

	return counter;
}

// ----------

template< class TestShape, class ResultVector >
static inline int templateIntersectAll ( TestShape const & shape, CollisionMesh const & mesh, ResultVector & results )
{
#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG

	if(ConfigSharedCollision::getEnableCollisionValidate())
	{
		ResultVector tempResults;

		int hitFast = templateIntersectAll_fast(shape,mesh,results);
		int hitSlow = templateIntersectAll_slow(shape,mesh,tempResults);

		DEBUG_FATAL(hitFast != hitSlow,("CollisionMesh::intersectRange - Fast and slow versions disagree\n"));

		return hitFast;
	}
	else
	{
		return templateIntersectAll_fast(shape,mesh,results);
	}

#else

	return templateIntersectAll_fast(shape,mesh,results);

#endif
}

// ----------

int CollisionMesh::intersectAll ( Line3d const & L,    LineIntersectResultVec & results ) const { return templateIntersectAll(L,*this,results); }
int CollisionMesh::intersectAll ( Ray3d const & R,     LineIntersectResultVec & results ) const { return templateIntersectAll(R,*this,results); }
int CollisionMesh::intersectAll ( Segment3d const & S, LineIntersectResultVec & results ) const { return templateIntersectAll(S,*this,results); }

// ----------------------------------------------------------------------

template< class TestShape >
static inline Range templateIntersectRange_slow( CollisionMesh const & mesh, TestShape const & shape )
{
	ContactPoint contact;

	Range range( Range::empty );

	int triCount = mesh.getTriCount();

	for(int i = 0; i < triCount; i++)
	{
		if(templateTestIntersectUnsided(shape,mesh,i,contact))
		{
			range = Range::enclose( range, contact.getOffset() );
		}
	}

	return range;
}

// ----------

template< class TestShape >
static inline Range templateIntersectRange_fast( CollisionMesh const & mesh, TestShape const & shape )
{
	BoxTree const * tree = mesh.getBoxTree();

	if(!tree)
	{
		return templateIntersectRange_slow(mesh,shape);
	}

	// ----------

	IntVector & tempResults = mesh.getTempResultVector();

	tempResults.clear();

	tree->testOverlap(shape,tempResults);

	ContactPoint contact;

	Range range( Range::empty );

	int resultCount = tempResults.size();

	for(int i = 0; i < resultCount; i++)
	{
		int triId = tempResults[i];

		if(templateTestIntersectUnsided(shape,mesh,triId,contact))
		{
			range = Range::enclose( range, contact.getOffset() );
		}
	}

	return range;
}

// ----------

template< class TestShape >
static inline Range templateIntersectRange( CollisionMesh const & mesh, TestShape const & shape )
{
#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG

	if(ConfigSharedCollision::getEnableCollisionValidate())
	{
		Range hitFast = templateIntersectRange_fast(mesh,shape);
		Range hitSlow = templateIntersectRange_slow(mesh,shape);

		DEBUG_FATAL(hitFast != hitSlow,("CollisionMesh::intersectRange - Fast and slow versions disagree\n"));

		return hitFast;
	}
	else
	{
		return templateIntersectRange_fast(mesh,shape);
	}

#else

	return templateIntersectRange_fast(mesh,shape);

#endif
}

Range   CollisionMesh::intersectRange ( Line3d const & L ) const    { return templateIntersectRange(*this,L); }
Range   CollisionMesh::intersectRange ( Ray3d const & R ) const     { return templateIntersectRange(*this,R); }
Range   CollisionMesh::intersectRange ( Segment3d const & S ) const { return templateIntersectRange(*this,S); }

// ----------------------------------------------------------------------

bool    CollisionMesh::findClosestPair  ( FloorLocator const & testLoc,
										  Vector const & down,
										  FloorLocator & outClosestAbove,
										  FloorLocator & outClosestBelow ) const
{
	Line3d line(testLoc.getPosition_l(),-down);
	int ignoreId = testLoc.getId();

	return findClosestPair( line, ignoreId, outClosestAbove, outClosestBelow );
}

// ----------------------------------------------------------------------
// Find the pair of triangles that intersect the line and are closest
// to the line's origin.

bool    CollisionMesh::findClosestPair  ( Line3d const & line,
										  int ignoreId,
  										  FloorLocator & outClosestFront,
										  FloorLocator & outClosestBack ) const
{
	outClosestFront = FloorLocator( this, Vector::zero, -1,  REAL_MAX, 0.0f );
	outClosestBack  = FloorLocator( this, Vector::zero, -1, -REAL_MAX, 0.0f );

	if(!testBounds(line))
	{
		return false;
	}

	static LineIntersectResultVec results;

	results.clear();

	if(intersectAll(line,results))
	{
		// ----------
		// Search the triangles for one that is below the test point

		int resultCount = results.size();

		for(int i = 0; i < resultCount; i++)
		{
			LineIntersectResult & result = results[i];

			ContactPoint & contact = result.m_hitLoc;

			if(contact.getId() == ignoreId) continue;

			// ----------
			// Found a triangle under/above the point.

			if(contact.getOffset() > 0)
			{
				if(contact.getOffset() < outClosestFront.getOffset())
				{
					outClosestFront = contact;
				}
			}
			else
			{
				if(contact.getOffset() > outClosestBack.getOffset())
				{
					outClosestBack = contact;
				}
			}
		}

		return outClosestBack.isAttached() || outClosestFront.isAttached();
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool	CollisionMesh::testBounds	( Line3d const & line ) const
{
	return Overlap3d::Test( line, getBoundingABox() );
}

bool	CollisionMesh::testBounds		( Segment3d const & seg ) const
{
	return Overlap3d::Test( seg, getBoundingABox() );
}

bool	CollisionMesh::testBounds		( Ray3d const & ray ) const
{
	return Overlap3d::Test( ray, getBoundingABox() );
}

// ----------------------------------------------------------------------

void CollisionMesh::transform( CollisionMesh const * sourceMesh, Transform const & tform, float scale )
{
	DEBUG_FATAL( sourceMesh->getVertexCount() != getVertexCount(), ("MeshExtent::transform - Source extent has a different number of vertices"));
	DEBUG_FATAL( sourceMesh->getTriCount() != getTriCount(), ("MeshExtent::transform - Source extent has a different number of indices"));

	// ----------

	for(int i = 0; i < sourceMesh->getVertexCount(); i++)
	{
		Vector newVert = tform.rotateTranslate_l2p( sourceMesh->getVertex(i) * scale);

		setVertex(i,newVert);
	}

	setBoundsDirty(true);
}

// ----------------------------------------------------------------------

bool CollisionMesh::hasBoxTree ( void ) const
{
	return m_boxTree != nullptr;
}

// ----------

void CollisionMesh::buildBoxTree ( void )
{
	typedef std::vector<AxialBox> BoxVec;

	BoxVec tempBoxes;

	for(int i = 0; i < getTriCount(); i++)
	{
		Triangle3d tri = getTriangle(i);

		AxialBox box(tri.getCornerA(), tri.getCornerB());
		box.add(tri.getCornerC());

		// Inflate the box by a centimeter to avoid possible numerical inaccuracy

		box = ShapeUtils::inflate(box,0.01f);

		tempBoxes.push_back(box);
	}

	delete m_boxTree;
	m_boxTree = new BoxTree();

	m_boxTree->build(tempBoxes);
}

// ----------------------------------------------------------------------

IntVector & CollisionMesh::getTempResultVector ( void ) const
{
	return gs_tempResultVec;
}

// ----------------------------------------------------------------------

void CollisionMesh::getVertices ( VectorVector & outVerts ) const
{
	int nVerts = getVertexCount();

	outVerts.clear();
	outVerts.reserve(nVerts);

	for(int i = 0; i < nVerts; i++)
	{
		outVerts.push_back( getVertex(i) );
	}
}

void CollisionMesh::getIndices ( IntVector & outIndices ) const
{
	int nTris = getTriCount();

	outIndices.clear();
	outIndices.reserve(nTris * 3);

	for(int i = 0; i < nTris; i++)
	{
		IndexedTri const & tri = getIndexedTri(i);

		outIndices.push_back(tri.getCornerIndex(0));
		outIndices.push_back(tri.getCornerIndex(1));
		outIndices.push_back(tri.getCornerIndex(2));
	}
}

void CollisionMesh::finalizeData()
{
}
