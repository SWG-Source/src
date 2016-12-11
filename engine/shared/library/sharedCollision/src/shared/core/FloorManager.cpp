// ======================================================================
//
// FloorManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/FloorManager.h"

#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"

#include "sharedObject/Object.h"

#include "sharedFoundation/DataResourceList.h"


namespace FloorManagerNamespace
{	

ObjectFactory  ms_pathGraphFactory = nullptr;
ObjectWriter   ms_pathGraphWriter = nullptr;
ObjectRenderer ms_pathGraphRenderer = nullptr;

};

using namespace FloorManagerNamespace;

// ----------------------------------------------------------------------

void FloorManager::setPathGraphFactory ( ObjectFactory factory )
{
	ms_pathGraphFactory = factory;
}

ObjectFactory FloorManager::getPathGraphFactory ( void )
{
	return ms_pathGraphFactory;
}

// ----------

void FloorManager::setPathGraphWriter ( ObjectWriter writer )
{
	ms_pathGraphWriter = writer;
}

ObjectWriter FloorManager::getPathGraphWriter ( void )
{
	return ms_pathGraphWriter;
}

// ----------

void FloorManager::setPathGraphRenderer( ObjectRenderer renderer )
{
	ms_pathGraphRenderer = renderer;
}

ObjectRenderer FloorManager::getPathGraphRenderer ( void )
{
	return ms_pathGraphRenderer;
}

// ----------------------------------------------------------------------

Floor * FloorManager::createFloor ( const char * floorMeshFilename, Object const * owner, Appearance const * appearance, bool objectFloor )
{
	FloorMesh * pMesh = const_cast<FloorMesh *>(FloorMeshList::fetch(floorMeshFilename));

	if(!pMesh)
	{
		DEBUG_WARNING(true, ("Could not find floor mesh %s", floorMeshFilename));
		return nullptr;
	}

	// ----------
	
	Floor * pFloor = new Floor( pMesh, owner, appearance, objectFloor );

	pMesh->releaseReference();

	return pFloor;
}

// ----------------------------------------------------------------------

