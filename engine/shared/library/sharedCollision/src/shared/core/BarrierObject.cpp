// ======================================================================
//
// BarrierObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/BarrierObject.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/DoorObject.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/CollisionUtils.h"

#include "sharedMath/AxialBox.h"
#include "sharedMath/VectorArgb.h"

#include "sharedObject/Appearance.h"
#include "sharedObject/Portal.h"
#include "sharedObject/CellProperty.h"

static VectorArgb gs_barrierColor(0.85f,0.3f,0.02f,0.01f);

// ----------------------------------------------------------------------

class BarrierCollisionProperty : public CollisionProperty
{
public:

    explicit BarrierCollisionProperty( Object & owner ) 
	: CollisionProperty(owner)
	{
	}	

	virtual bool canCollideWith ( CollisionProperty const * otherCollision ) const
	{
		if(otherCollision == nullptr)
		{
			return false;
		}

		BarrierObject const * barrier = safe_cast<BarrierObject const *>(&getOwner());

		if(barrier == nullptr)
		{
			return false;
		}

		if(barrier->getParentCell() != otherCollision->getLastCell())
		{
			return false;
		}

		if(otherCollision->isPlayer())
		{
			return barrier->isActive();
		}
		else
		{
			return false;
		}
	}

	virtual bool blocksInteraction ( InteractionType interaction ) const
	{
		BarrierObject const * barrier = safe_cast<BarrierObject const *>(&getOwner());

		bool barrierActive = barrier && barrier->isActive();

		bool doorClosed = barrier && barrier->getDoor() && barrier->getDoor()->isClosed();

		if(barrierActive)
		{
			// The barrier is active. All interactions are blocked except IT_See, which
			// is blocked if the door is closed.

			if(interaction == IT_See)
			{
				return doorClosed;
			}
			else
			{
				return true;
			}
		}
		else
		{
			// The barrier is inactive. 

			return doorClosed;
		}
	}

	virtual bool blocksMovement () const
	{
		BarrierObject const * barrier = safe_cast<BarrierObject const *>(&getOwner());

		bool barrierActive = barrier && barrier->isActive();

		if(barrierActive)
		{
			return CollisionProperty::blocksMovement();
		}
		else
		{
			return false;
		}
	}


protected:

private:

    BarrierCollisionProperty();
    BarrierCollisionProperty(const BarrierCollisionProperty&);
    BarrierCollisionProperty& operator= (const BarrierCollisionProperty&);
};


// ----------------------------------------------------------------------

BarrierObject::BarrierObject(DoorObject const * door, bool needsAppearance)
: NeighborObject(),
  m_door(door)
{
	setDebugName("Barrier object");

	BarrierCollisionProperty * collision = new BarrierCollisionProperty(*this); //lint !e1524 new in constructor for class which has no explicit destructor
	addProperty(*collision);

	createCollisionExtent();

	if(needsAppearance)
	{
		createAppearance();
	}
}//lint !e429 custodial pointer collision has not been freed or returned // actually, addProperty takes custody of collision

// ----------------------------------------------------------------------

void BarrierObject::createCollisionExtent ( void )
{
	CollisionProperty * collision = getCollisionProperty();

	if(collision)
	{
		AxialBox box = m_door->getPortal()->getLocalBox();

		float r = 0.05f;

		box.setMax( box.getMax() + Vector(r,r,r) );
		box.setMin( box.getMin() - Vector(r,r,r) );

		SimpleExtent * extent = new SimpleExtent( MultiShape(box) );

		collision->attachSourceExtent(extent);
	}
}

// ----------------------------------------------------------------------

void BarrierObject::createAppearance ( void )
{
	// Create the portal polygon and translate it
	// to the origin so that it matches the door
	// hardpoint

	VertexList verts;

	m_door->getPortal()->getLocalVerts(verts);

	Collision3d::MovePolyOnto(verts,Vector::zero);

	// Create our barrier appearance

	Appearance * appearance = CellProperty::createPortalBarrier(verts,gs_barrierColor);

	if(appearance != nullptr)
	{
		setAppearance( appearance );

		// Create an extent for the barrier appearance - without it the barrier won't be rendered

		Extent * appearanceExtent = new Extent( Containment3d::EncloseSphere(verts) );

		IGNORE_RETURN(appearanceExtent->incrementReference()); // Extent::incrementReference() returns the ref count, not an error condition that needs to be checked

		appearance->setExtent( appearanceExtent );
	}//lint !e429 custodial pointer 'appearanceExtent' has not been freed or returned // setExtent takes custody
}

// ----------------------------------------------------------------------

// ----------
// Barriers are active if their doors don't allow passage

bool BarrierObject::isActive ( void ) const
{
	return m_door && (!m_door->isPassageAllowed());
}

// Barriers should be rendered if their doors disallow passage in either
// direction

bool BarrierObject::shouldRender ( void ) const
{
	return m_door && m_door->isOpen() && (!m_door->isPassageAllowed());
}

DoorObject const * BarrierObject::getDoor ( void ) const
{
	return m_door;
}

// ----------------------------------------------------------------------
