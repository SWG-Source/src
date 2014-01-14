//===================================================================
//
// TerrainReferenceObjectNotification.h
// asommers
//
// copyright 2003, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_TerrainReferenceObjectNotification_H
#define INCLUDED_TerrainReferenceObjectNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

//===================================================================

class TerrainReferenceObjectNotification : public ObjectNotification
{
public:

	static TerrainReferenceObjectNotification& getInstance ();

public:

	TerrainReferenceObjectNotification ();
	virtual ~TerrainReferenceObjectNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;
	virtual void cellChanged (Object& object, bool dueToParentChange) const;

private:

	TerrainReferenceObjectNotification (const TerrainReferenceObjectNotification&);
	TerrainReferenceObjectNotification& operator= (const TerrainReferenceObjectNotification&);

private:

	static TerrainReferenceObjectNotification ms_instance;
};

//===================================================================

#endif
