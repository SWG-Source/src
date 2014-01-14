// ======================================================================
//
// DebugNotifcation.h
// Copyright 2003 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DebugNotifcation_H
#define INCLUDED_DebugNotifcation_H

// ======================================================================

#include "sharedObject/ObjectNotification.h"

// ======================================================================

class DebugNotification : public ObjectNotification
{
public:

	static DebugNotification & getInstance();

public:

	DebugNotification();
	virtual ~DebugNotification();

	virtual int getPriority() const;

	virtual void addToWorld(Object & object) const;
	virtual void removeFromWorld(Object & object) const;

	virtual bool positionChanged(Object & object, bool dueToParentChange, const Vector & oldPosition) const;
	virtual void rotationChanged(Object & object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged(Object & object, bool dueToParentChange, const Vector & oldPosition) const;
	virtual void cellChanged(Object & object, bool dueToParentChange) const;
	virtual void extentChanged(Object & object) const;

private:

	static DebugNotification ms_instance;

private:

	DebugNotification(DebugNotification const &);
	DebugNotification & operator=(DebugNotification const &);
};

// ======================================================================

#endif
