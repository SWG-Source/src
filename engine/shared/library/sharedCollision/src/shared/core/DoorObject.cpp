
// ======================================================================
//
// DoorObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/DoorObject.h"

#include "sharedCollision/BarrierObject.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/DoorInfo.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/NeighborObject.h"
#include "sharedCollision/SimpleExtent.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"

#include "sharedMath/DebugShapeRenderer.h"

#include "sharedObject/AlterResult.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Portal.h"
#include "sharedObject/Tweakable.h"

#include <algorithm>

static VectorArgb gs_forceFieldColor(0.6f,0.5f,0.5f,1.0f);

// ======================================================================

namespace DoorObjectNamespace
{
	int   const cs_maxHitByObjectsToTrack   = 25;
	float const cs_triggerRadiusFudgeFactor = 1.5f;
}

using namespace DoorObjectNamespace;

// ----------------------------------------------------------------------

class DoorCollisionProperty : public CollisionProperty
{
public:

    DoorCollisionProperty( DoorObject & owner )
	: CollisionProperty(owner)
	{
	}

	virtual bool canCollideWith ( CollisionProperty const * otherCollision ) const
	{
		UNREF(otherCollision);

		return true;
	}

	virtual bool blocksInteraction ( InteractionType interaction ) const
	{
		UNREF(interaction);

		return false;
	}

	virtual bool blocksMovement () const
	{
		return false;
	}

	virtual void hitBy ( CollisionProperty * dynamicCollider )
	{
		DoorObject & door = static_cast<DoorObject&>(getOwner());

		door.hitBy(dynamicCollider);
	}

protected:

private:

    DoorCollisionProperty();
    DoorCollisionProperty(const DoorCollisionProperty&);
    DoorCollisionProperty& operator= (const DoorCollisionProperty&);
};

// ----------------------------------------------------------------------

DoorObject::DoorObject ( DoorInfo const & info, Portal * portal )
: m_doorHelper	( info ),
  m_doorHelper2 ( info ),
  m_delta		( info.m_delta ),
  m_portal		( portal ),
  m_neighbor	( nullptr ),
  m_spring		( info.m_spring ),
  m_smoothness	( info.m_smoothness ),
  m_draw		( true ),
  m_barrier		( nullptr ),
  m_oldDoorPos  ( Vector::maxXYZ ),
  m_wasOpen(false),
  m_isForceField( false ),
  m_alwaysOpen  ( info.m_alwaysOpen ),
  m_triggerRadiusSquared ( sqr(info.m_triggerRadius * cs_triggerRadiusFudgeFactor) ),
  m_hitByObjects ()
{
	m_effects[0] = info.m_openBeginEffect;
	m_effects[1] = info.m_openEndEffect;
	m_effects[2] = info.m_closeBeginEffect;
	m_effects[3] = info.m_closeEndEffect;

	setDebugName("Door object");

	for (int i = 0; i < MAX_DRAWN_DOORS; ++i)
		m_drawnDoor[i] = nullptr;

	createAppearance(info);
	createTrigger(info);
	createBarrier(info);
}

// ----------------------------------------------------------------------

DoorObject::~DoorObject()
{
	m_hitByObjects.clear();
	m_portal = nullptr;
}

// ----------------------------------------------------------------------

Vector const DoorObject::getCurrentPos ( void ) const
{
	return tween( getOpenParam() ) * m_delta;
}

// ----------------------------------------------------------------------

Portal * DoorObject::getPortal ( void )
{
	return m_portal;
}

// ----------------------------------------------------------------------

Portal const * DoorObject::getPortal ( void ) const
{
	return m_portal;
}

// ----------------------------------------------------------------------

void DoorObject::setPortal ( Portal * newPortal )
{
	m_portal = newPortal;
}

// ----------------------------------------------------------------------

DoorObject * DoorObject::getNeighbor ( void )
{
	return m_neighbor;
}

// ----------------------------------------------------------------------

DoorObject const * DoorObject::getNeighbor ( void ) const
{
	return m_neighbor;
}

// ----------------------------------------------------------------------

int DoorObject::getNumberOfDrawnDoors ( void )
{
	for (int i = 0; i < MAX_DRAWN_DOORS; ++i)
		if (m_drawnDoor[i] == nullptr)
			return i;

	return MAX_DRAWN_DOORS;
}

// ----------------------------------------------------------------------

Object * DoorObject::getDrawnDoor ( int index )
{
	DEBUG_FATAL(index < 0 || index >= getNumberOfDrawnDoors(), ("door index out of range 0/%d/%d", index, getNumberOfDrawnDoors()));
	return m_drawnDoor[index];
}

// ----------------------------------------------------------------------

Object * DoorObject::getBarrier ( void )
{
	return m_barrier;
}

// ----------------------------------------------------------------------

void DoorObject::createAppearance ( DoorInfo const & info )
{
	if (info.m_frameAppearance && info.m_frameAppearance[0])
	{
		Appearance * const appearance = AppearanceTemplateList::createAppearance(info.m_frameAppearance); 

		if (appearance != nullptr) {
			setAppearance(appearance);
		} else {
			DEBUG_WARNING(true, ("FIX ME: Appearance template for DoorObject::createAppearance missing, first stanza."));
		}
	}

	m_drawnDoor[0] = new Object();
	m_drawnDoor[0]->attachToObject_p(this,true);

	if(info.m_forceField)
	{
		// Create the portal polygon and translate it
		// to the origin so that it matches the door
		// hardpoint

		m_isForceField = true;

		VertexList verts;
		getPortal()->getLocalVerts(verts);
		Collision3d::MovePolyOnto(verts,Vector::zero);
		Appearance * appearance = CellProperty::createForceField(verts,gs_forceFieldColor);

		if(appearance != nullptr)
		{
			m_drawnDoor[0]->setAppearance( appearance );
			Extent * appearanceExtent = new Extent( Containment3d::EncloseSphere(verts) );
			appearanceExtent->incrementReference();
			appearance->setExtent( appearanceExtent );
		} else {
			DEBUG_WARNING(true, ("FIX ME: Appearance template for DoorObject::createAppearance missing, second stanza."));
		}
	}
	else
	{
		if (info.m_doorAppearance && info.m_doorAppearance[0])
		{
			Appearance * const appearance = AppearanceTemplateList::createAppearance(info.m_doorAppearance);

			if (appearance != nullptr) {
				m_drawnDoor[0]->setAppearance(appearance);
			} else {
				DEBUG_WARNING(true, ("FIX ME: Appearance template for DoorObject::createAppearance missing, third stanza."));
			}
		}

		if (info.m_doorAppearance2 && info.m_doorAppearance2[0])
		{
			m_drawnDoor[1] = new Object();
			m_drawnDoor[1]->attachToObject_p(this,true);

			Appearance * const appearance = AppearanceTemplateList::createAppearance(info.m_doorAppearance2);

			if (appearance != nullptr) {
				m_drawnDoor[1]->setAppearance(appearance);
			} else {
				DEBUG_WARNING(true, ("FIX ME: Appearance template for DoorObject::createAppearance missing, fourth stanza."));
			}

			if (info.m_doorFlip2)
				m_drawnDoor[1]->yaw_o(PI);
		}
	}
}

// ----------------------------------------------------------------------

void DoorObject::createTrigger ( DoorInfo const & info )
{
	DoorCollisionProperty * collision = new DoorCollisionProperty(*this);

	Sphere collisionSphere( Vector(0.0f,1.0f,0.0f), info.m_triggerRadius );

	collision->attachSourceExtent( new SimpleExtent( MultiShape(collisionSphere) ) );

	addProperty(*collision);
}

// ----------------------------------------------------------------------

void DoorObject::createBarrier( DoorInfo const & info )
{
	UNREF(info);

	// Set up the barrier we use to keep players from going through doors they're
	// not allowed through

	// Force fields don't need appearances for their barriers

	if (!info.m_alwaysOpen)
	{
		m_barrier = new BarrierObject(this, !info.m_forceField);
	}
}

// ----------------------------------------------------------------------

float DoorObject::alter ( float time )
{
	time = AlterScheduler::getTimeSinceLastFrame();

	m_doorHelper.update(time);
	m_doorHelper.alter(time);
	m_doorHelper2.update(time);
	m_doorHelper2.alter(time);

	// ----------

	bool open = m_doorHelper.getPos() > 0.0f;

	if (m_portal)
	{
		if (m_alwaysOpen)
			m_portal->setClosed(false);
		else
			if (!m_isForceField)
				m_portal->setClosed(!open);
	}

	// ----------
	// play client effects via the hook

	PlayEffectHook playEffectHook = ConfigSharedCollision::getPlayEffectHook();

	DoorHelper::Event event = m_doorHelper.getEvent();

	if((event != DoorHelper::DHE_invalid) && playEffectHook && m_drawnDoor[0] && m_drawnDoor[0]->getAppearance())
	{
		const char *effect = m_effects[event];

		if(effect && effect[0])
		{
			bool playEffect = true;

			if ((event == DoorHelper::DHE_openBegin) && (!isPassageAllowed()))
			{
				playEffect = false;
			}

			if (playEffect)
			{
			// yuck, have to do an allocation to make a CrcLowerString

			const CrcLowerString c(effect);
			playEffectHook(c, m_drawnDoor[0], CrcLowerString::empty);
		}
	}
	}

	// ----------

	if(!m_isForceField)
	{
		if (m_drawnDoor[0])
		{
			Vector newPos = getCurrentPos();

			if(m_oldDoorPos != newPos)
			{
				m_drawnDoor[0]->setPosition_p( newPos );
				m_oldDoorPos = newPos;

				if (m_drawnDoor[1])
					m_drawnDoor[1]->setPosition_p( -newPos );
			}
		}
	}

	m_draw = true;

	// ----------
	// apply tweaks

	float tweak0 = tween(m_doorHelper.getPos());
	float tweak1 = tween(m_doorHelper2.getPos());

	if(getAppearance())
	{
		Tweakable * tweak = dynamic_cast<Tweakable*>(getAppearance());

		if(tweak)
		{
			tweak->setTweakValue(0,tweak0);
			tweak->setTweakValue(1,tweak1);
		}
	}

	if(m_drawnDoor[0])
	{
		Tweakable * tweak = dynamic_cast<Tweakable*>(m_drawnDoor[0]->getAppearance());

		if(tweak)
		{
			tweak->setTweakValue(0,tweak0);
			tweak->setTweakValue(1,tweak1);
		}
	}

	if(m_drawnDoor[1])
	{
		Tweakable * tweak = dynamic_cast<Tweakable*>(m_drawnDoor[1]->getAppearance());

		if(tweak)
		{
			tweak->setTweakValue(0,tweak0);
			tweak->setTweakValue(1,tweak1);
		}
	}

	if(m_barrier)
	{
		Tweakable * tweak = dynamic_cast<Tweakable*>(m_barrier->getAppearance());

		if(tweak)
		{
			tweak->setTweakValue(0,tweak0);
			tweak->setTweakValue(1,tweak1);
		}
	}

	// ----------
	// Check if there are any hit-by objects that are still within the door's trigger range.
	// If so, the door helper should be triggered as 'hit', this door object should still be
	// altered and the door will remain open.

	maintainHitByObjectVector();
	if (!m_hitByObjects.empty())
	{
		if (isPassageAllowed())
			m_doorHelper.hit();

		if (m_neighbor && m_neighbor->isPassageAllowed())
			m_neighbor->m_doorHelper.hit();
	}

	// ----------
	// Figure out if we need to alter next frame.
	float result = (m_hitByObjects.empty() && isClosed()) ? AlterResult::cms_keepNoAlter : AlterResult::cms_alterNextFrame;

	// Alter the base class.
	AlterResult::incorporateAlterResult(result, Object::alter(time));

	return result;
}

// ----------------------------------------------------------------------

float DoorObject::tween ( float t ) const
{
	// skew t using the spring factor to make the door seem to spring open faster

	float skew = 1.0f / m_spring;

	t = pow(t,skew);

	// and smooth it out with a cosine so it's not quite so jarring

	float smoothed = ( 1.0f - cos( t * PI ) ) / 2.0f;

	return (smoothed * m_smoothness) + ( t * (1.0f - m_smoothness) );
}

// ----------------------------------------------------------------------

void DoorObject::hitBy( CollisionProperty const * collision )
{
	if(collision == nullptr) return;

	CellProperty const * doorCell            = getParentCell();
	CellProperty const * doorNeighborCell    = m_neighbor ? m_neighbor->getParentCell() : doorCell;
	CellProperty const * colliderCurrentCell = collision->getCell();
	CellProperty const * colliderLastCell    = collision->getLastCell();

	// ignore things not in or moving into or out of the cell of the door or the door's neighbor.
	if ((colliderCurrentCell != doorCell) && (colliderLastCell != doorCell) && (colliderCurrentCell != doorNeighborCell) && (colliderLastCell != doorNeighborCell))
	{
		return;
	}

	scheduleForAlter();

	if(collision->isPlayer() && !isPassageAllowed())
	{
		// Even though we won't open the door here, we want to add this object to the
		// hitByObject list.  If the state of the door changes so that it is passable,
		// we want it to open (via alter()) without requiring the player to move.
		trackHitByObject(collision->getOwner());

		m_doorHelper2.hit();

		if(m_neighbor)
		{
			m_neighbor->m_doorHelper2.hit();
		}
	}
	else
	{
		// Track this object as a hit-by object.  The door will remain 'hit' if passable (i.e. open on the client)
		// until all hit by objects either are deleted or are farther away than the door trigger distance.
		trackHitByObject(collision->getOwner());

		if (isPassageAllowed())
			m_doorHelper.hit();

		if(m_neighbor && m_neighbor->isPassageAllowed())
		{
			m_neighbor->m_doorHelper.hit();
		}
	}
}

// ----------------------------------------------------------------------
// Only draw one of the pair of neighboring doors

void DoorObject::setNeighbor ( DoorObject * newNeighbor )
{
	DEBUG_FATAL(newNeighbor == this,("DoorObject::setNeighbor - Trying to make the door a neighbor of itself\n"));
	m_neighbor = newNeighbor;
	if(m_barrier)
	{
		if (newNeighbor)
			m_barrier->setNeighbor( newNeighbor->m_barrier );
		else
			m_barrier->setNeighbor( nullptr );
	}
}

// ----------------------------------------------------------------------

bool DoorObject::isPassageAllowed ( void ) const
{
	if(m_portal)
	{
		return m_portal->isPassageAllowed();
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool DoorObject::isAllPassageAllowed ( void ) const
{
	if(!isPassageAllowed())
	{
		return false;
	}

	if(m_neighbor && (!m_neighbor->isPassageAllowed()))
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void DoorObject::scheduleForAlter()
{
	NOT_NULL(getAttachedTo());
	getAttachedTo()->scheduleForAlter();
}


// ======================================================================
// class DoorObject: PRIVATE
// ======================================================================

void DoorObject::trackHitByObject(Object const &hitByObject)
{
	// Don't let this collection of tracked-by objects get too long.  Limit this amount.
	if (static_cast<int>(m_hitByObjects.size()) >= cs_maxHitByObjectsToTrack)
		return;

	// NOTE: This can't be a set because the key's value can change when a watcher's value changes from non-nullptr to nullptr.
	// Check if the object already exists in the list.
	if (std::find(m_hitByObjects.begin(), m_hitByObjects.end(), ConstWatcher<Object>(&hitByObject)) != m_hitByObjects.end())
		return;

	m_hitByObjects.push_back(ConstWatcher<Object>(&hitByObject));
}

// ----------------------------------------------------------------------

void DoorObject::maintainHitByObjectVector()
{
	if (m_hitByObjects.empty())
		return;

	Vector const doorPosition_w = getPosition_w();

	// Remove any objects that no longer exist or no longer are within the trigger radius.
	for (WatcherObjectVector::iterator it = m_hitByObjects.begin(); it != m_hitByObjects.end(); )
	{
		if (it->getPointer() == nullptr)
		{
			// This hit-by object has been deleted so clear it out of the tracking list.
			it = m_hitByObjects.erase(it);
			continue;
		}

		//-- Remove the tracking object if it is outside the trigger range of the door.

		// Get distance between track object and door.
		Vector const hitByObject_w          = it->getPointer()->getPosition_w();
		float const  distanceBetweenSquared = doorPosition_w.magnitudeBetweenSquared(hitByObject_w);

		if (distanceBetweenSquared > m_triggerRadiusSquared)
		{
			// Expel from list since hit by object has moved too far away.
			it = m_hitByObjects.erase(it);
		}
		else
		{
			// Process next item in list.
			++it;
		}
	}
}

// ----------------------------------------------------------------------
