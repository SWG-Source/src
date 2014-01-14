// ======================================================================
//
// VolumeContainmentProperty.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_VolumeContainmentProperty_H
#define INCLUDED_VolumeContainmentProperty_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Property.h"

// ======================================================================

class VolumeContainmentProperty : public Property
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:
	
	static PropertyId     getClassPropertyId();

public:

	VolumeContainmentProperty(Object& owner, int volume);
	~VolumeContainmentProperty();
	
	int  getVolume () const;
	void setVolume (int volume);

private:
	int m_volume;

	VolumeContainmentProperty();
	VolumeContainmentProperty(const VolumeContainmentProperty&);
	VolumeContainmentProperty& operator= (const VolumeContainmentProperty&);

};
// -----------------------------------------------------------




#endif
