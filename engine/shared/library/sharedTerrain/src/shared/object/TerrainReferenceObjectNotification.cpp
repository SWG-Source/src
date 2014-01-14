//===================================================================
//
// TerrainReferenceObjectNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/TerrainReferenceObjectNotification.h"

#include "sharedTerrain/TerrainObject.h"

//===================================================================

TerrainReferenceObjectNotification TerrainReferenceObjectNotification::ms_instance;

//===================================================================

TerrainReferenceObjectNotification& TerrainReferenceObjectNotification::getInstance ()
{
	return ms_instance;
}

//===================================================================

TerrainReferenceObjectNotification::TerrainReferenceObjectNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------
	
TerrainReferenceObjectNotification::~TerrainReferenceObjectNotification ()
{
}

//-------------------------------------------------------------------

void TerrainReferenceObjectNotification::addToWorld (Object& object) const
{
	//-- if there is no terrain, return
	if (!TerrainObject::getInstance ())
		return;

	if (object.isInWorldCell ())
		TerrainObject::getInstance ()->addReferenceObject (&object);
}

//-------------------------------------------------------------------

void TerrainReferenceObjectNotification::removeFromWorld (Object& object) const
{
	//-- if there is no terrain, return
	if (!TerrainObject::getInstance ())
		return;

	if (object.isInWorldCell ())
		TerrainObject::getInstance ()->removeReferenceObject (&object);
}

//-------------------------------------------------------------------

void TerrainReferenceObjectNotification::cellChanged (Object& object, bool /*dueToParentChange*/) const
{
	//-- if there is no terrain, return
	if (!TerrainObject::getInstance ())
		return;

	//-- if the object is in the world cell
	if (object.isInWorldCell ())
	{
		//-- and not a reference object, add it
		if (!TerrainObject::getInstance ()->isReferenceObject (&object))
			addToWorld (object);
	}
	else
	{
		//-- otherwise, if it is a reference object, remove it
		if (TerrainObject::getInstance ()->isReferenceObject (&object))
			removeFromWorld (object);
	}
}

//===================================================================

