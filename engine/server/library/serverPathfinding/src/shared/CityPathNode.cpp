// ======================================================================
//
// CityPathNode.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/CityPathNode.h"

#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"

#include "serverPathfinding/CityPathGraph.h"
#include "serverPathfinding/ServerPathfindingConstants.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/CollisionUtils.h"

#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/NetworkId.h"

#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"

#include "sharedPathfinding/PathEdge.h"

#include "sharedTerrain/TerrainObject.h"

#include <vector>

typedef std::vector<NetworkId> NetworkIdList;

namespace CityPathNodeNamespace
{
	int gs_debugIdCounter = 0;
};

using namespace CityPathNodeNamespace;

// ======================================================================

CityPathNode::CityPathNode ( Vector const & position_w, NetworkId const & sourceId )
: DynamicPathNode(position_w),
  m_source(sourceId),
  m_creator(),
  m_relativePosition_o(Vector::zero),
  m_spatialHandle(nullptr),
  m_debugId( gs_debugIdCounter++ )
{
	snapToTerrain();
}

// ----------

CityPathNode::CityPathNode ( Vector const & position_w, NetworkId const & sourceId, NetworkId const & creatorId )
: DynamicPathNode(position_w),
  m_source(sourceId),
  m_creator(creatorId),
  m_relativePosition_o(Vector::zero),
  m_spatialHandle(nullptr),
  m_debugId( gs_debugIdCounter++ )
{
	updateRelativePosition();

	snapToTerrain();
}

// ----------

CityPathNode::~CityPathNode()
{
}

// ----------------------------------------------------------------------

void CityPathNode::clearEdges ( void )
{
	ServerObject * sourceObject = getSourceObject();

	if(sourceObject)
	{
		sourceObject->removeObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_EDGES);
	}

	DynamicPathNode::clearEdges();
}

// ----------------------------------------------------------------------

CellProperty const * CityPathNode::getCell ( void ) const
{
	ServerObject const * sourceObject = getSourceObject();

	if(sourceObject)
	{
		return sourceObject->getParentCell();
	}
	else
	{
		return CellProperty::getWorldCellProperty();
	}
}

// ----------------------------------------------------------------------

Vector CityPathNode::getPosition_w ( void ) const
{
	return CollisionUtils::transformToWorld(getCell(),getPosition_p());
}

// ----------------------------------------------------------------------

void CityPathNode::setPosition_p ( Vector const & newPosition_p )
{
	DynamicPathNode::setPosition_p(newPosition_p);

	updateRelativePosition();

	snapToTerrain();
}

// ----------------------------------------------------------------------

void CityPathNode::setPosition_w ( Vector const & newPosition_w )
{
	Vector newPosition_p = CollisionUtils::transformFromWorld(newPosition_w,getCell());

	setPosition_p(newPosition_p);
}

// ----------------------------------------------------------------------

void CityPathNode::updateRelativePosition ( void )
{
	ServerObject const * sourceObject = getSourceObject();

	ServerObject const * creatorObject = getCreatorObject();

	if(sourceObject && creatorObject)
	{
		m_relativePosition_o = creatorObject->rotateTranslate_w2o(sourceObject->getPosition_w());
	}
}

// ----------------------------------------------------------------------

void CityPathNode::loadFromObjvars ( void )
{
	loadInfoFromObjvars();
	loadEdgesFromObjvars();
}

// ----------------------------------------------------------------------

void CityPathNode::loadInfoFromObjvars ( void )
{
	ServerObject const * sourceObject = getSourceObject();

	if(sourceObject == nullptr) return;

	const DynamicVariableList & objvars = sourceObject->getObjVars();

	// ----------

	m_creator = NetworkId::cms_invalid;
	m_key = -1;
	m_name.clear();
	m_type = PNT_Invalid;

	NetworkId creatorId;
	if (objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_CREATOR,creatorId))
		m_creator = creatorId;

	objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_KEY, m_key);
	objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_NAME, m_name);

	int typeInt;
	if(objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_TYPE, typeInt))
		m_type = static_cast<PathNodeType>(typeInt);

}

// ----------------------------------------------------------------------

void CityPathNode::loadEdgesFromObjvars ( void )
{
	ServerObject const * sourceObject = getSourceObject();

	if(sourceObject == nullptr) return;

	DynamicVariableList const & objvars = sourceObject->getObjVars();

	// ----------

	NetworkIdList idList;
	if(objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_EDGES,idList))
	{
		int idCount = idList.size();

		for(int i = 0; i < idCount; i++)
		{
			ServerObject * serverObject = ServerWorld::findObjectByNetworkId( idList[i] );

			if(serverObject == nullptr) continue;

			CityPathNode * otherNode = _getGraph()->findNodeForObject(*serverObject);

			if(otherNode == nullptr) continue;

			addEdge(otherNode->getIndex());
			otherNode->addEdge(getIndex());
		}
	}
	else
	{
		DEBUG_WARNING(true,("CityPathNode::loadEdgesFromObjvars - Source object has no edge list objvar\n"));
	}
}

// ----------------------------------------------------------------------

void CityPathNode::saveToObjvars ( void )
{
	saveInfoToObjvars();
	saveEdgesToObjvars();
}

// ----------------------------------------------------------------------

void CityPathNode::saveInfoToObjvars ( void )
{
	ServerObject * sourceObject = getSourceObject();

	if(sourceObject == nullptr) return;

	// ----------

	NetworkId const & creatorId = getCreatorId();

	if(creatorId.isValid())
	{
		sourceObject->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_CREATOR, creatorId);
	}
	else
	{
		sourceObject->removeObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_CREATOR);
	}

	if(m_key != -1)
	{
		sourceObject->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_KEY,static_cast<int>(m_key));
	}
	else
	{
		sourceObject->removeObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_KEY);
	}

	if(!m_name.empty())
	{
		sourceObject->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_NAME,m_name);
	}
	else
	{
		sourceObject->removeObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_NAME);
	}

	if(m_type != PNT_Invalid)
	{
		sourceObject->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_TYPE,static_cast<int>(m_type));
	}
	else
	{
		sourceObject->removeObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_TYPE);
	}
}

// ----------------------------------------------------------------------

void CityPathNode::saveEdgesToObjvars ( void )
{
	ServerObject * sourceObject = getSourceObject();

	// ----------

	NetworkIdList idList;

	int edgeCount = getEdgeCount();

	for(int i = 0; i < edgeCount; i++)
	{
		int neighborIndex = getNeighbor(i);

		CityPathNode * neighborNode = _getGraph()->_getNode(neighborIndex);

		if(neighborNode == nullptr) continue;

		NetworkId const & neighborSourceId = neighborNode->getSourceId();

		if(neighborSourceId.isValid())
		{
			idList.push_back(neighborSourceId);
		}
	}

	if(!idList.empty())
	{
		sourceObject->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_EDGES,idList);
	}
	else
	{
		sourceObject->removeObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_EDGES);
	}
}


// ----------------------------------------------------------------------

void CityPathNode::saveNeighbors ( void )
{
	int edgeCount = getEdgeCount();

	for(int i = 0; i < edgeCount; i++)
	{
		int neighborIndex = getNeighbor(i);

		CityPathNode * neighborNode = _getGraph()->_getNode(neighborIndex);

		if(neighborNode == nullptr) continue;

		neighborNode->saveToObjvars();
	}
}

// ----------------------------------------------------------------------

void CityPathNode::saveAllData ( void )
{
	saveToObjvars();
	saveNeighbors();
}

// ----------------------------------------------------------------------

ServerObject * CityPathNode::getSourceObject ( void )
{
	return safe_cast<ServerObject*>(m_source.getObject());
}

ServerObject const * CityPathNode::getSourceObject ( void ) const
{
	return safe_cast<ServerObject const *>(m_source.getObject());
}

NetworkId const & CityPathNode::getSourceId ( void ) const
{
	return m_source;
}

// ----------

ServerObject * CityPathNode::getCreatorObject ( void )
{
	return safe_cast<ServerObject *>(m_creator.getObject());
}

ServerObject const * CityPathNode::getCreatorObject ( void ) const
{
	return safe_cast<ServerObject const *>(m_creator.getObject());
}

NetworkId const & CityPathNode::getCreatorId ( void ) const
{
	return m_creator;
}

// ----------

void CityPathNode::setCreator ( NetworkId const & creatorId )
{
	m_creator = creatorId;

	updateRelativePosition();
}

// ----------------------------------------------------------------------

CityPathGraph * CityPathNode::_getGraph ( void )
{
	return safe_cast<CityPathGraph *>(getGraph());
}

CityPathGraph const * CityPathNode::_getGraph ( void ) const
{
	return safe_cast<CityPathGraph const *>(getGraph());
}

// ----------------------------------------------------------------------

int CityPathNode::getDebugId ( void ) const
{
	return m_debugId;
}

// ----------------------------------------------------------------------

bool CityPathNode::sanityCheck ( bool doWarnings ) const
{
	UNREF(doWarnings); //for release build
	int insaneCount = 0;

	int edgeCount = getEdgeCount();

	int i;

	for(i = 0; i < edgeCount; i++)
	{
		int neighborIndex = getNeighbor(i);

		CityPathNode const * neighborNode = _getGraph()->_getNode(neighborIndex);

		if(neighborNode == nullptr)
		{
			DEBUG_WARNING(doWarnings,("CityPathNode::sanityCheck - Node has an edge to a non-existent node\n"));
			insaneCount++;
		}

		if(!neighborNode->hasEdge(getIndex()))
		{
			DEBUG_WARNING(doWarnings,("CityPathNode::sanityCheck - Node has a one-way edge to another node\n"));
			insaneCount++;
		}
	}

	NetworkIdList idList;

	ServerObject const * sourceObject = getSourceObject();

	if(sourceObject && sourceObject->getObjVars().getItem(OBJVAR_PATHFINDING_WAYPOINT_EDGES,idList))
	{
		int idCount = idList.size();

		for(int i = 0; i < idCount; i++)
		{
			if(!hasEdgeTo(idList[i]))
			{
				DEBUG_WARNING(doWarnings,("CityPathNode::sanityCheck - Node has a link in its objvar list that's not present in the edge list\n"));
				insaneCount++;
			}
		}
	}

	if(getType() != PNT_CityBuilding)
	{
		for(i = 0; i < edgeCount; i++)
		{
			CityPathNode const * nodeA = this;
			CityPathNode const * nodeB = _getGraph()->_getNode(getNeighbor(i));

			if(nodeB == nullptr) continue;

			if(nodeB->getType() == PNT_CityBuilding) continue;

			Vector posA = nodeA->getPosition_p();
			Vector posB = nodeB->getPosition_p();

			bool moveAB = (CollisionWorld::canMove( CellProperty::getWorldCellProperty(), posA, posB, 0.0f, true, true, false ) == CMR_MoveOK);
			
			if(!moveAB)
			{
				DEBUG_WARNING(doWarnings,("CityPathNode::sanityCheck - Node has a link that's not traversable\n"));
				insaneCount++;
			}
		}
	}

	int redundantCount = markRedundantEdges();

	if(redundantCount > 0)
	{
		DEBUG_WARNING(doWarnings,("CityPathNode::sanityCheck - Node has %d redundant edges\n",redundantCount));
		insaneCount++;
	}
	
	return insaneCount == 0;
}

// ----------------------------------------------------------------------
// Reload the parts of the node data that are set up by designers

void CityPathNode::reload ( void )
{
	ServerObject const * sourceObject = getSourceObject();

	if(sourceObject == nullptr) return;

	// ----------

	DynamicVariableList const & objvars = sourceObject->getObjVars();

	// ----------

	objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_NAME,m_name);
	int pathNodeTypeInt;
	objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_TYPE,pathNodeTypeInt);
	m_type = static_cast<PathNodeType>(pathNodeTypeInt);
}

// ----------------------------------------------------------------------

bool CityPathNode::hasEdgeTo ( NetworkId const & neighborId ) const
{
	if(!neighborId.isValid()) return false;

	int edgeCount = getEdgeCount();

	for(int i = 0; i < edgeCount; i++)
	{
		int neighborIndex = getNeighbor(i);

		CityPathNode const * neighborNode = _getGraph()->_getNode(neighborIndex);

		if(neighborNode == nullptr) continue;

		if(neighborNode->getSourceId() == neighborId) return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void CityPathNode::snapToTerrain ( void ) 
{
	CellProperty const * cell = getCell();

	if((cell == nullptr) || (cell == CellProperty::getWorldCellProperty()))
	{
		Vector pos_w = getPosition_w();

		float height = 0.0f;

		if(TerrainObject::getConstInstance ()->getHeightForceChunkCreation(pos_w,height))
		{
			if(height > pos_w.y)
			{
				pos_w.y = height;

				Vector pos_p = CollisionUtils::transformFromWorld(pos_w,getCell());

				DynamicPathNode::setPosition_p(pos_p);
			}
		}
	}
}

// ----------------------------------------------------------------------
