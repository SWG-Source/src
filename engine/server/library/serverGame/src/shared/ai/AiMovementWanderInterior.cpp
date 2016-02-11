// ======================================================================
//
// AiMovementWanderInterior.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementWanderInterior.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/CreatureObject.h"

#include "serverPathfinding/ServerPathBuilder.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"

#include "sharedLog/Log.h"

#include "sharedObject/CellProperty.h"

#include "sharedPathfinding/PathGraph.h"
#include "sharedPathfinding/PathNode.h"
#include "sharedPathfinding/PathSearch.h"

#include "sharedRandom/Random.h"

// ----------------------------------------------------------------------
// Pick a random number in [0,max], except for exclude

int random_exclude( int max, int exclude )
{
	if(max == 0) return 0;

	int x = Random::random(max-1);

	if(x >= exclude) x++;

	return x;
}


// ======================================================================

AiMovementWanderInterior::AiMovementWanderInterior( AICreatureController * controller )
: AiMovementWander(controller,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f),
  m_iterator(),
  m_lastNodeIndex(-1)
{
}

AiMovementWanderInterior::AiMovementWanderInterior( AICreatureController * controller, Archive::ReadIterator & source )
: AiMovementWander(controller, source),
  m_iterator(),
  m_lastNodeIndex(-1)
{
	Archive::get(source, m_lastNodeIndex);
}

// ----------

AiMovementWanderInterior::~AiMovementWanderInterior()
{
}

// ----------------------------------------------------------------------

void AiMovementWanderInterior::pack( Archive::ByteStream & target ) const
{
	AiMovementWander::pack(target);
	Archive::put(target, m_lastNodeIndex);
}

// ----------------------------------------------------------------------
// pick a random pathnode on the floor of the cell and walk to it

bool AiMovementWanderInterior::updateWaypoint ( void )
{
	CreatureObject const * creature = m_controller->getCreature();

	if(creature == nullptr) return false;

	CellProperty const * cell = creature->getParentCell();

	if(cell == nullptr) return false;

	Floor const * floor = cell->getFloor();

	if(floor == nullptr) return false;

	FloorMesh const * floorMesh = floor->getFloorMesh();

	if(floorMesh == nullptr) return false;

	PathGraph const * graph = safe_cast<PathGraph const *>(floorMesh->getPathGraph());

	if(graph == nullptr) return false;

	// ----------
	
	int closestIndex = findClosestReachablePathNode(creature,graph);

	if(closestIndex == -2)
	{
		const CellObject *cellObject = dynamic_cast<const CellObject*>(&cell->getOwner());
		const BuildingObject *building = (cellObject ? cellObject->getOwnerBuilding() : nullptr);
		Vector creaturePosition = creature->getPosition_w();
		
		LOG("building-data-error",("Building id=%s has no path data but creature id=%s at (x=%.2f,y=%.2f,z=%.2f) requires it for wandering, stopping behavior.",	
			(building ? building->getNetworkId().getValueString().c_str() : "<nullptr building>"),
			creature->getNetworkId().getValueString().c_str(),
			creaturePosition.x,
			creaturePosition.y,
			creaturePosition.z));

		endBehavior();
	}

	if(closestIndex < 0) return false;

	int randomIndex = random_exclude( graph->getNodeCount() - 1, closestIndex );

	PathSearch search;

	bool searchOk = search.search(graph,closestIndex,randomIndex);

	if(searchOk)
	{
		PathNode const * closestNode = graph->getNode(closestIndex);

		if(closestNode != nullptr)
		{
			m_target = AiLocation(m_controller->getCreatureCell(),closestNode->getPosition_p());

			m_target.setRadius(0.1f);

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// ======================================================================
