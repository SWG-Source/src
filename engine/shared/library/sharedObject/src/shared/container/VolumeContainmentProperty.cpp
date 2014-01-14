// ======================================================================
//
// VolumeContainmentProperty.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/VolumeContainmentProperty.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/VolumeContainer.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(VolumeContainmentProperty, true, 0, 0, 0);

// ======================================================================

VolumeContainmentProperty::VolumeContainmentProperty(Object& owner, int volume) : 
Property(VolumeContainmentProperty::getClassPropertyId(), owner),
m_volume(volume)
{

}

//----------------------------------------------------------------------

void VolumeContainmentProperty::setVolume (int volume)
{
	m_volume = volume;
}

// -----------------------------------------------------------

VolumeContainmentProperty::~VolumeContainmentProperty()
{
}

// -----------------------------------------------------------

PropertyId VolumeContainmentProperty::getClassPropertyId()
{
	return PROPERTY_HASH(VolumeContainment, 0x8701647C);
}

// -----------------------------------------------------------

int VolumeContainmentProperty::getVolume() const
{
	int retval =  m_volume;
	const VolumeContainer* container = getOwner().getVolumeContainerProperty();
	if (container)
	{
		retval += container->getCurrentVolume();
	}
	return retval;
}

//-----------------------------------------------------------
