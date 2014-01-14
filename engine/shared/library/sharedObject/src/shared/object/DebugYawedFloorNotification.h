// ======================================================================
//
// DebugYawedFloorNotification.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DebugYawedFloorNotification_H
#define INCLUDED_DebugYawedFloorNotification_H

// ======================================================================

#include "sharedObject/ObjectNotification.h"

// ======================================================================

class DebugYawedFloorNotification : public ObjectNotification
{
public:

	static DebugYawedFloorNotification& getInstance ();

public:

	DebugYawedFloorNotification ();
	virtual ~DebugYawedFloorNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void rotationChanged(Object &object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	DebugYawedFloorNotification (const DebugYawedFloorNotification&);
	DebugYawedFloorNotification& operator= (const DebugYawedFloorNotification&);

private:

	static DebugYawedFloorNotification ms_instance;
};

// ======================================================================

#endif
