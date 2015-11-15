// ======================================================================
//
// ContainedByProperty.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ContainedByProperty_H
#define INCLUDED_ContainedByProperty_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Property.h"

// ======================================================================

class ContainedByProperty : public Property
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static PropertyId getClassPropertyId();

public:
	ContainedByProperty(Object& thisObject, Object* containedByObject);
	~ContainedByProperty();

	Object*             getContainedBy() const;
	const NetworkId &   getContainedByNetworkId() const;

	void setContainedBy(const NetworkId &id, bool local = true);

private:
	CachedNetworkId m_containedBy;

	ContainedByProperty();
	ContainedByProperty(const ContainedByProperty&);
	ContainedByProperty& operator= (const ContainedByProperty&);
};

// ======================================================================

#endif

