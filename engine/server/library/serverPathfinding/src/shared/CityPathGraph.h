// ======================================================================
//
// CityPathGraph.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_CityPathGraph_H
#define	INCLUDED_CityPathGraph_H

#include "sharedPathfinding/DynamicPathGraph.h"

#include "sharedObject/CachedNetworkId.h"

class CityPathNode;
class ServerObject;
class PathNodeSphereAccessor;
class AxialBox;

template<typename T, typename U>
class SphereTree;

typedef SphereTree<PathNode *, PathNodeSphereAccessor> PathNodeTree;

typedef std::vector<AxialBox> BoxList;

// ======================================================================
// CityPathGraph is a DynamicPathGraph with some specializations that
// allow it to be constructed from objvars on server-side waypoint 
// objects

class CityPathGraph : public DynamicPathGraph
{
public:

	CityPathGraph ( int cityToken );

	virtual ~CityPathGraph();

	// ----------

	virtual int  addNode     ( DynamicPathNode * newNode );
	virtual void removeNode  ( int nodeIndex );
	virtual void moveNode    ( int nodeIndex, Vector const & newPosition );

	virtual void relinkNode  ( int nodeIndex );
	virtual void destroyNode ( int nodeIndex );

	// ----------

	CityPathNode *       _getNode ( int whichNode );
	CityPathNode const * _getNode ( int whichNode ) const;

	// ----------

	CityPathNode * findNodeForObject            ( ServerObject const & object );
	CityPathNode const * findNodeForObject      ( ServerObject const & object ) const;
	CityPathNode * findNearestNodeForName       ( Unicode::String const & nodeName, Vector const & pos );
	CityPathNode const * findNearestNodeForName ( Unicode::String const & nodeName, Vector const & pos ) const;

	int  findNearestNode   ( Vector const & position_p ) const;
	int  findNearestNode   ( PathNodeType type, Vector const & position_p ) const;

	void findNodesInRange  ( Vector const & position_p, float range, PathNodeList & results ) const; 

	// ----------

	void saveGraph         ( void );
	bool sanityCheck       ( bool doWarnings ) const;

	void reloadPathNodes   ( void );

	int  getCityToken      ( void ) const;

	// ----------

	ServerObject const * getCreator      ( void ) const;
	void                 setCreator      ( NetworkId const & creator );

	void                 addDirtyBox     ( AxialBox const & box );
	bool                 isDirty         ( Vector const & point_w ) const;

	static float         getLinkDistance ( void );
	static void          setLinkDistance ( float dist );

	int                  getNeighborCode ( int whichNode ) const;

protected:

	PathNodeTree * m_nodeTree;
	//@todo: order the CityPathNodes below in such a way as to make searching for the closest one quick
	std::map<Unicode::String, std::set<CityPathNode *> > * m_namedNodes;
	int m_token;

	CachedNetworkId m_creatorId;
	
	BoxList * m_dirtyBoxes;
};

// ----------

inline int CityPathGraph::getCityToken ( void ) const
{
	return m_token;
}

// ======================================================================

#endif

