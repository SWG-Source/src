// SpatialSubdivision.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#pragma warning ( disable : 4514 ) // unreferenced inline function has been removed
#include "sharedMath/FirstSharedMath.h"
#include "SpatialSubdivision.h"

//-----------------------------------------------------------------------

SpatialSubdivisionHandle::SpatialSubdivisionHandle()
{
}

//-----------------------------------------------------------------------

SpatialSubdivisionHandle::SpatialSubdivisionHandle(const SpatialSubdivisionHandle &)
{
}

//-----------------------------------------------------------------------

SpatialSubdivisionHandle::~SpatialSubdivisionHandle()
{
}

//-----------------------------------------------------------------------

SpatialSubdivisionHandle & SpatialSubdivisionHandle::operator = (const SpatialSubdivisionHandle & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

