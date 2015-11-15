// ======================================================================
//
// SlowDownProperty.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlowDownProperty_H
#define INCLUDED_SlowDownProperty_H

// ======================================================================

#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Property.h"


// ======================================================================

/**
 * This property is used to keep track of data needed by a creature doing 
 * a slow down effect.
 */
class SlowDownProperty : public Property
{
public:

	static PropertyId getClassPropertyId();

public:
	SlowDownProperty(Object& thisObject, const CachedNetworkId & target, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime);
	~SlowDownProperty();

	const CachedNetworkId & getTarget() const;
	float getConeLength() const;
	float getConeAngle() const;
	float getSlopeAngle() const;
	unsigned long getExpireTime() const;

private:
	CachedNetworkId m_target;
	float           m_coneLength; 
	float           m_coneAngle;
	float           m_slopeAngle; 
	unsigned long   m_expireTime;

private:
	SlowDownProperty();
	SlowDownProperty(const SlowDownProperty&);
	SlowDownProperty& operator= (const SlowDownProperty&);
};


// ======================================================================

inline const CachedNetworkId & SlowDownProperty::getTarget() const
{
	return m_target;
}

inline float SlowDownProperty::getConeLength() const
{
	return m_coneLength;
}

inline float SlowDownProperty::getConeAngle() const
{
	return m_coneAngle;
}

inline float SlowDownProperty::getSlopeAngle() const
{
	return m_slopeAngle;
}

inline unsigned long SlowDownProperty::getExpireTime() const
{
	return m_expireTime;
}


// ======================================================================


#endif

