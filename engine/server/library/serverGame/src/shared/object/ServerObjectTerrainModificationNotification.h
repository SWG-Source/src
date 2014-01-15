//===================================================================
//
// ServerObjectTerrainModificationNotification.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ServerObjectTerrainModificationNotification_H
#define INCLUDED_ServerObjectTerrainModificationNotification_H

//===================================================================

class ProceduralTerrainAppearance;

#include "sharedObject/ObjectNotification.h"

//===================================================================

class ServerObjectTerrainModificationNotification : public ObjectNotification
{
public:

	static void setTerrainAppearance (ProceduralTerrainAppearance* const terrainAppearance);
	static ServerObjectTerrainModificationNotification &getInstance();

public:

	ServerObjectTerrainModificationNotification ();
	virtual ~ServerObjectTerrainModificationNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual void rotationChanged (Object& object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	void updateTerrain (Object& object, bool updateOldExtent, bool updateNewExtent) const;

private:

	ServerObjectTerrainModificationNotification (const ServerObjectTerrainModificationNotification&);
	ServerObjectTerrainModificationNotification& operator= (const ServerObjectTerrainModificationNotification&);

private:

	static ProceduralTerrainAppearance* ms_terrainAppearance;
	static ServerObjectTerrainModificationNotification ms_instance;
};

//===================================================================

#endif
