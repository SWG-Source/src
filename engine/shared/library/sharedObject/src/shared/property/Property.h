// ======================================================================
//
// Property.h
//
// Copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Property_H
#define INCLUDED_Property_H

// ======================================================================

class Object;

#include "sharedObject/PropertyId.h"

// ======================================================================

#define PROPERTY_HASH( label, value )	LABEL_HASH( Property, label, value )

// ======================================================================

class Property
{
protected:

	Property(PropertyId propertyId, Object& owner);

public:
	virtual ~Property() = 0;

	Object&             getOwner();
	const Object&       getOwner() const;
	const PropertyId    getPropertyId() const;

	virtual void        initializeFirstTimeObject();

	virtual void        addToWorld();
	virtual void        removeFromWorld();

	virtual void        conclude();


private:
	Object&             m_owner;
	const PropertyId    m_propertyId;

	Property();
	Property(const Property&);
	Property& operator= (const Property&);
};

// ------------------------------------------------------------------------

inline Object& Property::getOwner()
{
	return m_owner;
}

// ------------------------------------------------------------------------

inline const Object& Property::getOwner() const 
{
	return m_owner;
}

// ------------------------------------------------------------------------

inline const PropertyId Property::getPropertyId() const 
{
	return m_propertyId;
}

// ------------------------------------------------------------------------


#endif

