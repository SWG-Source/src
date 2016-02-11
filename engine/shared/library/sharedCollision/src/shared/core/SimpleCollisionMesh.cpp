// ======================================================================
//
// SimpleCollisionMesh.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/SimpleCollisionMesh.h"

#include "sharedCollision/CollisionBuckets.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/FloorTri.h"	// for IndexedTri
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Triangle3d.h"

#include <vector>
#include <algorithm>

namespace SimpleCollisionMeshNamespace
{
	bool s_intersectWithCollisionBuckets = true;
	bool s_renderCollisionBuckets = false;
	std::vector<int> gs_tempResultVec;
}

// ======================================================================

using namespace SimpleCollisionMeshNamespace;

// ======================================================================

SimpleCollisionMesh::SimpleCollisionMesh()
: CollisionMesh()
, m_tris( new IndexedTriangleList() )
, m_bucket(new CollisionBuckets(m_tris))
{
}

SimpleCollisionMesh::SimpleCollisionMesh( IndexedTriangleList * tris )
: CollisionMesh(),
  m_tris( tris )
, m_bucket(new CollisionBuckets(m_tris))
{
}

SimpleCollisionMesh::~SimpleCollisionMesh()
{
	delete m_tris;
	m_tris = nullptr;
	delete m_bucket;
}

SimpleCollisionMesh * SimpleCollisionMesh::clone ( void ) const
{
	return new SimpleCollisionMesh( m_tris->clone() );
}

// ----------
// Basic interface - accessors

int SimpleCollisionMesh::getVertexCount( void ) const
{
	return static_cast<int>(m_tris->getVertices().size());
}

Vector const & SimpleCollisionMesh::getVertex( int whichVertex ) const
{
	return m_tris->getVertices().at(static_cast<unsigned int>(whichVertex));
}

void SimpleCollisionMesh::setVertex( int whichVertex, Vector const & newPoint )
{
	m_tris->getVertices().at(static_cast<unsigned int>(whichVertex)) = newPoint;

	setBoundsDirty(true);
}

int SimpleCollisionMesh::getTriCount( void ) const
{
	return static_cast<int>(m_tris->getIndices().size()) / 3;
}

IndexedTri const & SimpleCollisionMesh::getIndexedTri( int whichTri ) const
{
	// This is yucky, but it works...

	return * reinterpret_cast<IndexedTri const *>( &(m_tris->getIndices().at(static_cast<unsigned int>(whichTri) * 3)) );
}

void SimpleCollisionMesh::setIndexedTri( int whichTri, IndexedTri const & newTri )
{
	std::vector<int> & indices = m_tris->getIndices();

	indices[static_cast<unsigned int>(whichTri) * 3 + 0] = newTri.getCornerIndex(0);
	indices[static_cast<unsigned int>(whichTri) * 3 + 1] = newTri.getCornerIndex(1);
	indices[static_cast<unsigned int>(whichTri) * 3 + 2] = newTri.getCornerIndex(2);

	setBoundsDirty(true);
}

Triangle3d SimpleCollisionMesh::getTriangle( int whichTri ) const
{
	IndexedTri const & T = getIndexedTri(whichTri);

	std::vector<Vector> & verts = m_tris->getVertices();

	unsigned int A = static_cast<unsigned int>(T.getCornerIndex(0));
	unsigned int B = static_cast<unsigned int>(T.getCornerIndex(1));
	unsigned int C = static_cast<unsigned int>(T.getCornerIndex(2));

	return Triangle3d( verts[A], verts[B], verts[C] );
}

void SimpleCollisionMesh::deleteVertex ( int whichVertex )
{
	UNREF(whichVertex);

	FATAL(true,("can't delete vertices yet..."));
}

void SimpleCollisionMesh::deleteVertices ( IntVector const & vertIndices )
{
	UNREF(vertIndices);

	FATAL(true,("can't delete vertices yet..."));
}

void SimpleCollisionMesh::deleteTri ( int whichTri )
{
	std::vector<int> & indices = m_tris->getIndices();

	indices[static_cast<unsigned int>(whichTri) * 3 + 0] = -2;
	indices[static_cast<unsigned int>(whichTri) * 3 + 1] = -2;
	indices[static_cast<unsigned int>(whichTri) * 3 + 2] = -2;

	std::vector<int>::iterator newEnd = std::remove( indices.begin(), indices.end(), -2 );

	if(newEnd != indices.end())
	{
		IGNORE_RETURN(indices.erase(newEnd,indices.end()));
	}

	setBoundsDirty(true);
}

void SimpleCollisionMesh::deleteTris( IntVector const & triIndices )
{
	std::vector<int> & indices = m_tris->getIndices();

	unsigned int indexCount = triIndices.size();

	for(unsigned int i = 0; i < indexCount; i++)
	{
		int base = triIndices[i];

		indices[static_cast<unsigned int>(base) * 3 + 0] = -2;
		indices[static_cast<unsigned int>(base) * 3 + 1] = -2;
		indices[static_cast<unsigned int>(base) * 3 + 2] = -2;
	}

	std::vector<int>::iterator newEnd = std::remove( indices.begin(), indices.end(), -2 );

	if(newEnd != indices.end())
	{
		IGNORE_RETURN(indices.erase(newEnd,indices.end()));
	}

	setBoundsDirty(true);
}

// ----------
// Debugging

void SimpleCollisionMesh::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG
	if(renderer == nullptr) return;

	if (s_renderCollisionBuckets)
	{
		m_bucket->drawDebugShapes(renderer);
	}
	else
	{
		renderer->setColor(VectorArgb::solidWhite);
		renderer->drawMesh(m_tris);
	}

	if (ConfigSharedCollision::getDrawCollisionNormals())
	{
		renderer->setColor(VectorArgb::solidMagenta);
		renderer->drawMeshNormals(m_tris);
	}
#endif
}

// ----------
// Misc

void SimpleCollisionMesh::attachTris ( IndexedTriangleList * tris )
{
	if(m_tris != tris)
	{
		delete m_tris;
		m_tris = tris;

		//-- SimpleCollisionMesh::attachTris is only called by the MayaExporter.  
		//   Since we have swapped out the IndexedTriangleList we were constructed 
		//   with, we need to rebuild the collision buckets.
		delete m_bucket;
		m_bucket = new CollisionBuckets(m_tris);
	}

	setBoundsDirty(true);
}

void SimpleCollisionMesh::copy( CollisionMesh const * mesh )
{
	SimpleCollisionMesh const * simpleMesh = dynamic_cast< SimpleCollisionMesh const * >(mesh);

	if(simpleMesh)
	{
		m_tris->copy( simpleMesh->getTris() );

		setBoundsDirty(true);
	}
}

IndexedTriangleList const * SimpleCollisionMesh::getTris ( void ) const
{
	return m_tris;
}

// ----------------------------------------------------------------------

void SimpleCollisionMesh::load(Iff &file)
{
	m_tris->clear();

	m_tris->load(file);
}

void SimpleCollisionMesh::write( Iff & file ) const
{
	m_tris->write(file);
}

// ----------------------------------------------------------------------

void SimpleCollisionMesh::transform( CollisionMesh const * sourceMesh, Transform const & tform, float scale )
{
	CollisionMesh::transform(sourceMesh,tform,scale);

	setBoundsDirty(true);
}

// ----------------------------------------------------------------------

void SimpleCollisionMesh::setIntersectWithCollisionBuckets(bool const intersectWithCollisionBuckets)
{
	s_intersectWithCollisionBuckets = intersectWithCollisionBuckets;
}

// ----------------------------------------------------------------------

bool SimpleCollisionMesh::intersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time ) const
{
	if (s_intersectWithCollisionBuckets)
	{
		return m_bucket->intersect(begin, end, surfaceNormal, time);
	}
	return CollisionMesh::intersect(begin, end, surfaceNormal, time);
}

// ----------------------------------------------------------------------

void SimpleCollisionMesh::finalizeData()
{
	m_bucket->destroy();
	m_bucket->build(getBoundingABox().getMin(), getBoundingABox().getMax());
}

