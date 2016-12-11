// ======================================================================
//
// CityPathGraphManager.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_CityPathGraphManager_H
#define	INCLUDED_CityPathGraphManager_H

class ServerObject;
class PathGraph;
class Vector;
class CityPathGraph;
class CityPathNode;
class BuildingObject;

typedef std::map< int, CityPathGraph * > CityGraphMap;

typedef std::vector< Vector > PositionList;

// ======================================================================

class CityPathGraphManager
{
public:

	static void install                  ( void );
	static void remove                   ( void );

	static void update                   ( float time );

	static void preloadComplete          ( void );
	
	static void sanityCheck              ( void );

	static void addWaypoint              ( ServerObject * object );
	static void removeWaypoint           ( ServerObject * object );
	static void moveWaypoint             ( ServerObject * object, Vector const & oldPosition );
	static void destroyWaypoint          ( ServerObject * object );

	static void addBuilding              ( BuildingObject * building );
	static void removeBuilding           ( BuildingObject * building );
	static void moveBuilding             ( BuildingObject * building, Vector const & oldPosition );
	static void destroyBuilding          ( BuildingObject * building );

	// ----------

	static bool destroyPathGraph         ( ServerObject const * creator );

	// ----------

	static bool createPathNodes          ( ServerObject * building );
	static bool destroyPathNodes         ( ServerObject * building );

	static bool reloadPathNodes          ( void );
	static bool reloadPathNodes          ( std::vector< ServerObject * > const & objects );

	static bool markCityEntrance         ( ServerObject * object );
	static bool unmarkCityEntrance       ( ServerObject * object );
	
	static float getLinkDistance         ( void );
	static void  setLinkDistance         ( float dist );

	static void relinkGraph              ( CityPathGraph const * graph );

	// ----------

	static int  getGraphCount            ( void );
	static CityPathGraph *  getGraph     ( int index );

	static CityPathGraph const *    getCityGraphFor ( ServerObject const * object );
	static CityPathGraph const *    getCityGraphFor ( Vector const & position );

	static CityPathNode const *     getNamedNodeFor( ServerObject const & object, Unicode::String const & nodeName );

	// ----------

	static bool getClosestPathNodePos    ( ServerObject const * object, Vector & outPos );

protected:

	CityPathGraphManager();
	~CityPathGraphManager();
};

// ======================================================================

#endif

