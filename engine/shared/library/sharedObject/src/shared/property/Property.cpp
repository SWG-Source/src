// ======================================================================
//
// Property.cpp
//
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Property.h"

// ------------------------------------------------------------------------

Property::Property(PropertyId propertyId, Object& owner)
: 
	m_owner(owner),
	m_propertyId(propertyId)
{
	DEBUG_FATAL(m_propertyId == 0, ("bad property value"));
}

// ------------------------------------------------------------------------

Property::~Property()
{
}

// ------------------------------------------------------------------------

void Property::initializeFirstTimeObject()
{
}

// ------------------------------------------------------------------------

void Property::addToWorld()
{
}

// ------------------------------------------------------------------------

void Property::removeFromWorld()
{
}

// ------------------------------------------------------------------------

void Property::conclude()
{
}

// ======================================================================
