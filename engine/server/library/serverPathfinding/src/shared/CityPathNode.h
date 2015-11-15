// ======================================================================
//
// CityPathNode.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_CityPathNode_H
#define	INCLUDED_CityPathNode_H

#include "serverGame/ServerObject.h"
#include "sharedFoundation/Watcher.h"
#include "sharedPathfinding/DynamicPathNode.h"
#include "Unicode.h"

class CityPathGraph;
class SpatialSubdivisionHandle;

typedef SpatialSubdivisionHandle SpatialHandle;

// ======================================================================
// CityPathNodes are DynamicPathNodes that know how to use objvars to
// persist their data.

class CityPathNode : public DynamicPathNode
{
public:

	CityPathNode ( Vector const & position_w, NetworkId const & sourceId );
	CityPathNode ( Vector const & position_w, NetworkId const & sourceId, NetworkId const & creatorId );
	
	virtual ~CityPathNode();
	
	// ----------
	
	virtual void                 clearEdges    ( void );
	
	virtual CellProperty const * getCell       ( void ) const;

	virtual Vector               getPosition_w ( void ) const;

	virtual void                 setPosition_p ( Vector const & newPosition_p );

	virtual void                 setPosition_w ( Vector const & newPosition_w );

	Vector const &               getRelativePosition_o  ( void ) const;
	void                         setRelativePosition_o  ( Vector const & position_o );

	void                         updateRelativePosition ( void );

	// ----------
	
	void loadFromObjvars ( void );
	void saveToObjvars   ( void );

	void loadInfoFromObjvars ( void );
	void saveInfoToObjvars   ( void );

	void loadEdgesFromObjvars ( void );
	void saveEdgesToObjvars   ( void );
	
	void saveNeighbors   ( void );
	void saveAllData     ( void );
	
	int  getDebugId      ( void ) const;
	
	bool sanityCheck     ( bool doWarnings ) const;

	void reload          ( void );

	// ----------

	ServerObject *          getSourceObject  ( void );
	ServerObject const *    getSourceObject  ( void ) const;

	NetworkId const &       getSourceId      ( void ) const;

	ServerObject *          getCreatorObject ( void );
	ServerObject const *    getCreatorObject ( void ) const;

	NetworkId const &       getCreatorId     ( void ) const;

	void                    setCreator       ( NetworkId const & networkId );

	// ----------

	CityPathGraph *         _getGraph        ( void );
	CityPathGraph const *   _getGraph        ( void ) const;

	Unicode::String const & getName          ( void ) const;
	void                    setName          ( Unicode::String const & name );

	SpatialHandle *         getSpatialHandle    ( void );
	SpatialHandle const *   getSpatialHandle    ( void ) const;
	
	void                    setSpatialHandle    ( SpatialHandle * newHandle );

	bool                    hasEdgeTo           ( NetworkId const & neighborId ) const;

	void                    snapToTerrain       ( void );

	int                     getNeighborCode     ( void ) const;
	void                    setNeighborCode     ( int newCode );
	
protected:

	typedef Watcher<ServerObject> ObjectWatcher;

	friend class CityPathGraph;

	CachedNetworkId m_source;
	CachedNetworkId m_creator;

	Vector          m_relativePosition_o; // Position relative to the creator

	Unicode::String m_name;

    SpatialHandle * m_spatialHandle;

	int             m_debugId;

	int             m_neighborCode;
};

// ----------------------------------------------------------------------
// position relative to the creator object

inline Vector const & CityPathNode::getRelativePosition_o ( void ) const
{
	return m_relativePosition_o;
}

inline void CityPathNode::setRelativePosition_o ( Vector const & position_o )
{
	m_relativePosition_o = position_o;
}

// ----------------------------------------------------------------------

inline Unicode::String const & CityPathNode::getName ( void ) const
{
	return m_name;
}

inline void CityPathNode::setName ( Unicode::String const & name )
{
	m_name = name;
}

// ----------------------------------------------------------------------

inline SpatialHandle * CityPathNode::getSpatialHandle ( void ) 
{
    return m_spatialHandle;
}

inline SpatialHandle const * CityPathNode::getSpatialHandle ( void ) const
{
    return m_spatialHandle;
}

inline void CityPathNode::setSpatialHandle ( SpatialHandle * newHandle )
{
    m_spatialHandle = newHandle;
}

// ======================================================================

#endif

