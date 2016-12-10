// ======================================================================
//
// MeshExtent.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/MeshExtent.h"

#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/ExtentList.h"
#include "sharedCollision/SimpleCollisionMesh.h"

#include "sharedFile/Iff.h"

#include "sharedMath/Range.h"
#include "sharedMath/Transform.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/DebugShapeRenderer.h"

const Tag TAG_CMSH = TAG(C,M,S,H);

typedef std::vector<Vector> VertexList;

// ----------------------------------------------------------------------

void MeshExtent::install ( void )
{
	ExtentList::assignBinding(TAG_CMSH,MeshExtent::build);
}

void MeshExtent::remove ( void )
{
}

Extent * MeshExtent::build( Iff & iff )
{
	MeshExtent * extent = new MeshExtent();

	extent->load(iff);

	return extent;
}

// ----------------------------------------------------------------------

SimpleCollisionMesh * MeshExtent::createMesh( IndexedTriangleList * tris )
{
	if(tris)
	{
		return new SimpleCollisionMesh(tris);
	}
	else
	{
		return new SimpleCollisionMesh();
	}
}

// ----------------------------------------------------------------------

MeshExtent::MeshExtent()
: Extent(ET_Mesh),
  m_mesh(nullptr),
  m_box()
{
	m_mesh = createMesh(nullptr);
}

MeshExtent::MeshExtent( IndexedTriangleList * tris )
: Extent(ET_Mesh),
  m_mesh(nullptr),
  m_box()
{
	m_mesh = createMesh(tris);
}

MeshExtent::MeshExtent( SimpleCollisionMesh * mesh )
: Extent(ET_Mesh),
  m_mesh(mesh),
  m_box()
{
}

MeshExtent::~MeshExtent()
{
	delete m_mesh;
	m_mesh = nullptr;
}

// ----------------------------------------------------------------------

void MeshExtent::load ( Iff & iff ) 
{
	iff.enterForm(TAG_CMSH);

		switch (iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

		default:
			FATAL (true, ("MeshExtent::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_CMSH);

	updateBounds();
}

// ----------

void MeshExtent::write ( Iff & iff ) const
{
	iff.insertForm(TAG_CMSH);

		iff.insertForm(TAG_0000);

			m_mesh->write(iff);

		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_CMSH);
}

// ----------------------------------------------------------------------

bool MeshExtent::contains ( Vector const & vector ) const
{
	return m_box.contains(vector);
}

bool MeshExtent::contains ( Vector const & begin, Vector const & end ) const
{
	return contains(begin) && contains(end);
}

// ----------------------------------------------------------------------

bool MeshExtent::realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time ) const
{
	return m_mesh->intersect(begin, end, surfaceNormal, time);
}

// ----------------------------------------------------------------------

Range MeshExtent::rangedIntersect( Line3d const & line ) const
{
	return m_mesh->intersectRange(line);
}

Range MeshExtent::rangedIntersect( Ray3d const & ray ) const
{
	return m_mesh->intersectRange(ray);
}

Range MeshExtent::rangedIntersect( Segment3d const & seg ) const
{
	return m_mesh->intersectRange(seg);
}

void MeshExtent::drawDebugShapes     ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	renderer->setColor( VectorArgb::solidWhite );
	m_mesh->drawDebugShapes(renderer);

#endif
}

// ----------------------------------------------------------------------

BaseExtent * MeshExtent::clone ( void ) const
{
	return new MeshExtent( m_mesh->clone() );
}

void MeshExtent::copy( BaseExtent const * source )
{
	MeshExtent const * meshSource = safe_cast< MeshExtent const * >(source);

	m_mesh->copy( meshSource->getMesh() );

	m_sphere = meshSource->m_sphere;
	m_box = meshSource->m_box;
}

void MeshExtent::transform( BaseExtent const * source, Transform const & tform, float scale )
{
	MeshExtent const * meshSource = safe_cast< MeshExtent const * >(source);

	CollisionMesh const * mesh = meshSource->getMesh();

	m_mesh->transform( mesh, tform, scale );

	updateBounds();
}

// ----------

void MeshExtent::updateBounds ( void )
{
	m_mesh->calcBounds();

	VertexList const & verts = m_mesh->getTris()->getVertices();

	m_sphere = Containment3d::EncloseSphere( verts );	
	m_box = Containment3d::EncloseABox( verts );
}

// ----------

AxialBox MeshExtent::getBoundingBox ( void ) const
{
	return m_box;

}

// ----------

Sphere MeshExtent::getBoundingSphere   ( void ) const
{
	return m_sphere;
}

// ----------------------------------------------------------------------

void MeshExtent::attachMesh( IndexedTriangleList * tris )
{
	m_mesh->attachTris(tris);

	updateBounds();
}

CollisionMesh * MeshExtent::getMesh ( void )
{
	return m_mesh;
}

CollisionMesh const * MeshExtent::getMesh ( void ) const
{
	return m_mesh;
}












// ======================================================================

void MeshExtent::load_0000 ( Iff & iff )
{
	iff.enterForm(TAG_0000);

		m_mesh->load(iff);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------
