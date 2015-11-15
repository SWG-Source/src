// ======================================================================
//
// ObjectNotifcation.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ObjectNotification.h"

// ======================================================================
/**
 * Create a notification object.
 */

ObjectNotification::ObjectNotification()
{
}

// ----------------------------------------------------------------------
/**
 * Destroy a notification object.
 */

ObjectNotification::~ObjectNotification()
{
}

// ----------------------------------------------------------------------
/**
 * Get the priority for this notification.
 *
 * This will affect the order in which the notification gets called.
 */

int ObjectNotification::getPriority() const
{
	return 0;
}

// ----------------------------------------------------------------------
/**
 * Notify that this object should be added to the world.
 */

void ObjectNotification::addToWorld(Object &) const
{
}

// ----------------------------------------------------------------------
/**
 * Notify that this object should be removed from the world.
 */

void ObjectNotification::removeFromWorld(Object &) const
{
}

// ----------------------------------------------------------------------
/**
 * Notify that this object's position likely has changed.
 */

bool ObjectNotification::positionChanged(Object &, bool, const Vector &) const
{
	return true;
}

// ----------------------------------------------------------------------
/**
 * Notify that this object's orientation likely has changed.
 */

void ObjectNotification::rotationChanged(Object &, bool) const
{
}

// ----------------------------------------------------------------------
/**
 * Notify that this object's position and/or orientation likely has changed.
 */

bool ObjectNotification::positionAndRotationChanged(Object &, bool, const Vector &) const
{
	return true;
}

// ----------------------------------------------------------------------
/**
 * Notify that this object is changing cells.
 */

void ObjectNotification::cellChanged(Object &, bool) const
{
}

// ----------------------------------------------------------------------
/**
 * Notify that this that its extent has changed
 */

void ObjectNotification::extentChanged(Object &) const
{
}

// ----------------------------------------------------------------------
/**
 * Notify that this that the parent's pob fixup is complete
 */

void ObjectNotification::pobFixupComplete(Object &) const
{
}


// ======================================================================
