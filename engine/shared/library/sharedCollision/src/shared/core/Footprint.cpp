// ======================================================================
//
// Footprint.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Footprint.h"

#include "sharedCollision/CollisionDetect.h"
#include "sharedCollision/CollisionEnums.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionResolve.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorContactShape.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorTri.h"
#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/SpatialDatabase.h"

#include "sharedFoundation/Fatal.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"

#include "sharedDebug/Report.h"
#include "sharedDebug/PerformanceTimer.h"

#include "sharedMath/AxialBox.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Triangle3d.h"

#include "sharedObject/CellProperty.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"

#include "sharedRandom/Random.h"

#include "sharedTerrain/TerrainObject.h"

#include <algorithm>

#pragma warning(disable : 4355)	// 'this' used in base member initializer list

// ----------

typedef MultiListDataIterator<FloorContactShape> ContactIterator;
typedef MultiListConstDataIterator<FloorContactShape> ConstContactIterator;

// ----------------------------------------------------------------------

namespace FootprintNamespace
{
}

using namespace FootprintNamespace;

extern real terrainHeightTime;
extern real canMoveTime;
extern real canMoveTime2;


// ----------------------------------------------------------------------

static bool epsilonEqual( float value, float target, float epsilon )
{
	float delta = value - target;

	if(delta > epsilon) return false;
	if(delta < -epsilon) return false;

	return true;
}

// ======================================================================

Footprint::Footprint	( Vector const & position, float radius, CollisionProperty * parent, const float swimHeight )
: BaseClass(),
  m_parent(parent),
  m_cellObject(nullptr),
  m_position_p(Vector::zero),
  m_position_w(Vector::zero),
  m_radius(radius),
  m_floorList( this, 0 ),
  m_swimHeight (swimHeight),
  m_hasTerrainHeight(false),
  m_hasFloorHeight(false),
  m_hasGroundHeight(false),
  m_terrainHeight(0.0f),
  m_floorHeight(0.0f),
  m_groundHeight(0.0f),
  m_groundNormal(Vector::unitY),
  m_addToWorldTime(-1),
  m_addToWorldPos(Vector::zero),
  m_floating(false),
  m_floatingTime(20)
#ifdef _DEBUG
  ,
  m_backupPosition_p(position),
  m_backupCell(nullptr),
  m_backupObjectPosition_p(position),
  m_backupObjectCell(nullptr),
  m_lineHitTime( -1.0f ),
  m_lineOrigin( Vector(0.0f,1.5f,0.0f) ),
  m_lineDelta( Vector(0.0f,0.0f,10.0f) ),
  m_canMove( true ),
  m_lineHitPos( Vector::zero ),
  m_bubbleSize(0.0f),
  m_bubbleEmpty(false)
#endif
{
	NOT_NULL(parent);

	setPosition(getObjectCell(),position);

	updateHeights();
}

// ----------

Footprint::~Footprint()
{
	detach();

	m_parent = nullptr;
	m_cellObject = nullptr;
}

// ======================================================================

void Footprint::detach ( void )
{
	FLOOR_LOG("Footprint::detach - Detaching all contact points\n");

	getFloorList().clear();
}

// ----------------------------------------------------------------------

void Footprint::reattachContacts ( void )
{
	// move detached contacts to the footprint

	ContactIterator it;

	for(it = getFloorList(); it; ++it)
	{
		FloorContactShape * contact = *it;

		if(!contact) continue;

		FloorLocator & loc = contact->m_contact;

		if(!loc.isAttached())
		{
			Vector footPos = getPosition_p(loc.getCell());

			loc.setPosition_p(footPos);
		}
	}

	// If all the foot's contacts have been detached, try and reattach them
	// before sweeping to catch walking-off-ledge cases

	for(it = getFloorList(); it; ++it)
	{
		FloorContactShape * contact = *it;

		if(!contact) continue;

		if(contact->m_contact.getId() != -1)
		{
			return;			
		}
	}

	for(it = getFloorList(); it; ++it)
	{
		FloorContactShape * contact = *it;

		if(!contact) continue;

		contact->m_contact.reattach();
	}
}

// ----------

void Footprint::forceReattachContacts ( void )
{
	for(ContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape * contact = *it;

		if(!contact) continue;

		if(contact->m_contact.getId() == -1)
		{
			contact->m_contact.reattach();
		}
	}
}

// ----------

void Footprint::snapContacts ( void )
{
	for(ContactIterator it(getFloorList()); it; ++it)
	{
		(*it)->m_contact.snapToFloor();
	}
}

// ----------------------------------------------------------------------
// Remove any contact points from the footprint that have been detached
// from their surfaces

void Footprint::sweepContacts ( void )
{
	ContactIterator it(getFloorList());

	CellProperty const * objectCell = getObjectCell();
	CellProperty const * worldCell = CellProperty::getWorldCellProperty();

	while(it)
	{
		FloorContactShape * contact = *it;

		MultiListNode * node = it.getNode();

		++it;

		FloorLocator & loc = contact->m_contact;

		FloorLocator temp;

		Floor const * floor = loc.getFloor();

		if((loc.getId() != -1) && floor && !floor->dropTest(loc.getPosition_p(),loc.getId(),temp))
		{
			loc.detach();
		}

		if(!loc.isAttached())
		{
			Floor const * floor = loc.getFloor();

			if(floor)
			{
				CellProperty const * contactCell = loc.getCell();

				if(contactCell == worldCell)
				{
					if(objectCell != worldCell)
					{
						// detached world cell contacts are erased immediately if the object isn't in the world cell
					}
					else
					{
						// otherwise the contact sticks around until it's more than 2 meters away from the floor's bounding box

						SimpleExtent const * extent = safe_cast<SimpleExtent const *>(floor->getFloorMesh()->getExtent_l());
						AxialBox box = extent->getShape().getAxialBox();

						float dist2 = Distance3d::Distance2PointABox(loc.getPosition_l(),box);

						if(dist2 < 4.0f) continue;
					}
				}
				else
				{
					// Remove all cell contacts as soon as they're detached. 
					// This may cause some thrashing on portal transitions, but leaving detached cell contacts
					// hanging around causes bugs (and is also slower due to the entrance tests)
				}
			}

			FLOOR_LOG("Footprint::sweepContacts - Removing detached contact\n");

			getFloorList().erase(node);
		}
		else
		{
			// If we're in a cell, remove any world cell contacts. Otherwise we might run into invisible edges
			// from things in the world (ex. bunker uberlair has fallthrough floor all along the interior; if
			// you don't detach the world contact you hit an edge of the corral floor it when you're down inside 
			// the bunker building)

			CellProperty const * contactCell = loc.getCell();

			if(objectCell != worldCell)
			{
				if(contactCell == worldCell)
				{
					getFloorList().erase(node);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

Object const * Footprint::getOwner ( void ) const
{
	NOT_NULL(m_parent);

	return &(m_parent->getOwner());
}

Object * Footprint::getOwner ( void )
{
	NOT_NULL(m_parent);

	return &(m_parent->getOwner());
}

// ----------------------------------------------------------------------

void Footprint::addContact ( FloorLocator const & loc )
{
	if(loc.getFloor() == nullptr)
	{
		FLOOR_LOG("Footprint::addContact - loc isn't attached to a floor\n");
	}

	FLOOR_LOG("Footprint::addContact - Adding contact point\n");

	// ----------

	FloorContactShape * contact = new FloorContactShape();

	contact->m_contact = loc;
	contact->m_contact.setRadius(getRadius());

	// ----------

	Floor const * pcFloor = loc.getFloor();

	Floor * floor = const_cast<Floor*>(pcFloor);

	getFloorList().connectTo( floor->getFootList(), contact );

} //lint !e429 // custodial pointer 'contact' has not been freed or returned - it's attached to the floor list

// ----------------------------------------------------------------------
// move the footprint over its contact points

//@todo - This should probably check to see if the contacts are aligned first.

bool Footprint::syncToContacts ( void )
{
	if(!getFloorList().isEmpty())
	{
		FloorContactShape * contact = safe_cast<FloorContactShape*>(getFloorList().getHead()->getData());

		FloorLocator & loc = contact->m_contact;

		Vector point = loc.getPosition_p(getCell());

		Vector foot = getPosition_p();

		foot.x = point.x;
		foot.z = point.z;

		setPosition_p(foot);
	}

	IGNORE_RETURN( updateHeights() );

	return true;
}

// ----------------------------------------------------------------------

bool Footprint::isOnSolidFloor ( void ) const
{
	for(ConstContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape const * contact = *it;

		if(!contact) continue;

		if(contact->m_contact.isFallthrough()) continue;

		if(contact->m_contact.isAttached()) return true;
	}

	return false;
}

// ----------------------------------------------------------------------
// The footprint is out of sync if the object it is attached to is not at 
// the same X-Z coordinates (within a small tolerance).

bool Footprint::isObjectInSync ( void ) const
{
	Vector objectPos = getObjectPosition_p();

	Vector footPos = getPosition_p(getObjectCell());

	if(!epsilonEqual(objectPos.x,footPos.x,0.00001f)) return false;
	
	if(!epsilonEqual(objectPos.z,footPos.z,0.00001f)) return false;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Set the swim height for this footprint.
 *
 * This is necessary because swim height varies per creature per scale,
 * and scale is not known until after end baselines.  Meanwhile, this
 * Footprint is created prior to knowledge of the scale.  CreatureObject's
 * endBaselines sets this value appropriately once scale factor is known.
 *
 * @param swimHeight  the depth of the water at which point the creature
 *                    starts swimming.
 */

void Footprint::setSwimHeight ( float swimHeight )
{
	NAN_CHECK(swimHeight);

	m_swimHeight = swimHeight;
}

// ----------------------------------------------------------------------

Vector const & Footprint::getPosition_p ( void ) const
{
	if(m_cellObject.getPointer() == nullptr)
	{
		WARNING(true,("Footprint::getPosition_p - Footprint's parent cell has disappeared"));
		
		Footprint * self = const_cast<Footprint*>(this);

		self->m_position_p = self->m_position_w;

		self->m_cellObject = &(CellProperty::getWorldCellProperty()->getOwner());
	}

	return m_position_p;
}

// ----------

Vector Footprint::getPosition_p ( CellProperty const * relativeCell ) const
{
	return CollisionUtils::transformToCell( getCell(), getPosition_p(), relativeCell );
}

// ----------

Vector const & Footprint::getPosition_w ( void ) const
{
	return m_position_w;
}

// ----------

void Footprint::setPosition_p ( Vector const & pos )
{
	NAN_CHECK(pos);

	m_position_p = pos;
	m_position_w = CollisionUtils::transformToWorld(getCell(),pos);
}

// ----------

void Footprint::setPosition_w ( Vector const & position_w )
{
	NAN_CHECK(position_w);

	m_position_p = CollisionUtils::transformFromWorld(position_w,getCell());
	m_position_w = position_w;
}

// ----------

CellProperty * Footprint::getCell ( void )
{
	if(m_cellObject.getPointer())
	{
		return m_cellObject->getCellProperty();
	}
	else
	{
		return CellProperty::getWorldCellProperty();
	}
}

CellProperty const * Footprint::getCell ( void ) const
{
	if(m_cellObject.getPointer())
	{
		return m_cellObject->getCellProperty();
	}
	else
	{
		return CellProperty::getWorldCellProperty();
	}
}

// ----------

void Footprint::setPosition ( CellProperty * pNewCell, Vector const & pos )
{
	NAN_CHECK(pos);

	if(pNewCell == nullptr) pNewCell = CellProperty::getWorldCellProperty();

	m_cellObject = &(pNewCell->getOwner());
	m_position_p = pos;
	m_position_w = CollisionUtils::transformToWorld(pNewCell,pos);
}

// ----------------------------------------------------------------------
// Teleport the footprint (and attached object) to a different point on the same floor.

bool Footprint::teleport ( FloorLocator const & loc )
{
	detach();

	addContact(loc);

	syncToContacts();

	setObjectPosition_p( getPosition_p() );

	m_parent->setIdle(false);

	return true;
}

// ----------------------------------------------------------------------

bool Footprint::isAttached ( void ) const
{
	for(ConstContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape const * contact = *it;

		if(!contact) continue;

		if(contact->m_contact.isAttached()) return true;
	}

	return false;
}

// ----------------------------------------------------------------------
// Force attach of footprint to cell floor
// Force attach of footprint to nearby cells

void Footprint::updatePreResolve ( float time )
{
	UNREF(time);
	
	CellProperty * objectCell = getObjectCell();

	Vector newPosition_p = getPosition_p(objectCell);

	setPosition(objectCell,newPosition_p);

	IGNORE_RETURN(attachTo(objectCell->getFloor(),false));

	// This requires some explanation - When creatures are moving around really close to portals,
	// the system in place now can occasionally miss moves that should really be portal crossings.

	// To detect these and make the collision system handle them, we create contact points for all
	// cells "near" the creature and let them sort themselves out.

	// #AMA 11-13-03 - this test is too expensive right now, and we can live without it (the
	// fixup code will take care of things). floor corners where a portal bisects the corner (like in
	// the tutorial) may act funky, but players shouldn't be able to get off the floor.

	/*
	static std::vector<CellProperty *> hitCells;

	hitCells.clear();

	Vector objectPos = getObjectPosition_p();
	Vector footPos = getPosition_p();

	Vector goalPos(objectPos.x,footPos.y,objectPos.z);

	Sphere testSphere = Containment3d::EncloseSphere(footPos,goalPos);

	if(testSphere.getRadius() > 5.0f)
	{
		Vector center = testSphere.getCenter();
		float radius = testSphere.getRadius();

		WARNING(true,("Footprint::updatePreResolve - Query sphere is too big - %f at (%f,%f,%f)\n",radius,center.x,center.y,center.z));

		testSphere = Sphere(goalPos,5.0f);
	}

	testSphere.setRadius( testSphere.getRadius() + getRadius() );

	if(getCell()->getDestinationCells(testSphere,hitCells))
	{
		int hitCount = hitCells.size();

		for(int i = 0; i < hitCount; i++)
		{
			IGNORE_RETURN( attachTo(hitCells[i]->getFloor(),false) );
		}
	}
	*/

#ifdef WIN32
	// This code absolutely kills the game server. rdelashmit says this doesn't need to be
	// done for the game server, but since there's no #ifdef for a game server build, I'm
	// using WIN32, which means it will sill get built for the WIN32 game server, but that
	// won't hurt; we just don't want this code in the the live game server - hnguyen

	// If we're crossing a portal, make sure we attach to the floor on the other side of it.
	float t = 0.f;
	CellProperty * const c = getCell()->getDestinationCell(getPosition_p(), getObjectPosition_p(), t, true);
	if (c)
		IGNORE_RETURN(attachTo(c->getFloor(), false));
#endif

	// Shove the object away from any walls it's embedded in. (but only on the client)

	if(!m_parent->isServerSide() && ConfigSharedCollision::getShoveEnabled())
	{
		if(!testClear())
		{
			shoveAway(time);
		}
	}
}

// ----------------------------------------------------------------------

bool Footprint::testClear ( void ) const
{
//	Vector const & objectPos_p = getObjectPosition_p();

	for(ConstContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape const * contact = *it;

		if(!contact) continue;

		FloorLocator const & loc = contact->m_contact;

		if(!loc.isAttached()) continue;

		FloorMesh const * mesh = loc.getFloorMesh();

		if(!mesh) continue;

		// ----------

		if (!isFloorWithinThreshold(*contact))
			continue;

		if(!mesh->testClear(loc)) return false;
	}

	return true;
}

// ----------

void Footprint::shoveAway ( float time )
{
	UNREF(time);

	Vector scootAccum = Vector::zero;

	for(ConstContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape const * contact = *it;

		if(!contact) continue;

		FloorLocator const & loc = contact->m_contact;

		if(!loc.isAttached()) continue;

		Floor const * floor = loc.getFloor();

		if(!floor) continue;

		FloorMesh const * mesh = floor->getFloorMesh();

		if(!mesh) continue;

		// ----------

		int closeTriId = -1;
		int closeEdgeId = -1;
		float dist = 0.0f;

		if(mesh->getClosestCollidableEdge(loc,closeTriId,closeEdgeId,dist))
		{
			float scootDist = getRadius() - dist;

			if(scootDist > 0.0f)
			{
				Vector contactNormal_l = mesh->getTriangle(closeTriId).getEdgeDir(closeEdgeId).cross( -Vector::unitY );

				if(contactNormal_l.normalize())
				{
					Vector contactNormal_p = floor->rotate_l2p(contactNormal_l);

					scootAccum += contactNormal_p * scootDist;
				}
			}
		}
	}

	// ----------

	if(scootAccum != Vector::zero)
	{
		Vector objectPos_p = getObjectPosition_p();

		// scale the scoot distance up just a bit so the player isn't exactly on top of the wall

		scootAccum *= 1.05f;

		objectPos_p += scootAccum;

		setObjectPosition_p(objectPos_p);

	}
}

// ----------------------------------------------------------------------

void Footprint::updatePostResolve ( float time )
{
	UNREF(time);
	
	setPosition(getObjectCell(),getObjectPosition_p());

	reattachContacts();

	sweepContacts();

	updateHeights();

	updateOffsets();

#ifdef _DEBUG

	runDebugTests();

#endif
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void Footprint::runDebugTests ( void )
{
	//@todo - HACK - Testing code for collision world line query

	if(ConfigSharedCollision::getEnableTestCode())
	{
		Vector delta = getOwner()->rotate_o2p(m_lineDelta);

		m_lineHitTime = -1.0f;

		Vector objPos = getOwner()->getPosition_p();
		CellProperty const * objCell = getOwner()->getParentCell();

		/*
		FloorLocator objLoc;
		CollisionWorld::makeLocator(objCell,objPos,objLoc);

		CanMoveResult result = CollisionWorld::canMove(getOwner(),objPos+delta,0.0f,false,true);

		m_canMove = (result == CMR_MoveOK);

		if(!m_canMove)
		{
			m_lineHitPos = m_lineOrigin + m_lineDelta;
			m_lineHitTime = 1.0f;
		}

		m_bubbleSize = 20.0f;

		IGNORE_RETURN(CollisionWorld::calcBubble(getCell(),getPosition_p(),20.0f,m_bubbleSize));
		*/

		Object const * hitObject = nullptr;

		QueryInteractionResult result = CollisionWorld::queryInteraction(objCell,
		                                                                 objPos + m_lineOrigin,
		                                                                 objCell,
		                                                                 objPos + m_lineOrigin + delta,
		                                                                 nullptr,
		                                                                 !ConfigSharedCollision::getIgnoreTerrainLos(),
		                                                                 ConfigSharedCollision::getGenerateTerrainLos(),
		                                                                 ConfigSharedCollision::getTerrainLOSMinDistance(),
		                                                                 ConfigSharedCollision::getTerrainLOSMaxDistance(),
		                                                                 m_lineHitTime,
		                                                                 hitObject);

		m_canMove = (result == QIR_None);

		m_lineHitPos = m_lineOrigin + m_lineDelta * m_lineHitTime;
	}
}
#endif


// ----------------------------------------------------------------------

FloorLocator const * Footprint::getAnyContact ( void ) const
{
	for(ConstContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape const * contact = *it;

		if(!contact) continue;

		if(contact->m_contact.isAttached()) return &contact->m_contact;
	}

	return nullptr;
}

// ----------

FloorLocator const * Footprint::getSolidContact ( void ) const
{
	FloorLocator const * temp = nullptr;

	float maxHeight = -REAL_MAX;

	for(ConstContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape const * contact = *it;

		if(!contact) continue;

		if(contact->m_contact.isFallthrough()) continue;

		if(!contact->m_contact.isAttached()) continue;

		Vector contactPos = contact->m_contact.getPosition_p();

		if(contactPos.y > maxHeight)
		{
			maxHeight = contactPos.y;

			temp = &contact->m_contact;
		}
	}

	return temp;
}

// ----------------------------------------------------------------------

Object const * Footprint::getStandingOn ( void ) const
{
	FloorLocator const * contact = getSolidContact();

	if(contact == nullptr) return nullptr;

	Floor const * floor = contact->getFloor();

	if(floor == nullptr) return nullptr;

	return floor->getOwner();
}

// ----------------------------------------------------------------------

bool Footprint::snapToCellFloor ( void )
{
	Floor * floor = getCell()->getFloor();

	if(floor)
	{
		Vector objectPos = getObjectPosition_p();

		FloorLocator closestLoc = FloorLocator::invalid;

		bool found = floor->findClosestLocation(objectPos,closestLoc);

		if(found)
		{
			return teleport(closestLoc);
		}
	}

	// ----------

	return false;
}

// ----------------------------------------------------------------------

bool Footprint::isAttachedTo ( Floor const * floor ) const
{
	if(floor == nullptr) return false;
	
	return getFloorList().find( floor->getFootList() ) != nullptr;
}

// ----------------------------------------------------------------------

bool Footprint::updateHeights ( void )
{
	// ----------
	// Update terrain height

	float logicalTerrainHeight = -REAL_MAX;
	Vector terrainNormal = Vector::unitY;

	if (isInCell () || !TerrainObject::getConstInstance ())
	{
		m_hasTerrainHeight = false;
		m_terrainHeight = -REAL_MAX;
	}
	else
	{
		//-- Capture both the logical and real terrain height.
		m_hasTerrainHeight = TerrainObject::getConstInstance ()->getLogicalHeight(getPosition_w(), m_swimHeight, logicalTerrainHeight, m_terrainHeight, terrainNormal);
	}

	// ----------
	// Update floor height

	m_hasFloorHeight = false;
	m_floorHeight = -REAL_MAX;

	Vector floorNormal = Vector::unitY;

	for(ContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape * contact = *it;

		if(!contact->m_contact.isAttached()) continue;
		if(contact->m_contact.isFallthrough()) continue;

		FloorLocator & loc = contact->m_contact;

		Vector localContact = loc.getPosition_p(getCell());

		real contactHeight = localContact.y;

		if(contactHeight > m_floorHeight)
		{
			m_floorHeight = contactHeight;
			floorNormal = loc.getSurfaceNormal_p();
			m_hasFloorHeight = true;
		}
	}

	// ----------
	// Update ground height

	if(m_hasFloorHeight || m_hasTerrainHeight)
	{
		m_hasGroundHeight = true;

		if(m_floorHeight > logicalTerrainHeight)
		{
			m_groundHeight = m_floorHeight;
			m_groundNormal = floorNormal;
		}
		else
		{
			m_groundHeight = logicalTerrainHeight;
			m_groundNormal = terrainNormal;
		}
	}
	else
	{
		m_hasGroundHeight = false;
		m_groundHeight = getObjectPosition_p().y;
		m_groundNormal = Vector::unitY;
	}

	// ----------

	snapToGround();

	return m_hasGroundHeight;
}

// ----------------------------------------------------------------------

void Footprint::snapToGround ( void )
{
	NAN_CHECK(m_groundHeight);

	Vector pos = getPosition_p();

	pos.y = m_groundHeight;

	setPosition_p(pos);
}

// ----------------------------------------------------------------------

bool Footprint::isFloating ( void ) const
{
	return m_floating;
}

// ----------------------------------------------------------------------
// Snap the footprint underneath the object

bool Footprint::snapToObject ( void )
{
	Vector lastPos = getObjectPosition_p();
	CellProperty * lastCell = getObjectCell();

	DEBUG_REPORT_LOG_PRINT(ConfigSharedCollision::getReportMessages(),("Footprint::snapToObject - Syncing to object position (%f,%f,%f)\n",lastPos.x,lastPos.y,lastPos.z));

	detach();

	setPosition(lastCell,lastPos);
	
	updateHeights();

	m_parent->storePosition();

	return true;
}

// ----------

bool Footprint::snapObjectToTerrain()
{
	if(!isOnSolidFloor() && !isInCell())
	{
		Vector position = getObjectPosition_p();

		float newLogicalHeight;
		float newRealHeight;

		if(TerrainObject::getConstInstance ()->getLogicalHeight (position, m_swimHeight, newLogicalHeight, newRealHeight))
		{
			if(newLogicalHeight != position.y)
			{
				setObjectPosition_p(position);

				return true;
			}
		}
	}

	return false;
}

// ----------

bool Footprint::snapObjectToGround ( void )
{
	if(m_hasGroundHeight)
	{
		Vector newPos = getObjectPosition_p();

		newPos.y = m_groundHeight;

		setObjectPosition_p(newPos);
	}

	return true;
}

// ----------------------------------------------------------------------

void Footprint::updateOffsets ( void )
{
	for(ContactIterator it(getFloorList()); it; ++it)
	{
		FloorLocator & contact = (*it)->m_contact;

		contact.snapToFloor();

		Vector const & contactPos_p = contact.getPosition_p();

		Vector objectPos_p = getObjectPosition_p(contact.getCell());

		float offset = objectPos_p.y - contactPos_p.y;

		contact.setOffset(offset);
	}
}

// ----------------------------------------------------------------------

bool Footprint::attachTo ( Floor const * floor, bool fromObject )
{
	if(floor == nullptr) return false;

	if(isAttachedTo(floor)) return false;

	// ----------

  	Vector localPos;
	
	if(fromObject)
	{
		localPos = getObjectPosition_p(floor->getCell());
	}
	else
	{
  		localPos = getPosition_p(floor->getCell());
	}
  
  	Vector floorPos = floor->transform_p2l(localPos);

	FloorLocator newContact( floor, floorPos, -1, 0.0f, getRadius() );

	addContact(newContact);

	return true;
}

// ----------------------------------------------------------------------

bool Footprint::isInCell ( void ) const
{
	return getCell() != CellProperty::getWorldCellProperty();
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void Footprint::backup ( void )
{
	m_backupObjectPosition_p = getObjectPosition_p();
	m_backupObjectCell = getObjectCell();
	m_backupPosition_p = getPosition_p();
	m_backupCell = getCell();

	for(ContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape * contact = (*it);

		contact->m_backupContact = contact->m_contact;
	}
}

void Footprint::revert ( void )
{
	setPosition(m_backupCell,m_backupPosition_p);

	updateHeights();

	for(ContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape * contact = (*it);

		contact->m_contact = contact->m_backupContact;
	}
	
	CellProperty::setPortalTransitionsEnabled(false);

	// can't set the cell from shared code

	getOwner()->setPosition_p(m_backupPosition_p);

	CellProperty::setPortalTransitionsEnabled(true);
}

#endif

// ----------------------------------------------------------------------

Vector Footprint::getObjectPosition_p ( void ) const
{
	return getOwner()->getPosition_p();
}

// ----------

Vector Footprint::getObjectPosition_p ( CellProperty const * relativeCell ) const
{
	return CollisionUtils::transformToCell ( getObjectCell(), getObjectPosition_p(), relativeCell );
}

// ----------

void	Footprint::setObjectPosition_p	( Vector const & newPos )
{
	NAN_CHECK(newPos);

	if(getObjectPosition_p() == newPos) return;

	getOwner()->setPosition_p(newPos);
}

// ----------------------------------------------------------------------

CellProperty const * Footprint::getObjectCell ( void ) const
{
	return getOwner()->getParentCell();
}

CellProperty * Footprint::getObjectCell ( void )
{
	return getOwner()->getParentCell();
}

// ----------------------------------------------------------------------
// returns the # of floors actually moved

int Footprint::elevatorMove ( int nFloors, Transform & outTransform )
{
	if(!isInCell()) return 0;

	bool bDown = nFloors < 0;
	int absDist = bDown ? -nFloors : nFloors;

	// ----------
	// Find the contact that's on the cell's floor
	
	FloorContactShape const * cellContact = nullptr;

	for(ConstContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape const * contact = (*it);

		if(!contact->m_contact.isAttached()) continue;

		FloorLocator const & loc = contact->m_contact;

		Floor const * floor = loc.getFloor();

		// ----------

		if(floor && floor->isCellFloor())
		{
			cellContact = contact;
			break;
		}
	}

	if(cellContact == nullptr) 
	{
		return 0;
	}

	Floor const * floor = cellContact->m_contact.getFloor();

	if(!floor) 
	{
		return 0;
	}

	// ----------

	int iFloor = 0;

	FloorLocator contactPoint = cellContact->m_contact;

	for(; iFloor < absDist; iFloor++)
	{
		FloorLocator nextUp = FloorLocator::invalid;
		FloorLocator nextDown = FloorLocator::invalid;;

		if(!floor) 
		{
			FLOOR_WARNING("Footprint::elevatorMove - In a cell, but no cell floor to do elevator moving on");
			return 0;
		}

		IGNORE_RETURN( floor->findElevatorNeighbors( contactPoint, nextUp, nextDown ) );

		if(bDown && nextDown.isAttached())
		{
			contactPoint = nextDown;
		}
		else if(!bDown && nextUp.isAttached())
		{
			contactPoint = nextUp;
		}
		else
		{
			break;
		}
	}

	if(iFloor == 0)
	{
		return 0;
	}
	else
	{
		outTransform = getOwner()->getTransform_o2p();

		outTransform.setPosition_p( contactPoint.getPosition_p() + Vector(0.0f,0.01f,0.0f) );

		return iFloor;
	}
}

// ----------------------------------------------------------------------

void Footprint::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	if(ConfigSharedCollision::getDrawFootprints())
	{
		renderer->setWorldSpace(true);

		// ----------
		//@todo HACK for testing line query

		Vector localEnd = m_lineOrigin + m_lineDelta;
		Vector localA = m_lineOrigin + Vector(0.04f,0.0f,0.0f);
		Vector localB = m_lineOrigin - Vector(0.04f,0.0f,0.0f);

		Vector worldEnd = getOwner()->rotateTranslate_o2w(localEnd);
		Vector worldA = getOwner()->rotateTranslate_o2w(localA);
		Vector worldB = getOwner()->rotateTranslate_o2w(localB);
		Vector worldHit = getOwner()->rotateTranslate_o2w(m_lineHitPos);

		if(ConfigSharedCollision::getEnableTestCode())
		{
			// These lines MUST be drawn in world space, otherwise they'll inherit the
			// player's scaling factor.

			if((!m_canMove) || (m_lineHitTime >= 0.0f))
			{
				VectorArgb lineColor = VectorArgb::solidRed;

				if(!m_canMove) lineColor = VectorArgb::solidMagenta;

				renderer->setColor(lineColor);
				renderer->drawLine(worldA,worldHit);
				renderer->drawLine(worldB,worldHit);

				renderer->setColor(VectorArgb::solidYellow);
				renderer->drawCylinder( Cylinder(worldHit,0.5f,0.2f), 16 );
			}
			else
			{
				VectorArgb lineColor = VectorArgb::solidGreen;

				renderer->setColor(lineColor);
				renderer->drawLine(worldA, worldEnd);
				renderer->drawLine(worldB, worldEnd);
			}

			// ----------
			// bubbles

			renderer->setColor(VectorArgb::solidCyan);
		}

		// ----------
		// draw the contacts

		float radiusTweak = 0.0f;

		for(ConstContactIterator it(getFloorList()); it; ++it)
		{
			FloorLocator const & loc = (*it)->m_contact;

			if(loc.isAttached())
			{
				renderer->setColor( VectorArgb::solidYellow );
			}
			else
			{
				renderer->setColor( VectorArgb::solidRed );
			}

			Vector position_w = loc.getPosition_w();

			float cylRadius = loc.getRadius() + radiusTweak;
			float cylHeight = loc.getOffset() + 0.02f;

			Cylinder C(position_w,cylRadius,cylHeight);

			renderer->drawCylinder(C,16);

			radiusTweak += 0.1f;
		}

		// draw the footprint
		{
			renderer->setColor( VectorArgb::solidCyan );

			Cylinder C(getPosition_w(),0.25,0.06f);

			renderer->drawCylinder(C,16);
		}

		// ----------

		renderer->setWorldSpace(false);
	}

#endif
}

// ----------------------------------------------------------------------

void Footprint::updateFloating ( void )
{
	bool reportFloating = ConfigSharedCollision::getReportFloating();

	m_parent->setIdle(false);

	CollisionWorld::getDatabase()->updateFloorCollision(m_parent,true);

	forceReattachContacts();
	snapContacts();

	updateHeights();

	int id = static_cast<int>(m_parent->getOwner().getNetworkId().getValue());

	if(!m_hasGroundHeight)
	{
		TerrainObject const * terrain = TerrainObject::getInstance();

		if(terrain && !terrain->isReferenceObject(&m_parent->getOwner()))
		{
			m_floating = false;
		}
		else
		{
			DEBUG_REPORT_LOG(reportFloating && !m_floating,("Floating: Object %d at height %f starting to float due to no ground height\n",id,m_addToWorldPos.y));
			m_floating = true;
		}
	}
	else
	{
		int elapsed = Clock::timeMs() - m_addToWorldTime;
		bool timeout = elapsed > (m_floatingTime * 1000);

		// Things newly added to the world are floating if the Y value of
		// their object is significantly (> 1 meter) above the ground

		if(m_addToWorldPos.y > (m_groundHeight + 0.5f))
		{
			if(timeout)
			{
				WARNING(reportFloating && m_floating,("Floating: Object %d at (%f,%f,%f) stopped floating due to timeout - ground at %f",id,m_addToWorldPos.x,m_addToWorldPos.y,m_addToWorldPos.z,m_groundHeight));
				m_floating = false;
			}
			else
			{
				DEBUG_REPORT_LOG(reportFloating && !m_floating,("Floating: Object %d starting to float due to being too high - object at %f, ground at %f\n",id,m_addToWorldPos.y,m_groundHeight));
				m_floating = true;
			}
		}
		else
		{
			if(m_hasFloorHeight && (m_floorHeight == m_groundHeight))
			{
				DEBUG_REPORT_LOG(reportFloating && m_floating,("Floating: Object %d stopped floating after %d msecs due to a floor appearing underneath it - object at %f, ground at %f\n",id,elapsed,m_addToWorldPos.y,m_groundHeight));
			}
			else
			{
				DEBUG_REPORT_LOG(reportFloating && m_floating,("Floating: Object %d stopped floating after %d msecs due to terrain appearing underneath it - object at %f, ground at %f\n",id,elapsed,m_addToWorldPos.y,m_groundHeight));
			}

			m_floating = false;
		}
	}
}

// ----------------------------------------------------------------------

void Footprint::stopFloating ( void )
{
	m_floating = false;
}

// ----------------------------------------------------------------------

void Footprint::setFloatingTime ( int seconds )
{
	m_floatingTime = seconds;
}

// ----------------------------------------------------------------------

void Footprint::alignToGroundNoFloat ( void )
{
	//-- Stop floating in the event that we are already marked as floating.
	stopFloating();

	//-- Lookup ground height.  This has the desired side effect of snapping us to the ground.
	bool const hasGroundHeight = updateHeights();
	UNREF(hasGroundHeight);
#ifdef _DEBUG
	if (!hasGroundHeight)
	{
		Object const *const owner = getOwner();
		if (owner)
			DEBUG_WARNING(true, ("Footprint::alignToGroundNoFloat(): called on object id=[%s],template=[%s] when no ground could be found.", owner->getNetworkId().getValueString().c_str(), owner->getObjectTemplateName()));
		else
			DEBUG_WARNING(true, ("Footprint::alignToGroundNoFloat(): called on object id=[<nullptr owner>],template=[<nullptr owner>] when no ground could be found."));
	}
#endif
}

// ----------------------------------------------------------------------

void Footprint::addToWorld ( void )
{
	IGNORE_RETURN( snapToObject() );

	if(isInCell() && !isAttached())
	{
		IGNORE_RETURN( snapToCellFloor() );
	}

	m_addToWorldTime = static_cast<int>(Clock::timeMs());
	m_addToWorldPos = getObjectPosition_p();

	updateFloating();
}

// ----------------------------------------------------------------------

float Footprint::getRadius ( void ) const
{
	// On the client, remote objects have a vry small radius

	if(m_parent && !m_parent->isServerSide() && !m_parent->isPlayer())
	{
		return 0.05f;
	}

	// On the server, AIs have a very small radius

	if(m_parent && m_parent->isServerSide() && !m_parent->isPlayerControlled())
	{
		return 0.05f;
	}

	// On both, players have a radius of 0.5

	if(m_parent && (m_parent->isPlayer() || m_parent->isPlayerControlled()))
	{
		// unless they're a mount and we have mount radii enabled, in which case we use the actual radius

		if(ConfigSharedCollision::getEnableMountRadii() && m_parent->isMount())
		{
			return m_radius;
		}
		else
		{
			return 0.5f;
		}
	}

	return m_radius;
}

// ----------------------------------------------------------------------

void Footprint::setRadius ( float newRadius )
{
	NAN_CHECK(newRadius);

	m_radius = newRadius;

	updateContactRadii();
}

// ----------------------------------------------------------------------

void Footprint::updateContactRadii( void )
{
	float radius = getRadius();

	for(ContactIterator it(getFloorList()); it; ++it)
	{
		FloorContactShape * contact = *it;

		if(!contact) continue;

		contact->m_contact.setRadius(radius);
	}
}

// ----------------------------------------------------------------------

void Footprint::cellChanged ( void )
{
	CellProperty const * objectCell = getObjectCell();

	if(m_parent->isServerSide())
	{
		snapToObject();
	}

	if(objectCell == CellProperty::getWorldCellProperty())
	{
		static FloorVec results;

		results.clear();

		Capsule queryCapsule_w = m_parent->getQueryCapsule_w();

		CollisionWorld::getDatabase()->queryFloors(queryCapsule_w,&results);

		for(uint i = 0; i < results.size(); i++)
		{
			Floor * floor = results.at(i);

			attachTo(floor,false);
		}
	}
	else
	{
		if(m_parent->isServerSide())
		{
			snapToCellFloor();
		}
	}
}

// ----------------------------------------------------------------------

void Footprint::objectWarped ( void )
{
		addToWorld();
}

// ----------------------------------------------------------------------

Vector Footprint::getGroundNormal_p ( void ) const
{
	return m_groundNormal;
}

// ----------------------------------------------------------------------

Vector Footprint::getGroundNormal_w ( void ) const
{
	return CollisionUtils::rotateToCell(getCell(),getGroundNormal_p(),CellProperty::getWorldCellProperty());
}

//----------------------------------------------------------------------

bool Footprint::isFloorWithinThreshold(FloorContactShape const & floorContactShape) const
{
	FloorLocator const & loc = floorContactShape.m_contact;
	
	Vector const & localContact = loc.getPosition_p(getCell());
	
	real const contactHeight = localContact.y;
	
	float floorHeight = 0.0f;
	if (getFloorHeight(floorHeight))
	{
		if(contactHeight < (floorHeight - 0.5f))
			return false;
	}

	return true;
}

// ======================================================================
