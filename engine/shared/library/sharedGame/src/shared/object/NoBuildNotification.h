//===================================================================
//
// NoBuildNotification.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_NoBuildNotification_H
#define INCLUDED_NoBuildNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

class StructureFootprint;

//===================================================================

class NoBuildNotification : public ObjectNotification
{
public:

	NoBuildNotification ();
	virtual ~NoBuildNotification ();

	virtual void addToWorld (Object& object) const=0;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual void rotationChanged (Object& object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

protected:

	void addToWorld (Object& object, float noBuildRadius) const;

protected:

	NoBuildNotification (const NoBuildNotification&);
	NoBuildNotification& operator= (const NoBuildNotification&);
};

//===================================================================

#endif
