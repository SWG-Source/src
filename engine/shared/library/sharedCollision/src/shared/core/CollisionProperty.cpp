// ======================================================================
//
// CollisionProperty.h.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionProperty.h"

#include "sharedCollision/BarrierObject.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionNotification.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/ComponentExtent.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/CylinderExtent.h"
#include "sharedCollision/DetailExtent.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/DoorObject.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorManager.h"
#include "sharedCollision/Footprint.h"
#include "sharedCollision/MeshExtent.h"
#include "sharedCollision/SimpleExtent.h"

#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedShipObjectTemplate.h"

#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/SphereTree.h"

#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedObject/World.h"

namespace CollisionPropertyNamespace
{
	CollisionProperty * ms_activeListHead = nullptr;
};

using namespace CollisionPropertyNamespace;

class CollisionSphereAccessor;

// ----------------------------------------------------------------------
// active/idle list methods

void CollisionProperty::detachList ( void )
{
	if(m_prev) m_prev->m_next = m_next;
	if(m_next) m_next->m_prev = m_prev;

	if(this == ms_activeListHead)
	{
		ms_activeListHead = m_next;
	}

	m_prev = nullptr;
	m_next = nullptr;
}

// ----------

void CollisionProperty::attachList ( CollisionProperty * & head )
{
	detachList();

	if(head) head->m_prev = this;

	m_prev = nullptr;
	m_next = head;

	head = this;
}

// ----------

void CollisionProperty::updateIdle ( void )
{
	m_idleCounter++;

	if(m_idleCounter >= 3)
	{
		if(m_footprint)
		{
			storePosition();
		}

		setIdle(true);
	}
}

// ----------

void CollisionProperty::setIdle ( bool idle )
{
	if(isMobile())
	{
		if(idle)
			detachList();
		else
		{
			m_idleCounter = std::min(m_idleCounter,0);

			attachList(ms_activeListHead);
		}
	}

	modifyFlags(F_idle, idle);
}

// ----------

CollisionProperty * CollisionProperty::getActiveHead ( void )
{
	return ms_activeListHead;
}

// ----------------------------------------------------------------------

Transform getTransform_o2c( Object const * object )
{
	if(object == nullptr) return Transform::identity;

	// If this object is a cell, its o2c transform is the identity transform

	if( object->getCellProperty() )
	{
		return Transform::identity;
	}

	// Otherwise this object's o2c transform is the product of its o2p transform
	// and its parent's o2c transform.

	Transform objectToParent = object->getTransform_o2p();
	Transform parentToCell = getTransform_o2c( object->getAttachedTo() );

	Transform objectToCell;

	objectToCell.multiply( parentToCell, objectToParent );

	return objectToCell;
}

// ======================================================================

PropertyId CollisionProperty::getClassPropertyId ( void )
{
	return PROPERTY_HASH(Collision, 0x01D26659);
}

// ----------------------------------------------------------------------

CollisionProperty::CollisionProperty( Object & owner )
: Property( CollisionProperty::getClassPropertyId(), owner ),
  m_lastTransform_w(Transform::identity),
  m_lastTransform_p(Transform::identity),
  m_lastCellObject(nullptr),
  m_stepHeight(0.0f),
  m_defaultRadius(0.0f),
  m_offsetX(0.0f),
  m_offsetZ(0.0f),
  m_extentsDirty(true),
  m_extent_l(nullptr),
  m_extent_p(nullptr),
  m_sphere_l(),
  m_sphere_w(),
  m_scale(owner.getScale().x),
  m_spatialSubdivisionHandle(nullptr),
  m_floor(nullptr),
  m_footprint(nullptr),
  m_idleCounter(3),
  m_next(nullptr),
  m_prev(nullptr),
  m_flags(F_collidable),
  m_spatialDatabaseStorageType(SpatialDatabase::Q_None)
{
	owner.addNotification(CollisionNotification::getInstance());

	SharedObjectTemplate const * objTemplate = owner.getObjectTemplate() ? owner.getObjectTemplate()->asSharedObjectTemplate() : 0;

	initFromTemplate(objTemplate);

	initFloor();

	if(objTemplate)
	{
		char const * const templateName = objTemplate->getName();

		if(templateName)
		{
			char const * const found = strstr(templateName,"lair");

			if(found != nullptr)
			{
				setCollidable(false);
			}
		}
	}
}

// ----------

CollisionProperty::CollisionProperty( Object & owner, SharedObjectTemplate const * objTemplate )
: Property( CollisionProperty::getClassPropertyId(), owner ),
  m_lastTransform_w(Transform::identity),
  m_lastTransform_p(Transform::identity),
  m_lastCellObject(nullptr),
  m_stepHeight(0.0f),
  m_defaultRadius(0.0f),
  m_offsetX(0.0f),
  m_offsetZ(0.0f),
  m_extentsDirty(true),
  m_extent_l(nullptr),
  m_extent_p(nullptr),
  m_sphere_l(),
  m_sphere_w(),
  m_scale(owner.getScale().x),
  m_spatialSubdivisionHandle(nullptr),
  m_floor(nullptr),
  m_footprint(nullptr),
  m_idleCounter(3),
  m_next(nullptr),
  m_prev(nullptr),
  m_flags(F_collidable),
  m_spatialDatabaseStorageType(SpatialDatabase::Q_None)
{
	owner.addNotification(CollisionNotification::getInstance());

	initFromTemplate(objTemplate);

	initFloor();

	if(objTemplate)
	{
		char const * const templateName = objTemplate->getName();

		if(templateName)
		{
			char const * const found = strstr(templateName,"lair");

			if(found != nullptr)
			{
				setCollidable(false);
			}
		}
	}
}

// ----------

void CollisionProperty::initFromTemplate( SharedObjectTemplate const * objTemplate )
{
	if (!objTemplate)
	{
		m_spatialDatabaseStorageType = SpatialDatabase::Q_Static;

		return;
	}

	SharedCreatureObjectTemplate const * creatureTemplate = dynamic_cast<SharedCreatureObjectTemplate const *>(objTemplate);

	if(creatureTemplate)
	{
        setMobile(true);

		m_spatialDatabaseStorageType = SpatialDatabase::Q_Dynamic;

		if(strstr(objTemplate->getCrcName().getString(), "player") != 0)
		{
			setPlayerControlled(true);
		}

		m_stepHeight = creatureTemplate->getStepHeight();
		m_defaultRadius = creatureTemplate->getCollisionRadius();
		m_offsetX = creatureTemplate->getCollisionOffsetX();
		m_offsetZ = creatureTemplate->getCollisionOffsetZ();

		const float waterHeight = creatureTemplate->getSwimHeight ();

        m_footprint = new Footprint( getOwner().getPosition_p(), getCollisionRadius(), this, waterHeight );
	}
	else
	{
		if (objTemplate->getForceNoCollision())
		{
			setCollidable(false);
		}

		int const gameObjectType = objTemplate->getGameObjectType();
		if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_ship))
		{
			setShip(true);

			SharedShipObjectTemplate const * const shipObjectTemplate = dynamic_cast<SharedShipObjectTemplate const *>(objTemplate);
			if (shipObjectTemplate)
			{
				setMobile(true);

				if (shipObjectTemplate->getPlayerControlled())
					setPlayerControlled(true);
			}

			// ships don't have a radius specified in the template, but we'll give them one once we've updated
			// the extents. to start, just give them a 1 meter default radius.

			m_stepHeight = 0.0f;
			m_defaultRadius = 1.0f;
			m_offsetX = 0.0f;
			m_offsetZ = 0.0f;

			if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_ship_station))
			{
				m_spatialDatabaseStorageType = SpatialDatabase::Q_Static;
			}
			else if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_ship_capital))
			{
				// this is a hack, putting the capital ships in the static database
				// this is done for the server to lessen the load...
				m_spatialDatabaseStorageType = SpatialDatabase::Q_Static;
			}
			else if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_ship_transport))
			{
				// this is a hack, putting the transport ships in the static database
				// this is done for the server to lessen the load...
				m_spatialDatabaseStorageType = SpatialDatabase::Q_Static;
			}
			else if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_ship_mining_asteroid_static))
			{
				m_spatialDatabaseStorageType = SpatialDatabase::Q_Static;
			}
			else if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic))
			{
				m_spatialDatabaseStorageType = SpatialDatabase::Q_Dynamic;
			}
			else
			{
				// all other ships
				m_spatialDatabaseStorageType = SpatialDatabase::Q_Dynamic;
			}
		}

		else if (GameObjectTypes::isTypeOf(gameObjectType, SharedObjectTemplate::GOT_ship_component))
		{
			m_spatialDatabaseStorageType = SpatialDatabase::Q_Dynamic;
		}

		else // everything else should be static
		{
			m_spatialDatabaseStorageType = SpatialDatabase::Q_Static;
		}
	}
}

// ----------

CollisionProperty::~CollisionProperty()
{
	// do this first and hope that the required data hasn't been deleted when we remove the object

	if(m_spatialSubdivisionHandle)
	{
		WARNING_STRICT_FATAL(true,("CollisionProperty::~CollisionProperty - Deleting a collision property that's still referenced by the collision sphere tree\n"));

		static_cast<SphereTreeNode<CollisionProperty *, CollisionSphereAccessor>::NodeHandle *>(m_spatialSubdivisionHandle)->removeObject();

		m_spatialSubdivisionHandle = nullptr;
	}

	FATAL(CollisionWorld::isUpdating(),("CollisionProperty::~CollisionProperty - Trying to destroy a collision property while the collision world is updating. This is baaaad\n"));
	getOwner().removeNotification(CollisionNotification::getInstance());

	detachList();

	delete m_extent_l;
	m_extent_l = nullptr;

	delete m_extent_p;
	m_extent_p = nullptr;

	delete m_floor;
	m_floor = nullptr;

	delete m_footprint;
	m_footprint = nullptr;

}

// ----------------------------------------------------------------------

void CollisionProperty::attachSourceExtent ( BaseExtent * newSourceExtent ) const
{
	if(m_extent_l != newSourceExtent)
	{
		delete m_extent_l;
		m_extent_l = newSourceExtent;

		delete m_extent_p;
		m_extent_p = nullptr;

		m_extentsDirty = true;
	}
}

// ----------------------------------------------------------------------

void CollisionProperty::setMount ( bool mount )
{
	if (!boolEqual(isMount(), mount))
	{
		modifyFlags(F_mount, mount);

		if(m_footprint) m_footprint->updateContactRadii();
	}
}

// ----------------------------------------------------------------------

void CollisionProperty::setShip ( bool ship )
{
	modifyFlags(F_ship, ship);
}

// ----------------------------------------------------------------------

void CollisionProperty::initFloor ( void )
{
	if (isMobile())
		return;

	if(m_floor != nullptr)
		return;

	// ----------

	char const *floorName = nullptr;

	Appearance * appearance = getOwner().getAppearance();
	if(appearance)
		floorName = appearance->getFloorName();

	//@todo - HACK - Server-side POBs don't currently load appearances, which means
	// that they don't load their building skirts. If we couldn't get a floor from
	// the appearance, try to get one from the portal property instead.
	if (!floorName)
	{
		PortalProperty * portalProperty = getOwner().getPortalProperty();
		if(portalProperty)
			floorName = portalProperty->getExteriorFloorName();
	}

	if (floorName)
	{
		m_floor = FloorManager::createFloor(floorName,&getOwner(),appearance,true);
		WARNING(!m_floor, ("Could not load exterior floor %s", floorName));

		if (appearance)
			appearance->setShadowBlobAllowed();
	}
}

// ----------------------------------------------------------------------

void CollisionProperty::addToCollisionWorld ( void )
{
	modifyFlags(F_inCollisionWorld, true);

	setIdle(false);
	m_idleCounter = -10; // Make AI collision not be flagged as idle for a short while after being added to the world.

	storePosition();

	setExtentsDirty(true);

	if(m_footprint)
	{
		//-- cached objects don't get footprints because we don't want to drop them to the floors/ground etc...
		if (getOwner().getNetworkId() < NetworkId::cms_invalid)
		{
			delete m_footprint;
			m_footprint = nullptr;
		}

		if (m_footprint)
			m_footprint->addToWorld();
	}
}

// ----------

void CollisionProperty::removeFromCollisionWorld ( void )
{
	detachList();

	if(m_footprint) m_footprint->detach();

	modifyFlags(F_inCollisionWorld, false);
}

// ----------

bool CollisionProperty::isInCollisionWorld ( void ) const
{
	return hasFlags(F_inCollisionWorld);
}

// ----------------------------------------------------------------------

Transform const & CollisionProperty::getTransform_o2p ( void ) const
{
	return getOwner().getTransform_o2p();
}

Transform const & CollisionProperty::getTransform_o2w ( void ) const
{
	return getOwner().getTransform_o2w();
}

Transform CollisionProperty::getTransform_o2c ( void ) const
{
	return ::getTransform_o2c( &getOwner() );
}

// ----------------------------------------------------------------------

BaseExtent * convertToSimpleExtent ( BaseExtent const * sourceExtent )
{
    if(!sourceExtent) return nullptr;

    switch(sourceExtent->getType())
    {
    case ET_Simple:
        {
            SimpleExtent const * extent = safe_cast<SimpleExtent const *>(sourceExtent);

            return new SimpleExtent( MultiShape( extent->getShape() ) );
        }

    case ET_Sphere:
        {
            Extent const * extent = safe_cast<Extent const *>(sourceExtent);

            if(!extent) return nullptr;

            return new SimpleExtent( MultiShape( extent->getShape() ) );
        }

    case ET_Cylinder:
        {
            CylinderExtent const * extent = safe_cast<CylinderExtent const *>(sourceExtent);

            if(!extent) return nullptr;

            return new SimpleExtent( MultiShape( extent->getShape() ) );
        }

    case ET_Box:
        {
            BoxExtent const * extent = safe_cast<BoxExtent const *>(sourceExtent);

            if(!extent) return nullptr;

            return new SimpleExtent( MultiShape( extent->getShape() ) );
        }

	case ET_Mesh:
		{
            MeshExtent const * extent = safe_cast<MeshExtent const *>(sourceExtent);

            if(!extent) return nullptr;

			return extent->clone();
		}

    case ET_Detail:
        {
            DetailExtent const * extent = safe_cast<DetailExtent const *>(sourceExtent);

            if(!extent) return nullptr;

            DetailExtent * newExtent = new DetailExtent();

            for(int i = 0; i < extent->getExtentCount(); i++)
            {
                BaseExtent * newChild = convertToSimpleExtent( extent->getExtent(i) );

                newExtent->attachExtent( newChild );
            }

            return newExtent;
        }

    case ET_Component:
        {
            ComponentExtent const * extent = safe_cast<ComponentExtent const *>(sourceExtent);

            if(!extent) return nullptr;

            ComponentExtent * newExtent = new ComponentExtent();

            for(int i = 0; i < extent->getExtentCount(); i++)
            {
                BaseExtent * newChild = convertToSimpleExtent( extent->getExtent(i) );

                newExtent->attachExtent( newChild );
            }

            return newExtent;
        }

    case ET_Terrain:
    case ET_OrientedBox:
    case ET_OrientedCylinder:
    case ET_Null:
    case ET_ExtentTypeCount:
    default:
        return nullptr;
    }
}


// ----------------------------------------------------------------------

void CollisionProperty::cellChanged()
{
	setExtentsDirty(true);

	if(m_footprint) m_footprint->cellChanged();
}

// ----------------------------------------------------------------------

extern int extentUpdateCount;

void CollisionProperty::updateExtents ( void ) const
{
	extentUpdateCount++;

	// We can't get an onScaleChanged callback, so detect if it changed ourselves
	// and if so rebuild the extents.

	float newScale = getOwner().getScale().x;

	if(m_scale != newScale)
	{
		delete m_extent_l;
		m_extent_l = nullptr;

		delete m_extent_p;
		m_extent_p = nullptr;

		m_scale = newScale;
	}

    if(!m_extent_l)
    {
        Appearance const * appearance = getOwner().getAppearance();

        if(appearance)
        {
            AppearanceTemplate const * appTemplate = appearance->getAppearanceTemplate();

            if(appTemplate)
            {
				BaseExtent * source = convertToSimpleExtent( appTemplate->getCollisionExtent() );

				if(source)
				{
					BaseExtent * scaled = source->clone();

					scaled->transform(source,Transform::identity,m_scale);

					attachSourceExtent(scaled);
				}

				delete source;
            }
        }

		if(!m_extent_l)
		{
			PortalProperty const * portalProperty = getOwner().getPortalProperty();

			if(portalProperty)
			{
				BaseExtent * source = convertToSimpleExtent( portalProperty->getPortalPropertyTemplate().getCell(0).getCollisionExtent() );

				if(source)
				{
					BaseExtent * scaled = source->clone();

					scaled->transform(source,Transform::identity,m_scale);

					attachSourceExtent(scaled);
				}

				delete source;
			}
		}

		if(!m_extent_l)
		{
			if(isMobile())
			{
				Vector center = Vector(m_offsetX, getCollisionHeight() + getCollisionRadius(), m_offsetZ) * m_scale;
				float radius = getCollisionRadius() * m_scale;

				Sphere defaultSphere( center, radius );

				attachSourceExtent( new SimpleExtent( MultiShape(defaultSphere) ) );

				if(m_footprint)
				{
					m_footprint->setRadius(radius);
				}
			}
		}

		// ----------
		// minor hack to make sure ships have a default radius even though it's not specified in the template

		if(isShip() && m_extent_l)
		{
			const_cast<CollisionProperty*>(this)->m_defaultRadius = m_extent_l->getRadius();
		}
    }

    // ----------

    if(!m_extent_p)
    {
        if(m_extent_l)
        {
            m_extent_p = m_extent_l->clone();
        }
    }

	// ----------

    if(m_extent_p && m_extent_l)
    {
		Transform t = getTransform_o2c();

		if(isMobile() && !isShip())
		{
			Vector pos = t.getPosition_p();

			t = Transform::identity;

			t.move_p(pos);
		}

		m_extent_p->transform( m_extent_l, t, 1.0f );
    }

	// ----------

	if(m_floor)
	{
		m_floor->updateExtent();
	}

	// ----------
	// local sphere

	{
		if(m_extent_l && m_floor)
		{
			m_sphere_l = Containment3d::EncloseSphere(m_extent_l->getBoundingSphere(),m_floor->getBoundingSphere_l());
		}
		else if(m_extent_l)
		{
			m_sphere_l = m_extent_l->getBoundingSphere();
		}
		else if (m_floor)
		{
			m_sphere_l = m_floor->getBoundingSphere_l();
		}
		else
		{
			Appearance const * appearance = getOwner().getAppearance();

			if(appearance)
			{
    			m_sphere_l = appearance->getSphere();
			}
			else
			{
				m_sphere_l = Sphere::zero;
			}
		}
	}

	// ----------
	// world-space sphere

	{
		if(m_extent_p && m_floor)
		{
			Sphere extentSphere_w = CollisionUtils::transformToWorld(getCell(),m_extent_p->getBoundingSphere());

			m_sphere_w = Containment3d::EncloseSphere(extentSphere_w,m_floor->getBoundingSphere_w());
		}
		else if(m_floor)
		{
			m_sphere_w = m_floor->getBoundingSphere_w();
		}
		else if(m_extent_p)
		{
			Sphere sphere_p = m_extent_p->getBoundingSphere();

			m_sphere_w = CollisionUtils::transformToWorld(getCell(),sphere_p);
		}
		else
		{
			m_sphere_w = getOwner().rotateTranslate_o2w(m_sphere_l);
		}
	}

	// ----------

	m_extentsDirty = false;
}

// ----------------------------------------------------------------------

Sphere const & CollisionProperty::getBoundingSphere_l ( void ) const
{
	if(m_extentsDirty) updateExtents();

#ifdef _DEBUG

	if(ConfigSharedCollision::getEnableCollisionValidate())
	{
		Sphere temp;

		if(m_extent_l && m_floor)
		{
			temp = Containment3d::EncloseSphere(m_extent_l->getBoundingSphere(),m_floor->getBoundingSphere_l());
		}
		else if(m_extent_l)
		{
			temp = m_extent_l->getBoundingSphere();
		}
		else if (m_floor)
		{
			temp = m_floor->getBoundingSphere_l();
		}
		else
		{
			Appearance const * appearance = getOwner().getAppearance();

			if(appearance)
			{
    			temp = appearance->getSphere();
			}
			else
			{
				temp = Sphere::zero;
			}
		}

		if(!CollisionUtils::epsilonEqual(temp,m_sphere_l,0.001f))
		{
			DEBUG_WARNING(true,("CollisionProperty::getBoundingSphere_l() - current sphere is different from cached sphere\n"));
		}
	}

#endif

	return m_sphere_l;

}

// ----------

Sphere const & CollisionProperty::getBoundingSphere_w ( void ) const
{
	if(m_extentsDirty) updateExtents();

#ifdef _DEBUG

	if(ConfigSharedCollision::getEnableCollisionValidate())
	{
		Sphere temp;

		if(m_extent_p && m_floor)
		{
			Sphere extentSphere_w = CollisionUtils::transformToWorld(getCell(),m_extent_p->getBoundingSphere());

			temp = Containment3d::EncloseSphere(extentSphere_w,m_floor->getBoundingSphere_w());
		}
		else if(m_floor)
		{
			temp = m_floor->getBoundingSphere_w();
		}
		else if(m_extent_p)
		{
			Sphere sphere_p = m_extent_p->getBoundingSphere();

			temp = CollisionUtils::transformToWorld(getCell(),sphere_p);
		}
		else
		{
			temp = getOwner().rotateTranslate_o2w(m_sphere_l);
		}

		if(!CollisionUtils::epsilonEqual(temp,m_sphere_w,0.001f))
		{
			DEBUG_WARNING(true,("CollisionProperty::getBoundingSphere_w() - current sphere is different from cached sphere\n"));
		}
	}

#endif

	return m_sphere_w;
}

// ----------

Capsule CollisionProperty::getQueryCapsule_w ( void ) const
{
	Sphere sphere_w = getBoundingSphere_w();

	Vector delta = getOwner().getPosition_w() - getLastPos_w();

	// stretch the capsule's radius out by a meter to help catch glancing collisions

	Sphere oldSphere_w ( sphere_w.getCenter() - delta, sphere_w.getRadius() + 1.0f);

	return Capsule(oldSphere_w,delta);
}

// ----------------------------------------------------------------------

float CollisionProperty::getCollisionRadius ( void ) const
{
	return m_defaultRadius;
}

float CollisionProperty::getCollisionHeight ( void ) const
{
	return m_stepHeight;
}

// ----------------------------------------------------------------------

void CollisionProperty::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

    if(renderer == nullptr) return;

    if(ConfigSharedCollision::getDrawExtents())
    {
		renderer->setApplyScale(false);

		if(isMobile())
		{
			renderer->setWorldSpace(true);

			renderer->setColor(VectorArgb::solidYellow);

			renderer->drawSphere(m_sphere_w);

			renderer->setWorldSpace(false);
		}
		else
		{
			if(m_extent_l)
			{
				m_extent_l->drawDebugShapes(renderer);
			}
		}

		if(m_footprint)
		{
			m_footprint->drawDebugShapes(renderer);
		}

		renderer->setApplyScale(true);
    }

	if(m_floor)
	{
		m_floor->drawDebugShapes(renderer,true);
	}

#endif
}

// ----------------------------------------------------------------------

void CollisionProperty::setServerSide ( bool serverSide )
{
	modifyFlags(F_serverSide, serverSide);
}

// ----------

bool CollisionProperty::isServerSide ( void ) const
{
	return hasFlags(F_serverSide);
}

// ----------

int CollisionProperty::elevatorMove ( int nFloors, Transform & outTransform )
{
    if(m_footprint)
    {
        return m_footprint->elevatorMove(nFloors,outTransform);
    }
    else
    {
        return 0;
    }
}

// ----------------------------------------------------------------------

bool CollisionProperty::canCollideWith( CollisionProperty const * otherCollision ) const
{
	if(!otherCollision)
	{
		return false;
	}

	if(!isCollidable())
	{
		return false;
	}

	if(!otherCollision->isCollidable())
	{
		return false;
	}

	if(!otherCollision->isMobile())
	{
		DoorObject const * const door = dynamic_cast<DoorObject const *>(&otherCollision->getOwner());

		if(door)
		{
			// Everybody always triggers doors
			return true;
		}

		bool const isBarrier = (dynamic_cast<BarrierObject const *>(&otherCollision->getOwner()) != 0);

		// ----------

		CellProperty const * thisCell = getOwner().getParentCell();
		CellProperty const * otherCell = otherCollision->getOwner().getParentCell();

		if(thisCell && !thisCell->isAdjacentTo(otherCell))
		{
			return false;
		}

		if(otherCell && (otherCell != CellProperty::getWorldCellProperty()))
		{
			PortalProperty const * otherPortalProperty = otherCell->getPortalProperty();

			if(otherPortalProperty)
			{
				Object const * otherBuilding = &otherPortalProperty->getOwner();

				IsPlayerHouseHook hook = ConfigSharedCollision::getIsPlayerHouseHook();

				if(hook && hook(otherBuilding))
				{
					// Target object is inside a player house, and thus is non-collidable

					if(isBarrier)
					{
						return true;
					}

					return false;
				}
			}
		}

		if(isBarrier)
		{
			return true;
		}
	}

	if(isMobile() && otherCollision->isMobile())
	{
		if(isPlayer())
		{
			if(otherCollision->isPlayerControlled())
			{
				// Players collide with each other if they're both in combat

				return isInCombat() && otherCollision->isInCombat();
			}
			else
			{
				// Players always collide with monsters

				return true;
			}
		}
		else
		{
			// Monsters don't collide with anything

			return true;
		}
	}
	else
	{
		return true;
	}
}

// ----------

bool CollisionProperty::blocksInteraction (InteractionType) const
{
	return isCollidable();
}

// ----------

bool CollisionProperty::blocksMovement () const
{
	//@todo - HACK - force mobiles to block movement so we can collide with
	// them

	if(isMobile())
		return true;
	else
	{
		return isCollidable();
	}
}

// ----------------------------------------------------------------------

void CollisionProperty::objectWarped ( void )
{
	setIdle(false);

	storePosition();

	Footprint * foot = getFootprint();

	if(foot) foot->objectWarped();
}

// ----------------------------------------------------------------------

void CollisionProperty::storePosition ( void )
{
	setLastPos( getCell(), getTransform_o2p() );
}

// ----------------------------------------------------------------------

void CollisionProperty::hit ( CollisionProperty * staticCollider )
{
	UNREF(staticCollider);

	if(isMobile())
	{
		setIdle(false);
	}
}

void CollisionProperty::hitBy ( CollisionProperty * dynamicCollider )
{
	UNREF(dynamicCollider);

	if(isMobile())
	{
		setIdle(false);
	}
}

// ----------------------------------------------------------------------

CellProperty * CollisionProperty::getCell ( void )
{
	return getOwner().getParentCell();
}

CellProperty const * CollisionProperty::getCell ( void ) const
{
	return getOwner().getParentCell();
}

// ----------------------------------------------------------------------

Vector CollisionProperty::getLastPos_w ( void ) const
{
	return getLastTransform_w().getPosition_p();;
}

// ----------

Vector CollisionProperty::getLastPos_p ( void ) const
{
	return getLastTransform_p().getPosition_p();
}

// ----------

Transform const & CollisionProperty::getLastTransform_w ( void ) const
{
	return m_lastTransform_w;
}

// ----------

Transform const & CollisionProperty::getLastTransform_p ( void ) const
{
	if(m_lastCellObject)
	{
		return m_lastTransform_p;
	}
	else
	{
		return m_lastTransform_w;
	}
}

// ----------

CellProperty * CollisionProperty::getLastCell ( void )
{
	if(m_lastCellObject)
	{
		return m_lastCellObject->getCellProperty();
	}
	else
	{
		return CellProperty::getWorldCellProperty();
	}
}

// ----------

CellProperty const * CollisionProperty::getLastCell ( void ) const
{
	if(m_lastCellObject)
	{
		return m_lastCellObject->getCellProperty();
	}
	else
	{
		return CellProperty::getWorldCellProperty();
	}
}

// ----------

void CollisionProperty::setLastPos ( CellProperty * cell, Transform const & transform_p )
{
	NAN_CHECK(transform_p);

	if(cell == nullptr)
	{
		m_lastCellObject = nullptr;
		m_lastTransform_p = transform_p;
		m_lastTransform_w = transform_p;
	}
	else
	{
		m_lastCellObject = &(cell->getOwner());
		m_lastTransform_p = transform_p;
		m_lastTransform_w = CollisionUtils::transformToCell(cell,transform_p,CellProperty::getWorldCellProperty());
	}
}

// ----------------------------------------------------------------------

Object const * CollisionProperty::getStandingOn ( void ) const
{
	if(m_footprint)
	{
		return m_footprint->getStandingOn();
	}
	else
	{
		return nullptr;
	}
}

// ----------------------------------------------------------------------

void getDistance ( Vector const & point_p, BaseExtent const * extent, float maxDistance, float & minDistance );

// ----------

void getDistance ( Vector const & point_p, SimpleExtent const * extent, float maxDistance, float & minDistance )
{
	UNREF(maxDistance);

	float distance = Distance3d::DistancePointShape(point_p,extent->getShape());

	if(distance < minDistance)
	{
		minDistance = distance;
	}
}

// ----------

void getDistance ( Vector const & point_p, ComponentExtent const * extent, float maxDistance, float & minDistance )
{
	//@todo - don't recurse if the bounds of this extent is farther away than minDistance

	int extentCount = extent->getExtentCount();

	for(int i = 0; i < extentCount; i++)
	{
		getDistance(point_p,extent->getExtent(i),maxDistance,minDistance);
	}
}

// ----------

void getDistance ( Vector const & point_p, DetailExtent const * extent, float maxDistance, float & minDistance )
{
	//@todo - don't recurse if the bounds of this extent is farther away than minDistance

	int extentCount = extent->getExtentCount();

	getDistance(point_p,extent->getExtent(extentCount-1),maxDistance,minDistance);
}

// ----------

void getDistance ( Vector const & point_p, BaseExtent const * extent, float maxDistance, float & minDistance )
{
	NOT_NULL(extent);

	ExtentType type = extent->getType();

	// ----------

	if     (type == ET_Simple)    getDistance( point_p, safe_cast<SimpleExtent const *>(extent), maxDistance, minDistance );
	else if(type == ET_Component) getDistance( point_p, safe_cast<ComponentExtent const *>(extent), maxDistance, minDistance );
	else if(type == ET_Detail)    getDistance( point_p, safe_cast<DetailExtent const *>(extent), maxDistance, minDistance );
}

// ----------

bool CollisionProperty::getDistance ( Vector const & point_w, float maxDistance, float & outDistance ) const
{
	if(Distance3d::DistancePointSphere(point_w,getBoundingSphere_w()) >= maxDistance)
	{
		return false;
	}

	// ----------

	float distance = maxDistance;

	Vector point_p = CollisionUtils::transformFromWorld(point_w,getCell());

	::getDistance(point_p,getExtent_p(),maxDistance,distance);

	if(distance < maxDistance)
	{
		outDistance = distance;
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

SpatialSubdivisionHandle * CollisionProperty::getSpatialSubdivisionHandle ( void )
{
    return m_spatialSubdivisionHandle;
}

void CollisionProperty::setSpatialSubdivisionHandle ( SpatialSubdivisionHandle * newHandle )
{
	if(getDisableCollisionWorldAddRemove())
	{
		FATAL(newHandle,("CollisionProperty::setSpatialSubdivisionHandle - Can't add a CollisionProperty flagged for disabled add/remove to a sphere tree\n"));
	}

    m_spatialSubdivisionHandle = newHandle;
}

// ----------

bool CollisionProperty::getDisableCollisionWorldAddRemove () const
{
	return hasFlags(F_disableCollisionWorldAddRemove);
}

void CollisionProperty::setDisableCollisionWorldAddRemove (bool disable)
{
	if(m_spatialSubdivisionHandle)
	{
		FATAL(disable,("CollisionProperty::setDisableCollisionWorldAddRemove - Can't disable add/remove for an object already in a collision sphere tree\n"));
	}

	modifyFlags(F_disableCollisionWorldAddRemove, disable);
}

