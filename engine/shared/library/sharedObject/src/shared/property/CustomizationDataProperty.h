// ======================================================================
//
// CustomizationDataProperty.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_CustomizationDataProperty_H
#define INCLUDED_CustomizationDataProperty_H

// ======================================================================

#include "sharedObject/Property.h"

class CustomizationData;
class Object;

// ======================================================================

class CustomizationDataProperty: public Property
{
public:

	static PropertyId getClassPropertyId();

public:

	explicit  CustomizationDataProperty(Object &owner);
	virtual  ~CustomizationDataProperty();

	const CustomizationData *fetchCustomizationData() const;
	CustomizationData       *fetchCustomizationData();

private:

	// disabled
	CustomizationDataProperty();
	CustomizationDataProperty(const CustomizationDataProperty&);
	CustomizationDataProperty &operator =(const CustomizationDataProperty&);

private:

	CustomizationData *const m_customizationData;

};

// ======================================================================

#endif
