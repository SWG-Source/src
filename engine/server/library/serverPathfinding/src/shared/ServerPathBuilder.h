// ======================================================================
//
// ServerPathBuilder.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ServerPathBuilder_H
#define	INCLUDED_ServerPathBuilder_H

#include <vector>
#include <list>

#include "serverGame/AiLocation.h"
#include "Unicode.h"

class CreatureObject;
class AiLocation;
class CellProperty;
class PortalProperty;
class CityPathGraph;
class PathGraph;
class PathSearch;
class PathNode;
class ServerObject;

typedef std::list<AiLocation> AiPath;
typedef std::vector<int> IndexList;

// ======================================================================

int findClosestPathNode( CreatureObject const * creature, PathGraph const * graph );
int findClosestReachablePathNode( CreatureObject const * creature, PathGraph const * graph );
int findClosestReachablePathNode ( CellProperty const * cell, Vector const & start, float radius, PathGraph const * graph );

// ======================================================================

class ServerPathBuilder
{
public:

	ServerPathBuilder();
	virtual ~ServerPathBuilder();

	bool     buildPath              ( CreatureObject const * creature, AiLocation const & goal );
	bool     buildPath              ( CreatureObject const * creature, Unicode::String const & goalName );
	bool     buildWorldPath         ( AiLocation const & startLocation, AiLocation const & goal );
	bool     buildPathImmediate     ( AiLocation const & startLocation, AiLocation const & goalLocation );

	bool     buildPath_Async        ( CreatureObject const * creature, AiLocation const & goal );
	bool     buildPath_Async        ( CreatureObject const * creature, Unicode::String const & goalName );

	void     update                 ( void );

	bool     buildDone              ( void ) const;
	bool     buildFailed            ( void ) const;

	AiPath * getPath                ( void );
	bool     getPathIncomplete      ( void ) const;

	bool     getQueued              ( void ) const;
	void     setQueued              ( bool queued );

	void     setEnableJitter        ( bool enable );

protected:

	bool     buildPath_ToGoal       ( void );
	bool     buildPath_Named        ( void );

	bool     setupBuildPath         ( CreatureObject const * creature, AiLocation const & goal );
	bool     setupBuildPath         ( CreatureObject const * creature, Unicode::String const & goalName );

	bool     buildPathInternal      ( CellProperty const * cell, PathGraph const * graph, int indexA, int indexB );
	bool     buildPathInternal      ( PortalProperty const * building, PathGraph const * graph, int indexA, int indexB );

	bool     buildPathInternal      ( CityPathGraph const * cityGraph, int indexA, int indexB );
	bool     buildPathInternal      ( CityPathGraph const * cityGraph, int indexA, IndexList const & indexBList );

	bool     expandPath             ( CityPathGraph const * cityGraph, IndexList const & path );
	
	bool     buildPath_World        ( void );

	void     addPathNode            ( CellProperty const * cell, PathNode const * node );

	// ----------

	CreatureObject const * m_creature;

	CellProperty const *   m_creatureCell;
	PathGraph const *      m_creatureCellGraph;
	int                    m_creatureCellKey;
	int                    m_creatureCellNodeIndex;
	int                    m_creatureCellPart;
	PortalProperty const * m_creatureBuilding;
	PathGraph const *      m_creatureBuildingGraph;
	int                    m_creatureBuildingKey;
	int                    m_creatureBuildingNodeIndex;
	CityPathGraph const *  m_creatureCityGraph;
	int                    m_creatureCityNodeIndex;
	Vector                 m_creaturePosition;

	// ----------

	AiLocation             m_goal;
	Unicode::String        m_goalName;

	CellProperty const *   m_goalCell;
	PathGraph const *      m_goalCellGraph;
	int                    m_goalCellKey;
	int                    m_goalCellNodeIndex;
	int                    m_goalCellPart;
	PortalProperty const * m_goalBuilding;
	PathGraph const *      m_goalBuildingGraph;
	int                    m_goalBuildingKey;
	int                    m_goalBuildingNodeIndex;
	CityPathGraph const *  m_goalCityGraph;
	int                    m_goalCityNodeIndex;

	AiPath *               m_path;

	// ----------

	bool                   m_async;
	bool                   m_buildDone;
	bool                   m_buildFailed;
	bool                   m_pathIncomplete;
	bool                   m_enableJitter;

	// ----------

	PathSearch *           m_cellSearch;

	PathSearch *           m_buildingSearch;
	
	PathSearch *           m_citySearch;

	// ----------

	bool                   m_queued;
};

// ----------------------------------------------------------------------

inline bool ServerPathBuilder::getQueued ( void ) const
{
	return m_queued;
}

inline void ServerPathBuilder::setQueued ( bool queued )
{
	m_queued = queued;
}

// ----------

inline void ServerPathBuilder::setEnableJitter ( bool enable )
{
	m_enableJitter = enable;
}

// ----------

inline AiPath * ServerPathBuilder::getPath ( void )
{
	return m_path;
}

// ----------

inline bool ServerPathBuilder::buildDone ( void ) const
{
	return m_buildDone;
}

inline bool ServerPathBuilder::buildFailed ( void ) const
{
	return m_buildFailed;
}

// ======================================================================

#endif

