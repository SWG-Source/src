//===================================================================
//
// ServerObjectTerrainModificationNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerObjectTerrainModificationNotification.h"

#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/Vector2d.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainModificationHelper.h"

//===================================================================

namespace ServerObjectTerrainModificationNotificationNamespace
{
	void snapObjectsToTerrain (Rectangle2d const & extent, float const height)
	{
		Vector const center (extent.getCenter ().x, height, extent.getCenter ().y);
		float const radius = sqrt (sqr (extent.getWidth ()) + sqr (extent.getHeight ()));

		typedef std::vector<ServerObject *> ServerObjectList;
		ServerObjectList result;
		ServerWorld::findObjectsInRange (center, radius, result);

		ServerObjectList::iterator end = result.end ();
		for (ServerObjectList::iterator iter = result.begin (); iter != end; ++iter)
		{
			ServerObject * const serverObject = *iter;
			if (serverObject)
			{
				SharedObjectTemplate const * const sharedObjectTemplate = serverObject->getSharedTemplate ();
				if (sharedObjectTemplate)
				{
					SharedObjectTemplate::GameObjectType const gameObjectType = sharedObjectTemplate->getGameObjectType ();
					if (gameObjectType == SharedObjectTemplate::GOT_corpse)
						World::snapObjectToTerrain (serverObject);
				}
			}
		}
	}
}

using namespace ServerObjectTerrainModificationNotificationNamespace;

//===================================================================

ProceduralTerrainAppearance* ServerObjectTerrainModificationNotification::ms_terrainAppearance;
ServerObjectTerrainModificationNotification ServerObjectTerrainModificationNotification::ms_instance;

//===================================================================

ServerObjectTerrainModificationNotification &ServerObjectTerrainModificationNotification::getInstance()
{
	return ms_instance;
}

//-------------------------------------------------------------------

void ServerObjectTerrainModificationNotification::setTerrainAppearance (ProceduralTerrainAppearance* const terrainAppearance)
{
	ms_terrainAppearance = terrainAppearance;
}

//-------------------------------------------------------------------

ServerObjectTerrainModificationNotification::ServerObjectTerrainModificationNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------
	
ServerObjectTerrainModificationNotification::~ServerObjectTerrainModificationNotification ()
{
}

//-------------------------------------------------------------------

void ServerObjectTerrainModificationNotification::addToWorld (Object& object) const
{
	//-- insert the layer into the terrain system
	ServerObject* const serverObject = object.asServerObject();
	TerrainGenerator::Layer* const layer = serverObject->getLayer ();
	ms_terrainAppearance->addLayer (layer);

	updateTerrain (object, false, true);
}

//-------------------------------------------------------------------

void ServerObjectTerrainModificationNotification::removeFromWorld (Object& object) const
{
	//-- remove the layer from the terrain system
	ServerObject* const serverObject = object.asServerObject();
	TerrainGenerator::Layer* const layer = serverObject->getLayer ();
	ms_terrainAppearance->removeLayer (layer);

	updateTerrain (object, true, false);
}

//-------------------------------------------------------------------

bool ServerObjectTerrainModificationNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	//-- update the layer with new position data
	if (object.isInWorld ())
		updateTerrain (object, true, true);

	return true;
}

//-------------------------------------------------------------------

void ServerObjectTerrainModificationNotification::rotationChanged (Object& object, bool /*dueToParentChange*/) const
{
	//-- update the layer with new position data
	if (object.isInWorld ())
		updateTerrain (object, false, false);
}

//-------------------------------------------------------------------

bool ServerObjectTerrainModificationNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	//-- update the layer with new position data
	if (object.isInWorld ())
		updateTerrain (object, true, true);

	return true;
}

//-------------------------------------------------------------------

void ServerObjectTerrainModificationNotification::updateTerrain (Object& object, bool const updateOldExtent, bool const updateNewExtent) const
{
	ServerObject* const serverObject = object.asServerObject();
	TerrainGenerator::Layer* const layer = serverObject->getLayer ();
	ProceduralTerrainAppearance* const proceduralTerrainAppearance = safe_cast<ProceduralTerrainAppearance*> (ms_terrainAppearance);
	NOT_NULL (proceduralTerrainAppearance);
	
	//-- get the new position of the object
	const Vector position = object.getPosition_w ();

	//-- save the current extent of the layer
	layer->calculateExtent ();
	Rectangle2d oldExtent;
	float oldHeight = 0.f;
	bool hasOldExtent = updateOldExtent && layer->getUseExtent ();
	if (hasOldExtent)
	{
		oldExtent = layer->getExtent ();
		oldHeight = layer->getModificationHeight ();
	}

	//-- calculate the updated layer information (position and heights)
	const Vector2d& position2d = Vector2d (position.x, position.z);
	TerrainModificationHelper::setPosition (layer, position2d);
	TerrainModificationHelper::setRotation (layer, object.getObjectFrameK_w ().theta ());
	TerrainModificationHelper::setHeight (layer, ms_terrainAppearance->generateHeight_expensive (position2d));

	//-- prepare the terrain
	ms_terrainAppearance->prepareGenerator ();

	//-- get the new extent of the layer
	layer->calculateExtent ();
	Rectangle2d newExtent;
	float newHeight = 0.f;
	bool hasNewExtent = updateNewExtent && layer->getUseExtent ();
	if (hasNewExtent)
	{
		newExtent = layer->getExtent ();
		newHeight = layer->getModificationHeight ();
	}

	//-- tell the terrain system the old and new extent regions have changed
	if (hasOldExtent)
	{
		ms_terrainAppearance->invalidateRegion (oldExtent);
		snapObjectsToTerrain (oldExtent, oldHeight);
	}

	if (hasNewExtent)
	{
		ms_terrainAppearance->invalidateRegion (newExtent);
		snapObjectsToTerrain (newExtent, newHeight);
	}
}

//===================================================================

