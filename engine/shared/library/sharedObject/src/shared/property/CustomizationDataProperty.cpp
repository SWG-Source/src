// ======================================================================
//
// CustomizationDataProperty.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/CustomizationDataProperty.h"

#include "sharedObject/CustomizationData.h"

// ======================================================================

PropertyId CustomizationDataProperty::getClassPropertyId()
{
	return PROPERTY_HASH(CustomizationData, 0xDE36CB96);
}

// ----------------------------------------------------------------------
/**
 * Construct a CustomizationDataProperty.
 *
 * @param owner  the Object instance owning this new property.
 */

CustomizationDataProperty::CustomizationDataProperty(Object &owner) :
	Property(getClassPropertyId(), owner),
	m_customizationData(new CustomizationData(owner))
{
	//-- fetch local reference
	m_customizationData->fetch();
}

// ----------------------------------------------------------------------
/**
 * Destroy the CustomizationDataProperty instance.
 */

CustomizationDataProperty::~CustomizationDataProperty()
{
	//lint -esym(1540, CustomizationDataProperty::m_customizationData) // not zero'ed or freed // it's okay, it's ref counted and released.
	m_customizationData->release();
}

// ----------------------------------------------------------------------
/**
 * Fetch a pointer to const of this property's CustomizationData instance.
 *
 * This function bumps up the reference count on the CustomizationData
 * instance for the caller.
 *
 * @return  a pointer to const of this property's CustomizationData instance.
 */

const CustomizationData *CustomizationDataProperty::fetchCustomizationData() const
{
	m_customizationData->fetch();
	return m_customizationData;
}

// ----------------------------------------------------------------------
/**
 * Fetch a pointer to this property's CustomizationData instance.
 *
 * This function bumps up the reference count on the CustomizationData
 * instance for the caller.
 *
 * @return  a pointer to this property's CustomizationData instance.
 */

CustomizationData *CustomizationDataProperty::fetchCustomizationData()
{
	m_customizationData->fetch();
	return m_customizationData;
}

// ======================================================================



// ======================================================================
