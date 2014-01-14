//===================================================================
//
// LotManagerNotification.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_LotManagerNotification_H
#define INCLUDED_LotManagerNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

class StructureFootprint;

//===================================================================

class LotManagerNotification : public ObjectNotification
{
public:

	LotManagerNotification ();
	virtual ~LotManagerNotification ();

	virtual void addToWorld (Object& object) const=0;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual void rotationChanged (Object& object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

protected:

	virtual void addToWorld (Object& object, const StructureFootprint& structureFootprint) const;

private:

	LotManagerNotification (const LotManagerNotification&);
	LotManagerNotification& operator= (const LotManagerNotification&);
};

//===================================================================

#endif
