//===================================================================
//
// LotManagerNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/LotManagerNotification.h"

#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedObject/LotManager.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================
// PUBLIC LotManagerNotification
//===================================================================

LotManagerNotification::LotManagerNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------
	
LotManagerNotification::~LotManagerNotification ()
{
}

//-------------------------------------------------------------------

void LotManagerNotification::removeFromWorld (Object& object) const
{
	LotManager* const lotManager = World::getLotManager ();
	NOT_NULL (lotManager);

	lotManager->removeStructureFootprintEntry (object);
}

//-------------------------------------------------------------------

bool LotManagerNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	rotationChanged (object, false);

	return true;
}

//-------------------------------------------------------------------

void LotManagerNotification::rotationChanged (Object& object, bool /*dueToParentChange*/) const
{
	if (object.isInWorld ())
	{
		removeFromWorld (object);
		addToWorld (object);
	}
}

//-------------------------------------------------------------------

bool LotManagerNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	rotationChanged (object, false);

	return true;
}

//===================================================================
// PROTECTED LotManagerNotification
//===================================================================

void LotManagerNotification::addToWorld (Object& object, const StructureFootprint& structureFootprint) const
{
	const Vector k = object.getObjectFrameK_w ();

	RotationType rotationType = RT_0;
	if (k.x > 0.95)
		rotationType = RT_90;
	else
		if (k.z < -0.95)
			rotationType = RT_180;
		else
			if (k.x < -0.95)
				rotationType = RT_270;

	const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
	NOT_NULL (terrainObject);

	const Vector createLocation = object.getPosition_w ();
	const int x = terrainObject->calculateChunkX (createLocation);
	const int z = terrainObject->calculateChunkZ (createLocation);

	LotManager* const lotManager = World::getLotManager ();
	NOT_NULL (lotManager);

	lotManager->addStructureFootprintEntry (object, structureFootprint, x, z, rotationType);
}

//===================================================================


