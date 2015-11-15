// ======================================================================
//
// PulseDynamics.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PulseDynamics_H
#define INCLUDED_PulseDynamics_H

// ======================================================================

#include "sharedObject/SimpleDynamics.h"

// ======================================================================

class PulseDynamics: public SimpleDynamics
{
public:
			PulseDynamics(Object *owner, float minimumSize, float maximumSize, float speed = 1.0f);
	virtual ~PulseDynamics();
	virtual float alter(float elapsedTime);

private:
	PulseDynamics();
	PulseDynamics(const PulseDynamics &);
	PulseDynamics &operator =(const PulseDynamics &);

private:
	float m_minimumSize;
	float m_maximumSize;
	float m_speed;

	Vector m_baseScale;
	float m_currentSize;
	float m_pulseGrowDirection; // this should only ever by 1.0f or -1.0f
};

// ======================================================================

#endif
