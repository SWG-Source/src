// ======================================================================
//
// Footprint.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Footprint_H
#define INCLUDED_Footprint_H

// ======================================================================

#include "sharedCollision/BaseClass.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedCollision/MultiList.h"

#include "sharedFoundation/Watcher.h"

class Floor;
class FloorContactShape;
class FloorTri;
class Object;
class CellProperty;
class DebugShapeRenderer;
class CollisionProperty;

// ======================================================================

class Footprint : public BaseClass
{
public:

	Footprint( Vector const & position, float radius, CollisionProperty * parent, float swimHeight );
	virtual ~Footprint();

	// ----------

	void    updatePreResolve    ( float time );
	void    updatePostResolve   ( float time );

	int     elevatorMove        ( int nFloors, Transform & outTransform );
	void    drawDebugShapes     ( DebugShapeRenderer * R ) const;
	void    addToWorld          ( void );
	void    cellChanged         ( void );
	void    objectWarped        ( void );

	bool    isInCell            ( void ) const;
	bool    isOnSolidFloor      ( void ) const;
	bool    isAttached          ( void ) const;
	bool    isObjectInSync      ( void ) const;     // returns true if the object is over the footprint
	bool    hasContacts         ( void ) const;

	void    setSwimHeight       ( float swimHeight );

	bool    getTerrainHeight    ( float & outHeight ) const;
	bool    getFloorHeight      ( float & outHeight ) const;
	bool    getGroundHeight     ( float & outHeight ) const;

	Vector  getGroundNormal_p   ( void ) const;
	Vector  getGroundNormal_w   ( void ) const;

	float   getRadius           ( void ) const;
	void    setRadius           ( float newRadius );
	void    updateContactRadii  ( void );

	FloorLocator const * getAnyContact   ( void ) const;
	FloorLocator const * getSolidContact ( void ) const;

	Object const * getStandingOn ( void ) const;

	// ----------

	void                    addContact          ( FloorLocator const & loc );

	MultiListHandle &       getFloorList        ( void );
	MultiListHandle const & getFloorList        ( void ) const;

	void    detach              ( void );

	bool    isFloating          ( void ) const;
	void    stopFloating        ( void );
	void    setFloatingTime     ( int seconds );

	// ----------

	void  alignToGroundNoFloat ( void );

	bool isFloorWithinThreshold(FloorContactShape const & floorContactShape) const;

protected:

	friend class FloorManager;
	friend class Floor;
	friend class SpatialDatabase;
	friend class CollisionWorld;

	// These priorities represent how important it is that we get the footprint
	// attached to a floor

	enum ForceAttachPriority
	{
		FAP_None,               // No force. If the footprint doesn't fall onto the floor, it's not attached to the floor
		FAP_AllowJump,          // The footprint should be attached to the floor even if doing so would require a jump
		FAP_IfInsideBounds,     // The footprint should be attached to the floor if it is inside the floor's bounding box
		FAP_AtAllCosts,         // The footprint must be attached to the floor
	};

	// ----------
	// accessors

	Object *                getOwner        ( void );
	Object const *          getOwner        ( void ) const;

	Vector const &          getPosition_p   ( void ) const;
	Vector                  getPosition_p   ( CellProperty const * relativeCell ) const;
	Vector const &          getPosition_w   ( void ) const;

	void                    setPosition_p   ( Vector const & position_p );
	void                    setPosition_w   ( Vector const & position_w );

	CellProperty *          getCell         ( void );
	CellProperty const *    getCell         ( void ) const;

	void                    setPosition     ( CellProperty * cell, Vector const & position_p );

	// ----------
	// accessors

	Vector                  getObjectPosition_p ( void ) const;
	Vector                  getObjectPosition_p ( CellProperty const * relativeCell ) const;
	void                    setObjectPosition_p ( Vector const & newPos );

	CellProperty const *    getObjectCell       ( void ) const;
	CellProperty *          getObjectCell       ( void ) ;

	// ----------
	// synchronization

	bool    snapToObject        ( void );
	bool    snapObjectToTerrain ( void );
	bool    snapObjectToGround  ( void );

	bool    syncToContacts      ( void );


	// ----------
	// misc

	void    updateOffsets       ( void );

	bool    updateHeights       ( void );
	void    snapToGround        ( void );
	bool    teleport            ( FloorLocator const & loc );

	void    runDebugTests       ( void );

	void    updateFloating      ( void );

	bool    testClear           ( void ) const;

	void    shoveAway           ( float time );


	// these are just for debugging

#ifdef _DEBUG

	void    backup              ( void );
	void    revert              ( void );

#endif

	// ----------
	// attach/detach code

	bool    attachTo            ( Floor const * pFloor, bool fromObject );
	bool    snapToCellFloor     ( void );
	bool    isAttachedTo        ( Floor const * pFloor ) const;

	void    reattachContacts    ( void );
	void    forceReattachContacts ( void );
	void    sweepContacts       ( void );
	void    snapContacts        ( void );

	// ----------------------------------------------------------------------

protected:

	CollisionProperty *     m_parent;

	typedef Watcher<Object> CellWatcher;

	CellWatcher             m_cellObject;
	Vector                  m_position_p;
	Vector                  m_position_w;


	float                   m_radius;

	MultiListHandle         m_floorList;        //! List of floors this footprint is on

	float                   m_swimHeight;

	bool                    m_hasTerrainHeight;
	bool                    m_hasFloorHeight;
	bool                    m_hasGroundHeight;

	float                   m_terrainHeight;
	float                   m_floorHeight;
	float                   m_groundHeight;

	Vector                  m_groundNormal;

	int                     m_addToWorldTime;   // time in seconds
	Vector                  m_addToWorldPos;
	bool                    m_floating;
	int                     m_floatingTime;

	// ----------
	// Data used for debugging

#ifdef _DEBUG

	Vector                  m_backupPosition_p;
	CellProperty *          m_backupCell;

	Vector                  m_backupObjectPosition_p;
	CellProperty *          m_backupObjectCell;

	float                   m_lineHitTime; // HACK for testing line query
	Vector                  m_lineOrigin;
	Vector                  m_lineDelta;
	bool                    m_canMove;
	Vector                  m_lineHitPos;
	float                   m_bubbleSize;
	bool                    m_bubbleEmpty;

#endif

	// ----------

private:

	Footprint( Footprint const & copy );
	Footprint & operator = (Footprint const & copy );
};

// ----------------------------------------------------------------------

inline bool Footprint::hasContacts ( void ) const
{
	return m_floorList.isConnected();
}

// ----------------------------------------------------------------------

inline bool Footprint::getTerrainHeight ( float & outHeight ) const
{
	outHeight = m_terrainHeight;

	return m_hasTerrainHeight;		
}

inline bool Footprint::getFloorHeight ( float & outHeight ) const
{
	outHeight = m_floorHeight;

	return m_hasFloorHeight;
}

inline bool Footprint::getGroundHeight ( float & outHeight ) const
{
	outHeight = m_groundHeight;

	return m_hasGroundHeight;
}

// ----------------------------------------------------------------------

inline MultiListHandle & Footprint::getFloorList ( void ) 
{
	return m_floorList;
}

// ----------

inline MultiListHandle const & Footprint::getFloorList ( void ) const
{
    return m_floorList;
}

// ======================================================================

#endif

