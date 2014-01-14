// ======================================================================
//
// SlowDownProperty.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SlowDownProperty.h"


// ======================================================================

PropertyId SlowDownProperty::getClassPropertyId()
{
//	PropertyId id = LabelHash::hashLabel( "Property", "SlowDown" );
//	WARNING(true, ("PROPERTY SLOWDOWN HASH = %x", id));
//	return id;
	return PROPERTY_HASH(SlowDown, 0x1faf8ccb);
}

// ======================================================================

SlowDownProperty::SlowDownProperty(Object& thisObject, const CachedNetworkId & target, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime) : 
Property(getClassPropertyId(), thisObject),
m_target(target),
m_coneLength(coneLength),
m_coneAngle(coneAngle),
m_slopeAngle(slopeAngle),
m_expireTime(expireTime)
{
}

//-------------------------------------------------------------------------

SlowDownProperty::~SlowDownProperty()
{
}

// ======================================================================
