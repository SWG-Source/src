// ======================================================================
//
// CollisionResolve.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionResolve.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Resolve3d.h"
#include "sharedCollision/Contact3d.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/ComponentExtent.h"
#include "sharedCollision/DetailExtent.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/Footprint.h"
#include "sharedCollision/FloorContactShape.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"

#include "sharedMath/Cylinder.h"
#include "sharedMath/ShapeUtils.h"

#include "sharedObject/Object.h"
#include "sharedObject/CellProperty.h"

#include <vector>

typedef std::vector<FloorContactShape *> FloorContactList;

typedef MultiListDataIterator<FloorContactShape> ContactIterator;

// ======================================================================

Vector MoveSegment::getBegin ( CellProperty const * relativeCell ) const
{
	return CollisionUtils::transformToCell(m_cellA,m_pointA,relativeCell);
}

Vector MoveSegment::getEnd ( CellProperty const * relativeCell ) const
{
	return CollisionUtils::transformToCell(m_cellB,m_pointB,relativeCell);
}

Vector MoveSegment::getDelta ( CellProperty const * relativeCell ) const
{
	return getEnd(relativeCell) - getBegin(relativeCell);
}

Vector MoveSegment::getDelta_b ( void ) const
{
	return m_pointB - getBegin(m_cellB);
}

Vector MoveSegment::getDelta_a ( void ) const
{
	return getEnd(m_cellA) - m_pointA;
}

Vector MoveSegment::atParam ( CellProperty const * relativeCell, float time ) const
{
	Vector begin = getBegin(relativeCell);
	Vector end = getEnd(relativeCell);

	return begin + (end-begin)*time;
}

float MoveSegment::lengthSquared ( void ) const
{
	return getDelta_a().magnitudeSquared();
}

// ======================================================================

namespace CollisionResolveNamespace
{
	int ms_colliderCount = 0;
	int ms_obstacleCount = 0;
	int ms_collisionCount = 0;
	int ms_bounceCount = 0;

	ObstacleList ms_obstacleList;
	ContactList ms_contactList;

	FloorContactList ms_floorContactList;

	Floor const * ms_ignoreFloor = nullptr;
	int ms_ignoreTriId = -1;
	int ms_ignoreEdge = -1;
	Vector ms_ignoreNormal = Vector::zero;
	float ms_ignoreTime = REAL_MAX;

	bool ms_debugBackupContacts = false;
	bool ms_debugTestClear = false;
	bool ms_debugRedoResolve = false;

};

using namespace CollisionResolveNamespace;

// ----------------------------------------------------------------------

void CollisionResolve::install ( void )
{
}

void CollisionResolve::remove ( void )
{
}

// ----------------------------------------------------------------------
// Resolve all collisions in the collision world

int CollisionResolve::getColliderCount ( void )
{
	return ms_colliderCount;
}

int CollisionResolve::getObstacleCount ( void )
{
	return ms_obstacleCount;
}

int CollisionResolve::getCollisionCount ( void )
{
	return ms_collisionCount;
}

int CollisionResolve::getBounceCount ( void )
{
	return ms_bounceCount;
}

void CollisionResolve::resetCounters ( void )
{
	ms_colliderCount = 0;
	ms_obstacleCount = 0;
	ms_collisionCount = 0;
	ms_bounceCount = 0;
}

// ----------------------------------------------------------------------
// @todo  - handle extents in cells

void translateExtent ( CellProperty const * cell, BaseExtent * extent, Vector const & delta )
{
	UNREF(cell);

	Transform tform = Transform::identity;

	tform.move_p(delta);

	extent->transform(extent,tform,1.0f);
}

void translateExtent ( CellProperty const * cell, BaseExtent * extent, MoveSegment const & moveSeg, float moveParam )
{
	Transform tform = Transform::identity;

	Vector delta = moveSeg.getDelta(cell);

	delta *= moveParam;

	tform.move_p(delta);

	extent->transform(extent,tform,1.0f);
}

void translateFloorContacts ( FloorContactList & floorContacts, MoveSegment const & moveSeg, float moveParam, int ignoreTriId, int ignoreEdge )
{
	int floorContactCount = ms_floorContactList.size();

	for(int i = 0; i < floorContactCount; i++)
	{
		FloorContactShape * shape = floorContacts[i];

		FloorLocator & loc = shape->m_contact;

		Floor const * floor = loc.getFloor();

		CellProperty const * floorCell = loc.getCell();

		Vector begin = loc.getPosition_p();
		Vector end = moveSeg.getEnd(floorCell);

		Vector delta = end - begin;

		Vector goal = begin + delta * moveParam;

		if(floor)
		{
			FloorLocator tempLoc;

			IGNORE_RETURN( floor->moveLocator( loc, goal, ignoreTriId, ignoreEdge, tempLoc ) );

			loc = tempLoc;
		}
		else
		{
			Vector end = moveSeg.getEnd(loc.getCell());

			loc.setPosition_p(end);
		}
	}
}

// ----------------------------------------------------------------------

void moveObjectAlong ( Object * object, CellPositionList const & positions )
{
	int count = positions.size();

	for(int i = 0; i < count; i++)
	{
		CellProperty * const objectCell = object->getParentCell();

		CellProperty const * newCell = positions[i].m_cell;
		Vector const & newEnd = positions[i].m_position;

		Vector localEnd = CollisionUtils::transformToCell(newCell,newEnd,objectCell);

		object->setPosition_p( localEnd );
	}
}

// ----------

void CollisionResolve::translateEverything ( CellProperty const * cell, SimpleExtent * extent, FloorContactList & floorContacts, MoveSegment const & moveSeg, float moveParam, int ignoreTriId, int ignoreEdge )
{
	translateExtent(cell,extent,moveSeg,moveParam);
	translateFloorContacts(floorContacts,moveSeg,moveParam,ignoreTriId,ignoreEdge);
}

// ----------------------------------------------------------------------
// Explode the collidee list into a list of simple extents, move the
// collider back to the beginning of the timestep, resolve all collisions
// during the timestep, and then move the collider's object to the
// resolved position.

ResolutionResult CollisionResolve::resolveCollisions(CollisionProperty * colliderA, ColliderList const & colliderList)
{
	NOT_NULL(colliderA);

	if (colliderList.empty())
	{
		Footprint * foot = colliderA->getFootprint();

		NOT_NULL(foot);

		if(!foot->hasContacts())
		{
			return RR_NoCollision;
		}
	}

	Object * objectA = &(colliderA->getOwner());

	NOT_NULL(objectA);

	MoveSegment moveSeg;

	moveSeg.m_cellA  = colliderA->getLastCell();
	moveSeg.m_pointA = colliderA->getLastPos_p();
	moveSeg.m_cellB  = objectA->getParentCell();
	moveSeg.m_pointB = objectA->getPosition_p();

	if(moveSeg.lengthSquared() == 0.0f)
	{
		return RR_NoCollision;
	}

	ResolutionResult result = resolveCollisions(colliderA, moveSeg, colliderList);

	return result;
}

// ----------------------------------------------------------------------

ResolutionResult CollisionResolve::resolveCollisions(CollisionProperty * colliderA, MoveSegment const & moveSeg, ColliderList const & colliderList)
{
	Object * objectA = &(colliderA->getOwner());

	BaseExtent const * baseExtentA = colliderA->getExtent_p();
	if (!baseExtentA)
		return RR_NoCollision;

	CellProperty const * objectCell = objectA->getParentCell();
	SimpleExtent const * objectExtent = safe_cast<SimpleExtent const *>(baseExtentA);

	NOT_NULL(objectExtent);
	if (!objectExtent)
		return RR_NoCollision;

	explodeCollider(colliderA, colliderList);

	// ----------

	SimpleExtent * tempExtent = safe_cast<SimpleExtent*>(objectExtent->clone());

	translateExtent(objectCell,tempExtent,-moveSeg.getDelta(objectCell));

	static CellPositionList positions;

	positions.clear();

	ResolutionResult result = resolveCollisions(objectCell,tempExtent,ms_floorContactList,moveSeg,&ms_obstacleList,positions, colliderA->getFootprint());

	delete tempExtent;

	if(result == RR_Resolved)
	{
		colliderA->hitBy(nullptr);
		colliderA->setExtentsDirty(true);

		Vector resetPos = moveSeg.getBegin(moveSeg.m_cellB);

		objectA->setPosition_p(resetPos);

		moveObjectAlong(objectA,positions);
	}

	return result;
}

// ----------------------------------------------------------------------

ResolutionResult CollisionResolve::resolveCollisions ( CellProperty const * cellA, SimpleExtent * extentA, FloorContactList & floorContactsA,
													   MoveSegment const & startMoveSeg,
													   ObstacleList * obstacleList,
													   CellPositionList & outList, Footprint * footprint )
{
	CellProperty const * startCell = startMoveSeg.m_cellA;
	Vector startDelta = startMoveSeg.getDelta(startCell);

	MoveSegment moveSeg = startMoveSeg;

	const int iterationCount = 8;

	ms_ignoreFloor = nullptr;
	ms_ignoreTriId = -1;
	ms_ignoreEdge = -1;
	ms_ignoreTime = REAL_MAX;
	ms_ignoreNormal = Vector::zero;

	for(int i = 0; i < iterationCount; i++)
	{
		int minIndex = -1;
		Contact contact;

		bool foundContact = findFirstContact(cellA,extentA,moveSeg,obstacleList,minIndex,contact, footprint);

		if(!foundContact)
		{
			outList.push_back( CellPosition(moveSeg.m_cellB,moveSeg.m_pointB) );

			translateEverything(cellA,extentA,floorContactsA,moveSeg,1.0f,ms_ignoreTriId,ms_ignoreEdge);

			if(i == 0)
			{
				return RR_NoCollision;
			}
			else
			{
				return RR_Resolved;
			}
		}

		if(!contact.m_resolvable)
		{
			return RR_DontKnowHow;
		}

		if(!contact.m_exists)
		{
			return RR_Resolved;
		}

		// ----------

		ms_bounceCount++;

		CellProperty const * contactCell = contact.m_cell;

		Vector contactPoint = moveSeg.atParam(contactCell,contact.m_time);

		outList.push_back( CellPosition(contactCell,contactPoint) );

		MoveSegment secondSeg;

		secondSeg.m_cellA = contactCell;
		secondSeg.m_pointA = contactPoint;

		secondSeg.m_cellB = moveSeg.m_cellB;
		secondSeg.m_pointB = moveSeg.m_pointB;

		if(contact.m_time != 0.0f)
		{
			translateEverything(cellA,extentA,floorContactsA,moveSeg,contact.m_time,ms_ignoreTriId,ms_ignoreEdge);
		}

		// ----------

		int contactCount = ms_contactList.size() + 1;

		if(contactCount == 1)
		{
			Vector delta = secondSeg.getDelta(contactCell);

			Vector slidDelta = Collision3d::SingleSlide(delta,contact.m_normal);

			// Stop resolution if the velocity starts to move backwards

			Vector localStartDelta = CollisionUtils::rotateToCell(startCell,startDelta,contactCell);

			if(slidDelta.dot(localStartDelta) <= 0.0f)
			{
				return RR_Resolved;
			}

			ms_contactList.push_back(contact);

			moveSeg.m_cellA = contactCell;
			moveSeg.m_pointA = contactPoint;

			moveSeg.m_cellB = contactCell;
			moveSeg.m_pointB = contactPoint + slidDelta;
		}
		else if (contactCount == 2)
		{
			Vector delta = secondSeg.getDelta(contactCell);

			Contact contactA = contact;
			Contact contactB = ms_contactList.front();

			Vector normalA = contactA.m_normal;
			Vector normalB = CollisionUtils::rotateToCell(contactB.m_cell,contactB.m_normal,contactA.m_cell);

			Vector slidDelta = Collision3d::DoubleSlide(delta,normalA,normalB);

			// Stop resolution if the velocity starts to move backwards

			Vector localStartDelta = CollisionUtils::rotateToCell(startCell,startDelta,contactCell);

			if(slidDelta.dot(localStartDelta) <= 0.0f)
			{
				return RR_Resolved;
			}

			// ----------
			// Keep only the contacts that the new velocity is not moving away
			// from

			ms_contactList.clear();

			if(slidDelta.dot(normalA) < 0.0001f)
			{
				ms_contactList.push_back(contactA);
			}

			if(slidDelta.dot(normalB) < 0.0001f)
			{
				ms_contactList.push_back(contactB);
			}

			// ----------

			moveSeg.m_cellA = contactCell;
			moveSeg.m_pointA = contactPoint;

			moveSeg.m_cellB = contactCell;
			moveSeg.m_pointB = contactPoint + slidDelta;
		}
		else
		{
			// triple-contact - stop movement because this is too much of a pain
			// to resolve correctly.

			return RR_Resolved;
		}

		// ----------
		// Remove the hit obstacle from the extent list

		if(obstacleList->at(minIndex).m_extent != nullptr)
		{
			obstacleList->at(minIndex) = obstacleList->back();
			obstacleList->resize(obstacleList->size()-1);

			ms_ignoreFloor = nullptr;
			ms_ignoreTriId = -1;
			ms_ignoreEdge = -1;
			ms_ignoreTime = REAL_MAX;
			ms_ignoreNormal = Vector::zero;
		}
		else
		{
			ms_ignoreFloor = obstacleList->at(minIndex).m_floorContact->m_contact.getFloor();
			ms_ignoreTriId = contact.m_surfaceId1;
			ms_ignoreEdge = contact.m_surfaceId2;
			ms_ignoreTime = contact.m_time;
			ms_ignoreNormal = contact.m_normal;
		}
	}

	// For all practical purposes the code should never get here.

	DEBUG_WARNING(true,("CollisionResolve::resolveCollisions - Resolution went through 8 iterations without success"));

	outList.push_back( CellPosition(moveSeg.m_cellA,moveSeg.m_pointA) );

	return RR_Resolved;
}

// ----------------------------------------------------------------------

void CollisionResolve::explodeExtent ( CellProperty const * cell, SimpleExtent const * extent, ObstacleList & list )
{
	list.push_back( ObstacleInfo(cell,extent) );
}

// ----------

void CollisionResolve::explodeExtent ( CellProperty const * cell, ComponentExtent const * extent, ObstacleList & list )
{
	int extentCount = extent->getExtentCount();

	for(int i = 0; i < extentCount; i++)
	{
		explodeExtent(cell,extent->getExtent(i),list);
	}
}

// ----------

void CollisionResolve::explodeExtent ( CellProperty const * cell, DetailExtent const * extent, ObstacleList & list )
{
	int extentCount = extent->getExtentCount();

	explodeExtent(cell,extent->getExtent(extentCount-1),list);
}

// ----------

void CollisionResolve::explodeExtent ( CellProperty const * cell, BaseExtent const * extent, ObstacleList & list )
{
	NOT_NULL(extent);

	ExtentType type = extent->getType();

	// ----------

	if     (type == ET_Simple)    explodeExtent(cell,safe_cast<SimpleExtent const *>(extent),list);
	else if(type == ET_Component) explodeExtent(cell,safe_cast<ComponentExtent const *>(extent),list);
	else if(type == ET_Detail)    explodeExtent(cell,safe_cast<DetailExtent const *>(extent),list);
}

// ----------

void CollisionResolve::explodeFootprint ( Footprint * foot, ObstacleList & obstacleList, FloorContactList & floorContacts )
{
	if(foot == nullptr) return;

	for(ContactIterator it(foot->getFloorList()); it; ++it)
	{
		FloorContactShape * contact = (*it);

		if(contact == nullptr) 
			continue;

		obstacleList.push_back( ObstacleInfo(contact->m_contact.getCell(),contact) );
		floorContacts.push_back(contact);
	}
}

// ----------
// Explode the extents of all colliders into SimpleExtents, accumuluate
// the extents into ms_obstacleList

void CollisionResolve::explodeCollider(CollisionProperty * colliderA, ColliderList const & colliderList)
{
	NOT_NULL(colliderA);

	ms_obstacleList.clear();
	ms_contactList.clear();
	ms_floorContactList.clear();

	ColliderList::const_iterator ii = colliderList.begin();
	ColliderList::const_iterator iiEnd = colliderList.end();

	for(; ii != iiEnd; ++ii)
	{
		CollisionProperty const * const colliderB = *ii;

		if(colliderB == nullptr) continue;

		BaseExtent const * const extentB = colliderB->getExtent_p();

		explodeExtent(colliderB->getOwner().getParentCell(), extentB, ms_obstacleList);
	}

	if(!(colliderA->isServerSide() && colliderA->isPlayerControlled()) && ConfigSharedCollision::getEnableFloorCollision())
	{
		explodeFootprint(colliderA->getFootprint(),ms_obstacleList,ms_floorContactList);
	}
	else
	{
		Footprint * const foot = colliderA->getFootprint();

		for(ContactIterator it(foot->getFloorList()); it; ++it)
		{
			FloorContactShape * const contact = (*it);

			if(contact == nullptr) continue;

			ms_floorContactList.push_back(contact);
		}
	}

	ms_obstacleCount += ms_obstacleList.size();
}

// ----------------------------------------------------------------------

bool CollisionResolve::findFirstContact ( CellProperty const * cellA, SimpleExtent const * extentA, MoveSegment const & moveSeg, ObstacleList * obstacleList, int & outMinIndex, Contact & outContact, Footprint * footprint )
{
	int extentCount = obstacleList->size();

	outContact = Contact::noContact;
	outMinIndex = -1;

	for(int i = 0; i < extentCount; i++)
	{
		ObstacleInfo & obstacle = obstacleList->at(i);

		Contact contact = findContactWithObstacle(cellA,extentA,moveSeg,obstacle, footprint);

		if(!contact.m_resolvable)
		{
			continue;
		}
		else if(contact.m_exists && (contact.m_time < outContact.m_time))
		{
			if(contact.m_time < 0.0f)
			{
				Vector delta = moveSeg.getDelta(contact.m_cell);

				float stepDistance = delta.magnitude();

				float contactDistance = stepDistance * -contact.m_time;

				if(contactDistance > 0.05f) continue;
			}

			outContact = contact;
			outMinIndex = i;
		}
	}

	return outContact.m_exists;
}

// ----------------------------------------------------------------------

Contact CollisionResolve::findContactWithExtent ( CellProperty const * cellA, SimpleExtent const * extentA, MoveSegment const & moveSeg, CellProperty const * cellB, SimpleExtent const * extentB )
{
	NOT_NULL(extentA);
	NOT_NULL(extentB);

	MultiShape const & shapeA = extentA->getShape();
	MultiShape const & shapeB = extentB->getShape();

	MultiShape tempShapeA = shapeA;

	if(cellA != cellB)
	{
		tempShapeA = CollisionUtils::transformToCell(cellA,shapeA,cellB);
	}

	Vector delta = moveSeg.getDelta(cellA);

	Contact temp = Contact3d::FindContact(tempShapeA,delta,shapeB);

	temp.m_cell = cellB;
	temp.m_extent = extentB;

	Vector normV = delta;

	IGNORE_RETURN(normV.normalize());

	float dot = normV.dot(temp.m_normal);

	if(dot < 0.0f)
	{
		return temp;
	}
	else
	{
		return Contact::noContact;
	}
}

// ----------------------------------------------------------------------

Contact CollisionResolve::findContactWithFloor ( FloorContactShape * floorContact, MoveSegment const & moveSeg, Footprint * footprint )
{
	NOT_NULL(floorContact);

	FloorLocator & loc = floorContact->m_contact;
	FloorLocator & result = floorContact->m_result;

	Floor const * floor = loc.getFloor();

	if(floor == nullptr) return Contact::noContact;

	CellProperty const * floorCell = loc.getCell();

	Vector begin = loc.getPosition_p();
	Vector end = moveSeg.getEnd(floorCell);

	Vector delta = end - begin;

	// The Y component must be zeroed out or floor contacts far above/below the avatar can
	// produce erroneous results

	delta.y = 0.0f;

	PathWalkResult walkResult;

	if(delta.magnitudeSquared() == 0.0f)
	{
		return Contact::noContact;
	}

	if (footprint)
	{ 
		if (!footprint->isFloorWithinThreshold(*floorContact))
			return Contact::noContact;
	}

	if((ms_ignoreFloor == nullptr) || (floor != ms_ignoreFloor))
	{
		walkResult = floor->canMove(loc,delta,-1,-1,result);
	}
	else
	{
		walkResult = floor->canMove(loc,delta,ms_ignoreTriId,ms_ignoreEdge,result);
	}

	Contact tempContact = Contact::noContact;

	if(walkResult != PWR_WalkOk)
	{
		if((walkResult == PWR_HitEdge) || (walkResult == PWR_InContact) || (walkResult == PWR_HitBeforeEnter) || (walkResult == PWR_HitPast) || (walkResult == PWR_CenterHitEdge) || (walkResult == PWR_CenterInContact))
		{
			Vector normV = delta;

			IGNORE_RETURN(normV.normalize());

			float dot = normV.dot(result.getContactNormal());

			if(dot < 0.0f)
			{
				if((result.getContactNormal() != ms_ignoreNormal) || (result.getTime() != ms_ignoreTime))
				{
					tempContact.m_exists     = true;
					tempContact.m_resolvable = true;

					tempContact.m_time       = result.getTime();
					tempContact.m_point      = Vector::zero; //result.clipPoint;
					tempContact.m_normal     = result.getContactNormal();

					tempContact.m_surfaceId1 = result.getHitTriId();
					tempContact.m_surfaceId2 = result.getHitEdgeId();

					tempContact.m_cell       = floorCell;
					tempContact.m_extent     = nullptr;
					tempContact.m_surface    = result.getFloor();
				}
			}
		}
	}

	return tempContact;
}

// ----------------------------------------------------------------------

Contact CollisionResolve::findContactWithObstacle ( CellProperty const * cellA, SimpleExtent const * extentA, MoveSegment const & seg, ObstacleInfo const & obstacle, Footprint * footprint )
{
	NOT_NULL(extentA);

	if(obstacle.m_extent)
	{
		return findContactWithExtent(cellA,extentA,seg,obstacle.m_cell,obstacle.m_extent);
	}
	else if(obstacle.m_floorContact)
	{
		return findContactWithFloor(obstacle.m_floorContact,seg, footprint);
	}
	else
	{
		return Contact::noContact;
	}
}

// ----------------------------------------------------------------------

/*
ResolutionResult CollisionResolve::resolveCollisionAsCylinders( SimpleExtent const * extentA,
                                                                Vector const & delta,
                                                                SimpleExtent const * extentB,
                                                                Vector & outDelta )
{
	NOT_NULL(extentA);
	NOT_NULL(extentB);

	MultiShape shapeA = extentA->getShape();
	MultiShape shapeB = extentB->getShape();

	// Stretch the cylinders vertically so that players can't squeeze under creatures

	shapeA.setExtentY( shapeA.getExtentY() * 5.0f );
	shapeB.setExtentY( shapeB.getExtentY() * 5.0f );

	return Resolve3d::Resolve(shapeA.getCylinder(),delta,shapeB.getCylinder(),outDelta);
}
*/


// ----------------------------------------------------------------------

