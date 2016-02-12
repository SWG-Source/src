// ======================================================================
//
// ContainedByProperty.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ContainedByProperty.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/Object.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ContainedByProperty, true, 0, 0, 0);

// ======================================================================

PropertyId ContainedByProperty::getClassPropertyId()
{
	return PROPERTY_HASH(ContainedBy, 0x704360EC);
}

// ======================================================================

ContainedByProperty::ContainedByProperty(Object& thisObject, Object* containedByObject) : 
Property(getClassPropertyId(), thisObject),
m_containedBy(CachedNetworkId::cms_cachedInvalid) 
{
	if (containedByObject)
		m_containedBy = CachedNetworkId(*containedByObject);
}

//-------------------------------------------------------------------------

ContainedByProperty::~ContainedByProperty()
{
}

// ======================================================================
/**
 * Return the container that contains the object that has this ContainedBy
 * property.
 *
 * @return  Pointer to the object that contains the object with this 
 *          ContainedByProperty.  Returns nullptr if the object isn't
 *          contained by anything at the moment.
 */

Object* ContainedByProperty::getContainedBy() const
{
	return m_containedBy.getObject();
}

// ----------------------------------------------------------------------

const NetworkId& ContainedByProperty::getContainedByNetworkId() const
{
	return m_containedBy;
}

// ----------------------------------------------------------------------

void ContainedByProperty::setContainedBy(const NetworkId & id, bool local)
{
	if (m_containedBy != id)
	{
		NetworkId oldValue = m_containedBy;
		m_containedBy = id;
		getOwner().containedByModified(oldValue, id, local);
	}
}

// ======================================================================

