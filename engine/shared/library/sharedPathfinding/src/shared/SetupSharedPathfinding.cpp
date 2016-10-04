// ======================================================================
//
// SetupSharedPathfinding.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/SetupSharedPathfinding.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedPathfinding/ConfigSharedPathfinding.h"
#include "sharedPathfinding/PathEdge.h"
#include "sharedPathfinding/Pathfinding.h"
#include "sharedPathfinding/PathNode.h"
#include "sharedPathfinding/SimplePathGraph.h"

#include <vector>
#include <set>

typedef BaseClass * (*ExpandBuildingGraphHook)( PortalPropertyTemplate * portalTemplate, BaseClass * baseBuildingGraph );

BaseClass * expandBuildingGraph ( PortalPropertyTemplate * portalTemplate, BaseClass * baseBuildingGraph );

void SetupSharedPathfinding::install ( void )
{
	InstallTimer const installTimer("SetupSharedPathfinding::install");

	ConfigSharedPathfinding::install();
	Pathfinding::install();

	PortalPropertyTemplate::setExpandBuildingGraphHook(expandBuildingGraph);
}

void SetupSharedPathfinding::remove ( void )
{
}

// ----------------------------------------------------------------------

int findNeighbor ( PathNode * node, PathNodeType neighborType, int neighborKey )
{
	int nodeIndex = node->getIndex();

	PathGraph * graph = node->getGraph();

	int neighborCount = graph->getEdgeCount(nodeIndex);

	for(int i = 0; i < neighborCount; i++)
	{
		PathNode * neighbor = graph->getNeighbor(nodeIndex,i);

		if((neighbor->getType() == neighborType) && (neighbor->getKey() == neighborKey))
		{
			return neighbor->getIndex();
		}
	}

	return -1;
}

// ----------------------------------------------------------------------
// Expand the building graph by creating BuildingCellPart nodes for all
// parts of the cell graph in each cell

BaseClass * expandBuildingGraph ( PortalPropertyTemplate * portalTemplate, BaseClass * baseBuildingGraph )
{
	if(baseBuildingGraph == nullptr) return nullptr;

	SimplePathGraph * buildingGraph = safe_cast<SimplePathGraph*>(baseBuildingGraph);

	SimplePathGraph::NodeList * newBuildingNodes = new SimplePathGraph::NodeList( buildingGraph->getNodes() );
	SimplePathGraph::EdgeList * newBuildingEdges = new SimplePathGraph::EdgeList( buildingGraph->getEdges() );

	// ----------

	int buildingNodeCount = buildingGraph->getNodeCount();

	for(int iBuildingNode = 0; iBuildingNode < buildingNodeCount; iBuildingNode++)
	{
		PathNode * buildingCellNode = buildingGraph->getNode(iBuildingNode);

		if(buildingCellNode->getType() != PNT_BuildingCell) continue;

		// ----------

		int cellIndex = buildingCellNode->getKey();

		PortalPropertyTemplateCell const * cell = &portalTemplate->getCell(cellIndex);

		FloorMesh const * floorMesh = cell->getFloorMesh();

		if(floorMesh == nullptr) continue;

		PathGraph const * cellGraph = safe_cast<PathGraph const *>(floorMesh->getPathGraph());

		if(cellGraph == nullptr) continue;

		// ----------

		int cellPartCount = cellGraph->getPartCount();

		for(int i = 0; i < cellPartCount; i++)
		{
			int partNodeIndex = newBuildingNodes->size();

			newBuildingNodes->resize(newBuildingNodes->size() + 1);

			PathNode & partNode = newBuildingNodes->back();

			partNode.setType(PNT_BuildingCellPart);
			partNode.setPosition_p(buildingCellNode->getPosition_p());
			partNode.setKey(buildingCellNode->getKey());
			partNode.setId(i);

			// ----------
			// search through the nodes in the cell graph, add the indices of all
			// portals that have a portal node in this part of the graph to our
			// portal id set

			typedef std::set<int> IntSet;

			IntSet portalIds;

			for(int j = 0; j < cellGraph->getNodeCount(); j++)
			{
				PathNode const * cellNode = cellGraph->getNode(j);

				NOT_NULL(cellNode);

				if((cellNode->getPartId() == i) && (cellNode->getType() == PNT_CellPortal))
				{
					portalIds.insert(cellNode->getKey());
				}
			}

			// ----------
			// for each portal ID in the set, find the neighboring BuildingPortal node
			// of the BuildingCell node that corresponds with that ID, and add path
			// edges for it

			for(IntSet::iterator it = portalIds.begin(); it != portalIds.end(); ++it)
			{
				int portalId = *it;

				int portalNodeIndex = findNeighbor(buildingCellNode,PNT_BuildingPortal,portalId);

				if(portalNodeIndex == -1) portalNodeIndex = findNeighbor(buildingCellNode,PNT_BuildingEntrance,portalId);

				if(portalNodeIndex != -1)
				{
					newBuildingEdges->push_back( PathEdge(partNodeIndex,portalNodeIndex) );
					newBuildingEdges->push_back( PathEdge(portalNodeIndex,partNodeIndex) );
				}
			}

			// and add a link from the building cell node to the building cell part node

			newBuildingEdges->push_back( PathEdge(iBuildingNode,partNodeIndex) );
		}
	}

	// ----------

	SimplePathGraph * newBuildingGraph = new SimplePathGraph ( newBuildingNodes, newBuildingEdges );

	newBuildingGraph->setType(PGT_Building);

	return newBuildingGraph;
}
