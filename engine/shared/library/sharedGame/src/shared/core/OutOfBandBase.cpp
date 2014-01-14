// OutOfBandBase.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedGame/FirstSharedGame.h"
#include "OutOfBandBase.h"

//-----------------------------------------------------------------------

OutOfBandBase::OutOfBandBase(const char t, const int p) :
position(p),
typeId(t)
{
}

//-----------------------------------------------------------------------

OutOfBandBase::~OutOfBandBase()
{
}

//-----------------------------------------------------------------------

const int OutOfBandBase::getPosition() const
{
	return position;
}

//-----------------------------------------------------------------------

const unsigned char OutOfBandBase::getTypeId() const
{
	return typeId;
}

//-----------------------------------------------------------------------

