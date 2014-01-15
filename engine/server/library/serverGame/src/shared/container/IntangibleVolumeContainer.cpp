// ======================================================================
//
// IntangibleVolumeContainer.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================


#include "serverGame/FirstServerGame.h"
#include "serverGame/IntangibleVolumeContainer.h"

#include "serverGame/IntangibleObject.h"

// ======================================================================

IntangibleVolumeContainer::IntangibleVolumeContainer(Object & owner, int totalVolume) :
	VolumeContainer(owner, totalVolume)
{
}

//-----------------------------------------------------------------------

IntangibleVolumeContainer::~IntangibleVolumeContainer()
{
}

//-----------------------------------------------------------------------

bool IntangibleVolumeContainer::mayAdd(Object const &item, ContainerErrorCode &error) const
{
	error = CEC_Success;

	ServerObject const * const so = item.asServerObject();
	if (so && so->asIntangibleObject())
		return VolumeContainer::mayAdd(item, error);

	error = CEC_WrongType;
	return false;
}

// ======================================================================

