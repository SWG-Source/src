// ======================================================================
//
// SlopeEffectProperty.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SlopeEffectProperty.h"


// ======================================================================

PropertyId SlopeEffectProperty::getClassPropertyId()
{
//	PropertyId id = LabelHash::hashLabel( "Property", "SlopeEffect" );
//	return id;
	return PROPERTY_HASH(SlopeEffect, 0xc8fd989a);
}

// ======================================================================

SlopeEffectProperty::SlopeEffectProperty(Object& thisObject) : 
Property(getClassPropertyId(), thisObject),
m_normals()
{
}

//-------------------------------------------------------------------------

SlopeEffectProperty::~SlopeEffectProperty()
{
}

//-------------------------------------------------------------------------

void SlopeEffectProperty::addNormal(const Vector & normal)
{
	m_normals.push_back(normal);
}

// ======================================================================
