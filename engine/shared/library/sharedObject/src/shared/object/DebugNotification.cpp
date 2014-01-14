// ======================================================================
//
// DebugNotification.cpp
// Copyright 2003 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/DebugNotification.h"

// ======================================================================

DebugNotification DebugNotification::ms_instance;

// ======================================================================

DebugNotification & DebugNotification::getInstance()
{
	return ms_instance;
}

// ======================================================================

DebugNotification::DebugNotification()
{
}

// ----------------------------------------------------------------------

DebugNotification::~DebugNotification()
{
}

// ----------------------------------------------------------------------

int DebugNotification::getPriority() const
{
	return 0;
}

// ----------------------------------------------------------------------

void DebugNotification::addToWorld(Object &) const
{
}

// ----------------------------------------------------------------------

void DebugNotification::removeFromWorld(Object &) const
{
}

// ----------------------------------------------------------------------

bool DebugNotification::positionChanged(Object &, bool, const Vector &) const
{
	return true;
}

// ----------------------------------------------------------------------

void DebugNotification::rotationChanged(Object &, bool) const
{
}

// ----------------------------------------------------------------------

bool DebugNotification::positionAndRotationChanged(Object &, bool, const Vector &) const
{
	return true;
}

// ----------------------------------------------------------------------

void DebugNotification::cellChanged(Object &, bool) const
{
}

// ----------------------------------------------------------------------

void DebugNotification::extentChanged(Object &) const
{
}

// ======================================================================
