// ======================================================================
//
// Pathfinding.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/Pathfinding.h"

#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorManager.h"

#include "sharedFile/Iff.h"

#include "sharedObject/CellProperty.h"

#include "sharedPathfinding/PathSearch.h"
#include "sharedPathfinding/SimplePathGraph.h"

const Tag TAG_PGRF = TAG(P,G,R,F);
const Tag TAG_PNOD = TAG(P,N,O,D);

// ======================================================================

void Pathfinding::install ( void )
{
	FloorManager::setPathGraphFactory( &Pathfinding::graphFactory );
	FloorManager::setPathGraphWriter( &Pathfinding::graphWriter );
	FloorManager::setPathGraphRenderer( &Pathfinding::graphRenderer );

	PathSearch::install();
}

// ----------

void Pathfinding::remove ( void )
{
}

// ----------

BaseClass * Pathfinding::graphFactory ( Iff & iff )
{
	if(iff.getCurrentName() == TAG_PGRF)
	{
		SimplePathGraph * graph = new SimplePathGraph();

		graph->read(iff);

		graph->setPartTags();

		return graph;
	}
	else if(iff.getCurrentName() == TAG_PNOD)
	{
		// This is an old IFF, the path information isn't in a PGRF form.

		SimplePathGraph * graph = new SimplePathGraph();

		graph->read_old(iff);

		graph->setPartTags();

		return graph;
	}
	else
	{
		DEBUG_WARNING(true,("Pathfinding::graphFactory - Don't know how to construct a path graph from the IFF in %s\n",iff.getFileName()));

		return nullptr;
	}
}

// ----------

void Pathfinding::graphWriter ( BaseClass const * baseGraph, Iff & iff )
{
	SimplePathGraph const * graph = safe_cast<SimplePathGraph const *>(baseGraph);

	graph->write(iff);
}

// ----------

void Pathfinding::graphRenderer ( BaseClass const * baseGraph, DebugShapeRenderer * renderer )
{
	PathGraph const * graph = safe_cast<PathGraph const *>(baseGraph);

	graph->drawDebugShapes(renderer);
}

// ======================================================================
