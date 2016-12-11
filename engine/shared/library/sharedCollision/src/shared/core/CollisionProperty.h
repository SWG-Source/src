// ======================================================================
//
// CollisionProperty.h
//
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionProperty_H
#define INCLUDED_CollisionProperty_H

#include "sharedCollision/SpatialDatabase.h"
#include "sharedObject/Property.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Capsule.h"
#include "sharedFoundation/Watcher.h"

class BaseExtent;
class CellProperty;
class CollisionProperty;
class DebugShapeRenderer;
class Floor;
class Footprint;
class SharedObjectTemplate;
class SpatialDatabase;
class SpatialSubdivisionHandle;
class Transform;

typedef std::vector<CollisionProperty*>  ColliderList;

// ======================================================================

class CollisionProperty : public Property
{
public:
	
	static PropertyId getClassPropertyId ( void );

	enum Flags
	{
		F_mobile                           = 0x0001,
		F_flora                            = 0x0002,
		F_player                           = 0x0004,
		F_mount                            = 0x0008,
		F_ship                             = 0x0010,
		F_playerControlled                 = 0x0020,
		F_collidable                       = 0x0040,
		F_serverSide                       = 0x0080,
		F_inCombat                         = 0x0100,
		F_inCollisionWorld                 = 0x0200,
		F_idle                             = 0x0400,
		F_disableCollisionWorldAddRemove   = 0x0800
	};

public:

	CollisionProperty( Object & owner );
	CollisionProperty( Object & owner, SharedObjectTemplate const * objTemplate );
	
	virtual ~CollisionProperty();
	
	void                    initFloor           ( void );
	void                    initFromTemplate    ( SharedObjectTemplate const * objTemplate );
	
	// ----------
	
	void                    addToCollisionWorld      ( void );
	void                    removeFromCollisionWorld ( void );
	bool                    isInCollisionWorld       ( void ) const;

	// ----------

	void                    setDisableCollisionWorldAddRemove (bool disable);
	bool                    getDisableCollisionWorldAddRemove () const;
	
	// ----------
	// Accessors
	
	Transform const &       getTransform_o2p    ( void ) const;
	Transform const &       getTransform_o2w    ( void ) const;
	Transform               getTransform_o2c    ( void ) const;

	// ----------

	CellProperty *          getCell             ( void );
	CellProperty const *    getCell             ( void ) const;

	Vector                  getLastPos_p        ( void ) const;
	Vector                  getLastPos_w        ( void ) const;

	Transform const &       getLastTransform_p  ( void ) const;
	Transform const &       getLastTransform_w  ( void ) const;

	CellProperty *          getLastCell         ( void );
	CellProperty const *    getLastCell         ( void ) const;

	void                    setLastPos          ( CellProperty * cell, Transform const & transform_p );

	// ----------

	Floor *                 getFloor            ( void );
	Floor const *           getFloor            ( void ) const;
	
	Footprint *             getFootprint        ( void );
	Footprint const *       getFootprint        ( void ) const;

	// ----------
	// Extents
	
	bool                    getExtentsDirty     ( void ) const;
	void                    setExtentsDirty     ( bool dirty ) const;

	BaseExtent const *      getExtent_l         ( void ) const;
	BaseExtent const *      getExtent_p         ( void ) const;

	Sphere const &          getBoundingSphere_l ( void ) const;
	Sphere const &          getBoundingSphere_w ( void ) const;

	Capsule                 getQueryCapsule_w   ( void ) const;

	float                   getCollisionRadius  ( void ) const;
	float                   getCollisionHeight  ( void ) const;

	void                    attachSourceExtent  ( BaseExtent * newSourceExtent ) const;
		
	// ----------

	float                   getScale            ( void ) const;
	
	// ----------
	// Flags
	
	void                    setMobile           ( bool mobile );
	void                    setFlora            ( bool flora );
	void                    setPlayer           ( bool player );
	void                    setMount            ( bool mount );
	void                    setShip             ( bool ship );
	void                    setPlayerControlled ( bool playerControlled );
	void                    setCollidable       ( bool collidable );
	void                    setServerSide       ( bool serverSide );
	void                    setInCombat         ( bool inCombat );
	
	bool                    isMobile            ( void ) const;
	bool                    isFlora             ( void ) const;
	bool                    isPlayer            ( void ) const;
	bool                    isMount             ( void ) const;
	bool                    isShip              ( void ) const;
	bool                    isPlayerControlled  ( void ) const;
	bool                    isCollidable        ( void ) const;
	bool                    isServerSide        ( void ) const;
	bool                    isInCombat          ( void ) const;
	
	// ----------

	bool                    isIdle              ( void ) const;

	void                    updateIdle          ( void );
	void                    setIdle             ( bool active );

	void                    attachList          ( CollisionProperty * & head );
	void                    detachList          ( void );
	
	CollisionProperty *     getPrev             ( void );
	CollisionProperty *     getNext             ( void );

	static CollisionProperty * getActiveHead    ( void );

	// ----------

	virtual bool            canCollideWith      ( CollisionProperty const * otherCollision ) const;
	virtual bool            blocksInteraction   (InteractionType interaction) const;
	virtual bool            blocksMovement      () const;
	virtual void            objectWarped        ( void );
	
	void                    storePosition       ( void );
	
	// ----------

	SpatialSubdivisionHandle * getSpatialSubdivisionHandle ( void );
	void                       setSpatialSubdivisionHandle ( SpatialSubdivisionHandle * newHandle );
	
	// ----------
	// Misc
	
	void                    drawDebugShapes     ( DebugShapeRenderer * renderer ) const;
	void                    cellChanged         ( void );
	void                    updateExtents       ( void ) const;
	int                     elevatorMove        ( int nFloors, Transform & outTransform );
	
	virtual void            hit                 ( CollisionProperty * staticCollider );
	virtual void            hitBy               ( CollisionProperty * dynamicCollider );
	
	Object const *          getStandingOn       ( void ) const;

	bool                    getDistance         ( Vector const & point_w, float maxDistance, float & outDistance ) const;
	
	int getFlags() const;
	void setFlags(int flags);
	bool hasFlags(int flags) const;
	void modifyFlags(int flags, bool onOff);

	// ----------

	SpatialDatabase::Query getSpatialDatabaseStorageType() const;

protected:

	typedef Watcher<Object> CellWatcher;

	Transform             m_lastTransform_w;
	Transform             m_lastTransform_p;
	CellWatcher           m_lastCellObject;

	// ----------
	// Since I can't get to the shared object template from shared code when running on the server,
	// I have to cache these four variables from the template here.

	float                 m_stepHeight;
	float                 m_defaultRadius;
	float                 m_offsetX;
	float                 m_offsetZ;

	// ----------

	mutable bool          m_extentsDirty;

	mutable BaseExtent *  m_extent_l;
	mutable BaseExtent *  m_extent_p;

	mutable Sphere        m_sphere_l;
	mutable Sphere        m_sphere_w;

	mutable float         m_scale;

	// ----------

	SpatialSubdivisionHandle * m_spatialSubdivisionHandle;

	Floor *               m_floor;

	Footprint *           m_footprint;

	int                   m_idleCounter;

	CollisionProperty *   m_next;
	CollisionProperty *   m_prev;

	bool                  m_disableCollisionWorldAddRemove;

	uint16                m_flags;

private:

    CollisionProperty();
    CollisionProperty(const CollisionProperty&);
    CollisionProperty& operator= (const CollisionProperty&);

private:

	SpatialDatabase::Query m_spatialDatabaseStorageType; // SpatialDatabase::Query values

};

// ----------------------------------------------------------------------

inline Floor * CollisionProperty::getFloor ( void )
{
    return m_floor;
}

inline Floor const * CollisionProperty::getFloor ( void ) const
{
    return m_floor;
}

// ----------

inline Footprint * CollisionProperty::getFootprint ( void )
{
    return m_footprint;
}

inline Footprint const * CollisionProperty::getFootprint ( void ) const
{
    return m_footprint;
}

// ----------
// Extents

inline bool CollisionProperty::getExtentsDirty ( void ) const
{
	return m_extentsDirty;
}

inline void CollisionProperty::setExtentsDirty ( bool dirty ) const
{
	m_extentsDirty = dirty;
}

inline BaseExtent const * CollisionProperty::getExtent_l ( void ) const
{
	if(getExtentsDirty()) updateExtents();

	return m_extent_l;
}

inline BaseExtent const * CollisionProperty::getExtent_p ( void ) const
{
	if(getExtentsDirty()) updateExtents();

	return m_extent_p;
}

// ----------

inline float CollisionProperty::getScale ( void ) const
{
	return m_scale;
}

// ----------
// Flags

inline void CollisionProperty::setMobile ( bool mobile )
{
	modifyFlags(F_mobile, mobile);
}

inline void CollisionProperty::setFlora ( bool flora )
{
	modifyFlags(F_flora, flora);
}

inline void CollisionProperty::setPlayer ( bool player )
{
	modifyFlags(F_player, player);
}

inline void CollisionProperty::setPlayerControlled ( bool playerControlled )
{
	modifyFlags(F_playerControlled, playerControlled);
}

inline void CollisionProperty::setCollidable ( bool collidable )
{
	modifyFlags(F_collidable, collidable);
}

inline void CollisionProperty::setInCombat ( bool inCombat )
{
	modifyFlags(F_inCombat, inCombat);
}

// ----------

inline bool CollisionProperty::isMobile ( void ) const
{
	return hasFlags(F_mobile);
}

inline bool CollisionProperty::isFlora ( void ) const
{
	return hasFlags(F_flora);
}

inline bool CollisionProperty::isPlayer ( void ) const
{
	return hasFlags(F_player);
}

inline bool CollisionProperty::isMount ( void ) const
{
	return hasFlags(F_mount);
}

inline bool CollisionProperty::isShip ( void ) const
{
	return hasFlags(F_ship);
}

inline bool CollisionProperty::isPlayerControlled ( void ) const
{
	return hasFlags(F_playerControlled);
}

inline bool CollisionProperty::isCollidable ( void ) const
{
	return hasFlags(F_collidable);
}

inline bool CollisionProperty::isInCombat ( void ) const
{
	return hasFlags(F_inCombat);
}

// ----------

inline bool CollisionProperty::isIdle ( void ) const
{
	return hasFlags(F_idle);
}

// ----------

inline CollisionProperty * CollisionProperty::getPrev ( void )
{
	return m_prev;
}

inline CollisionProperty * CollisionProperty::getNext ( void )
{
	return m_next;
}

// ----------

inline SpatialDatabase::Query CollisionProperty::getSpatialDatabaseStorageType() const
{
	return m_spatialDatabaseStorageType;
}

//----------------------------------------------------------------------

inline int CollisionProperty::getFlags() const
{
	return m_flags;
}

//----------------------------------------------------------------------

inline void CollisionProperty::setFlags(int flags)
{
	m_flags = static_cast<uint16>(flags);
}

//----------------------------------------------------------------------

inline bool CollisionProperty::hasFlags(int flags) const
{
	return (m_flags & flags) == flags;
}

//----------------------------------------------------------------------

inline void CollisionProperty::modifyFlags(int flags, bool onOff)
{
	if (onOff)
		m_flags |= flags;
	else
		m_flags &= ~flags;
}

// ======================================================================

#endif // #ifndef INCLUDED_CollisionProperty_H

