// ======================================================================
//
// TrackingDynamics.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TrackingDynamics_H
#define INCLUDED_TrackingDynamics_H

// ======================================================================

#include "sharedObject/SimpleDynamics.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMath/Vector.h"

class MemoryBlockManager;

// ======================================================================

class TrackingDynamics: public SimpleDynamics
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	TrackingDynamics(Object *owner, Object const *targetObject, float speed, Vector const &rotationRadiansPerSecond);

	virtual float alter(float elapsedTime);

	void          setTarget(Object const *targetObject);
	float         getDistanceToTargetSquared() const;

private:

	// Disabled.
	TrackingDynamics();
	TrackingDynamics(const TrackingDynamics&);
	TrackingDynamics &operator =(const TrackingDynamics&);

private:

	ConstWatcher<Object>  m_targetWatcher;
	float                 m_speed;
	Vector                m_radiansPerSecond;

};

// ======================================================================

#endif
