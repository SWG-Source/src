// ======================================================================
//
// FloorLocator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/FloorLocator.h"

#include "sharedFile/Iff.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Triangle3d.h"

#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h" // so we can get the floor tri from the contact point
#include "sharedCollision/FloorTri.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/CollisionUtils.h"

#include "sharedCollision/ConfigSharedCollision.h"

#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"

FloorLocator FloorLocator::invalid;
FloorTri gs_dummyFloorTri;

// ----------------------------------------------------------------------

FloorLocator::FloorLocator() 
: ContactPoint(),
  m_valid(false),
  m_floor(nullptr),
  m_radius(0.0f),
  m_time(0.0f),
  m_edgeId(-1),
  m_hitTriId(-1),
  m_hitEdgeId(-1),
  m_contactNormal(Vector::zero)
{
}

// ----------

FloorLocator::FloorLocator( Floor const * floor, Vector const & localPoint, int triId, float dist, float radius )
: ContactPoint((floor ? floor->getFloorMesh() : nullptr),localPoint,triId,dist),
  m_valid(true),
  m_floor(floor),
  m_radius(radius),
  m_time(0.0f),
  m_edgeId(-1),
  m_hitTriId(-1),
  m_hitEdgeId(-1),
  m_contactNormal(Vector::zero)
{
}

// ----------

FloorLocator::FloorLocator( CollisionSurface const * mesh, Vector const & point, int triId, float dist, float radius )
: ContactPoint(mesh,point,triId,dist),
  m_valid(true),
  m_floor(nullptr),
  m_radius(radius),
  m_time(0.0f),
  m_edgeId(-1),
  m_hitTriId(-1),
  m_hitEdgeId(-1),
  m_contactNormal(Vector::zero)
{
	NOT_NULL(dynamic_cast<FloorMesh const *>(mesh));
}

// ----------

FloorLocator::FloorLocator( Floor const * floor, Vector const & localPoint )
: ContactPoint((floor ? floor->getFloorMesh() : nullptr),localPoint,-1,0.0f),
  m_valid(true),
  m_floor(floor),
  m_radius(0.0f),
  m_time(0.0f),
  m_edgeId(-1),
  m_hitTriId(-1),
  m_hitEdgeId(-1),
  m_contactNormal(Vector::zero)
{
}

// ----------

FloorLocator::FloorLocator( CollisionSurface const * mesh, Vector const & point )
: ContactPoint(mesh,point,-1,0.0f),
  m_valid(true),
  m_floor(nullptr),
  m_radius(0.0f),
  m_time(0.0f),
  m_edgeId(-1),
  m_hitTriId(-1),
  m_hitEdgeId(-1),
  m_contactNormal(Vector::zero)
{
	NOT_NULL(dynamic_cast<FloorMesh const *>(mesh));
}

// ----------

FloorLocator::FloorLocator( Vector const & point, float radius )
: ContactPoint(nullptr,point,-1,0.0f),
  m_valid(true),
  m_floor(nullptr),
  m_radius(radius),
  m_time(0.0f),
  m_edgeId(-1),
  m_hitTriId(-1),
  m_hitEdgeId(-1),
  m_contactNormal(Vector::zero)
{
}

// ----------

FloorLocator::FloorLocator ( FloorLocator const & locCopy ) 
: ContactPoint(locCopy),
  m_valid(locCopy.m_valid),
  m_floor(locCopy.m_floor),
  m_radius(locCopy.m_radius),
  m_time(locCopy.m_time),
  m_edgeId(locCopy.m_edgeId),
  m_hitTriId(locCopy.m_hitTriId),
  m_hitEdgeId(locCopy.m_hitEdgeId),
  m_contactNormal(locCopy.m_contactNormal)
{
}

// ----------

FloorLocator::~FloorLocator()
{
}

// ----------

FloorLocator & FloorLocator::operator = ( ContactPoint const & contact )
{
	NOT_NULL(dynamic_cast<FloorMesh const *>(contact.getSurface()));

	m_pSurface = contact.getSurface();
	m_position = contact.getPosition_l();
	m_offset = contact.getOffset();
	m_hitId = contact.getId();

	m_valid = true;

	return *this;
}

// ----------------------------------------------------------------------

Vector FloorLocator::getPosition_p ( void ) const
{
	if(m_floor)
	{
		return m_floor->getTransform_o2p().rotateTranslate_l2p(getPosition_l());
	}
	else
	{
		return getPosition_l();
	}
}

// ----------

Vector FloorLocator::getPosition_p ( CellProperty const * relativeCell ) const
{
	return CollisionUtils::transformToCell(getCell(),getPosition_p(),relativeCell);
}

// ----------

void FloorLocator::setPosition_p ( Vector const & newPos )
{
	NAN_CHECK(newPos);

	if(m_floor)
	{
		setPosition_l(m_floor->getTransform_o2p().rotateTranslate_p2l(newPos));
	}
	else
	{
		setPosition_l(newPos);
	}
}

// ----------

Vector FloorLocator::getPosition_w ( void ) const
{
	return CollisionUtils::transformToWorld(getCell(),getPosition_p());
}

// ----------------------------------------------------------------------

Floor const * FloorLocator::getFloor ( void ) const
{
	return m_floor;
}

// ----------

void FloorLocator::setFloor ( Floor const * newFloor )
{
	if(newFloor)
	{
		if(m_floor)
		{
			if(m_floor != newFloor)
			{
				Vector oldPos = getPosition_p();

				setPosition_l( newFloor->getTransform_o2p().rotateTranslate_p2l(oldPos) );
			}
		}
		else
		{
			if(getFloorMesh() == nullptr)
			{
				Vector oldPos = getPosition_p();

				setPosition_l( newFloor->getTransform_o2p().rotateTranslate_p2l(oldPos) );
			}
		}

		m_floor = newFloor;
		setFloorMesh(newFloor->getFloorMesh());
	}
	else
	{
		m_floor = nullptr;
	}
}

// ----------------------------------------------------------------------

FloorMesh const * FloorLocator::getFloorMesh ( void ) const
{
	return safe_cast<FloorMesh const *>(getSurface());
}

// ----------

void FloorLocator::setFloorMesh ( FloorMesh const * mesh )
{
	setSurface(mesh);
}

// ----------------------------------------------------------------------

FloorTri const & FloorLocator::getFloorTri ( void ) const
{
	FloorMesh const * mesh = getFloorMesh();
	
	if(mesh && (m_hitId != -1))
	{
		return mesh->getFloorTri(static_cast<uint>(m_hitId));
	}
	else
	{
		DEBUG_FATAL(true, ("FloorLocator::getFloorTri - Tried to get a floor tri for a locator that's not attached to a floor mesh\n"));

		return gs_dummyFloorTri;
	}
}

// ----------------------------------------------------------------------

CellProperty const * FloorLocator::getCell ( void ) const
{
	Floor const * floor = getFloor();

	if (floor)
	{
		return floor->getCell();
	}
	else
	{
		if(getFloorMesh())
		{
			return nullptr;
		}
		else
		{
			return CellProperty::getWorldCellProperty();
		}
	}
}

// ----------

CellProperty * FloorLocator::getCell ( void )
{
	Floor const * floor = getFloor();

	if (floor)
	{
		return const_cast<CellProperty *>(floor->getCell());
	}
	else
	{
		if(getFloorMesh())
		{
			return nullptr;
		}
		else
		{
			return CellProperty::getWorldCellProperty();
		}
	}
}

// ----------------------------------------------------------------------

Vector FloorLocator::getSurfaceNormal_l ( void ) const
{
	if(isAttached())
	{
		return getFloorTri().getNormal();
	}
	else
	{
		return Vector::unitY;
	}
}

// ----------

Vector FloorLocator::getSurfaceNormal_p ( void ) const
{
	Floor const * floor = getFloor();
	
	if(floor)
	{
		return floor->getTransform_o2p().rotate_l2p(getSurfaceNormal_l());
	}
	else
	{
		return getSurfaceNormal_l();
	}
}

// ----------

Vector FloorLocator::getSurfaceNormal_w ( void ) const
{
	Floor const * floor = getFloor();
	
	if(floor)
	{
		return floor->getTransform_o2w().rotate_l2p(getSurfaceNormal_l());
	}
	else
	{
		return getSurfaceNormal_l();
	}
}

// ----------------------------------------------------------------------

bool    FloorLocator::isFallthrough  ( void ) const
{
	if(isAttached() && getFloorTri().isFallthrough())
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool FloorLocator::isAttachedTo( Floor const * pFloor ) const
{
	return getFloor() == pFloor;
}

// ----------------------------------------------------------------------

void FloorLocator::read_0000 ( Iff & iff )
{
	ContactPoint::read_0000(iff);

	m_radius = iff.read_float();
	
	Vector down = iff.read_floatVector();
	UNREF(down);

	bool portalAdjacent = iff.read_bool8();

	UNREF(portalAdjacent);
}

// ----------

void FloorLocator::write ( Iff & iff ) const
{
	ContactPoint::write(iff);

	iff.insertChunkData(m_radius);
	iff.insertChunkData(-Vector::unitY);
	iff.insertChunkData(false);
}

// ----------------------------------------------------------------------

void FloorLocator::detach ( void )
{
	setFloor(nullptr);
	setId(-1);
}

// ----------------------------------------------------------------------

void FloorLocator::reattach ( void )
{
	Floor const * floor = getFloor();

	if(floor)
	{
		FloorLocator temp;

		if(floor->dropTest(getPosition_p(),2.0f,temp))
		{
			temp.setRadius(getRadius());

			*this = temp;
		}
	}
}

// ----------------------------------------------------------------------

void FloorLocator::snapToFloor ( void )
{
	if(!isAttached()) return;

	float floorHeight = getFloorTri().getHeightAt(m_position.x,m_position.z);

	float newOffset = m_offset + m_position.y - floorHeight;

	Vector newPos(m_position.x,floorHeight,m_position.z);

	setPosition_l(newPos);
	setOffset(newOffset);
}
