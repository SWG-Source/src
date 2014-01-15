// ======================================================================
//
// TangibleVolumeContainer.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TangibleVolumeContainer.h"

#include "serverGame/TangibleObject.h"

// ======================================================================

TangibleVolumeContainer::TangibleVolumeContainer(Object &owner, int totalVolume) :
	VolumeContainer(owner, totalVolume)
{
}

// ----------------------------------------------------------------------

TangibleVolumeContainer::~TangibleVolumeContainer()
{
}

// ----------------------------------------------------------------------

bool TangibleVolumeContainer::mayAdd(Object const &item, ContainerErrorCode &error) const
{
	error = CEC_Success;

	ServerObject const * const so = item.asServerObject();
	if (so && so->asTangibleObject())
		return VolumeContainer::mayAdd(item, error);

	error = CEC_WrongType;
	return false;
}

// ======================================================================

